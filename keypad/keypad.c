/*
    Get keypress data from keypad (Adafruit 1824) connected via I/O expander
 */

// keypad connections:
// C0 -- col 1 -- internal pullup resistor
// C1 -- col 2 -- internal pullup resistor
// C2 -- col 3 -- internal pullup resistor
// D0 -- row 1 -- thru 220 ohm resistor -- avoid short when two buttons pushed
// D1 -- row 2 -- thru 220 ohm resistor
// D2 -- row 3 -- thru 220 ohm resistor
// D3 -- row 4 -- thru 220 ohm resistor

#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include "io_expander.h"

// A precise, fixed, short delay
// to use for extending pulse durations on the keypad
#define NOP asm("nop");
// 0.5 microsecond
#define wait20 NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;

int keytable[12] = {0x82,             //    0
                    0x11, 0x12, 0x14, // 1, 2, 3
                    0x21, 0x22, 0x24, // 4, 5, 6
                    0x41, 0x42, 0x44, // 7, 8, 9
                    0x81, 0x84};      // *,    #

char get_key();
void beep(int duration);

int main(void)
{
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    // Buzzer
    PORTSetPinsDigitalOut(IOPORT_A, BIT_1);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left
    tft_setCursor(0, 0);
    tft_setTextColor(ILI9340_YELLOW);
    tft_setTextSize(2);
    tft_setTextWrap(1);

    ioe_init();
    ioe_PortDSetPinsOut(BIT_0 | BIT_1 | BIT_2 | BIT_3);
    ioe_PortCSetPinsIn(BIT_0 | BIT_1 | BIT_2);
    ioe_PortCEnablePullUp(BIT_0 | BIT_1 | BIT_2);

    // Buzzer off
    LATAbits.LATA1 = 0;

    // Repeatedly scan the keypad. If a button was pressed (and is not just being
    // held down), print the button on the TFT LCD and beep.
    char c; 
    int cleared = 1;
    while (1)
    {
        c = get_key();
        if (c == -1) {
            // no key is pressed
            cleared = 1;
            continue;
        }
        
        if (cleared)
        {
            cleared = 0;
            tft_write(c);
            beep(100);
        }
        delay(20);
    }
    return 0;
}

char get_key()
{
    int i;
    int keypad, pattern;
    int pressed = 0;

    pattern = 1;
    for (i = 0; i < 4; i++)
    {
        ioe_write(OLATD, ~pattern); // pull row i low
        wait20; // wait 0.5 microsecond
        keypad = ~ioe_read(GPIOC) & 0x7; // read the three columns
        if (keypad != 0)
        {
            keypad |= (pattern << 4);
            break;
        }
        pattern <<= 1;
    }

    // search for keycode
    if (keypad > 0)
    { // then at least one button is pushed
        for (i = 0; i < 12; i++)
        {
            if (keytable[i] == keypad)
            {
                pressed = 1; // got a valid keypress (single button)
                break;
            }
        }
    }

    if (pressed)
    { 
        if (i < 10)
            return '0' + i;
        else if (i == 10)
            return '*';
        else // i == 11
            return '#';
    }

    return -1;
}

/*
    Beep for the given number of milliseconds.
*/
void beep(int duration)
{
    // Assumes we're using a buzzer that emits a tone as long as
    // a constant voltage (3.3 V) is applied.
    LATAbits.LATA1 = 1;
    delay(100);
    LATAbits.LATA1 = 0;
}
