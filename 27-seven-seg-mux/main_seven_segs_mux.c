/**
 *  @file   main_seven_segs.c
 *
 *  @brief  multiplex a 4-digit, 7-segment LED display (5461A),
 *          and use a 4051B 8-channel multiplexer to drive segments
 *
 *  @author Jeff Lutgen
 */

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"

// PORT B (to shift register)
#define STROBE  BIT_7
#define DATA    BIT_8
#define CLK     BIT_9

// PORT A (digit select)
#define DIG1 BIT_1  // pin 12
#define DIG2 BIT_2  // pin 9
#define DIG3 BIT_3  // pin 8
#define DIG4 BIT_4  // pin 6

// precise delays for ADC acquisition, wait time between clock transitions, etc.
#define NOP asm("nop");
// 200 ns
#define NS_200 {NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;}
#define US     {NS_200;NS_200;NS_200;NS_200;NS_200;}
#define US_2   {US;US;}
#define US_5   {US;US;US;US;US;}
#define US_8   {US;US;US;US;US;US;US;US;}
#define US_25  {US_5;US_5;US_5;US_5;US_5;}
#define WAIT   US_2  // 2 microseconds is enough when 2.2K pull-up resistors are used on CLK,DATA,STROBE lines

#define SAMPLING_PERIOD 15625 // 200 ms (if timer prescaler is 256)
#define NSAMP 16

char msg[80];
volatile int ct_per = 100000;

// Seven-segment display font
// In shift register language, parallel output pins are Q1-Q8.
// Segment <-> pin correspondence used:
// e    Q1
// d    Q2
// .    Q3
// c    Q4
// g    Q5
// b    Q6
// f    Q7
// a    Q8
uint8_t symbols[10] = {
    0b11010111,     // 0
    0b00010100,     // 1
    0b11001101,     // 2
    0b01011101,     // 3
    0b00011110,     // 4
    0b01011011,     // 5
    0b11011011,     // 6
    0b00010101,     // 7
    0b11011111,     // 8
    0b01011111      // 9
};

volatile char digits[4];

void adc_init() {
    // 1. Configure the analog port pins
    // We'll use AN0 (pin 2), which is configured as an analog
    // input by default.

    // 2. Select the analog inputs to the ADC multiplexers.
    // Nothing to do here, since AN0 is the default input.

    // 3. Select the format of the ADC result using FORM<2:0>
    // We use the default 16-bit integer result format.

    // 4. Select the sample clock source using SSRC<2:0> (AD1CON1<7:5>)
    //AD1CON1bits.SSRC = 2; // Use Timer3 period match as conversion trigger
    AD1CON1bits.SSRC = 0; // Clearing SAMP bit ends sampling, starts conversion

    // 5. Select the voltage reference source using VCFG<2:0>
    // We use the default: low end = VSS, high end = VDD (0 to 3.3 V)

    // 6. Select the Scan mode using CSCNA (AD1CON2<10>)
    // We don't want to scan through multiple inputs, so leave as default (0).

    // 7. Set the number of conversions per interrupt SMP<3:0> (AD1CON2<5:2>),
    // if interrupts are to be used.
    // We're not using interrupts, so do nothing.

    // 8. Set Buffer Fill mode using BUFM (AD1CON2<1>)
    // We'll try the default (one 16-word results buffer)

    // 9. Select the MUX to be connected to the ADC in ALTS AD1CON2<0>
    // Use default (do not alternate between the two mulitplexers).

    // 10. Select the ADC clock source using ADRC (AD1CON3<15>)
    // Use default (peripheral bus clock).

    // 11. Select the sample time using SAMC<4:0> (AD1CON3<12:8>),
    // if auto-convert is to be used
    // We're not using auto-convert.

    // 12. Select the ADC clock prescaler using ADCS<7:0> (AD1CON3<7:0>)
    AD1CON3bits.ADCS = 0x01; // Tad = 4 * Tpb

    // 13. Turn the ADC module on using AD1CON1<15>
    AD1CON1bits.ON = 1;
}

// Write data a bit at a time to shift register
void serial_write(uint8_t data) {
    int i;
    for (i = 0; i < 8; i++) {
        if (data & 0x01) {
            mPORTBSetBits(DATA);
        } else {
            mPORTBClearBits(DATA);
        }
        data >>= 1;
        WAIT;
        mPORTBSetBits(CLK);
        WAIT;
        mPORTBClearBits(CLK);
    }
    WAIT;
    mPORTBSetBits(STROBE);
    WAIT;
    mPORTBClearBits(STROBE);
}

void __ISR(_CORE_TIMER_VECTOR, IPL6SOFT) CoreTimerISR(void) {
    static int digit = 1;

    switch (digit) {
    case 1:
        serial_write(symbols[(short) digits[3]]);
        mPORTAClearBits(DIG1);               // digit 1 on (cathode to ground)
        mPORTASetBits(DIG2 | DIG3 | DIG4);   // other digits off
        digit++;
        break;
    case 2:
        serial_write(symbols[(short) digits[2]]);
        mPORTAClearBits(DIG2);
        mPORTASetBits(DIG1 | DIG3 | DIG4);
        digit++;
        break;
    case 3:
        serial_write(symbols[(short) digits[1]]);
        mPORTAClearBits(DIG3);
        mPORTASetBits(DIG1 | DIG2 | DIG4);
        digit++;
        break;
    case 4:
        serial_write(symbols[(short) digits[0]]);
        mPORTAClearBits(DIG4);
        mPORTASetBits(DIG1 | DIG2 | DIG3);
        digit = 1;
        break;
    }

    _CP0_SET_COUNT(0);         // set core timer counter to 0
    _CP0_SET_COMPARE(ct_per);   // must set CP0_COMPARE again after interrupt
    IFS0bits.CTIF = 0;         // clear CT int flag IFS0<0>, same as IFS0CLR=0x0001
}


void __ISR(_TIMER_2_VECTOR, IPL2SOFT) ADC_run(void) {
    uint32_t reading;
    int ms;
    static uint32_t curr_ms = 9999;
    int i;
    //average NSAMP samples to smooth things out
    reading = 0;
    for (i = 0; i < NSAMP; i++) {
        AD1CON1bits.SAMP = 1; // start sampling
        NS_200; // wait the minimum 200 ns acquisition time
        AD1CON1bits.SAMP = 0; // end sampling, start conversion
        while (!AD1CON1bits.DONE) {
        }
        reading += ADC1BUF0;
    }
    reading = reading / NSAMP;
    ms = reading / 6 - 15;
    if (ms < 1) {
        ms = 1;
    }
    ct_per = 20000 * ms;
    if (ms != curr_ms) {
        curr_ms = ms;
        sprintf(msg, "%4d", reading);
        tft_printLine(1, 5, msg);
        sprintf(msg, "%4d ms", ms);
        tft_printLine(5, 5, msg);
    }
    mT2ClearIntFlag();
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // Tell wcpic32lib our system clock and peripheral bus clock rates
    wclib_init(SYSCLK, PBCLK);

    INTCONbits.MVEC = 1;   // multi-vector mode ons
    __builtin_disable_interrupts();

    int i;

    adc_init();

    tft_init();
    tft_fillScreen(ILI9340_BLUE);
    tft_setRotation(0);
    //tft_printLine(2, 5, "ready");

    mPORTASetPinsDigitalOut(DIG1 | DIG2 | DIG3 | DIG4);
    mPORTAOpenDrainOpen(DIG1 | DIG2 | DIG3 | DIG4);
    mPORTBSetPinsDigitalOut(BIT_7 | BIT_8 | BIT_9);
    mPORTBOpenDrainOpen(CLK | STROBE | DATA);
    mPORTASetBits(DIG1 | DIG2 | DIG3 | DIG4);   // all digits off (cathode open)
    mPORTBClearBits(CLK | STROBE | DATA);

    _CP0_SET_COMPARE(ct_per);        // CP0_COMPARE
    IPC0bits.CTIP = 6;              // interrupt priority
    IPC0bits.CTIS = 0;              // subpriority 0 (default)
    IFS0bits.CTIF = 0;              // clear CT interrupt flag
    IEC0bits.CTIE = 1;              // enable core timer interrupt
    _CP0_SET_COUNT(0);              // set core timer counter to 0

    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, SAMPLING_PERIOD);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
    mT2ClearIntFlag();

    __builtin_enable_interrupts();

    bool carry = false;
    while(1) {
        __builtin_disable_interrupts();
        digits[3] = digits[2] = digits[1] = digits[0] = 0;
        __builtin_enable_interrupts();
        for (i = 0; i <= 9999; i++) {
            __builtin_disable_interrupts();
            if (digits[0] == 9) {
                digits[0] = 0;
                carry = true;
            } else {
                digits[0]++;
            }
            if (carry) {
                if (digits[1] == 9) {
                    digits[1] = 0;
                } else {
                    digits[1]++;
                    carry = false;
                }
            }
            if (carry) {
                if (digits[2] == 9) {
                    digits[2] = 0;
                } else {
                    digits[2]++;
                    carry = false;
                }
            }
            if (carry) {
                if (carry && digits[3] == 9) {
                    digits[3] = 0;
                } else {
                    digits[3]++;
                    carry = false;
                }
            }
            __builtin_enable_interrupts();
            delay(30);
        }
    }
    return 0;
}

