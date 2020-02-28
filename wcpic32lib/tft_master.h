#ifndef TFT_MASTER_H
#define TFT_MASTER_H

/**
 *  @file   tft_master.h
 *
 *  @brief  A PIC32 library for the Adafruit 2.2" TFT liquid crystal display.
 *
 *          Intended for use with the PIC32MX250F128B.
 *
 *          Code rewritten from the Adafruit Arduino library for the TFT
 *          by Syed Tahmid Mahbub.
 *
 *          The TFT is Adafruit product 1480.
 *
 *          This file includes the text header from the original Adafruit
 *          library, followed by the code.
 *
 *  @authors Syed Tahmid Mahbub,
 *           Jeff Lutgen (minor modifications)
 */

/***************************************************
  This is an Arduino Library for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

extern unsigned short _width, _height;

// Predefined colors (16 bits, 5-6-5 RGB)
#define	ILI9340_BLACK   0x0000
#define	ILI9340_BLUE    0x001F
#define	ILI9340_RED     0xF800
#define	ILI9340_GREEN   0x07E0
#define ILI9340_CYAN    0x07FF
#define ILI9340_MAGENTA 0xF81F
#define ILI9340_YELLOW  0xFFE0
#define ILI9340_WHITE   0xFFFF

void tft_init();
void tft_drawPixel(short x, short y, unsigned short color);
void tft_drawFastVLine(short x, short y, short h, unsigned short color);
void tft_drawFastHLine(short x, short y, short w, unsigned short color);
void tft_fillScreen(unsigned short color);
void tft_fillRect(short x, short y, short w, short h, unsigned short color);
unsigned short tft_Color565(unsigned char r, unsigned char g, unsigned char b);
void tft_setRotation(unsigned char m);

#endif
