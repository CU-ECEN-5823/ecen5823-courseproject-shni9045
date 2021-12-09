/*
   gpio.h
  
    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>


// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

#define LED0_port gpioPortF  // change to correct ports and pins
#define LED0_pin  4u
#define LED1_port gpioPortF
#define LED1_pin  5u

#define EXTCOMIN_port gpioPortD
#define EXTCOMIN_pin 13u

#define PUSHBUTTON_PB0port  gpioPortF
#define PUSHBUTTON_PB0pin   6u


#define PUSHBUTTON_PB1port gpioPortF
#define PUSHBUTTON_PB1pin  7u

#define MOTOR_PORT    gpioPortD
#define MOTOR_PIN_1   12u               //P6
#define MOTOR_PIN_2   11u               //P8
#define MOTOR_EN      10u               //P4

// Function prototypes
void gpioInit();

void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();

//sets the motor pin 2 on (logic 1)
void gpioMotorPin2On();

//sets the motor pin 1 on (logic 1)
void gpioMotorPin1On();

//sets the motor pin enable on (logic 1)
void gpioMotorEnOn();

//sets the motor pin 1 off (logic 0)
void gpioMotorPin1Off();

//sets the motor pin 2 off (logic 0)
void gpioMotorPin2Off();

//sets the motor pin enable off (logic 0)
void gpioMotorEnOff();

void gpioSetDisplayExtcomin(bool state);





#endif /* SRC_GPIO_H_ */
