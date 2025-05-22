/*
 * servo.h
 *
 *  Created on: May 22, 2025
 *      Author: willw
 */

#ifndef SERVO_H_
#define SERVO_H_

void enableClocks(void);

void setupGPIOPinsTim2(void);

void setupTim2Pwm(void);

void servoAngle(uint8_t channel, float degree);


#endif /* SERVO_H_ */
