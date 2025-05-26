#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f303xc.h"

// Buffer sizes
#define TX_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 128
#define RX_QUEUE_SIZE 8

// Baudrate definitions
#define BAUD_9600   9600
#define BAUD_19200  19200
#define BAUD_38400  38400
#define BAUD_57600  57600
#define BAUD_115200 115200

// RX Message structure
typedef struct {
    char message[RX_BUFFER_SIZE];
    uint32_t length;
} RXMessage;

// SerialPort structure
typedef struct {
    USART_TypeDef *UART;
    GPIO_TypeDef *GPIO;
    uint32_t MaskAPB2ENR;
    uint32_t MaskAPB1ENR;
    uint32_t MaskAHBENR;
    uint32_t SerialPinModeValue;
    uint32_t SerialPinSpeedValue;
    uint32_t SerialPinAlternatePinValueLow;
    uint32_t SerialPinAlternatePinValueHigh;
    void (*tx_completion_function)(uint32_t);
    void (*rx_completion_function)(char *, uint32_t);
} SerialPort;

// External variable declaration
extern SerialPort USART1_PORT;
extern volatile bool processingMessage;
// Function prototypes
void SerialInitialise(uint32_t baudRate, SerialPort *serial_port,
                     void (*tx_completion_function)(uint32_t),
                     void (*rx_completion_function)(char *, uint32_t));

void SerialOutputString(uint8_t *pt, SerialPort *serial_port);
void Serial_WaitUntilTXComplete(void);
void SerialOutputBuffer(uint8_t *data, uint32_t length, SerialPort *serial_port);
// RX Queue functions
bool Serial_DequeueMessage(RXMessage *msg);
bool Serial_GetNextMessage(char *buffer, size_t size);
bool Serial_IsQueueEmpty(void);

#endif /* SERIAL_H */
