#ifndef TFT_GFX_H
#define TFT_GFX_H

/**
 *  @file   tft_gfx.h
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
 *          library.
 *
 *  @authors Syed Tahmid Mahbub,
 *           Jeff Lutgen (minor modifications)
 */

/*
   This is the core graphics library for all our displays, providing a common
   set of graphics primitives (points, lines, circles, etc.).  It needs to be
   paired with a hardware-specific library for each display device we carry
   (to handle the lower-level functions).

   Adafruit invests time and resources providing this open source code, please
   support Adafruit & open-source hardware by purchasing products from Adafruit!

   Copyright (c) 2013 Adafruit Industries.  All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#define tabspace 4 // number of spaces for a tab

inline short tft_width(void);
inline short tft_height(void);

void tft_drawLine(short x0, short y0, short x1, short y1, unsigned short color);
void tft_drawRect(short x, short y, short w, short h, unsigned short color);

void tft_drawCircle(short x0, short y0, short r, unsigned short color);

void tft_fillCircle(short x0, short y0, short r, unsigned short color);

void tft_drawTriangle(short x0, short y0, short x1, short y1,
                      short x2, short y2, unsigned short color);
void tft_fillTriangle(short x0, short y0, short x1, short y1,
                      short x2, short y2, unsigned short color);
void tft_drawRoundRect(short x0, short y0, short w, short h,
                       short radius, unsigned short color);
void tft_fillRoundRect(short x0, short y0, short w, short h, short radius,
                       unsigned short color);
void tft_drawBitmap(short x, short y, const unsigned char *bitmap, short w,
                    short h, unsigned short color);
void tft_drawChar(short x, short y, unsigned char c, unsigned short color,
                  unsigned short bg, unsigned char size);
void tft_setCursor(short x, short y);
void tft_setTextColor(unsigned short c);
void tft_setTextColor2(unsigned short c, unsigned short bg);
void tft_setTextSize(unsigned char s);
void tft_setTextWrap(char w);
void tft_write(unsigned char c);
void tft_writeString(char* str);

#endif
