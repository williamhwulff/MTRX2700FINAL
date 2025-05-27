/*
 * callbacks.h
 *
 *  Created on: May 25, 2025
 *      Author: willw
 */

#ifndef INC_CALLBACKS_H_
#define INC_CALLBACKS_H_

void finishedTransmissionCallback(void); // Create delay after finishing transmission

uint8_t morseEXTI0Callback(uint8_t messageSent);

void morseEXTI1Callback(void);

void morseEXTI4Callback(void);

uint8_t TIM3Callback(uint8_t messageSent);


#endif /* INC_CALLBACKS_H_ */
