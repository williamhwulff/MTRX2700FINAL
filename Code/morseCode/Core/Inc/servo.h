/*
 * servo.h
 *
 *  Created on: May 22, 2025
 *      Author: willw
 */

#ifndef SERVO_H_
#define SERVO_H_


void setupGPIOPinsTim8(void);

void setupTim8Pwm(void);

void servoAngle(uint8_t channel, uint8_t degree);


#endif /* SERVO_H_ */
