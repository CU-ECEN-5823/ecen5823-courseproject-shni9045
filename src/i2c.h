/*
 * i2c.h - Header file for I2C0 functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "em_gpio.h"
#include "src/timers.h"
#include "app.h"
#include "em_i2c.h"
#include <sl_i2cspm.h>
#include "em_letimer.h"
#include "efr32bg13p632f512gm48.h"

extern uint32_t read_data;

extern uint8_t ambilight;

/*
 * Function to initialize i2c0
 * PARAMTERS - NONE
 * RETURNS   - NONE
 */
void I2C_init(void);

/*
 * Function to read from buffer of Si7021
 * PARAMTERS - NONE
 * RETURNS   - read buffer
 */
void I2C_Read_Si7021(void);

/*
 * Function to write command to Si7021
 * PARAMTERS - NONE
 * RETURNS   - True for Success , False for failure
 */
void I2C_Write_Si7021(void);

/*
 * Function to enable/disable Si7021
 * PARAMTERS - State to enable/disable
 * RETURNS   - NONE
 */
void Enable_si7021(bool state);

/*
 * Function to perform temperature read fro Si7021
 * PARAMTERS - NONE
 * RETURNS   - Converted temperature value
 */
void process_temp_si7021(void);


void Enable_CCS811(bool state);

void Wake_CCS811(bool state);

uint32_t init_CCS811(void);

uint32_t readMailbox_CCS811(uint8_t id, uint8_t length, uint8_t *data);

uint32_t setappmode_CCS811(void);

uint32_t startapp_CCS811(void);

uint32_t setMode_CCS811(uint8_t mode);

uint32_t measurequality_CCS811(uint16_t *eco2,uint16_t *tvoc);

bool dataavailaible(void);

/*
 * Function to perform ambient light read from VLM6030
 * PARAMTERS - NONE
 * RETURNS   - Converted temperature value
 */
uint16_t read_ALS_VLM6030(void);



bool I2C_Write_VEML6030init(void);


#endif /* SRC_I2C_H_ */
