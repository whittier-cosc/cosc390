#include "config.h"
#include "tft_master.h"
#include "tft_gfx.h"

/*
 * Experiment with DMA transfers
 */

char msg[40];

char src[4] = {'a', 'b', 'c', 'd'};
char dst[17] = "XXXXXXXXXXXXXXXX";

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 319, 10*char_size, 1, ILI9340_BLACK);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_BLUE); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

void delay() {
    volatile int i;
    for (i = 0; i < 1000; i++) {
    }
}

int main() {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    tft_init();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    printLine(1, 3, dst);

    DCH0SSA = KVA_TO_PA(src);
    DCH0SSIZ = 4;
    DCH0DSA = KVA_TO_PA(dst);
    DCH0DSIZ = 16;
    DCH0CSIZ = 16;
    DMACONbits.ON = 1;
    DCH0CONbits.CHEN = 1; // enable channel 0
    DCH0ECONbits.CFORCE = 1;

    delay(); // allow tranfer to finish

    printLine(5, 3, dst);

    
    //TRISB = 0xFFF7; // RB3 pin 7 LED output

    while(1) {
    }
    return 0;
}

