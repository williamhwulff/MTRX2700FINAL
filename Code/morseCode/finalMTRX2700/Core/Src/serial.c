#include "main.h"

// TX Side Ring Buffer
static volatile uint32_t txIndex = 0;    // Next position to send
static volatile uint32_t txLength = 0;   // Total number of characters queued
static char txBuffer[TX_BUFFER_SIZE];
static volatile bool txInProgress = false;
SerialPort USART1_PORT = {
    USART1,
    GPIOC,
    RCC_APB2ENR_USART1EN,   // USART1 clock on APB2
    0x00,                   // No APB1 enable needed
    RCC_AHBENR_GPIOCEN,     // GPIOC clock on AHB
    0xA00,                  // Pin mode value (set alternate function for TX/RX)
    0xF00,                  // Pin speed value
    0x770000,               // AFR[0] for PC10 and PC11 (AF7 for USART1)
    0x00,                   // No change for AFR[1]
    0x00,                   // tx_completion_function initially NULL
    0x00                    // rx_completion_function initially NULL
};

/**
 Sends a string over the specified serial port
 pt Pointer to the string to send
 serial_port Pointer to the SerialPort structure
 */
void SerialOutputString(uint8_t *pt, SerialPort *serial_port) {
    uint32_t counter = 0;

    txIndex = 0;
    counter = 0;

    // Copy string to TX buffer
    while (*pt && counter < (TX_BUFFER_SIZE - 1)) {
        txBuffer[counter++] = *pt++;
    }
    txBuffer[counter] = '\0';
    txLength = counter;
    txIndex = 0;

    // Enable TX interrupt
    serial_port->UART->CR1 |= USART_CR1_TXEIE;

    // Call completion function if registered
    if (serial_port->tx_completion_function) {
        serial_port->tx_completion_function();
    }
}

void SerialOutputBuffer(uint8_t *data, uint32_t length, SerialPort *serial_port) {
    // reset indices
    txIndex  = 0;
    txLength = length > TX_BUFFER_SIZE ? TX_BUFFER_SIZE : length;
    txInProgress = true;

    // copy exactly `length` bytes into the TX buffer
    memcpy(txBuffer, data, txLength);

    // kick off the interrupt‐driven send
    serial_port->UART->CR1 |= USART_CR1_TXEIE;
}

// RX Side Queue Implementation
// Temporary buffers for double buffering
static char rxBuffer1[RX_BUFFER_SIZE];
static char rxBuffer2[RX_BUFFER_SIZE];
static volatile char *activeRxBuffer = rxBuffer1;
static volatile uint32_t rxIndex = 0;

// Internal RX message queue
static RXMessage rxQueue[RX_QUEUE_SIZE];
static volatile uint8_t rxQueueHead = 0;
static volatile uint8_t rxQueueTail = 0;


// Checks if the RX queue is full
// return true if the queue is full, false otherwise
static bool isRxQueueFull(void) {
    return ((rxQueueTail + 1) % RX_QUEUE_SIZE) == rxQueueHead;
}


// Checks if the RX queue is empty
// return true if the queue is empty, false otherwise
static bool isRxQueueEmpty(void) {
    return rxQueueHead == rxQueueTail;
}


// API to check if the RX queue is empty
// return true if the queue is empty, false otherwise
bool Serial_IsQueueEmpty(void) {
    return isRxQueueEmpty();
}


//Enqueues a complete message
// msg Pointer to the message to enqueue
// length Length of the message
// return true if the message was enqueued, false if queue is full

static bool EnqueueRxMessage(const char *msg, uint32_t length) {
    if (isRxQueueFull()) {
        return false;
    }

    uint8_t tail = rxQueueTail;
    uint32_t copyLength = (length < (sizeof(rxQueue[tail].message) - 1)) ?
                          length : (sizeof(rxQueue[tail].message) - 1);

    memcpy(rxQueue[tail].message, msg, copyLength);
    rxQueue[tail].message[copyLength] = '\0';
    rxQueue[tail].length = copyLength;
    rxQueueTail = (rxQueueTail + 1) % RX_QUEUE_SIZE;

    return true;
}


// Dequeues a message from the RX queue
// msg Pointer to store the dequeued message
// return true if a message was dequeued, false if queue is empty
bool Serial_DequeueMessage(RXMessage *msg) {
    if (isRxQueueEmpty()) {
        return false;
    }

    *msg = rxQueue[rxQueueHead];
    rxQueueHead = (rxQueueHead + 1) % RX_QUEUE_SIZE;

    return true;
}


// Gets the next message from the queue
// buffer Buffer to copy the message into
// size Size of the buffer
// return true if a message was retrieved, false if queue is empty
bool Serial_GetNextMessage(char *buffer, size_t size) {
    if (isRxQueueEmpty()) {
        return false;
    }

    // Copy the message
    RXMessage *msg = &rxQueue[rxQueueHead];
    size_t copyLength = (msg->length < (size - 1)) ? msg->length : (size - 1);
    memcpy(buffer, msg->message, copyLength);
    buffer[copyLength] = '\0';

    // Remove the message from the queue
    rxQueueHead = (rxQueueHead + 1) % RX_QUEUE_SIZE;

    return true;
}

// SerialInitialise
/*
 baudRate Baud rate to use
 serial_port Pointer to the SerialPort structure
 tx_completion_function Function to call when TX is complete
 rx_completion_function Function to call when RX is complete
 */
void SerialInitialise(uint32_t baudRate, SerialPort *serial_port,
                      void (*tx_completion_function)(void),
                      void (*rx_completion_function)(void)) {

	__disable_irq(); // Disable interrupts before setup

    serial_port->tx_completion_function = tx_completion_function;
    serial_port->rx_completion_function = rx_completion_function;

    // Enable required clocks
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHBENR  |= serial_port->MaskAHBENR;

    // Configure GPIO pins
    serial_port->GPIO->MODER   = serial_port->SerialPinModeValue;
    serial_port->GPIO->OSPEEDR = serial_port->SerialPinSpeedValue;
    serial_port->GPIO->AFR[0] |= serial_port->SerialPinAlternatePinValueLow;
    serial_port->GPIO->AFR[1] |= serial_port->SerialPinAlternatePinValueHigh;

    // Enable UART clocks
    RCC->APB1ENR |= serial_port->MaskAPB1ENR;
    RCC->APB2ENR |= serial_port->MaskAPB2ENR;

    // === Compute BRR dynamically based on PCLK and baudRate ===
    uint32_t pclk;
    if (serial_port->UART == USART1) {
        pclk = SystemCoreClock;  // USART1 is on APB2
    } else {
        // For USART2, USART3, etc. (typically APB1)
        // Replace 36000000 with your actual APB1 clock if not using HAL
        pclk = 48000000;
    }

    serial_port->UART->BRR = (pclk + baudRate / 2U) / baudRate;

    // Enable RX interrupt
    serial_port->UART->CR1 |= USART_CR1_RXNEIE;

    // Enable UART TX/RX and UART itself
    serial_port->UART->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    // Configure NVIC (only if using USART1 here—can make dynamic if needed)
    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);

    __enable_irq(); // Enable interrupts after setup
}


void Serial_WaitUntilTXComplete(void) {
    while (txInProgress);  // spin until TX is done
}


//USART Interrupt Handler (TX and RX)
void USART1_IRQHandler(void) {
    // Handle TX interrupts
    if (USART1_PORT.UART->ISR & USART_ISR_TXE) {
        if (txIndex < txLength) {
            // Send next character
            USART1_PORT.UART->TDR = txBuffer[txIndex++];
        } else {
            // Transmission complete, disable TX interrupt
            USART1_PORT.UART->CR1 &= ~USART_CR1_TXEIE;
            txIndex = 0;
            txLength = 0;
            txInProgress = false;
        }
    }

    // Handle RX interrupts
    if (USART1_PORT.UART->ISR & USART_ISR_RXNE) {
        char c = USART1_PORT.UART->RDR;

        // Check if end of program character
        if (c == 0x01) {
        	setLevelCompleteFlag(); // Set level complete flag
        }

        // Add character to active buffer if there's space
        if (rxIndex < RX_BUFFER_SIZE - 1) {
            activeRxBuffer[rxIndex++] = c;
        } else {
            rxIndex = 0; // Reset buffer if overflow occurs
        }


        // Check for end of message
        if (c == '\r') {
            // Null-terminate the message
            activeRxBuffer[rxIndex] = '\0';

            // Save pointer and length for the completed message
            volatile char *completedMessage = activeRxBuffer;
            uint32_t messageLength = rxIndex;

            // Switch active buffer before processing, so new chars go into the other one
            activeRxBuffer = (activeRxBuffer == rxBuffer1) ? rxBuffer2 : rxBuffer1;
            rxIndex = 0;

            // Process immediately if not busy,
            // otherwise enqueue the message for later processing.
            volatile bool processingMessage;
            if (!processingMessage) {
                // Not busy: process the message immediately via the registered callback
                if (USART1_PORT.rx_completion_function) {
                    USART1_PORT.rx_completion_function();
                }
            } else {
                // Busy: enqueue the message
                if (!EnqueueRxMessage((const char*)completedMessage, messageLength)) {
                    // If the queue is full... I should add error here
                }
            }
        }
    }
}
