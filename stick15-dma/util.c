/*!
 *  @file   util.c
 *
 *  @brief  Assorted utility functions for PIC32 development
 *
 *  @author Jeff Lutgen
 */

#include <xc.h>
#include "../hwprofile.h"

/*!
 * @brief   Delay for a given number of milliseconds.
 *
 *          This crude implementation is often good enough, but accuracy will suffer
 *          if significant time is spent in interrupt service routines. See delay_ms()
 *          in peripherals/tft_master.c
 *          for a better implementation that uses the core timer.
 *
 *          Calibrated on PIC32MX250F128B.
 *
 * @param ms
 *          length of delay, in milliseconds
 */
void delay(int ms) {
    volatile int j;
    for (j = 0; j < (SYSCLK / 8920) * ms; j++) { // magic constant 8920 obtained empirically
    }
}

/*!
 * @brief Set the OSCTUN register to tune the internal fast RC oscillator
 *
 *        OSCTUN = 56 gives best accuracy on the PIC32MX250F128B chips that I have tested
 *
 * @param osctun
 *          the desired value of OSCTUN (6 bits; see Microchip reference manual DS61112H,
 *          page 6-8)
 */
void osc_tune(int osctun) {
    SYSKEY = 0xAA996655;    // two-step unlocking sequence
    SYSKEY = 0x556699AA;
    OSCTUN = osctun;        //
    SYSKEY = 0;             // lock
}
