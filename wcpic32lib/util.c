/*
 *  @file   util.c
 *
 *  @brief  Miscellaneous utility functions for PIC32 development.
 *
 *          Intended for use with the PIC32MX250F128B.
 *
 *  @author Jeff Lutgen
 */

#include <xc.h>
#include "tft.h"
#include "private/clocks.h"

/**
 *  Delays for a given number of milliseconds.
 *
 *  Uses a crude implementation, but good enough for many use cases. Accuracy
 *  of this routine will suffer if significant time is being spent in interrupt
 *  service routines. See `delay_ms()` in tft_master.c for a better
 *  implementation that uses the core timer.
 *
 *  Calibrated on a PIC32MX250F128B.
 */
void delay(int ms) {
    volatile int j;
    // magic constant 8920 obtained empirically
    for (j = 0; j < (_sysclk / 8920) * ms; j++) {
    }
}

/**
 *  Sets the OSCTUN register to the given value.
 *
 *  Useful for tuning the internal fast RC oscillator for better accuracy.
 *
 *  `osctun` should be a 6-bit value; see Microchip reference manual DS61112H,
 *  page 6-8, for details.
 *
 *  Setting OSCTUN to 56 gives the best accuracy on the three PIC32MX250F128B
 *  chips that I have tested.
 */
void osc_tune(int osctun) {
    SYSKEY = 0xAA996655;    // two-step unlocking sequence
    SYSKEY = 0x556699AA;
    OSCTUN = osctun;
    SYSKEY = 0;             // relock
}
