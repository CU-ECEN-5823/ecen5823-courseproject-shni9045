/*
 * oscillators.h - Header file for implementation of CMU functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 *
 */

#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_

#include <stdbool.h>
#include <string.h>
#include "app.h"
#include "em_cmu.h"

/* Function that initializes CMU by enabling and selecting appropriate oscillators
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Init_CMU(void);

/* Function that configures CMU by enabling and prescaling appropriate clocks
 * PARAMETERS : NONE
 * RETURNS    : NONE
 */
void Configure_CMU(void);


#endif /* SRC_OSCILLATORS_H_ */
