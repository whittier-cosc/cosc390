/*
 * Try out the parallel port
 */

#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"

char msg[80];

void timer3_init() {
    T3CONbits.TCKPS = 0; // prescaler = 1, so freq = PB freq = 40 MHz 
    PR3 = 0xffff; // default
    T3CONbits.ON = 1;
}

void parallel_init() {
    // All 16 bits of address multiplexed on PMD0-7
    PMCONbits.ADRMUX = 2; 
    // Master mode 1
    PMMODEbits.MODE = 3;
    // Enable PMRD/PMWR on PMRD pin (pin 24) 
    PMCONbits.PTRDEN = 1;
    PMMODEbits.WAITB = 3;  // extend the addressing period
    PMMODEbits.WAITM = 1;  // can't set this to 0, else WAITB is ignored
    //PMMODEbits.WAITE = 3;
    PMCONbits.ON = 1;
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    parallel_init();

    //TRISB = 0xFFF7; // RB3 pin 7 LED output

    char c = 'a';
    PMADDR = 0x8899;
    while(1) {
     //   delay(250);
     //   LATBINV = 8;
        PMDIN = c++;
        if (c == 'z' + 1)
            c = 'a';
        while (PMMODEbits.BUSY)
            ;
    }
    return 0;
}
