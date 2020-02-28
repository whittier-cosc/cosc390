/**
 *  @file   main_dma-vref.c
 *
 *  @brief  Use DMA to transfer 44000 bytes per second to 4-bit DAC
 *          (a.k.a. CVREF) with no CPU intervention.
 *
 *          CVREFOUT -> B14 (pin 25)
 *
 *  @author Jeff Lutgen
 */

#include <stdint.h>
#include <math.h>
#include "config.h"
#include "uart.h"
#include "util.h"

#define CVREF_CONFIG (CVREF_ENABLE | CVREF_OUTPUT_ENABLE | CVREF_RANGE_HIGH | CVREF_SOURCE_AVDD)

uint8_t samples[100]; // 100 samples per cycle gives 440 Hz output when sampled at 44000 Hz
int sample_index = 0;
char msg[80];

// round a positive double to nearest int
int iround(double x) {
    double p;
    double frac = modf(x, &p);
    if (frac < 0.5)
        return (int) p;
    return (int) p + 1;
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // Tell wcpic32lib our system clock and peripheral bus clock rates
    wclib_init(SYSCLK, PBCLK);

    uart_init();

    // sin table, 1 cycle, 100 samples, scaled to range 0..15
    int i;
    for (i = 0; i < 100; ++i) {
        samples[i] = (CVREF_CONFIG & 0xff) | (uint32_t) iround(7.5*sin(2.0*M_PI*i/100) + 7.5);
    }

    CVREFOpen(CVREF_CONFIG);

    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, PBCLK/44000);

    // There are 4 DMA channels on this PIC32, numbered 0 through 3. We use channel 0.
    DmaChnOpen(0, DMA_CHN_PRI2, DMA_OPEN_AUTO);
    DmaChnSetEventControl(0, DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(_TIMER_1_IRQ));
    DmaChnSetTxfer(0, samples, (void *) &CVRCON, 100, 1, 1);
    DmaChnEnable(0);

    uart_write("begin\n");
    while (1) {
//        sprintf(msg, "0x%04x\n", CVRCON);
//        uart_write(msg);
//        delay(1000);
    }
    return 0;
}
