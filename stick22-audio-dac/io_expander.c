/*!
 *  @file   io_expander.c
 *
 *  @brief  A PIC32 library for interfacing with the MCP23S17 I/O expander
 *
 *  @authors    Sean Carroll,
 *              Bruce Land,
 *              Jeff Lutgen
 */

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>

#include "hwprofile.h"
#include "io_expander.h"

#define IOE_SPI_CHN     SPI_CHANNEL2
#define IOE_SET_CS()    mPORTBSetBits(BIT_9)
#define IOE_CLEAR_CS()  mPORTBClearBits(BIT_9)

// Change the SPI bit modes on the fly, mid-transaction if necessary
inline void SPI_Mode16(void){  // configure SPI2 for 16-bit mode
    SPI2CONSET = 0x400;
    SPI2CONCLR = 0x800;
}

inline void SPI_Mode8(void){   // configure SPI2 for 8-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONCLR = 0x800;
}

inline void SPI_Mode32(void){  // configure SPI2 for 32-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONSET = 0x800;
}

/*!
 *  @brief  Initializes the MCP23S17 I/O expander and configures one of the
 *          PIC's SPI modules for communication with the I/O expander
 *
 *          Pin mappings:
 *              RPB5 (pin 14) --> SDO2 (MOSI)
 *              RPA4 (pin 12) --> SDI2 (MISO)
 *
 *          Uses RB9 (pin 18) as the chip select (CS) line.
 */
void ioe_init() {
    mPORTBSetPinsDigitalOut(BIT_9); // use RPB9 (pin 18) as CS
    IOE_SET_CS(); // CS high initially

    // These pin mappings assume that IOE_SPI_CHN is SPI_CHANNEL2!
    PPSOutput(2, RPB5, SDO2);   // use RPB5 (pin 14) for SDO2 (MOSI)
    PPSInput(3, SDI2, RPA4);    // use RPA4 (pin 12) for SDI2 (MISO)

    SpiChnOpen(IOE_SPI_CHN,
               SPI_OPEN_ON | SPI_OPEN_MODE8 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV,
               PBCLK / 10000000);  // SPI clock divisor to give 10 MHz max speed

    ioe_write(IOCON, CLEAR_BANK | CLEAR_MIRROR | SET_SEQOP |
              CLEAR_DISSLW | CLEAR_HAEN | CLEAR_ODR |
              CLEAR_INTPOL);
}

void clearBits(unsigned char addr, unsigned char bitmask){
    if (addr <= 0x15){
        unsigned char cur_val = ioe_read(addr);
        ioe_write(addr, cur_val & ~bitmask);
    }
}

void setBits(unsigned char addr, unsigned char bitmask){
    if (addr <= 0x15){
        unsigned char cur_val = ioe_read(addr);
        ioe_write(addr, cur_val | bitmask);
    }
}

void toggleBits(unsigned char addr, unsigned char bitmask){
    if (addr <= 0x15){
        unsigned char cur_val = ioe_read(addr);
        ioe_write(addr, cur_val ^ bitmask);
    }
}

unsigned char readBits(unsigned char addr, unsigned char bitmask){
    if (addr <= 0x15){
        unsigned char cur_val = ioe_read(addr) & bitmask ;
        return cur_val ;
    }
}

/*!
 *  @brief Sets a given collection of Port C pins as outputs
 *
 *  @param bitmask
 *              the Port C pins to set as outputs
 */
void ioe_PortCSetPinsOut(unsigned char bitmask){
    clearBits(IODIRY, bitmask);
}

/*!
 *  @brief Sets a given collection of Port D pins as outputs
 *
 *  @param bitmask
 *              the Port D pins to set as outputs
 */
void ioe_PortDSetPinsOut(unsigned char bitmask){
    clearBits(IODIRZ, bitmask);
}

/*!
 *  @brief Sets a given collection of Port C pins as inputs
 *
 *  @param bitmask
 *              the Port C pins to set as inputs
 */
void ioe_PortCSetPinsIn(unsigned char bitmask){
    setBits(IODIRY, bitmask);
}

/*!
 *  @brief Sets a given collection of Port D pins as inputs
 *
 *  @param bitmask
 *              the Port D pins to set as inputs
 */
void ioe_PortDSetPinsIn(unsigned char bitmask){
    setBits(IODIRZ, bitmask);
}

/*!
 *  @brief Enables interrupts for a given collection of Port C pins
 *
 *  @param bitmask
 *              the Port C pins for which to enable interrupts
 */
void ioe_PortCIntEnable(unsigned char bitmask){
    setBits(GPINTENY, bitmask);
}

/*!
 *  @brief Enables interrupts for a given collection of Port D pins
 *
 *  @param bitmask
 *              the Port D pins for which to enable interrupts
 */
void ioe_PortDIntEnable(unsigned char bitmask){
    setBits(GPINTENZ, bitmask);
}

/*!
 *  @brief Disables interrupts for a given collection of Port C pins
 *
 *  @param bitmask
 *              the Port C pins for which to disable interrupts
 */
void ioe_PortCIntDisable(unsigned char bitmask){
    clearBits(GPINTENY, bitmask);
}

/*!
 *  @brief Disables interrupts for a given collection of Port D pins
 *
 *  @param bitmask
 *              the Port D pins for which to disable interrupts
 */
void ioe_PortDIntDisable(unsigned char bitmask){
    clearBits(GPINTENZ, bitmask);
}

/*!
 *  @brief Enables the internal pull-up resistor on a given collection
 *         of Port C pins
 *
 *  @param bitmask
 *              the Port C pins on which to enable the internal pull-up resistor
 */
void ioe_PortCEnablePullUp(unsigned char bitmask){
    setBits(GPPUY, bitmask);
}

/*!
 *  @brief Enables the internal pull-up resistor on a given collection
 *         of Port D pins
 *
 *  @param bitmask
 *              the Port D pins on which to enable the internal pull-up resistor
 */
void ioe_PortDEnablePullUp(unsigned char bitmask){
    setBits(GPPUZ, bitmask);
}

/*!
 *  @brief Disables the internal pull-up resistor on a given collection
 *         of Port C pins
 *
 *  @param bitmask
 *              the Port C pins on which to disable the internal pull-up resistor
 */
void ioe_PortCDisablePullUp(unsigned char bitmask){
    clearBits(GPPUY, bitmask);
}

/*!
 *  @brief Disables the internal pull-up resistor on a given collection
 *         of Port D pins
 *
 *  @param bitmask
 *              the Port D pins on which to disable the internal pull-up resistor
 */
void ioe_PortDDisablePullUp(unsigned char bitmask){
    clearBits(GPPUZ, bitmask);
}

/*!
 * @brief Writes a byte of data to a register on the I/O expander
 * @param reg_addr
 *          the address of the target register
 * @param data
 *          the byte of data to be written
 */
inline void ioe_write(unsigned char reg_addr, unsigned char data) {

    // wait until ready
    while (!SpiChnTxBuffEmpty(IOE_SPI_CHN)) { ; }
    SPI_Mode8();

    // CS low to start transaction
    IOE_CLEAR_CS();

    // opcode and hw address (Should always be 0b0100000)
    SpiChnWriteC(IOE_SPI_CHN, IOE_OPCODE_HEADER | IOE_WRITE);
    while (SpiChnIsBusy(IOE_SPI_CHN)) { ; } // wait for byte to be sent
    SpiChnReadC(IOE_SPI_CHN); // ignore junk return value

    // register address
    SpiChnWriteC(IOE_SPI_CHN, reg_addr);
    while (SpiChnIsBusy(IOE_SPI_CHN)) { ; }
    SpiChnReadC(IOE_SPI_CHN);

    // one byte of data
    SpiChnWriteC(IOE_SPI_CHN, data);
    while (SpiChnIsBusy(IOE_SPI_CHN)) { ; }
    SpiChnReadC(IOE_SPI_CHN);

    // CS high to end transaction
    IOE_SET_CS();

}

/*!
 * @brief Reads and returns the data byte from a register on the I/O expander
 * @param reg_addr
 *          the address of the target register
 * @return the byte of data that was read
 */
inline unsigned char ioe_read(unsigned char reg_addr) {
    unsigned char data;

    // wait until ready
    while (!SpiChnTxBuffEmpty(IOE_SPI_CHN)) { ; }
    SPI_Mode8();

    // CS low to start transaction
    IOE_CLEAR_CS();

    // opcode and hw address (Should always be 0b0100000)
    SpiChnWriteC(IOE_SPI_CHN, IOE_OPCODE_HEADER | IOE_READ);
    while (SpiChnIsBusy(IOE_SPI_CHN)) { ; } // wait for byte to be sent
    data = SpiChnReadC(IOE_SPI_CHN); // junk

    // register address
    SpiChnWriteC(IOE_SPI_CHN, reg_addr);
    while (SpiChnIsBusy(IOE_SPI_CHN)) { ; }
    data = SpiChnReadC(IOE_SPI_CHN); // junk

    // tx one byte of dummy data so we can rx real data
    SpiChnWriteC(IOE_SPI_CHN, data);
    while (SpiChnIsBusy(IOE_SPI_CHN)) { ; }
    data = SpiChnReadC(IOE_SPI_CHN); // the byte we want

    // CS high to end transaction
    IOE_SET_CS();

    return data;
}
