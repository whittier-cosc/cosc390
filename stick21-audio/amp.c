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

//void amp_init(uint8_t addr = TPA2016_I2CADDR, TwoWire *theWire = &Wire) { // need
//}

void amp_enableChannel(bool r, bool l);

/*!
 *    @brief  Set gain in dB by writing to TPA2016_GAIN.
 *    @param  g
 *            value in dB (clamped between -28 to 30)
 */
void amp_setGain(int8_t g) { // need
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
void amp_setReleaseControl(uint8_t release) { // need
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
void amp_setAGCCompression(uint8_t x) { // need
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
	uint8_t data;
	I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
    

	// Start the transfer to write data to the EEPROM
    if( !start_transfer(FALSE) )
    {
        while(1);
    }
	
    // Transmit the byte
	while(!I2CTransmitterIsReady(TPA2016_I2C_BUS)) { ; }
	I2CSendByte(TPA2016_I2C_BUS, SlaveAddress.byte);
	while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS)) { ; }
	
	while(!I2CTransmitterIsReady(TPA2016_I2C_BUS)) { ; }
	I2CSendByte(TPA2016_I2C_BUS, address);
    while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS)) { ; }
	
	// Restart to do the read
	if( !start_transfer(TRUE) )
    {
        while(1);
    }
    
	I2CReceiverEnable(TPA2016_I2C_BUS, TRUE);
	while(!I2CReceivedDataIsAvailable(TPA2016_I2C_BUS)) { ; }
	data = I2CGetByte(TPA2016_I2C_BUS);
	
	stop_transfer();
	
	return data;
	
}

void write8(uint8_t address, uint8_t data)
{
	I2C_7_BIT_ADDRESS SlaveAddress;
    I2C_FORMAT_7_BIT_ADDRESS(SlaveAddress, TPA2016_I2CADDR, I2C_WRITE);
    
    // Wait for the transmitter to be ready
    while(!I2CTransmitterIsReady(TPA2016_I2C_BUS)) { ; }

	// Start the transfer to write data to the EEPROM
    if( !start_transfer(FALSE) )
    {
        while(1);
    }
	
    // Transmit the byte
	while(!I2CTransmitterIsReady(TPA2016_I2C_BUS)) { ; }
	I2CSendByte(TPA2016_I2C_BUS, SlaveAddress.byte);
	while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS)) { ; }
	
	while(!I2CTransmitterIsReady(TPA2016_I2C_BUS)) { ; }
	I2CSendByte(TPA2016_I2C_BUS, address);
	while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS)) { ; }
	
	while(!I2CTransmitterIsReady(TPA2016_I2C_BUS)) { ; }
    I2CSendByte(TPA2016_I2C_BUS, data);
	while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS)) { ; }
	
    
	stop_transfer();
	
    // Wait for the transmission to finish
    while(!I2CTransmissionHasCompleted(TPA2016_I2C_BUS)) { ; }
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
            //DBPRINTF("Error: Bus collision during transfer Start\n");
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