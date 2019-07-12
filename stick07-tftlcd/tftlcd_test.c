#include "config.h"
#include "tft.h"
#include "util.h"

char msg[80];

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 239, 10, 1, ILI9340_BLACK);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_BLUE); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    tft_init();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left
    sprintf(msg, "Hello from stick!");
    printLine(0, 2, msg);
    printLine(2, 2, msg);
    tft_drawLine(0, 0, 319, 239, 0x67F3); 
    tft_drawCircle(100, 100, 25, tft_Color565(255, 255, 0));

    while(1) {
        // Very annoying flashing effect
        //delay(250);
        //tft_writecommand(0x21);  // invert all pixels
        //delay(250);
        //tft_writecommand(0x20);  // back to normal
    }
    return 0;
}
