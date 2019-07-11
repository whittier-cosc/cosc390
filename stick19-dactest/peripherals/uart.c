/*!
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
#include "../hwprofile.h"
#include "uart.h"

/*!
 * \brief Initializes the UART1 module for 9600 baud serial communication
 *        (but rate can be changed by redifining BAUDRATE in uart.h),
 *        with 8 data bits, no parity bit, one stop bit ("8N1"),
 *        mapping pins as follows:
 *              RPA2 (pin 9) --> U1RX
 *              RPB3 (pin 7) --> U1TX
 */
void uart_init() {
    CFGCONbits.IOLOCK = 0;
    U1RXR = 0; // Map RPA2 (pin 9) to U1RX
    RPB3R = 1; // Map RPB3 (pin 7) to U1TX
    CFGCONbits.IOLOCK = 1;

    // Set baud to BAUDRATE
    U1MODEbits.BRGH = 0;  // High-speed mode disabled
    // With PBCLK = SYSCLK = 40 M, we have U1BRG = 259, giving
    // baud rate = 9615.4 (see DS61107F, Table 21-2).
    U1BRG = (PBCLK  / BAUDRATE) / 16 - 1;
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

/*!
 * \brief Reads a string from UART1. Blocks until an '\r' or '\n' is seen.
 * \param message
 *              char array in which to store the received string
 * \param maxLength
 *              number of elements in the array
 */
void uart_read(char * message, int maxLength) {
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

/*!
 * \brief Write a null-terminated string to UART1
 * \param string
 *              char array containing the string
 */
void uart_write(const char * string) {
    while (*string != '\0') {
        while (U1STAbits.UTXBF) {
            ; // wait until TX buffer isn't full
        }
        U1TXREG = *string;
        ++string;
    }
}
