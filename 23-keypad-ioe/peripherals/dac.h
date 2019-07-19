#ifndef DAC_H
#define DAC_H

/**
 *  @file dac.h
 *
 *  @brief A PIC32 library for the MCP4822 dual 12-bit DAC.
 *
 *      Intended for use with the PIC32MX250F128B.
 *
 *  @author Jeff Lutgen (based on code by Bruce R. Land)
 */

#include <stdint.h>

// DAC registers are 16 bits

// channel selection
#define DAC_A           0x0000
#define DAC_B           0x8000

// output gain selection
#define DAC_GAIN1X      0x2000
#define DAC_GAIN2X      0x0000

// output shutdown control
#define DAC_SHUTDOWN    0x0000
#define DAC_ACTIVE      0x1000

void dac_init();
inline void dac_write(uint16_t msg);

#endif
