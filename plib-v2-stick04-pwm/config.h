#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>                 // Load the proper header for the processor
#include <sys/attribs.h>        // For __ISR macro

#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h> 

#pragma config FNOSC = FRCPLL   // Fast internal RC oscillator (8 MHz) with PLL
#pragma config FPLLIDIV = DIV_2 // PLL requires 4-5 MHz input, so divide by 2
#pragma config FPLLMUL = MUL_20 // then multiply by 20 to get 80 MHz
#pragma config FPLLODIV = DIV_2 // then divide by 2 to get 40 MHz

#pragma config FPBDIV = DIV_1   // Peripheral Bus Clock: Divide by 1

#pragma config FWDTEN = OFF   // Watchdog timer: OFF
#pragma config FSOSCEN = OFF  // Free up pins 11 and 12 (secondary oscillator)
#pragma config JTAGEN = OFF   // Free up pins 14, 16, 17, 18 (JTAG)

//#pragma config OSCIOFNC = ON // enable CLKO output signal on OSC2 pin (pin 10)

#define CORE_TICKS 8000000 // 8M ticks
#define STICK_SYS_FREQ  40000000 // 40 MHz

#endif
