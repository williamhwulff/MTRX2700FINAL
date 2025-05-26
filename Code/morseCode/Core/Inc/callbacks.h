/*
 * callbacks.h
 *
 *  Created on: May 25, 2025
 *      Author: willw
 */

#ifndef INC_CALLBACKS_H_
#define INC_CALLBACKS_H_

void finishedTransmissionCallback(void); // Create delay after finishing transmission

uint8_t EXTI0Callback(uint8_t messageSent);

uint8_t TIM3Callback(uint8_t messageSent);


#endif /* INC_CALLBACKS_H_ */
