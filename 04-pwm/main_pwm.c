#include "config.h"
#include "uart.h"

#define BUFSZ 80
char msg[BUFSZ];

void pwm_config(void) {
    // PWM on OC4. Default is 16-bit timer, Timer2.
    PR2 = 0xffff;   // Timer2 period = 65535 (the maximum),
                    // so PWM period = 65535 + 1.
    OC4RS = 0x8000; // Duty cycle = 50% (0x8000 is half of (0xffff + 1))
    OC4R = 0x8000;  // We have to set this once, before firing up PWM.
                    // Subsequent changes to duty cycle are made by writing
                    // to OC4RS.
    OC4CONbits.OCM = 6; // 3 bits, 110 = PWM mode, fault pin disabled
    T2CONbits.ON = 1;   // Turn on Timer2
    OC4CONbits.ON = 1;  // Turn on PWM.
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    U1RXR = 0; // Map RPA2 (pin 9) to U1RX
    RPB3R = 1; // Map RPB3 (pin 7) to U1TX
    RPB2R = 5; // Map RPB2 (pin 6) to OC4 (PWM output)

    uart_init();
    pwm_config();

    TRISA = 0xFFFE;        // A0 is LED, so make it an output.
    LATAbits.LATA0 = 0;    // Turn LED off.

    sprintf(msg, "U1BRG = %d\n", U1BRG);
    uart_write(msg);
    unsigned int duty_cycle;
    while(1) {
        uart_read(msg, BUFSZ - 1); // Block until we receive a string
        duty_cycle = atoi(msg); // 0..100 percent
        OC4RS = duty_cycle * 0x10000 / 100; // Update duty cycle
        LATAINV = 0x0001;      // Toggle LED
        uart_write(msg); // Echo the string we received
        uart_write("\n");
    }
    return 0;
}
