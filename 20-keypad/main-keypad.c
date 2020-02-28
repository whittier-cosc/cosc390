/*
    Scan keypresses from keypad (Adafruit 1824)
 */

// keypad connections:
// A0 -- row 1 -- thru 300 ohm resistor -- avoid short when two buttons pushed
// A1 -- row 2 -- thru 300 ohm resistor
// A2 -- row 3 -- thru 300 ohm resistor
// A3 -- row 4 -- thru 300 ohm resistor
// B7 -- col 1 -- internal pulldown resistor -- avoid open circuit input when no button pushed
// B8 -- col 2 -- internal pulldown resistor
// B9 -- col 3 -- internal pulldown resistor

#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include "io_expander.h"

// some precise, fixed, short delays
// to use for extending pulse durations on the keypad
// if behavior is erratic
#define NOP asm("nop");
// 1/2 microsec
#define wait20 NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
// one microsec
#define wait40 wait20;wait20;

// pullup/down macros for keypad

//PORT A
#define EnablePullDownA(bits)   CNPUACLR=bits; CNPDASET=bits;
#define DisablePullDownA(bits)  CNPDACLR=bits;
#define EnablePullUpA(bits)     CNPDACLR=bits; CNPUASET=bits;
#define DisablePullUpA(bits)    CNPUACLR=bits;
// PORT B
#define EnablePullDownB(bits)   CNPUBCLR=bits; CNPDBSET=bits;
#define DisablePullDownB(bits)  CNPDBCLR=bits;
#define EnablePullUpB(bits)     CNPDBCLR=bits; CNPUBSET=bits;
#define DisablePullUpB(bits)    CNPUBCLR=bits;

char buffer[80];

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    osc_tune(56);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    static int keypad, i, pattern;
    // order is 0 thru 9 then * ==10 and # ==11
    // no press = -1
    // table is decoded to natural digit order (except for * and #)
    // 0x80 for col 1 ; 0x100 for col 2 ; 0x200 for col 3
    // 0x01 for row 1 ; 0x02 for row 2; etc
    static int keytable[12] = {0x108, 0x81, 0x101, 0x201, 0x82, 0x102, 0x202, 0x84, 0x104, 0x204, 0x88, 0x208};
    // init the keypad pins A0-A3 and B7-B9
    // PortA ports as digital outputs
    mPORTASetPinsDigitalOut(BIT_0 | BIT_1 | BIT_2 | BIT_3);    //Set port as output
    // PortB as inputs
    mPORTBSetPinsDigitalIn(BIT_7 | BIT_8 | BIT_9);    //Set port as input
    // and turn on pull-down on inputs
    EnablePullDownB(BIT_7 | BIT_8 | BIT_9);

    char curr_char = 0;
    while(1) {
        // read each row sequentially
        mPORTAClearBits(BIT_0 | BIT_1 | BIT_2 | BIT_3);
        pattern = 1; mPORTASetBits(pattern);

        for (i = 0; i < 4; i++) {
            wait20;
            keypad  = mPORTBReadBits(BIT_7 | BIT_8 | BIT_9);
            if (keypad != 0) {
                keypad |= pattern;
                break;
            }
            mPORTAClearBits(pattern);
            pattern <<= 1;
            mPORTASetBits(pattern);
        }
        
        // search for keycode
        if (keypad > 0) { // then button is pushed
            for (i = 0; i < 12; i++){
                if (keytable[i] == keypad) break;
            }
            // if invalid, two button push, set to -1
            if (i == 12)
                i = -1;
        }
        else
            i = -1; // no button pushed

        // draw key number
        //tft_fillRoundRect(30,200, 100, 14, 1, ILI9340_BLACK);// x,y,w,h,radius,color
        //tft_setCursor(30, 200);
        //tft_setTextColor(ILI9340_YELLOW); tft_setTextSize(2);
        sprintf(buffer,"%d", i);
        if (i == 10)
            sprintf(buffer,"*");
        if (i == 11)
            sprintf(buffer,"#");
        //tft_writeString(buffer);
        if (buffer[0] != curr_char) {
            curr_char = buffer[0];
            tft_printLine(2, 3, buffer);
        }
    }
    return 0;
}
