/*
 *  @file dac.c
 *
 *  @brief A PIC32 library for the MCP4822 2-channel, 12-bit DAC
 *
 *      Intended for use with the PIC32MX250F128B.
 *
 *  @author Jeff Lutgen (based on code by Bruce R. Land)
 */

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>
#include "dac.h"
#include "../hwprofile.h"

#define DAC_SPI_CHN     SPI_CHANNEL2
#define DAC_SET_CS()    (mPORTBSetBits(BIT_4))
#define DAC_CLEAR_CS()  (mPORTBClearBits(BIT_4))

static inline void SPI_Mode16(void){  // configure SPI2 for 16-bit mode
    SPI2CONSET = 0x400;
    SPI2CONCLR = 0x800;
}

/**
 *  Configures and enables a SPI channel and CS line for communicating
 *  with the DAC.
 *
 *  Pins used:
 *
 *      CS:         RB4  (pin 11)
 *      SCK:        SCK2 (pin 26)
 *      SDO (MOSI): RPB5 (pin 14) --> SDO2
 */
void dac_init() {
    // This assumes CS for DAC is RB4 (pin 11)
    // set CS high initially
    mPORTBSetPinsDigitalOut(BIT_4);
    DAC_SET_CS();

    // SCK2 is pin 26. SDO2 must be assigned using PPS.
    // The following PPS mapping is already done by ioe_init, so it's redundant
    // (but harmless) if the I/O Expander has been initialized.
    PPSOutput(2, RPB5, SDO2); // RPB5 (pin 14) --> SDO2 (MOSI)

    // The following SpiChnOpen is redundant if using I/O expander. ioe_init()
    // does the same thing, only using MODE8, but that's fine, as when talking
    // to the DAC we always set MODE16 first.
    SpiChnOpen(SPI_CHANNEL2, SPI_OPEN_ON | SPI_OPEN_MODE16 |
               SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV ,
               PBCLK / 10000000);   // We use an SPI clock divisor that gives
                                    // 10 MHz because the I/O expander can't
                                    // handle anything faster.
}

/**
 *  Writes a 16-bit word to the DAC.
 *
 *  Upper 4 bits:  configuration bits <br>
 *  Lower 12 bits: data
 *
 *  Example:
 *
 *      // 0 <= my_data < 4096
 *      dac_write(DAC_A | DAC_GAIN1X | DAC_ACTIVE | my_data);
 */
inline void dac_write(uint16_t msg) {
    // wait until ready
    while (!SpiChnTxBuffEmpty(DAC_SPI_CHN)) { ; }
    SPI_Mode16();
    DAC_CLEAR_CS(); // CS low to start transaction
    SpiChnWriteC(DAC_SPI_CHN, msg);
    while (SpiChnIsBusy(DAC_SPI_CHN)) { ; } // wait for end of transaction
    DAC_SET_CS(); // CS high to end transaction
    // need to read SPI channel to avoid possibly confusing I/O expander
    SpiChnReadC(DAC_SPI_CHN); // (ignore return value)
}

