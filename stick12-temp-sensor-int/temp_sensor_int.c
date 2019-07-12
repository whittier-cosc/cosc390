/*
 * Use the ADC to read a temperature sensor (Analog Devices TMP36).
 * 750 mV output from TMP36 -> 25 C
 * 1 V    output            -> 50 C
 * So 10 mV change -> 1 C change
 *
 * ADC conversion trigger is Timer3, set to a period of 65536 * 256 * TPB,
 * or 0.42 seconds (since PBCLK = 40 MHz). ADC set to issue interrupt after
 * every 8 conversions, so every 3.4 seconds. The ISR averages the 8 readings,
 * and then averages that with the previous average (this has the effect of
 * smoothing out glitches due to glitches in the supply voltage to the temp-
 * erature sensor.
 */

#include "config.h"
#include "tft.h"

char msg[80];
int prev_reading;

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 319, 10*char_size, 1, ILI9340_BLACK);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_BLUE); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

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
    AD1CON1bits.SSRC = 2; // Use Timer3 period match as conversion trigger
    AD1CON1bits.ASAM = 1; // Automatically start sampling again when 
                          // conversion finished

    // 5. Select the voltage reference source using VCFG<2:0>
    // We use the default: low end = VSS, high end = VDD (0 to 3.3 V)

    // 6. Select the Scan mode using CSCNA (AD1CON2<10>)
    // We don't want to scan through multiple inputs, so leave as default (0).
    
    // 7. Set the number of conversions per interrupt SMPI<3:0> (AD1CON2<5:2>),
    // if interrupts are to be used.
    AD1CON2bits.SMPI = 7; // 8 conversions per interrupt

    // 8. Set Buffer Fill mode using BUFM (AD1CON2<1>)
    // We'll use the default (one 16-word results buffer)
    
    // 9. Select the MUX to be connected to the ADC in ALTS AD1CON2<0> 
    // Use default (do not alternate between the two mulitplexers).

    // 10. Select the ADC clock source using ADRC (AD1CON3<15>)
    // Use default (peripheral bus clock).

    // 11. Select the sample time using SAMC<4:0> (AD1CON3<12:8>), 
    // if auto-convert is to be used 
    // We're not using auto-convert. Datasheet says these bits are
    // used only if SSRC = 7 (internal trigger), so should be OK to
    // not set them.

    // 12. Select the ADC clock prescaler using ADCS<7:0> (AD1CON3<7:0>)
    // We'll be doing only a few conversions per second, so we can use
    // the largest possible prescaler (512).
    AD1CON3bits.ADCS = 0xff; // T_AD = 512 * T_PB = 12.8 us

    // 13. Turn the ADC module on using AD1CON1<15>
    AD1CON1bits.ON = 1;

    // 14. Configure interrupt
    IFS0bits.AD1IF = 0;
    IPC5bits.AD1IP = 2;
    IEC0bits.AD1IE = 1;
}

void __ISR(_ADC_VECTOR, IPL2SOFT) adc_isr(void) {
    int adc_reading;
    float tempC, tempF;
    // for debugging
    LATAINV = 0x0002; // toggle A1

    adc_reading = ADC1BUF0;
    adc_reading += ADC1BUF1;
    adc_reading += ADC1BUF2;
    adc_reading += ADC1BUF3;
    adc_reading += ADC1BUF4;
    adc_reading += ADC1BUF5;
    adc_reading += ADC1BUF6;
    adc_reading += ADC1BUF7;
    adc_reading /= 8; // average 

    // smooth things out a bit
    adc_reading = (adc_reading + prev_reading) / 2;
    prev_reading = adc_reading;

    // Interrupt frequency is very low, so just do everything in this ISR
    tempC =  -50 + (adc_reading * 325) / (float) 1023; // Vdd = 3.25 V
    tempF =  (9 * tempC) / 5 + 32;
    sprintf(msg, "%3.1f C", tempC);
    printLine(4, 4, msg);
    sprintf(msg, "%3.1f F", tempF);
    printLine(10, 4, msg);

    IFS0bits.AD1IF = 0; // Persistent interrupt, so clearing flag has no effect
                        // unless ADC1BUF0 has been read!
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // For debugging
    TRISA = 0x1D; // set A1 as output

    INTCONbits.MVEC = 1; // multi-vector interrupt mode

    tft_init();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    timer3_init();
    adc_init();
    __builtin_enable_interrupts();

    while(1) {
        ;
    }
    return 0;
}

