/*
 * timers.h -  Header File for implementation of
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "app.h"
#include "em_letimer.h"
#include "efr32bg13p632f512gm48.h"


/* Function that initializes LETIMER by setting various bit field in LETIMER0_CTRL register
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Timer_init();

/* Function that loads vale into comp0 & comp1 register
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */

void initLETIMER(void);

void Timer_load();

/* Function to enable/disable the timer
 * PARAMETERS : true to enable,false to disable
 * RETURNS    : NONE
 *
 */
void Timer_Onoff(bool state);




void timerWaitUs(uint32_t us_wait);



/* Function to print LETIMER0 counter value
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Timer_Printcount();

/* Function to enable particular interrupts of LETIMER0
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Timer_InterruptEnable();


void timerWaitUs_irq(uint32_t us_wait);

#endif /* SRC_TIMERS_H_ */
