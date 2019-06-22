#ifndef UART_H
#define UART_H


#define STICK_DESIRED_BAUD 9600

void Stick_ReadUART1(char * string, int maxLength);
void Stick_WriteUART1(const char * string);

#endif
