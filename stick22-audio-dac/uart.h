#ifndef UART_H
#define UART_H

#define BAUDRATE 9600

void uart_read(char * string, int maxLength);
void uart_write(const char * string);

#endif
