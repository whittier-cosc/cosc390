#include "io_expander.h"

#define IOE_SPI_CHN	  SPI_CHANNEL2
#define IOE_SET_CS    {mPORTBSetBits(BIT_9);}
#define IOE_CLEAR_CS  {mPORTBClearBits(BIT_9);}

// === spi bit widths ====================================================
// hit the SPI control register directly, SPI2
// Change the SPI bit modes on the fly, mid-transaction if necessary
inline void SPI_Mode16(void){  // configure SPI2 for 16-bit mode
    SPI2CONSET = 0x400;
    SPI2CONCLR = 0x800;
}
// ========
inline void SPI_Mode8(void){  // configure SPI2 for 8-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONCLR = 0x800;
}
// ========
inline void SPI_Mode32(void){  // configure SPI2 for 32-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONSET = 0x800;
}

void ioe_init() {
  mPORTBSetPinsDigitalOut(BIT_9); // use RPB9 (pin 18) as CS
  IOE_SET_CS // CS high initially
  PPSOutput(2, RPB5, SDO2); // use RPB5 (pin 14) for SDO2 (MOSI)
  PPSInput(3, SDI2, RPA4); //  use RPA4 (pin 12) for SDI2 (MISO)
  
  SpiChnOpen( IOE_SPI_CHN, 
              SPI_OPEN_ON | SPI_OPEN_MODE8 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV, 
			  4 ); // SPI clock divisor 4 to give 10 MHz max speed 
			       // for this I/O expander (4 assumes PBCLK is 40 M)
  
  ioe_write(IOCON, CLEAR_BANK   | CLEAR_MIRROR | SET_SEQOP |
                   CLEAR_DISSLW | CLEAR_HAEN   | CLEAR_ODR |
                   CLEAR_INTPOL );
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

void ioe_portYSetPinsOut(unsigned char bitmask){
  clearBits(IODIRY, bitmask);
}

void ioe_portZSetPinsOut(unsigned char bitmask){
  clearBits(IODIRZ, bitmask);
}

void ioe_portYSetPinsIn(unsigned char bitmask){
  setBits(IODIRY, bitmask);
}

void ioe_portZSetPinsIn(unsigned char bitmask){
  setBits(IODIRZ, bitmask);
}

void ioe_portYIntEnable(unsigned char bitmask){
  setBits(GPINTENY, bitmask);
}

void ioe_portZIntEnable(unsigned char bitmask){
  setBits(GPINTENZ, bitmask);
}

void ioe_portYIntDisable(unsigned char bitmask){
  clearBits(GPINTENY, bitmask);
}

void ioe_portZIntDisable(unsigned char bitmask){
  clearBits(GPINTENZ, bitmask);
}

void ioe_portYEnablePullUp(unsigned char bitmask){
  setBits(GPPUY, bitmask);
}

void ioe_portZEnablePullUp(unsigned char bitmask){
  setBits(GPPUZ, bitmask);
}

void ioe_portYDisablePullUp(unsigned char bitmask){
  clearBits(GPPUY, bitmask);
}

void ioe_portZDisablePullUp(unsigned char bitmask){
  clearBits(GPPUZ, bitmask);
}

inline void ioe_write(unsigned char reg_addr, unsigned char data) {
  
  // wait until ready
  while (!SpiChnTxBuffEmpty(IOE_SPI_CHN)) { ; }
  SPI_Mode8();
  
  // CS low to start transaction
  IOE_CLEAR_CS
  
  // opcode and hw address (Should always be 0b0100000)
  SpiChnWriteC(IOE_SPI_CHN, IOE_OPCODE_HEADER | IOE_WRITE);
  while (SpiChnIsBusy(IOE_SPI_CHN)) { ; } // wait for byte to be sent
  SpiChnReadC(IOE_SPI_CHN); // ignore junk return value
  
  // output register address
  SpiChnWriteC(IOE_SPI_CHN, reg_addr);
  while (SpiChnIsBusy(IOE_SPI_CHN)) { ; }
  SpiChnReadC(IOE_SPI_CHN);
  
  // one byte of data
  SpiChnWriteC(IOE_SPI_CHN, data);
  while (SpiChnIsBusy(IOE_SPI_CHN)) { ; }
  SpiChnReadC(IOE_SPI_CHN);
  
  // CS high to end transaction
  IOE_SET_CS
  
}

inline unsigned char ioe_read(unsigned char reg_addr) {
  unsigned char data;
  
  // wait until ready
  while (!SpiChnTxBuffEmpty(IOE_SPI_CHN)) { ; }
  SPI_Mode8();
  
  // CS low to start transaction
  IOE_CLEAR_CS
  
  // opcode and hw address (Should always be 0b0100000)
  SpiChnWriteC(IOE_SPI_CHN, IOE_OPCODE_HEADER | IOE_READ);
  while (SpiChnIsBusy(IOE_SPI_CHN)) { ; } // wait for byte to be sent
  data = SpiChnReadC(IOE_SPI_CHN); // junk
  
  // input register address
  SpiChnWriteC(IOE_SPI_CHN, reg_addr);
  while (SpiChnIsBusy(IOE_SPI_CHN)) { ; } 
  data = SpiChnReadC(IOE_SPI_CHN); // junk
  
  // tx one byte of dummy data so we can rx real data
  SpiChnWriteC(IOE_SPI_CHN, data);
  while (SpiChnIsBusy(IOE_SPI_CHN)) { ; }
  data = SpiChnReadC(IOE_SPI_CHN); // the byte we want
  
  // CS high to end transaction
  IOE_SET_CS
  
  return data;
}
