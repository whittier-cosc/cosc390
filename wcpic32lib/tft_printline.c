/*
 *  @file   tft_printline.c
 *
 *  @brief  Provides a line-based printing function for the TFT.
 */

#include "tft.h"

/**
 *  Prints a string on a given "line number" of the TFT using the given text
 *  size.
 *
 *  Lines are numbered starting at 0. Text size can be 1, 2, 3, 4, or 5.
 *
 *  The height of a line (and therefore the number of lines on the TFT display)
 *  depends on the text size used.
 */
void tft_printLine(int line_num, int text_size, char *str) {
    int y = line_num * 10;
    // erase the line by drawing a black filled rectangle
    tft_fillRect(0, y, tft_width() - 1, 10*text_size, ILI9340_BLACK);
    tft_setCursor(0, y);
    tft_setTextColor(ILI9340_YELLOW);
    tft_setTextSize(text_size);
    tft_writeString(str);
}
