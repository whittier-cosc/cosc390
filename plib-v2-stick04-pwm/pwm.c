#include "config.h"
#include "uart.h"

#define BUFLEN 40
char msg[BUFLEN];

void uart_config(void) {
    // With PBCLK = CORE_TICKS = 40 M, we set U1BRG = 259, giving 
    // baud rate = 9615.4 (see DS61107F, Table 21-2).
    // Default is no parity bit, 8 bits data, no stop bit (8N1).
    OpenUART1(UART_EN, 
              UART_TX_ENABLE | UART_RX_ENABLE,
              ((STICK_SYS_FREQ / STICK_DESIRED_BAUD) / 16) - 1);
}

void pwm_config(void) {
    // PWM on OC4. Default is 16-bit timer, Timer2.
    // Timer2 period = 65535 (the maximum),
    // so PWM period = 65535 + 1.
    // Duty cycle = 50% (0x8000 is half of (0xffff + 1))
    OpenTimer2(T2_ON, 0xffff);
    OpenOC4(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0x8000, 0x8000);
}

int main(void) {
    __builtin_disable_interrupts();

    PPSUnLock
    PPSInput(3, U1RX, RPA2);    // pin 9
    PPSOutput(1 , RPB3, U1TX);  // pin 7
    PPSOutput(3, RPB2, OC4);    // pin 6
    PPSLock

    uart_config();
    pwm_config();

    // A0 is LED, so make it an output.
    mPORTASetPinsDigitalOut(BIT_0);
    // Turn LED off.
    mPORTAClearBits(BIT_0);

    sprintf(msg, "U1BRG = %d\n", U1BRG);
    Stick_WriteUART1(msg);
    unsigned int duty_cycle;
    while(1) {
        Stick_ReadUART1(msg, BUFLEN - 1); // Block until we receive a string
        duty_cycle = atoi(msg); // 0..100 percent
        // Update duty cycle
        SetDCOC4PWM(duty_cycle * 0x10000 / 100);
        // Toggle LED
        mPORTAToggleBits(BIT_0);
        Stick_WriteUART1(msg); // Echo the string we received
    }
    return 0;
}
