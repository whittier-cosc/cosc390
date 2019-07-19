#include "config.h"
#include "tft.h"
#include "tft_printline.h"
#include "util.h"

char msg[80];

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left
    sprintf(msg, "Hello from stick!");
    tft_printLine(0, 2, msg);
    tft_printLine(2, 2, msg);
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
