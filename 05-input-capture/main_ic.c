// A basic frequency counter using input capture (IC).
//
// We configure PWM so you can hook up the PWM output to the IC input for
// testing.
//
// If the input frequency is less than about 600 Hz (the frequency at which the
// timer rolls over (assuming 40 MHz PBCLK), then of course the frequency
// calculation is nonsense.
//
// Also, if the input frequency is too large (greater than around 800 KHz, our
// delay-loop method slows things down to the point where the program is
// unusable, because interrupts are occurring too frequently.
//
// Finally, the frequency calculation assumes PBCLK is accurate. If it's not,
// then some calibration is needed.

#include "config.h"
#include "util.h"
#include "uart.h"

char msg[80];
static volatile int ticks;
static volatile int interrupts;

void pwm_config(void) {
    // PWM on OC4. 
    // PWM period = PR3 + 1.
    OC4CONbits.OCTSEL = 1; // Timer3
    PR3 = 39; // period 40 cycles, so freq = PBCLK/40 = 40 MHz/40 = 1 MHz
    OC4RS = (PR3  + 1)/ 2; // Duty cycle = 50% 
    OC4R = OC4RS;          // We have to set this once, before firing up PWM.
    // Subsequent changes to duty cycle are made by writing
    // to OC4RS.
    OC4CONbits.OCM = 6; // 3 bits, 110 = PWM mode, fault pin disabled
    T3CONbits.ON = 1;   // Turn on Timer2
    OC4CONbits.ON = 1;  // Turn on PWM.
}

void __ISR(_INPUT_CAPTURE_1_VECTOR, IPL3SOFT) InputCompareISR(void) {
    int t0 = IC1BUF;
    int t1 = IC1BUF;
    ticks = t1 - t0; 
    if (ticks < 0) // Account for overflow
        ticks += 65536;
    interrupts++;
    IFS0bits.IC1IF = 0; // clear IC1 int flag
}

void ic_config(void) {
    // Configure input capture IC1
    IC1CONbits.ICM = 3;   // every rising edge;
    IC1CONbits.ICTMR = 1; // Timer2
    IC1CONbits.ICI = 1;   // interrupt every 2nd capture
    IEC0bits.IC1IE = 1;   // enable IC1 interrupt
    IPC1bits.IC1IP = 3;   // interrupt priority 3
    IFS0bits.IC1IF = 0;   // clear interrupt flag 
    T2CONbits.ON = 1;     // Turn on Timer2
    IC1CONbits.ON = 1;    // Turn on InputCapture1
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);    osc_tune(56);

    __builtin_disable_interrupts();

    TRISA = 0xFFFE;        // A0 is LED, so make it an output.
    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // peripheral pin select
    U1RXR = 0; // map RPA2 (pin 9) to U1RX
    RPB3R = 1; // map RPB3 (pin 7) to U1TX
    RPB2R = 5; // map RPB2 (pin 6) to OC4 (PWM)
    IC1R = 2;  // map RPA4 (pin 12) to IC1

    uart_init();
    pwm_config();
    ic_config();

    LATAbits.LATA0 = 0;    // Turn LED off.

    __builtin_enable_interrupts();

    uart_write("RESET\r\n");
    int myticks, myinterrupts;
    while(1) {
        uart_read(msg, 39); // block until user hits ENTER
        uart_write("running\r\n");
        delay(200); // allow some time to measure input
        __builtin_disable_interrupts(); // while we copy
        myticks = ticks;
        myinterrupts = interrupts;
        __builtin_enable_interrupts();
        sprintf(msg, "ticks: %d\r\n", myticks);
        uart_write(msg);
        if (myticks == 0)
            uart_write("freq: infinity\r\n");
        else {
            sprintf(msg, "freq: %d Hz\r\n", PBCLK / myticks);
            uart_write(msg);
        }
        sprintf(msg, "interrupts: %d\r\n", myinterrupts);
        uart_write(msg);
        LATAINV = 0x0001;    // toggle LED
    }
    return 0;
}

