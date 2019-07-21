/*
 *  @file amp.c
 *
 *  @brief A PIC32 library for the TPA2016D2 class D stereo amplifier
 *
 *      Intended for use with the PIC32MX250F128B
 *
 *      Ported from Adafruit's Arduino library
 *
 *  @author Jeff Lutgen
 */

// Portions of this code adapted from Adafruit's Arduino library:
/*
 *  @file Adafruit_TPA2016.cpp
 *
 *  This is a library for the TPA2016D2 Class D Amplifier Breakout
 *
 *  Designed specifically to work with the Adafruit TPA2016 Stereo 2.8W Class D
 *  Audio Amplifier - I2C Control AGC
 *
 *  Pick one up today in the adafruit shop!
 *  ------> https://www.adafruit.com/product/1712
 *
 *  This amplifier uses I2C to communicate, 2 pins are required to interface
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  Limor Fried/Ladyada (Adafruit Industries).
 *
 *  BSD license, all text above must be included in any redistribution
 */

#include "amp.h"
#include "uart.h"
#include "private/clocks.h"

#define DEBUG   // If DEGUG is defined, UART1 will be configured and used for
                // debug and error message output.

#define I2C_CLOCK_FREQ 400000  // standard 10 KHz I2C clock speed

// utility functions
static void write8(uint8_t address, uint8_t data);
static uint8_t read8(uint8_t address);
static bool start_transfer(bool restart);
static void stop_transfer();
static bool transmit_byte(uint8_t data);
static void debug_log(const char *string);
static void delay();

/**
 *  Configures and enables an I2C module for communicating with the TPA2016.
 *
 *  Sets up I2C1:
 *
 *          SCL1: pin 17 (RB8)
 *          SDA1: pin 18 (RB9)
 *
 *  and UART1 (for debug and error output):
 *
 *          U1RX: pin 9  (RPA2)
 *          U1TX: pin 7  (RPB3)
 *
 *  Example:
 *
 *      amp_init();
 */
void amp_init() {

#ifdef DEBUG
    uart_init(_pbclk);
#endif

    char msg[80];
    uint32_t actual_freq;

    debug_log("amp_init\n");
    // Set the I2C baudrate
    actual_freq = I2CSetFrequency(TPA2016_I2C_BUS, _pbclk, I2C_CLOCK_FREQ);
    if (abs(actual_freq - I2C_CLOCK_FREQ) > I2C_CLOCK_FREQ/10) {
        sprintf(msg, "Error: I2C1 clock frequency (%u) error exceeds 10%%.\n",
                (unsigned) actual_freq);
        debug_log(msg);
    }
    sprintf(msg, "I2C1 clock freq = %u\n", (unsigned) actual_freq);
    debug_log(msg);
    // Enable the I2C bus
    I2C1CONbits.DISSLW = 1; // workaround for silicon error #9 (see PIC32MX errata sheet DS80000531J)
    I2CEnable(TPA2016_I2C_BUS, true);
}

/**
 *  Puts the amplifier to sleep (`sleep = true`) or awakens it (`sleep = false`).
 */
void amp_sleep(bool sleep) {
    uint8_t setup = read8(TPA2016_SETUP);
    if (sleep)
        setup |= TPA2016_SETUP_SWS;
    else
        setup &= ~TPA2016_SETUP_SWS;

    write8(TPA2016_SETUP, setup);
}

/**
 *  Turns on/off right and left channels.
 */
void amp_enableChannel(bool r, bool l) {
    uint8_t setup = read8(TPA2016_SETUP);
    if (r)
        setup |= TPA2016_SETUP_R_EN;
    else
        setup &= ~TPA2016_SETUP_R_EN;
    if (l)
        setup |= TPA2016_SETUP_L_EN;
    else
        setup &= ~TPA2016_SETUP_L_EN;

    write8(TPA2016_SETUP, setup);
}

/**
 *  Sets gain in dB to the given value `g` (clamped to be in range -28..+30)
 *
 *  According to the datasheet, "[t]hese bits are used to select the
 *  fixed gain of the amplifier. If the Compression is enabled, fixed gain is
 *  adjustable from –28dB to 30dB. If the Compression is disabled, fixed gain
 *  is adjustable from 0dB to 30dB."
 */
void amp_setGain(int8_t g) {
    if (g > 30)
        g = 30;
    if (g < -28)
        g = -28;

    if (g < 0)
        g = g & 0x3f; // convert to 6-bit two's complement
    write8(TPA2016_GAIN, g);
}

/**
 *  Returns the current gain setting in dB.
 */
int8_t amp_getGain() {
    int8_t gain = (int8_t) read8(TPA2016_GAIN);
    gain = gain << 2;
    if ((gain & 0x80) > 0)
        gain = (gain >> 2) | 0xC0;   // it's a negative value
    else
        gain = gain >> 2;            // it's a positive value
    return gain;
}

/**
 *  Sets the AGC release time.
 *
 * `release` should be a 6-bit value (1..63); see the datasheet for details.
 */
void amp_setReleaseControl(uint8_t release) {
    if (release > 0x3F)
        return; // only 6 bits!

    write8(TPA2016_REL, release);
}

/**
 *  Sets the AGC Attack time.
 *
 * `attack` should be a 6-bit value (1..63); see the datasheet for details.
 */
void amp_setAttackControl(uint8_t attack) {
    if (attack > 0x3F)
        return; // only 6 bits!

     write8(TPA2016_ATK, attack);
}

/**
 *  Sets the AGC hold time.
 *
 * `hold` should be a 6-bit value (0..63); see the datasheet for details.
 */
void amp_setHoldControl(uint8_t hold) {
    if (hold > 0x3F)
        return; // only 6 bits!

    write8(TPA2016_HOLD, hold);
}

/**
 *  Turns the power limiter on.
 */
void amp_setLimitLevelOn() {
    uint8_t agc = read8(TPA2016_AGCLIMIT);
    agc &= ~(0x80); // mask off top bit
    write8(TPA2016_AGCLIMIT, agc);
}

/**
 *  Turns the power limiter off.
 */
void amp_setLimitLevelOff() {
    uint8_t agc = read8(TPA2016_AGCLIMIT);
    agc |= 0x80; // turn on top bit
    write8(TPA2016_AGCLIMIT, agc);
}

/**
 *  Sets the limit level for the power limiter.
 *
 *  `limit` should be a 5-bit value (0..31); see the datasheet for details.
 */
void amp_setLimitLevel(uint8_t limit) {
    if (limit > 31)
        return;
    debug_log("amp_setLimitLevel\n");
    uint8_t agc = read8(TPA2016_AGCLIMIT);

    agc &= ~(0x1F); // mask off bottom 5 bits
    agc |= limit;   // set the limit level.

    write8(TPA2016_AGCLIMIT, agc);
}

/**
 *  Sets the AGC compression ratio to the value specified by `x`, which must be
 *  one of the following:
 *
 *      TPA2016_AGC_OFF  --> 1:1 ratio (compression off)
 *      TPA2016_AGC_2    --> 1:2
 *      TPA2016_AGC_4    --> 1:4
 *      TPA2016_AGC_8    --> 1:8
 */
void amp_setAGCCompression(uint8_t x) {
    debug_log("amp_setAGCCompression\n");
    if (x > 3)
        return; // only 2 bits!

    uint8_t agc = read8(TPA2016_AGC);
    agc &= ~(0x03); // mask off bottom 2 bits
    agc |= x;       // set the compression ratio.
    write8(TPA2016_AGC, agc);
}

/**
 *  Sets the AGC's maximum gain, specifed by `x` (0..12) as follows:
 *
 *       x    max gain
 *      --------------
 *       0      18 dB
 *       1      19 dB
 *      ...     ...    (gain increases by 1 dB with every step)
 *      12      30 dB
 */
void amp_setAGCMaxGain(uint8_t x) {
    if (x > 12)
        return; // max gain max is 12 (30dB)

    uint8_t agc = read8(TPA2016_AGC);
    agc &= ~(0xF0);  // mask off top 4 bits
    agc |= (x << 4); // set the max gain
    write8(TPA2016_AGC, agc);
}

/*
 * @brief   read one byte from a TPA2016 register
 * @param   address
 *          the address of the register from which to read
 * @return  the byte that was read
 */
static uint8_t read8(uint8_t address) {
    bool success;
    uint8_t data;
    I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
//    debug_log("read8: start_transfer\n");

    // Start the transfer to write data to the TPA2016
    if(!start_transfer(false)) {
        debug_log("read8: start_transfer failed\n");
        while(1) { ; }
    }
//    debug_log("read8: transfer byte 1\n");
    // Send slave address and register address
    success = transmit_byte(SlaveAddress.byte);
    if (!success) {
        debug_log("read8: transmitting slave address byte failed\n");
        while(1) { ; }
    }
//    debug_log("read8: transfer byte 2\n");
    success = transmit_byte(address);
    if (!success) {
        debug_log("read8: transmitting slave address byte failed\n");
        while(1) { ; }
    }
//    debug_log("read8: restart\n");
    // Restart to do the read
    if(!start_transfer(true)) {
        debug_log("read8: start_transfer (restart) failed\n");
        while(1) { ; }
    }
//    debug_log("read8: transmit slave addr (switch to read)\n");
    // Transmit the slave address with the READ bit set
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_READ);
    success = transmit_byte(SlaveAddress.byte);
    if (!success) {
        debug_log("read8: transmitting slave address byte (switch to read) failed\n");
        while(1) { ; }
    }
//    debug_log("read8: enable receiver\n");
    if (I2CReceiverEnable(TPA2016_I2C_BUS, true) == I2C_RECEIVE_OVERFLOW) {
        debug_log("read8: receiver enable failed (receive overflow)\n");
        while(1) { ; }
    }
//    debug_log("read8: wait for recv'd data available\n");
    while(!I2CReceivedDataIsAvailable(TPA2016_I2C_BUS)) { ; }
//    debug_log("read8: send nack\n");
    I2CAcknowledgeByte(TPA2016_I2C_BUS, false); // send NACK after receiving final (and only) byte!
//    debug_log("read8: get byte\n");
    data = I2CGetByte(TPA2016_I2C_BUS);

    delay(); // kludge -- the following stop_transfer() hangs otherwise
    stop_transfer();

    return data;
}

/*
 * @brief   Writes a byte to a TPA2016 register
 * @param   address
 *          the address of the register to which to write
 * @param   data
 *          the byte to be written
 */
static void write8(uint8_t address, uint8_t data) {
    bool success;
    I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
    // Start the transfer to write data to the TPA2016
    if(!start_transfer(false)) {
        debug_log("write8: start transfer failed\n");
        while(1) { ; }
    }

    // Transmit the byte
    success = transmit_byte(SlaveAddress.byte);
    if (!success) {
        debug_log("write8: transmitting slave address byte failed\n");
        while(1) { ; }
    }
    success = transmit_byte(address);
    if (!success) {
        debug_log("write8: transmitting register address byte failed\n");
        while(1) { ; }
    }
    success = transmit_byte(data);
    if (!success) {
        debug_log("write8: transmitting data byte failed\n");
        while(1) { ; }
    }

    delay(); // kludge -- the following stop_transfer() hangs otherwise
    stop_transfer();
}

/*
 * @brief   Starts (or restarts) a transfer to/from the TPA2016
 *
 *          This routine starts (or restarts) a transfer to/from the TPA2016,
 *          waiting (in a blocking loop) until the start (or re-start) condition
 *          has completed.
 *
 *          This is a blocking routine that waits for the bus to be idle and the
 *          Start (or Restart) signal to complete.
 *
 * @pre     The I2C module must have been initialized.
 * @param   restart
 *          if false, send a "Start" condition
            if true, send a "Restart" condition
 * @return  true if successful, false if a collision occured during Start
 *          signaling
 */
static bool start_transfer( bool restart ) {
    I2C_STATUS  status;

    // Send the Start (or Restart) signal
    if(restart) {
        I2CRepeatStart(TPA2016_I2C_BUS);
    }
    else {
        // Wait for the bus to be idle, then start the transfer
//      debug_log("start_transfer: wait for bus idle\n");
        while(!I2CBusIsIdle(TPA2016_I2C_BUS)) { ; }

        if(I2CStart(TPA2016_I2C_BUS) != I2C_SUCCESS) {
            debug_log("Error: Bus collision during transfer Start\n");
            return false;
        }
    }

    // Wait for the signal to complete
//    debug_log("start_transfer: wait for status == START\n");
    do {
        status = I2CGetStatus(TPA2016_I2C_BUS);
    } while (!(status & I2C_START));

    return true;
}

/*
 * \brief Stops a transfer to or from the TPA2016
 */
static void stop_transfer() {
    I2C_STATUS  status;

    // Send the Stop signal
    I2CStop(TPA2016_I2C_BUS);

    // Wait for the signal to complete
    debug_log("stop_transfer: wait for status == STOP\n");
    do {
        status = I2CGetStatus(TPA2016_I2C_BUS);
    } while (!(status & I2C_STOP));
    debug_log("stop_transfer: got status == STOP\n");
}

/*
 * \brief   Transmit a byte via I2C to the TPA2016
 * \param   data
 *              the byte to be transmitted
 * \return  true if successful
 *          false if unsuccessful
 */
static bool transmit_byte(uint8_t data) {
    // Wait for the transmitter to be ready
//    debug_log("transmit_byte: wait for transmitter ready\n");
    while(!I2CTransmitterIsReady(TPA2016_I2C_BUS)) { ; }

    // Transmit the byte
//    debug_log("transmit_byte: send byte\n");
    if(I2CSendByte(TPA2016_I2C_BUS, data) == I2C_MASTER_BUS_COLLISION) {
        debug_log("Error: I2C Master Bus Collision\n");
        return false;
    }

    // Wait for the transmission to finish
//    debug_log("transmit_byte: wait for transmission to finish\n");
    while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS)) { ; }

//    debug_log("check if byte ack'd\n");
    if(!I2CByteWasAcknowledged(TPA2016_I2C_BUS)) {
        debug_log("Error: Sent byte was not acknowledged\n");
        return false;
    }
    return true;
}

static void debug_log(const char *string) {
#ifdef DEBUG
    uart_write(string);
#endif
}

static void delay() {
    volatile int j;
    for (j = 0; j < 100; j++) {
    }
}
