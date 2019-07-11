#include "config.h"
#include "uart.h"

#define BUFLEN 40
char msg[BUFLEN];

void uart_config(void) {
    // Set baud to STICK_DESIRED_BAUD
    PLIB_USART_BaudRateHighDisable(USART_ID_1);
    // With PBCLK = CORE_TICKS = 40 M, we have U1BRG = 259, giving 
    // baud rate = 9615.4 (see DS61107F, Table 21-2).
    PLIB_USART_BaudRateSet(USART_ID_1, STICK_SYS_FREQ, STICK_DESIRED_BAUD);
    // 8 bit, no parity bit, 1 stop bit (8N1)
    PLIB_USART_LineControlModeSelect(USART_ID_1, USART_8N1);

    // Enable TX & RX, taking over U1RX/TX pins
    // Do not enable RTS or CTS
    PLIB_USART_OperationModeSelect(USART_ID_1, USART_ENABLE_TX_RX_USED);
    PLIB_USART_ReceiverEnable(USART_ID_1);
    PLIB_USART_TransmitterEnable(USART_ID_1);

    // Enable the UART
    PLIB_USART_Enable(USART_ID_1);
}

void pwm_config(void) {
    // PWM on OC4. 

    // Use 16-bit timer, Timer2.
    PLIB_TMR_Mode16BitEnable(TMR_ID_2);

    // Timer2 period = 65535 (the maximum),
    // so PWM period = 65535 + 1.
    PLIB_TMR_Period16BitSet(TMR_ID_2, 0xffff); 
    // Duty cycle = 50% (0x8000 is half of (0xffff + 1))
    PLIB_OC_PulseWidth16BitSet(OC_ID_4, 0x8000);
    // Turn on Timer2
    PLIB_TMR_Start(TMR_ID_2);
    PLIB_OC_TimerSelect(OC_ID_4, OC_TIMER_16BIT_TMR2);
    PLIB_OC_ModeSelect(OC_ID_4, OC_COMPARE_PWM_MODE_WITHOUT_FAULT_PROTECTION);

    PLIB_OC_Enable(OC_ID_4);
}

int main(void) {
    __builtin_disable_interrupts();
    PLIB_DEVCON_SystemUnlock(DEVCON_ID_0);
    PLIB_DEVCON_DeviceRegistersUnlock(DEVCON_ID_0, DEVCON_PPS_REGISTERS);
    PLIB_PORTS_RemapInput(PORTS_ID_0, INPUT_FUNC_U1RX, INPUT_PIN_RPA2); // pin 9
    PLIB_PORTS_RemapOutput(PORTS_ID_0, OUTPUT_FUNC_U1TX, OUTPUT_PIN_RPB3); // pin 7
    PLIB_PORTS_RemapOutput(PORTS_ID_0, OUTPUT_FUNC_OC4, OUTPUT_PIN_RPB2); // pin 6
    PLIB_DEVCON_DeviceRegistersLock(DEVCON_ID_0, DEVCON_PPS_REGISTERS);

    uart_config();
    pwm_config();

    // A0 is LED, so make it an output.
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0);

    // Turn LED off.
    PLIB_PORTS_PinClear(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0);

    sprintf(msg, "U1BRG = %d\n", U1BRG);
    uart_write(msg);
    unsigned int duty_cycle;
    while(1) {
        uart_read(msg, BUFLEN - 1); // Block until we receive a string
        duty_cycle = atoi(msg); // 0..100 percent
        // Update duty cycle
        PLIB_OC_PulseWidth16BitSet(OC_ID_4, duty_cycle * 0x10000 / 100);
        // Toggle LED
        PLIB_PORTS_PinToggle(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0);
        uart_write(msg); // Echo the string we received
    }
    return 0;
}
