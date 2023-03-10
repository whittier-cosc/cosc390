/*
 *  @file   uart.c
 *
 *  @brief  A PIC32 library for serial communication using the UART1 module.
 *
 *          This code is intended for use with the PIC32MX250F128B.
 *
 *          Portions based on the Northwestern University NU32 package
 *          by Nick Marchuk, et al.
 *
 *  @author Jeff Lutgen
 */

#include <xc.h>
#include <stdarg.h>
#include "private/common.h"
#include "uart.h"

#define BAUDRATE 115200

/**
 *  Initializes the UART1 module for 9600 baud serial communication
 *  (rate can be changed by redefining BAUDRATE in this file),
 *  with 8 data bits, no parity bit, and one stop bit ("8N1").
 *
 *  Pins used:
 *
 *      RPA2 (pin 9) --> U1RX
 *      RPB3 (pin 7) --> U1TX
 */
void uart_init() {
    // U1RXR = 0; // Map RPA2 (pin 9) to U1RX
    // RPB3R = 1; // Map RPB3 (pin 7) to U1TX
    PPSInput(3, U1RX, RPA2);  // Map RPA2 (pin 9) to U1RX
    PPSOutput(1, RPB3, U1TX); // Map RPB3 (pin 7) to U1TX

    // Set baud to BAUDRATE
    U1MODEbits.BRGH = 0;  // High-speed mode disabled
    // With PBCLK = SYSCLK = 40 M, we have U1BRG = 259, giving
    // baud rate = 9615.4 (see DS61107F, Table 21-2).
    U1BRG = (_pbclk  / BAUDRATE) / 16 - 1;
    // 8 bit, no parity bit, 1 stop bit (8N1)
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;

    // Enable TX & RX, taking over U1RX/TX pins
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    // Do not enable RTS or CTS
    U1MODEbits.UEN = 0;

    // Enable the UART
    U1MODEbits.ON = 1;
}

/**
 *  Reads a string from UART1. Blocks until an `\r` or `\n` is seen.
 *
 *  The received string (not including the `\r` or `\n') is stored in `message`,
 *  which should have at least `maxLength` elements. If there are more than `maxLength`
 *  elements in the received string, wraparound occurs.
 *
 *  Example:
 *
 *      char msg[80];
 *      uart_read(msg, 80);
 */
void uart_read(char *message, int maxLength) {
    char data = 0;
    int complete = 0, num_bytes = 0;
    // loop until you get a '\r' or '\n'
    while (!complete) {
        if (U1STAbits.URXDA) { // if data is available
            data = U1RXREG;      // read the data
            if ((data == '\r') || (data == '\n')) {
                complete = 1;
            } else {
                message[num_bytes] = data;
                ++num_bytes;
                // overwrite from beginning of array if we would overflow
                if (num_bytes >= maxLength) {
                    num_bytes = 0;
                }
            }
        }
    }
    // end the string
    message[num_bytes] = '\0';
}

/**
 *  Writes a null-terminated string to UART1.
 */
void uart_write(const char *string) {
    while (*string != '\0') {
        while (U1STAbits.UTXBF) {
            ; // wait until TX buffer isn't full
        }
        U1TXREG = *string;
        ++string;
    }
}

/*
 *  This is a replacement for the standard library version of _mon_putc().
 *
 *  The standard version writes to UART2, but this one writes to UART1, which
 *  means that printf() will write to UART1.
 */
void _mon_putc(char c) {
        while (U1STAbits.UTXBF) {
            ; // wait until TX buffer isn't full
        }
        U1TXREG = c;
}
