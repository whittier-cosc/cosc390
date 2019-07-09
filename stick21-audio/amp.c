// This is a library for the TPA2016D2 Class D Amplifier


#include "amp.h"

// Portions of this code adapted from following Adafruit code.
/*!
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

 
// private utility functions
void write8(uint8_t address, uint8_t data);
uint8_t read8(uint8_t address);
BOOL start_transfer( BOOL restart );
void stop_transfer( void );
BOOL transmit_byte( UINT8 data );

// NOTE:
// amp_init() sets up I2C:
//				SCL1 pin 17 (B8)
//				SDA1 pin 18 (B9)
// and UART1 (for debugging):
//              U1RX pin 9  (RPA2)
//              U1TX pin 7  (RPB3)
void amp_init() {

    /********************** UART SETUP (DEBUGGING) *************/
    // Map pins for UART1 RX/TX
    CFGCONbits.IOLOCK = 0;
    U1RXR = 0; // Map RPA2 (pin 9) to U1RX
    RPB3R = 1; // Map RPB3 (pin 7) to U1TX
    CFGCONbits.IOLOCK = 1;

    // Configure UART1

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


    char msg[80];
    UINT32 actualClock;

    // Set the I2C baudrate
    I2CSetFrequency(TPA2016_I2C_BUS, GetPeripheralClock(), I2C_CLOCK_FREQ);
    if (abs(actualClock - I2C_CLOCK_FREQ) > I2C_CLOCK_FREQ/10)
    {
        sprintf(msg, "Error: I2C1 clock frequency (%u) error exceeds 10%%.\n", (unsigned)actualClock);
        debug_log(msg);
    }

    // Enable the I2C bus
    I2CEnable(TPA2016_I2C_BUS, TRUE);
}

void amp_enableChannel(bool r, bool l);

/*!
 *    @brief  Set gain in dB by writing to TPA2016_GAIN.
 *    @param  g
 *            value in dB (clamped between -28 to 30)
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
  
uint8_t read8(uint8_t address)
{
    BOOL success;
	uint8_t data;
	I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
	// Start the transfer to write data to the EEPROM
    if( !start_transfer(FALSE) )
    {
        while(1);
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
	if( !start_transfer(TRUE) )
    {
        while(1);
    }
    
    // Transmit the slave address with the READ bit set
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_READ);
    if (!success) {
        debug_log("read8: transmitting slave address byte (switch to read) failed\n");
        while(1) { ; }
    }

    if (I2CReceiverEnable(TPA2016_I2C_BUS, TRUE) == I2C_RECEIVE_OVERFLOW) {
        debug_log("read8: receiver enable failed (receive overflow)\n");
        while(1) { ; }
    }

	while(!I2CReceivedDataIsAvailable(TPA2016_I2C_BUS)) { ; }
	data = I2CGetByte(TPA2016_I2C_BUS);
	
	stop_transfer();
	
	return data;
}

void write8(uint8_t address, uint8_t data)
{
    BOOL success;
	I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
	// Start the transfer to write data to the EEPROM
    if( !start_transfer(FALSE) )
    {
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

/*******************************************************************************
  Function:
    BOOL start_transfer( BOOL restart )

  Summary:
    Starts (or restarts) a transfer to/from the EEPROM.

  Description:
    This routine starts (or restarts) a transfer to/from the EEPROM, waiting (in
    a blocking loop) until the start (or re-start) condition has completed.

  Precondition:
    The I2C module must have been initialized.

  Parameters:
    restart - If FALSE, send a "Start" condition
            - If TRUE, send a "Restart" condition
    
  Returns:
    TRUE    - If successful
    FALSE   - If a collision occurred during Start signaling
    
  Remarks:
    This is a blocking routine that waits for the bus to be idle and the Start
    (or Restart) signal to complete.
  *****************************************************************************/
BOOL start_transfer( BOOL restart )
{
    I2C_STATUS  status;

    // Send the Start (or Restart) signal
    if(restart)
    {
        I2CRepeatStart(TPA2016_I2C_BUS);
    }
    else
    {
        // Wait for the bus to be idle, then start the transfer
        while( !I2CBusIsIdle(TPA2016_I2C_BUS) );

        if(I2CStart(TPA2016_I2C_BUS) != I2C_SUCCESS)
        {
            debug_log("Error: Bus collision during transfer Start\n");
            return FALSE;
        }
    }

    // Wait for the signal to complete
    do
    {
        status = I2CGetStatus(TPA2016_I2C_BUS);

    } while ( !(status & I2C_START) );

    return TRUE;
}

void stop_transfer( void )
{
    I2C_STATUS  status;

    // Send the Stop signal
    I2CStop(TPA2016_I2C_BUS);

    // Wait for the signal to complete
    do
    {
        status = I2CGetStatus(TPA2016_I2C_BUS);

    } while ( !(status & I2C_STOP) );
}

BOOL transmit_byte( UINT8 data )
{
    // Wait for the transmitter to be ready
    while(!I2CTransmitterIsReady(TPA2016_I2C_BUS));

    // Transmit the byte
    if(I2CSendByte(TPA2016_I2C_BUS, data) == I2C_MASTER_BUS_COLLISION)
    {
        debug_log("Error: I2C Master Bus Collision\n");
        return FALSE;
    }

    // Wait for the transmission to finish
    while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS));

    if(!I2CByteWasAcknowledged(TPA2016_I2C_BUS))
    {
        debug_log("Error: Sent byte was not acknowledged\n");
        return FALSE;
    }
    return TRUE;
}

void debug_log(const char *string) {
    Stick_WriteUART1(string);
}
