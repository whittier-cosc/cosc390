/*!
 *  @file   config.h
 *
 *  @brief  Initialize system configuration registers for the
 *          PIC32MX250F128B
 *
 * Because this file contains "#pragma config" lines, you must ensure that this
 * file is included in **at most one** .c file in your project.
 * Otherwise, compilation will generate more than one object (.o) file
 * containing .configX sections. The linker will try to cram these
 * together in the executable, and you will get "will not fit" linker errors.

 * This is mentioned in the xc32 user's guide, section 7.5 (Configuration Bit
 * Access): "Configuration settings should be specified in only a single
 * translation unit (a C/C++ file with all of its include files after
 * preprocessing)."
 *
 *  @author Jeff Lutgen
 */

#ifndef CONFIG_H
#define CONFIG_H

/*
 * Remember to change SYSCLK (and possibly PBCLK) in hwprofile.h if you change
 * the oscillator setup here!
 */

#pragma config FNOSC = FRCPLL   // Fast internal RC oscillator (8 MHz) with PLL.
#pragma config FPLLIDIV = DIV_2 // PLL requires 4-5 MHz input, so divide by 2
#pragma config FPLLMUL = MUL_20 // then multiply by 20 to get 80 MHz
#pragma config FPLLODIV = DIV_2 // then divide by 2 to get SYSCLK = 40 MHz.

#pragma config FPBDIV = DIV_1   // Peripheral Bus Clock: Divide SYSCLK by 1

#pragma config FWDTEN = OFF     // Watchdog timer off
#pragma config FSOSCEN = OFF    // Free up pins 11 and 12 (secondary oscillator)
#pragma config JTAGEN = OFF     // Free up pins 14, 16, 17, 18 (JTAG)

#include <xc.h>                 // Load the proper header for the processor
#include <sys/attribs.h>        // For __ISR macro

#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>
#include "hwprofile.h"          // For definitions of SYSCLK and PBCLK

#endif
