#include "main.h"


// We store the pointers to the GPIO and USART that are used
//  for a specific serial port. To add another serial port
//  you need to select the appropriate values.
struct _SerialPort {
	USART_TypeDef *UART;
	GPIO_TypeDef *GPIO;
	volatile uint32_t MaskAPB2ENR;	// mask to enable RCC APB2 bus registers
	volatile uint32_t MaskAPB1ENR;	// mask to enable RCC APB1 bus registers
	volatile uint32_t MaskAHBENR;	// mask to enable RCC AHB bus registers
	volatile uint32_t SerialPinModeValue;
	volatile uint32_t SerialPinSpeedValue;
	volatile uint32_t SerialPinAlternatePinValueLow;
	volatile uint32_t SerialPinAlternatePinValueHigh;
	void (*completion_function)(void);
};



// instantiate the serial port parameters
//   note: the complexity is hidden in the c file
SerialPort USART1_PORT = {USART1,
		GPIOC,
		RCC_APB2ENR_USART1EN, // bit to enable for APB2 bus
		0x00,	// bit to enable for APB1 bus
		RCC_AHBENR_GPIOCEN, // bit to enable for AHB bus
		0xA00,
		0xF00,
		0x770000,  // for USART1 PC10 and 11, this is in the AFR low register
		0x00, // no change to the high alternate function register
		0x00 // default function pointer is NULL
		};


// InitialiseSerial - Initialise the serial port
// Input: baudRate is from an enumerated set
void SerialInitialise(uint32_t baudRate, SerialPort *serial_port, void (*completion_function)(void)) {

	__disable_irq(); // Disable interrupts for setup

	serial_port->completion_function = completion_function;

	// enable clock power, system configuration clock and GPIOC
	// common to all UARTs
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// enable the GPIO which is on the AHB bus
	RCC->AHBENR |= serial_port->MaskAHBENR;

	// set pin mode to alternate function for the specific GPIO pins
	serial_port->GPIO->MODER = serial_port->SerialPinModeValue;

	// enable high speed clock for specific GPIO pins
	serial_port->GPIO->OSPEEDR = serial_port->SerialPinSpeedValue;

	// set alternate function to enable USART to external pins
	serial_port->GPIO->AFR[0] |= serial_port->SerialPinAlternatePinValueLow;
	serial_port->GPIO->AFR[1] |= serial_port->SerialPinAlternatePinValueHigh;

	// enable the device based on the bits defined in the serial port definition
	RCC->APB1ENR |= serial_port->MaskAPB1ENR;
	RCC->APB2ENR |= serial_port->MaskAPB2ENR;

	// Get a pointer to the 16 bits of the BRR register that we want to change
	uint16_t *baud_rate_config = (uint16_t*)&serial_port->UART->BRR; // only 16 bits used!

	switch(baudRate){
		case BAUD_9600:
			*baud_rate_config = 0x1388; // 9600 at 48MHz
			break;
		case BAUD_19200:
			*baud_rate_config = 0x09C4; // 19200 at 48MHz
			break;
		case BAUD_38400:
			*baud_rate_config = 0x04E2; // 38400 at 48MHz
			break;
		case BAUD_57600:
			*baud_rate_config = 0x0341; // 57600 at 48MHz
			break;
		case BAUD_115200:
			*baud_rate_config = 0x01A1; // 115200 at 48MHz
			break;
	}


	// enable serial port for tx and rx and rx interrupts
	serial_port->UART->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

	NVIC_EnableIRQ(USART1_IRQn);        // Enable USART1 IRQ
	NVIC_SetPriority(USART1_IRQn, 2);   // Set priority to 2


	__enable_irq(); // Enable interrupts post setup
}


void SerialOutputChar(uint8_t data, SerialPort *serial_port) {

	while((serial_port->UART->ISR & USART_ISR_TXE) == 0){
	}

	serial_port->UART->TDR = data;

	serial_port->completion_function();
}


uint8_t SerialInputChar(SerialPort* serial_port) {

	// Check for overrun and frame errors and clear
	while((serial_port->UART->ISR & (USART_ISR_ORE | USART_ISR_FE)) != 0){
		serial_port->UART->ICR = USART_ICR_ORECF | USART_ICR_FECF; // Clear error flags
	}

	while ((serial_port->UART->ISR & USART_ISR_RXNE) == 0) {
	}

	return serial_port->UART->RDR;
}
