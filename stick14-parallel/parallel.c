#include "config.h"
#include "tft_master.h"
#include "tft_gfx.h"

/*
 * Try out the parallel port
 */

#define	GetSystemClock() 			(40000000ul)
#define	GetPeripheralClock()		(GetSystemClock()/(1 << OSCCONbits.PBDIV))
#define	GetInstructionClock()		(GetSystemClock())

char msg[40];

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

// Delay for a given number of milliseconds. This crude implementation
// is often good enough, but accuracy will suffer if significant time
// is spent in interrupt service routines. See delay_ms() in peripherals/tft_master.c
// for a better implementation that uses the core timer.
void delay(int ms) {
    volatile int j;
    for (j = 0; j < (SYSCLK / 8920) * ms; j++) { // magic constant 8920 obtained empirically
    }
}

int main(void) {
    SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    tft_init();
    tft_begin();
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
