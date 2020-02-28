/*
 * Experiment with DMA transfers
 */

#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"

char msg[80];
char src[4] = {'a', 'b', 'c', 'd'};
char dst[17] = "XXXXXXXXXXXXXXXX";

int main() {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    tft_printLine(1, 3, dst);

    DCH0SSA = KVA_TO_PA(src);
    DCH0SSIZ = 4;
    DCH0DSA = KVA_TO_PA(dst);
    DCH0DSIZ = 16;
    DCH0CSIZ = 16;
    DMACONbits.ON = 1;
    DCH0CONbits.CHEN = 1; // enable channel 0
    DCH0ECONbits.CFORCE = 1;

    delay(1); // allow tranfer to finish

    tft_printLine(5, 3, dst);

    //TRISB = 0xFFF7; // RB3 pin 7 LED output

    while(1) {
    }
    return 0;
}

