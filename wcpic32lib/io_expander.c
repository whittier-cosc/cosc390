/*
 *  @file   io_expander.c
 *
 *  @brief  A PIC32 library for interfacing with the MCP23S17 I/O expander
 *
 *  @authors    Sean Carroll,
 *              Bruce Land,
 *              Jeff Lutgen
 *
 *  The ports on the MCP23S17 are named GPA0..GPA7 and GPB0..GPB7, but
 *  we rename these to C0..C7 and D0..D7, respectively, to avoid confusion
 *  with the PIC's I/O ports.
 */

#include "private/common.h"
#include "io_expander.h"

#define IOE_SPI_CHN     SPI_CHANNEL2
#define IOE_SET_CS()    (mPORTBSetBits(BIT_7))
#define IOE_CLEAR_CS()  (mPORTBClearBits(BIT_7))

// Change the SPI bit modes on the fly, mid-transaction if necessary
static inline void SPI_Mode16(void) {  // configure SPI2 for 16-bit mode
    SPI2CONSET = 0x400;
    SPI2CONCLR = 0x800;
}

static inline void SPI_Mode8(void) {   // configure SPI2 for 8-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONCLR = 0x800;
}

static inline void SPI_Mode32(void) {  // configure SPI2 for 32-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONSET = 0x800;
}

/**
 *  Initializes the MCP23S17 I/O expander and configures one of the
 *  PIC's SPI modules for communication with the I/O expander.
 *
 *  Specifically, initializes and enables SPI2 in 8-bit mode, setting the
 *  SPI clock divisor for PBCLK to 4, which gives an SPI clock rate of
 *  10MHz SPI clock (the fastest possible for the MCP23S17).
 *
 *  Pins used on PIC:
 *
 *      CS:  RB7           (Pin 16)
 *      SCK: SCK2          (Pin 26)
 *      SDI: RPA4 --> SDI2 (Pin 12)
 *      SDO: RPB5 --> SDO2 (Pin 14)
 *
 *  Example:
 *
 *      ioe_init();
 */
void ioe_init() {
    mPORTBSetPinsDigitalOut(BIT_7); // use RPB9 (pin 18) as CS
    IOE_SET_CS(); // CS high initially

    // These pin mappings assume that IOE_SPI_CHN is SPI_CHANNEL2!
    PPSOutput(2, RPB5, SDO2);   // use RPB5 (pin 14) for SDO2 (MOSI)
    PPSInput(3, SDI2, RPA4);    // use RPA4 (pin 12) for SDI2 (MISO)

    SpiChnOpen(IOE_SPI_CHN,
               SPI_OPEN_ON | SPI_OPEN_MODE8 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV,
               _pbclk / 10000000);  // SPI clock divisor to give 10 MHz max speed

    ioe_write(IOCON, CLEAR_BANK | CLEAR_MIRROR | SET_SEQOP |
              CLEAR_DISSLW | CLEAR_HAEN | CLEAR_ODR |
              CLEAR_INTPOL);
}

/**
 *  Sets the given Port C pins to be outputs
 */
void ioe_PortCSetPinsOut(unsigned char bitmask){
    ioe_clearBits(IODIRC, bitmask);
}

/**
 *  Sets the given Port D pins to be outputs
 */
void ioe_PortDSetPinsOut(unsigned char bitmask){
    ioe_clearBits(IODIRD, bitmask);
}

/**
 *  Sets the given Port C pins to be inputs
 */
void ioe_PortCSetPinsIn(unsigned char bitmask){
    ioe_setBits(IODIRC, bitmask);
}

/**
 *  Sets the given Port D pins to be inputs
 */
void ioe_PortDSetPinsIn(unsigned char bitmask){
    ioe_setBits(IODIRD, bitmask);
}

/**
 *  Enables interrupts for the given Port C pins
 */
void ioe_PortCIntEnable(unsigned char bitmask){
    ioe_setBits(GPINTENC, bitmask);
}

/**
 *  Enables interrupts the given Port D pins
 */
void ioe_PortDIntEnable(unsigned char bitmask){
    ioe_setBits(GPINTEND, bitmask);
}

/**
 *  Disables interrupts for the given Port C pins
 */
void ioe_PortCIntDisable(unsigned char bitmask){
    ioe_clearBits(GPINTENC, bitmask);
}

/**
 *  Disables interrupts for the given Port D pins
 */
void ioe_PortDIntDisable(unsigned char bitmask){
    ioe_clearBits(GPINTEND, bitmask);
}

/**
 *  Enables the internal pull-up resistor on the given Port C pins
 */
void ioe_PortCEnablePullUp(unsigned char bitmask){
    ioe_setBits(GPPUC, bitmask);
}

/**
 *  Enables the internal pull-up resistor on the given Port D pins
 */
void ioe_PortDEnablePullUp(unsigned char bitmask){
    ioe_setBits(GPPUD, bitmask);
}

/**
 *  Disables the internal pull-up resistor on the given Port C pins
 */
void ioe_PortCDisablePullUp(unsigned char bitmask){
    ioe_clearBits(GPPUC, bitmask);
}

/**
 *  Disables the internal pull-up resistor on the given Port D pins
 */
void ioe_PortDDisablePullUp(unsigned char bitmask){
    ioe_clearBits(GPPUD, bitmask);
}

/**
 *  Writes a byte of data to a register on the I/O expander
 *
 *  Example:
 *
 *      ioe_write(OLATC, 0x42);
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

/**
 *  Reads and returns a byte of data from a register on the I/O expander
 *
 *  Example:
 *
 *      unsigned char signal = ioe_read(GPIOD);
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

/**
 *  Clears the bits set in `bitmask` in the given register on the
 *  I/O expander. Does not modify other bits in that register.
 */
void ioe_clearBits(unsigned char addr, unsigned char bitmask){
    if (addr <= 0x15){
        unsigned char cur_val = ioe_read(addr);
        ioe_write(addr, cur_val & ~bitmask);
    }
}

/**
 *  Sets the bits specified by `bitmask` in the given register on the
 *  I/O expander. Does not modify other bits in that register.
 */
void ioe_setBits(unsigned char addr, unsigned char bitmask){
    if (addr <= 0x15){
        unsigned char cur_val = ioe_read(addr);
        ioe_write(addr, cur_val | bitmask);
    }
}

/**
 *  Toggles the bits specified by `bitmask` in the given register on the
 *  I/O expander. Does not modify other bits in that register.
 */
void ioe_toggleBits(unsigned char addr, unsigned char bitmask){
    if (addr <= 0x15){
        unsigned char cur_val = ioe_read(addr);
        ioe_write(addr, cur_val ^ bitmask);
    }
}

//static unsigned char readBits(unsigned char addr, unsigned char bitmask){
//    if (addr <= 0x15){
//        unsigned char cur_val = ioe_read(addr) & bitmask ;
//        return cur_val ;
//    }
//}
