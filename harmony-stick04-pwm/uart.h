#ifndef UART_H
#define UART_H

#define STICK_DESIRED_BAUD 9600

void uart_read(char * string, int maxLength);
void uart_write(const char * string);

#endif
