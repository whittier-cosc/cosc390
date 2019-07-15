#ifndef IO_EXPANDER_H
#define	IO_EXPANDER_H

/**
 *  @file   io_expander.h
 *
 *  @brief  A PIC32 library for the MCP23S17 I/O expander.
 *
 *  The ports on the MCP23S17 are named GPA0..GPA7 and GPB0..GPB7, but
 *  we rename these to C0..C7 and D0..D7, respectively, to avoid confusion
 *  with the PIC's I/O ports.
 *
 *  @authors    Sean Carroll and Bruce Land (Cornell University),
 *              Jeff Lutgen
 */

#define IOE_OPCODE_HEADER 0b01000000
#define IOE_READ 0b00000001
#define IOE_WRITE 0b00000000

// IOCON Settings
#define SET_BANK     0x80
#define CLEAR_BANK   0x00
#define SET_MIRROR   0x40
#define CLEAR_MIRROR 0x00
#define SET_SEQOP    0x20
#define CLEAR_SEQOP  0x00
#define SET_DISSLW   0x10
#define CLEAR_DISSLW 0x00
#define SET_HAEN     0x08
#define CLEAR_HAEN   0x00
#define SET_ODR      0x04
#define CLEAR_ODR    0x00
#define SET_INTPOL   0x02
#define CLEAR_INTPOL 0x00

/**
 *  @name Register Addresses (BANK = 0)
 *
 *  Note: Renamed all PortA to PortC, PortB to PortD to avoid confusion with PIC
 */
/**@{*/
#define IODIRC   0x00   ///< I/O direction
#define IODIRD   0x01   ///< I/O direction
#define IPOLC    0x02   ///< input polarity
#define IPOLD    0x03   ///< input polarity
#define GPINTENC 0x04   ///< interrupt-on-change enable
#define GPINTEND 0x05   ///< interrupt-on-change enable
#define DEFVALC  0x06   ///< default comparison value
#define DEFVALD  0x07   ///< default comparison value
#define INTCONC  0x08   ///< interrupt-on-change control
#define INTCOND  0x09   ///< interrupt-on-change control
#define IOCON    0x0A   ///< I/O expander configuration
//#define IOCON    0x0B
#define GPPUC    0x0C   ///< GPIO pull-up resistor configuration
#define GPPUD    0x0D   ///< GPIO pull-up resistor configuration
#define INTFC    0x0E   ///< interrupt flag
#define INTFD    0x0F   ///< interrupt flag
#define INTCAPC  0x10   ///< interrupt captured
#define INTCAPD  0x11   ///< interrupt captured
#define GPIOC    0x12   ///< GPIO port C
#define GPIOD    0x13   ///< GPIO port D
#define OLATC    0x14   ///< output latch
#define OLATD    0x15   ///< output latch
/**@}*/

void ioe_init();

void ioe_PortCSetPinsOut(unsigned char);
void ioe_PortDSetPinsOut(unsigned char);
void ioe_PortCSetPinsIn(unsigned char);
void ioe_PortDSetPinsIn(unsigned char);
void ioe_PortCIntEnable(unsigned char);
void ioe_PortCIntDisable(unsigned char);
void ioe_PortDIntEnable(unsigned char);
void ioe_PortDIntDisable(unsigned char);
void ioe_PortCEnablePullUp(unsigned char);
void ioe_PortDEnablePullUp(unsigned char);
void ioe_PortCDisablePullUp(unsigned char);
void ioe_PortDDisablePullUp(unsigned char);

/* Takes a register address on I/O expander and a data byte, and writes the 
 * data to the target register. */
inline void ioe_write(unsigned char, unsigned char);

/* Takes a register address on I/O expander and returns the data byte from that
 * target register. */
inline unsigned char ioe_read(unsigned char);

#endif

