/*
    Get sequential keypress data from keypad (Adafruit 1824) connected via I/O expander
 */

// keypad connections:
// C0 -- col 1 -- internal pullup resistor -- avoid open circuit input when no button pushed
// C1 -- col 2 -- internal pullup resistor
// C2 -- col 3 -- internal pullup resistor
// D0 -- row 1 -- thru 300 ohm resistor -- avoid short when two buttons pushed
// D1 -- row 2 -- thru 300 ohm resistor
// D2 -- row 3 -- thru 300 ohm resistor
// D3 -- row 4 -- thru 300 ohm resistor


#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include "io_expander.h"
//#include "uart.h"
//#include <stdio.h>

// some precise, fixed, short delays
// to use for extending pulse durations on the keypad
// if behavior is erratic
#define NOP asm("nop");
// 1/2 microsec
#define wait20 NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
// one microsec
#define wait40 wait20;wait20;

char buffer[80];
int keytable[12] = {0x82,                //    0
                               0x11, 0x12, 0x14,    // 1, 2, 3
                               0x21, 0x22, 0x24,    // 4, 5, 6
                               0x41, 0x42, 0x44,    // 7, 8, 9
                               0x81, 0x84};         // *,    #

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    osc_tune(56);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left
    tft_setCursor(0, 0);
    tft_setTextColor(ILI9340_YELLOW);
    tft_setTextSize(5);
    tft_setTextWrap(1);

    int i;
    int keypad, pattern;
    int pressed = 0,
        cleared = 1;

    ioe_init();
    // init the keypad pins D0-D3 and C0-C2
    // PortA ports as digital outputs
    ioe_PortDSetPinsOut(BIT_0 | BIT_1 | BIT_2 | BIT_3);
    // PortB as inputs
    ioe_PortCSetPinsIn(BIT_0 | BIT_1 | BIT_2);
    // and turn on pull-up resistors on inputs
    ioe_PortCEnablePullUp(BIT_0 | BIT_1 | BIT_2);

    while(1) {

        pattern = 1;
        for (i = 0; i < 4; i++) {
            ioe_write(OLATD, ~pattern); // pull row i low
            wait20;
            keypad = ~ioe_read(GPIOC) & 0x7; // read the three columns
            if (keypad != 0) {
                keypad |= (pattern << 4);
                break;
            }
            pattern <<= 1;
        }
        
        // search for keycode
        if (keypad > 0) { // then at least one button is pushed
            for (i = 0; i < 12; i++){
                if (keytable[i] == keypad) {
                    pressed = 1;  // got a valid keypress (single button)
                    break;
                }
            }
        }
        else {
            cleared = 1;
            continue; // back to top of while-loop
        }

        if (cleared && pressed) { // valid key, but not just previous one still being held down
            cleared = 0;
            if (i < 10)
                tft_write('0' + i);
            else if (i == 10)
                tft_write('*');
            else    // i == 11
                tft_write('#');
            // brain-dead debounce algorithm, but works fine:
            delay(50);
        }
    }
    return 0;
}
