/*
 * Use the ADC to read a temperature sensor (Analog Devices TMP36).
 * 750 mV output from TMP36 -> 25 C
 * 1 V    output            -> 50 C
 * So 10 mV change -> 1 C change
 *
 * TMP36 operates on supply voltage between 2.7 and 5.5 V.
 *
 * A big smoothing capacitor on the output of the TMP36 helps.
 * On the breadboard, there's some interference generated by
 * the writes to the TFT display.
 */

#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"

char msg[80];

void timer3_init() {
    T3CONbits.TCKPS = 7; // prescaler = 256
    PR3 = 0xffff; // default
    T3CONbits.ON = 1;
}

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
    // We'll be doing only a few conversions per second, so we can use
    // the largest possible prescaler (512).
    AD1CON3bits.ADCS = 0xff; // Tad = 512 * Tpb

    // 13. Turn the ADC module on using AD1CON1<15>
    AD1CON1bits.ON = 1;
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    timer3_init();
    adc_init();

    float tempF;
    float tempC;
    while(1) {
        AD1CON1bits.SAMP = 1; // start sampling 
        delay(250);
        AD1CON1bits.SAMP = 0; // end sampling, start conversion
        delay(250);
        tempC =  -50 + (ADC1BUF0 * 325) / (float) 1023; // Vdd = 3.25 V
        tempF =  (9 * tempC) / 5 + 32;
        sprintf(msg, "%3.1f C", (double) tempC);
        tft_printLine(4, 4, msg);
        sprintf(msg, "%3.1f F", (double) tempF);
        tft_printLine(10, 4, msg);
    }
    return 0;
}