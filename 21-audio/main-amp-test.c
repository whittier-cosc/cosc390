/*!
 *  @file   amp_test.c
 *
 *  @brief  Test the routines in our library for the TPA2016D2 audio amplifier
 *  @author Jeff Lutgen
 */

// Adapted from Adafruit's testing code for their Arduino library:
/***************************************************
  This is an example for our Adafruit TPA2016 Class D Amplifier Breakout
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1712
  This amplifier uses I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "config.h"
#include "util.h"
#include "amp.h"
#include "tft.h"
#include "tft_printline.h"

char msg[80];

void display_message(char *message) {
    tft_printLine(1, 2, message);
}

int main(void) {
    // Configure the device for maximum performance for the given system clock,
    // but do not change the PBDIV.
    // With the given options, this function will change the flash wait states,
    // RAM wait state, and enable prefetch and cache mode.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
	
    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left
	
    // amp_init() sets up I2C:
    //      SCL1 pin 17 (B8)
    //  SDA1 pin 18 (B9)
    // and UART1 (for debugging):
    //              U1RX pin 9  (RPA2)
    //              U1TX pin 7  (RPB3)
    amp_init();

    sprintf(msg, "compression off");
    display_message(msg);
    // Turn off AGC for the gain test
    amp_setAGCCompression(TPA2016_AGC_OFF);

    sprintf(msg, "release off");
    display_message(msg);
    // we also have to turn off the release to really turn off AGC
    amp_setReleaseControl(0);

    // We can update the gain, from -28dB up to 30dB
    int8_t i, gain;
    for (i = -28; i <= 30; i++) {
        if (i < 0)
            sprintf(msg, "Gain = %d", -(-i & 0x00ff));
        else
            sprintf(msg, "Gain = %d", i & 0x00ff);
        display_message(msg);
        amp_setGain(i);
        delay(250);
        gain = amp_getGain();
        if (gain < 0)
            sprintf(msg, "getGain returned %d", -(-gain & 0x00ff));
        else
            sprintf(msg, "getGain returned %d", gain & 0x00ff);
        display_message(msg);
        delay(500);
    }

    // Set gain back to reasonable level
    display_message("Setting gain to 0");
    amp_setGain(0);
    delay(250);

    // Each channel can be individually controlled
    display_message("Left off");
    amp_enableChannel(true, false);
    delay(250);
    display_message("Left On, Right off");
    amp_enableChannel(false, true);
    delay(250);
    display_message("Left On, Right On");
    amp_enableChannel(true, true);
    delay(250);

    // OK now we'll turn the AGC back on and mess with the settings :)

    // AGC can be TPA2016_AGC_OFF (no AGC) or
    //  TPA2016_AGC_2 (1:2 compression)
    //  TPA2016_AGC_4 (1:4 compression)
    //  TPA2016_AGC_8 (1:8 compression)
    display_message("Setting AGC Compression");
    amp_setAGCCompression(TPA2016_AGC_2);
    delay(250);

    // See Datasheet page 22 for value -> dBV conversion table
    display_message("Setting Limit Level");
    amp_setLimitLevelOn();
    // or turn off with setLimitLevelOff()
    amp_setLimitLevel(25);  // range from 0 (-6.5dBv) to 31 (9dBV)
    delay(250);

    // See Datasheet page 23 for value -> ms conversion table
    display_message("Setting AGC Attack");
    amp_setAttackControl(5);
    delay(250);

    // See Datasheet page 24 for value -> ms conversion table
    display_message("Setting AGC Hold");
    amp_setHoldControl(0);
    delay(250);

    // See Datasheet page 24 for value -> ms conversion table
    display_message("Setting AGC Release");
    amp_setReleaseControl(11);

    while(1) {
        ;
    }
    return 0;
}
