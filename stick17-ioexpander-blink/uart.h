#ifndef UART_H
#define UART_H

#define BAUDRATE 9600

void Stick_ReadUART1(char * string, int maxLength);
void Stick_WriteUART1(const char * string);

#endif
