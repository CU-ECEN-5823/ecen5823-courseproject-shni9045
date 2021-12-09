/*
  gpio.c
 
   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 */




#include "gpio.h"





// Set GPIO drive strengths and modes of operation
void gpioInit()
{

  // Student Edit:

  //GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

  //GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

 // Enable Pin of Si7021 Sensor
  GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, false);

  // Enable Pin of Si7021 Sensor
  GPIO_PinModeSet(EXTCOMIN_port, EXTCOMIN_pin, gpioModePushPull, false);

  // Set PB0 as input
  GPIO_PinModeSet(PUSHBUTTON_PB0port,PUSHBUTTON_PB0pin,gpioModeInput, false);

  // Set PB1 as input
  GPIO_PinModeSet(PUSHBUTTON_PB1port,PUSHBUTTON_PB1pin,gpioModeInput, false);

  GPIO_PinModeSet(MOTOR_PORT, MOTOR_PIN_1, gpioModePushPull, false);
  GPIO_PinModeSet(MOTOR_PORT, MOTOR_PIN_2, gpioModePushPull, false);
  GPIO_PinModeSet(MOTOR_PORT, MOTOR_EN, gpioModePushPull, false);


  // Enable GPIO interrupts
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);

  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  GPIO_ExtIntConfig(PUSHBUTTON_PB0port, PUSHBUTTON_PB0pin,PUSHBUTTON_PB0pin, true, true, true);

  GPIO_ExtIntConfig(PUSHBUTTON_PB1port, PUSHBUTTON_PB1pin,PUSHBUTTON_PB1pin, true, true, true);



} // gpioInit()

//Led0 on
void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED0_port,LED0_pin);
}

//motor pin 1 on
void gpioMotorPin1On()
{
  GPIO_PinOutSet(MOTOR_PORT,MOTOR_PIN_1);
}

//motor pin 2 on
void gpioMotorPin2On()
{
  GPIO_PinOutSet(MOTOR_PORT,MOTOR_PIN_2);
}

//motor pin enable on
void gpioMotorEnOn()
{
  GPIO_PinOutSet(MOTOR_PORT,MOTOR_EN);
}

//motor pin enable off
void gpioMotorEnOff()
{
  GPIO_PinOutClear(MOTOR_PORT,MOTOR_EN);
}

//motor pin 1 off
void gpioMotorPin1Off()
{
  GPIO_PinOutClear(MOTOR_PORT,MOTOR_PIN_1);
}

//motor pin 2 on
void gpioMotorPin2Off()
{
  GPIO_PinOutClear(MOTOR_PORT,MOTOR_PIN_2);
}

//Led0 off
void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED0_port,LED0_pin);
}

//Led1 on
void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED1_port,LED1_pin);
}

//Led1 off
void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED1_port,LED1_pin);
}


void gpioSetDisplayExtcomin(bool state){

  if (state == true){

      GPIO_PinOutSet(EXTCOMIN_port,EXTCOMIN_pin);

  }

  else if (state == false){

      GPIO_PinOutClear(EXTCOMIN_port,EXTCOMIN_pin);

  }

}



