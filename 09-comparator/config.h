#ifndef CONFIG_H
#define CONFIG_H

/**
 *  @file   config.h
 *
 *  @brief  Initializes some system configuration registers on the
 *          PIC32MX250F128B
 *
 * Because this file specifies configuration settings for the PIC, you must
 * ensure that this file is included in **at most one** .c file in your project.
 * Otherwise, compilation will generate more than one object (.o) file
 * containing .configX sections, and the linker will try to cram these into a
 * single such section in the executable, producing cryptic "will not fit"
 * linker errors.

 * Such trouble is alluded to in the XC32 User's Guide, Section 7.5
 * (Configuration Bit Access): "Configuration settings should be specified in
 * only a single translation unit (a C/C++ file with all of its include files
 * after preprocessing)."
 *
 *  @author Jeff Lutgen
 */

//==============================================================================
/*
 * Remember to change the definitions of SYSCLK and/or PBCLK as necessary if you
 * change the oscillator configuration here!
 */
#pragma config FNOSC = FRCPLL   // Fast internal RC oscillator (8 MHz) with PLL.

#pragma config FPLLIDIV = DIV_2 // PLL requires 4-5 MHz input, so divide by 2.
#pragma config FPLLMUL = MUL_20 // Now multiply by 20 to get 80 MHz,
#pragma config FPLLODIV = DIV_2 // then divide by 2 to get SYSCLK = 40 MHz.

#pragma config FPBDIV = DIV_1   // Peripheral Bus Clock: Divide SYSCLK by 1

#define SYSCLK 40000000 ///< 40 MHz system clock
#define PBCLK  SYSCLK   ///< 40 MHz peripheral bus clock
//==============================================================================

#pragma config FWDTEN = OFF     // Watchdog timer off
#pragma config FSOSCEN = OFF    // Free up pins 11 and 12 (secondary oscillator)
#pragma config JTAGEN = OFF     // Free up pins 14, 16, 17, 18 (JTAG)

#include <xc.h>                 // Load the proper header for the processor
#include <sys/attribs.h>        // For __ISR macro

#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>
#include <peripheral/pps.h>  // Why do I need this on Mac but not Windows??
#include "init.h"

#endif
