// This is a library for the TPA2016D2 Class D Amplifier

#ifndef AMP_H
#define AMP_H

#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>

typedef unsigned char uint8_t;
typedef char int8_t;
typedef char bool;

// Clock Constants
#define SYS_CLOCK 40000000 // 40 MHz
#define GetSystemClock()            (SYS_CLOCK)
#define GetPeripheralClock()        (SYS_CLOCK) // FPBDIV = DIV_1
#define GetInstructionClock()       (SYS_CLOCK)
#define I2C_CLOCK_FREQ              100000

// Portions of this code adapted from following Adafruit code.
/*!
 *  @file Adafruit_TPA2016.h
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
 
#define TPA2016_SETUP 0x1       ///< Function Control
#define TPA2016_SETUP_R_EN 0x80 ///< Enables right amplifier
#define TPA2016_SETUP_L_EN 0x40 ///< Enables left amplifier
#define TPA2016_SETUP_SWS 0x20  ///< Shutdown IC when bit = 1
#define TPA2016_SETUP_R_FAULT 0x10 ///< Changes to a 1 when there is a short on the right channel. Reset by
       ///< writing a 0.
#define TPA2016_SETUP_L_FAULT 0x08 ///< Changes to a 1 when there is a short on the left channel. Reset by
       ///< writing a 0
#define TPA2016_SETUP_THERMAL 0x04 ///< Changes to a 1 when die temperature is above 150°C
#define TPA2016_SETUP_NOISEGATE 0x01 ///< Enables Noise Gate function

#define TPA2016_ATK 0x2  ///< AGC Attack time (gain ramp down)
#define TPA2016_REL 0x3  ///< AGC Release time (gain ramp down)
#define TPA2016_HOLD 0x4 ///< AGC Hold time
#define TPA2016_GAIN 0x5 ///< Sets the fixed gain of the amplifier: two's compliment
#define TPA2016_AGCLIMIT 0x6 ///< Disables the output limiter function. Can only be disabled when the
      ///< AGC compression ratio is 1:1 (off)
#define TPA2016_AGC 0x7      ///< Selects the maximum gain the AGC can achieve
#define TPA2016_AGC_OFF 0x00 ///<  Turn off AGC
#define TPA2016_AGC_2 0x01   ///< AGC compression ratio 1:2
#define TPA2016_AGC_4 0x02   ///< AGC compression ratio 1:4
#define TPA2016_AGC_8 0x03   ///< AGC compression ratio 1:8

#define TPA2016_I2C_BUS I2C1
#define TPA2016_I2CADDR 0x58 ///< Default TPA2016 i2c address

//void amp_init(uint8_t addr = TPA2016_I2CADDR, TwoWire *theWire = &Wire);

void amp_enableChannel(bool r, bool l);

// Register #5
void amp_setGain(int8_t g);
int8_t amp_getGain();

// Register #3
void amp_setReleaseControl(uint8_t release);

// Register #2
void amp_setAttackControl(uint8_t attack);

// Register #4
void amp_setHoldControl(uint8_t hold);

// Register #6
void amp_setLimitLevelOn();
void amp_setLimitLevelOff();
void amp_setLimitLevel(uint8_t limit);

// Register #7
void amp_setAGCCompression(uint8_t x);
void amp_setAGCMaxGain(uint8_t x);
  
#endif
