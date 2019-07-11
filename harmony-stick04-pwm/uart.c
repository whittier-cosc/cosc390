#include <xc.h>
#include <peripheral/peripheral.h>
#include "uart.h"

// Read from UART1
// block other functions until you get a '\r' or '\n'
// send the pointer to your char array and the number of elements in the array
void uart_read(char * message, int maxLength) {
    char data = 0;
    int complete = 0, num_bytes = 0;
    // loop until you get a '\r' or '\n'
    while (!complete) {
        if (PLIB_USART_ReceiverDataIsAvailable(USART_ID_1)) {
            data = PLIB_USART_ReceiverByteReceive(USART_ID_1);
            if ((data == '\n') || (data == '\r')) {
                complete = 1;
            } else {
                message[num_bytes] = data;
                ++num_bytes;
                // roll over if the array is too small
                if (num_bytes >= maxLength) {
                    num_bytes = 0;
                }
            }
        }
    }
    // end the string
    message[num_bytes] = '\0';
}

// Write a character array using UART1
void uart_write(const char * string) {
    while (*string != '\0') {
        while (PLIB_USART_TransmitterBufferIsFull(USART_ID_1)) {
            ; // wait until tx buffer isn't full
        }
        PLIB_USART_TransmitterByteSend(USART_ID_1, *string);
        ++string;
    }
}
