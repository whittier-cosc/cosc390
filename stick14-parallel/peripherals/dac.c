/*!
 *  @file dac.c
 *
 *  @brief A PIC32 library for the MCP4822 2-channel, 12-bit DAC
 *
 *      Intended for use with the PIC32MX250F128B.
 *
 *  @author Jeff Lutgen
 */

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>
#include "dac.h"

#define DAC_SPI_CHN     SPI_CHANNEL2
#define DAC_SET_CS()    mPORTBSetBits(BIT_4)
#define DAC_CLEAR_CS()  mPORTBClearBits(BIT_4)

static inline void SPI_Mode16(void){  // configure SPI2 for 16-bit mode
    SPI2CONSET = 0x400;
    SPI2CONCLR = 0x800;
}

/*!
 * @brief Configures and enables a SPI channel and CS line for communicating with the DAC.
 *
 * @remark
 *      Pins used:
 *          RB4 (pin 11)  --> CS
 *          SCK2 (pin 26)
 *          RPB5 (pin 14) --> SDO2 (MOSI)
 */
void dac_init() {
    // Assumes CS for DAC is RB4 (pin 11)
    // set CS high initially
    mPORTBSetPinsDigitalOut(BIT_4);
    DAC_SET_CS();

    // SCK2 is pin 26. SDO2 must be assigned using PPS.
    // The following PPS mapping is already done by ioe_init, so it's redundant (but harmless)
    // when using the I/O Expander.
    PPSOutput(2, RPB5, SDO2); // RPB5 (pin 14) --> SDO2 (MOSI)

    // The following SpiChnOpen is redundant if using IO Expander. ioe_init() does the same
    // thing, but using MODE8, but that's fine, as when talking to the DAC
    // we always set MODE16 first.
    SpiChnOpen(SPI_CHANNEL2, SPI_OPEN_ON | SPI_OPEN_MODE16 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV , 4);
}

/*!
 * @brief Write a 16-bit word to the DAC
 *
 *              Upper 4 bits:  configuration bits
 *              Lower 12 bits: data
 *
 * @param msg
 *              the word to be written
 *
 */
inline void dac_write(uint16_t msg) {
    // wait until ready
    while (!SpiChnTxBuffEmpty(DAC_SPI_CHN)) { ; }
    SPI_Mode16();
    DAC_CLEAR_CS(); // CS low to start transaction
    SpiChnWriteC(DAC_SPI_CHN, msg);
    while (SpiChnIsBusy(DAC_SPI_CHN)) { ; } // wait for end of transaction
    DAC_SET_CS(); // CS high to end transaction
    // need to read SPI channel to avoid possibly confusing port expander
    SpiChnReadC(DAC_SPI_CHN); // (ignore return value)
}

