#include <xc.h>

#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h> 

// Read from UART1
// block other functions until you get a '\r' or '\n'
// send the pointer to your char array and the number of elements in the array
void uart_read(char * message, int maxLength) {
    char data = 0;
    int complete = 0, num_bytes = 0;
    // loop until you get a '\r' or '\n'
    while (!complete) {
        if (DataRdyUART1()) { // if data is available
            data = ReadUART1();      // read the data
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
        while (BusyUART1()) {
            ; // wait until tx buffer isn't full
        }
        WriteUART1(*string);
        ++string;
    }
}
