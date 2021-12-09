/*
 * oscillators.c - Source file containing implementation of CMU functionality
 *
 *
 */


#include "oscillators.h"

// Macro definition for clock prescaling value


#if(LOWEST_ENERGY_MODE==3)
#define OSC_TYPE (cmuOsc_ULFRCO)
#define OSC_SEL (cmuSelect_ULFRCO)
#define CLK_PRESCALER (1)
#else
#define OSC_TYPE (cmuOsc_LFXO)
#define OSC_SEL (cmuSelect_LFXO)
#define CLK_PRESCALER (4)
#endif

// Desired frequency in Hz
#define OUT_FREQ 1

// Duty cycle percentage
#define DUTY_CYCLE 30

// LETIMER0 frequency
#define LETIMER0_FREQ CMU_ClockFreqGet(cmuClock_LETIMER0)


/* Function that initializes CMU by enabling and selecting appropriate oscillators
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Init_CMU(void){

  // Enable the oscillator
  CMU_OscillatorEnable(OSC_TYPE,true,true);

  // Enable oscillator for I2C
  //CMU_OscillatorEnable(cmuOsc_HFXO,true,true);
  CMU_OscillatorEnable(cmuOsc_HFRCO,true,true);

  // Select the reference oscillator for LFA
  CMU_ClockSelectSet(cmuClock_LFA,OSC_SEL);

  // Select the reference oscillator for HF
  CMU_ClockSelectSet(cmuClock_HFPER,cmuSelect_HFRCO);


}


/* Function that configures CMU by enabling and prescaling appropriate clocks
 * PARAMETERS : NONE
 * RETURNS    : NONE
 */
void Configure_CMU(void){


  // Enable LFA CLock tree
  CMU_ClockEnable(cmuClock_LFA,true);

  // Enable clock to the LE modules interface
  CMU_ClockEnable(cmuClock_HFLE, true);

  // Enable LFA CLock tree
  CMU_ClockEnable(cmuClock_HFPER,true);

  // Prescaling of LETIMER clock
  CMU_ClockDivSet(cmuClock_LETIMER0,CLK_PRESCALER);

  // Enabling the LETIMER0 peripheral clock
  CMU_ClockEnable(cmuClock_LETIMER0,true);

  // Enabling the I2C0 peripheral clock
   CMU_ClockEnable(cmuClock_I2C0,true);

   CMU_ClockEnable(cmuClock_GPIO,true);




}
