#ifndef UART_H
#define UART_H

/**
 *  @file   uart.h
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

void uart_read(char *message, int maxLength);
void uart_write(const char *string);
void uart_printf(const char *fmt, ...);

#endif
