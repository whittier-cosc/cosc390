/*
    Bit-banging UART TX    
 */

#include "config.h"
#include "util.h"
#include <string.h>

#define NOP asm("nop");
// 0.5 microsecond
#define wait20 NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;

// We need to wait 104.2 us between bits for 9600 baud
#define one_us wait20;wait20;
#define four_us one_us;one_us;one_us;one_us;
#define ten_us one_us;one_us;one_us;one_us;one_us;one_us;one_us;one_us;one_us;one_us;
#define hundred_us ten_us;ten_us;ten_us;ten_us;ten_us;ten_us;ten_us;ten_us;ten_us;ten_us;
#define baud_delay_9600 hundred_us;four_us;

void my_delay() {
    volatile int j;
    // magic constant 467 obtained empirically
    for (j = 0; j < 467; j++) {
    }
}

void tx_byte(char c) {
    LATBbits.LATB3 = 0; // start bit
    // baud_delay_9600;
    my_delay();
    for (int i = 0; i < 8; i++) { 
        LATBbits.LATB3 = c % 2;
        // baud_delay_9600;
        my_delay();
        c /= 2;
    }
    LATBbits.LATB3 = 1; // idle state
    my_delay();
    // baud_delay_9600;
}

int main(void)
{
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    // char msg = 'X';
    char secret[] = "Meet me at midnight under the bridge.";
    TRISBbits.TRISB3 = 0;
    LATBbits.LATB3 = 1; // idle state
    delay(5000);
    int n = strlen(secret);
    for (int i = 0; i < n; i++) {
        tx_byte(secret[i]);
    }
    while (1){ 
    }
    return 0;
}