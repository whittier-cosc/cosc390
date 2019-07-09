/*!
 * @file amp.c
 *
 * @author Jeff Lutgen
 * @brief A PIC32 library for the TPA2016D2 Class D Amplifier.
 *
 * Ported from Adafruit's Arduino library.
 */

#include "amp.h"
#include "hwprofile.h"

#define DEBUG

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

#define I2C_CLOCK_FREQ              100000
 
// utility functions
void write8(uint8_t address, uint8_t data);
uint8_t read8(uint8_t address);
bool start_transfer(bool restart);
void stop_transfer();
bool transmit_byte(uint8_t data);

// NOTE:
// amp_init() sets up I2C on channel 1:
//				SCL1 pin 17 (B8)
//				SDA1 pin 18 (B9)
// and UART on channel 1 (for debugging) with these pin mappings:
//              U1RX pin 9  (RPA2)
//              U1TX pin 7  (RPB3)
void amp_init() {

#ifdef DEBUG
    // ------------ Configure UART1 ----------------

    // Map pins for UART1 RX/TX
    CFGCONbits.IOLOCK = 0;
    U1RXR = 0; // Map RPA2 (pin 9) to U1RX
    RPB3R = 1; // Map RPB3 (pin 7) to U1TX
    CFGCONbits.IOLOCK = 1;

    // Set baud to BAUDRATE
    U1MODEbits.BRGH = 0;  // High-speed mode disabled
    // With PBCLK = SYSCLK = 40 M, we have U1BRG = 259, giving
    // baud rate = 9615.4 (see DS61107F, Table 21-2).
    U1BRG = ((GetPeripheralClock()  / BAUDRATE) / 16) - 1;
    // 8 bit, no parity bit, 1 stop bit (8N1)
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;

    // Enable TX & RX, taking over U1RX/TX pins
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    // Do not enable RTS or CTS
    U1MODEbits.UEN = 0;

    // Enable the UART
    U1MODEbits.ON = 1;
    /************************************************************/
#endif

    char msg[80];
    uint32_t actual_clock;

    // Set the I2C baudrate
    actual_clock = I2CSetFrequency(TPA2016_I2C_BUS, PBCLK, I2C_CLOCK_FREQ);
    if (abs(actual_clock - I2C_CLOCK_FREQ) > I2C_CLOCK_FREQ/10) {
        sprintf(msg, "Error: I2C1 clock frequency (%u) error exceeds 10%%.\n",
                (unsigned) actual_clock);
        debug_log(msg);
    }

    // Enable the I2C bus
    I2CEnable(TPA2016_I2C_BUS, true);
}

void amp_enableChannel(bool r, bool l) {
    // TODO
}

/*!
 *    @brief  Sets gain in dB by writing to TPA2016_GAIN.
 *    @param  g
 *            value in dB (clamped to be in range -28..+30)
 */
void amp_setGain(int8_t g) {
	if (g > 30)
		g = 30;
	if (g < -28)
		g = -28;

	write8(TPA2016_GAIN, g);
}

int8_t amp_getGain() {
	// TODO
	return 0;
}

/*!
 *    @brief  Sets AGC Release time by writing to TPA2016_REL.
 *    @param  release
 *            release value (only 6 bits)
 */
void amp_setReleaseControl(uint8_t release) {
	if (release > 0x3F)
		return; // only 6 bits!

	write8(TPA2016_REL, release);
}

// Register #2
void amp_setAttackControl(uint8_t attack) {
	// TODO
}

// Register #4
void amp_setHoldControl(uint8_t hold) {
	// TODO
}

// Register #6
void amp_setLimitLevelOn() {
	// TODO
}
void amp_setLimitLevelOff() {
	// TODO
}
void amp_setLimitLevel(uint8_t limit) {
	// TODO
}

/*!
 *    @brief  Sets AGC Compression by writing to TPA2016_AGC
 *    @param  x
 *            TPA2016_AGC_2 1:2
 *            TPA2016_AGC_4 1:4
 *            TPA2016_AGC_8 1:8
 */
void amp_setAGCCompression(uint8_t x) {
	if (x > 3)
		return; // only 2 bits!

	uint8_t agc = read8(TPA2016_AGC);
	agc &= ~(0x03); // mask off bottom 2 bits
	agc |= x;       // set the compression ratio.
	write8(TPA2016_AGC, agc);
}

void amp_setAGCMaxGain(uint8_t x) {
	// TODO
}

/*!
 * @brief   read one byte from a TPA2016 register
 * @param   address
 *          the address of the register from which to read
 * @return  the byte that was read
 */
uint8_t read8(uint8_t address) {
    bool success;
	uint8_t data;
	I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
    // Start the transfer to write data to the TPA2016
    if(!start_transfer(false)) {
        while(1) { ; }
        debug_log("read8: start_transfer failed\n");
    }
	
    // Send slave address and register address
    success = transmit_byte(SlaveAddress.byte);
    if (!success) {
        debug_log("read8: transmitting slave address byte failed\n");
        while(1) { ; }
    }
    success = transmit_byte(address);
    if (!success) {
        debug_log("read8: transmitting slave address byte failed\n");
        while(1) { ; }
    }
	
	// Restart to do the read
    if(!start_transfer(true)) {
        debug_log("read8: start_transfer (restart) failed\n");
        while(1) { ; }
    }
    
    // Transmit the slave address with the READ bit set
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_READ);
    if (!success) {
        debug_log("read8: transmitting slave address byte (switch to read) failed\n");
        while(1) { ; }
    }

    if (I2CReceiverEnable(TPA2016_I2C_BUS, true) == I2C_RECEIVE_OVERFLOW) {
        debug_log("read8: receiver enable failed (receive overflow)\n");
        while(1) { ; }
    }

	while(!I2CReceivedDataIsAvailable(TPA2016_I2C_BUS)) { ; }
	data = I2CGetByte(TPA2016_I2C_BUS);
	
	stop_transfer();
	
	return data;
}

/*!
 * @brief   Writes a byte to a TPA2016 register
 * @param   address
 *          the address of the register to which to write
 * @param   data
 *          the byte to be written
 */
void write8(uint8_t address, uint8_t data) {
    bool success;
	I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
    // Start the transfer to write data to the TPA2016
    if(!start_transfer(false)) {
        while(1);
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

	stop_transfer();
}

/*!
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
bool start_transfer( bool restart ) {
    I2C_STATUS  status;

    // Send the Start (or Restart) signal
    if(restart) {
        I2CRepeatStart(TPA2016_I2C_BUS);
    }
    else {
        // Wait for the bus to be idle, then start the transfer
        while(!I2CBusIsIdle(TPA2016_I2C_BUS)) { ; }

        if(I2CStart(TPA2016_I2C_BUS) != I2C_SUCCESS) {
            debug_log("Error: Bus collision during transfer Start\n");
            return false;
        }
    }

    // Wait for the signal to complete
    do {
        status = I2CGetStatus(TPA2016_I2C_BUS);
    } while (!(status & I2C_START));

    return true;
}

void stop_transfer() {
    I2C_STATUS  status;

    // Send the Stop signal
    I2CStop(TPA2016_I2C_BUS);

    // Wait for the signal to complete
    do {
        status = I2CGetStatus(TPA2016_I2C_BUS);
    } while (!(status & I2C_STOP));
}

bool transmit_byte(uint8_t data) {
    // Wait for the transmitter to be ready
    while(!I2CTransmitterIsReady(TPA2016_I2C_BUS));

    // Transmit the byte
    if(I2CSendByte(TPA2016_I2C_BUS, data) == I2C_MASTER_BUS_COLLISION) {
        debug_log("Error: I2C Master Bus Collision\n");
        return false;
    }

    // Wait for the transmission to finish
    while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS));

    if(!I2CByteWasAcknowledged(TPA2016_I2C_BUS)) {
        debug_log("Error: Sent byte was not acknowledged\n");
        return false;
    }
    return true;
}

void debug_log(const char *string) {
#ifdef DEBUG
    Stick_WriteUART1(string);
#endif
}
