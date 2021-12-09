/*
 * timers.c - Source file for implementation of CMU functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 *
 */

#include "timers.h"


#if(LOWEST_ENERGY_MODE==3)
#define CLOCK_FREQ (1000)
// Macro definition for Clock Prescaler
#define PRESCALER (1)
#else
#define CLOCK_FREQ (32768)
// Macro definition for Clock Prescaler
#define PRESCALER (4)
#endif

// Desired frequency in Hz
#define OUT_FREQ 1

// Duty cycle percentage
#define DUTY_CYCLE 100

// LETIMER0 frequency
#define LETIMER0_FREQ CMU_ClockFreqGet(cmuClock_LETIMER0)

#define ACTUAL_CLK_FREQ ((CLOCK_FREQ) / (PRESCALER))

// Macro definition for Compare Register 1 Value
#define COMPARE0_VALUE ((LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000)


#define MICROSEC_PER_SEC (1000000)

/* Function that initializes LETIMER by setting various bit field in LETIMER0_CTRL register
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */

#if (DEVICE_IS_BLE_SERVER == 1)
void Timer_init(){


  const LETIMER_Init_TypeDef LEINIT = {
    .enable   = false,                       //Don't start counting upon init
    .debugRun = true,                       //Freeze timer in debug
    .comp0Top = true,                        //Load comp0 as top value
    .bufTop   = false,                       //Write comp0 only by software
    .out0Pol  = 0,                           //Low polarity of output1
    .out1Pol  = 0,                           //Low polarity of output2
    .ufoa0    = letimerUFOANone,             //No action on underflow
    .ufoa1    = letimerUFOANone,             //No action on underflow
    .repMode  = letimerRepeatFree,           //Stop timer by software
    .topValue = 65535,
    };

    /* Initialize LETIMER */
    LETIMER_Init(LETIMER0, &LEINIT);


}

#else
void initLETIMER(void)
{
LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;

  // Enable clock to the LE modules interface
  CMU_ClockEnable(cmuClock_HFLE, true);

  // Select LFXO for the LETIMER
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_LETIMER0, true);

  // Reload COMP0 on underflow, pulse output, and run in repeat mode
  letimerInit.comp0Top = true;
  letimerInit.ufoa0 = letimerUFOAPwm;
  letimerInit.repMode = letimerRepeatFree;
  letimerInit.topValue = LETIMER0_FREQ / OUT_FREQ;

  // Need REP0 != 0 to run PWM
  LETIMER_RepeatSet(LETIMER0, 0, 1);

  // Set COMP1 to control duty cycle
  LETIMER_CompareSet(LETIMER0, 1,
       CMU_ClockFreqGet(cmuClock_LETIMER0) * DUTY_CYCLE / (OUT_FREQ * 100));

  // Enable LETIMER0 output0 on PF4 (Route 28)
  LETIMER0->ROUTEPEN |=  LETIMER_ROUTEPEN_OUT0PEN;
  LETIMER0->ROUTELOC0 |= LETIMER_ROUTELOC0_OUT0LOC_LOC20;

  // Initialize LETIMER
  LETIMER_Init(LETIMER0, &letimerInit);
}

#endif

/* Function that loads vale into comp0 & comp1 register
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Timer_load(){

   // Load Value into compare register 0
   LETIMER_CompareSet(LETIMER0,0,COMPARE0_VALUE);
}

/* Function to enable/disable the timer
 * PARAMETERS : true to enable,false to disable
 * RETURNS    : NONE
 *
 */
void Timer_Onoff(bool state){

  if (state == true){

      // Start Timer
      LETIMER_Enable(LETIMER0,true);

  }

  else if(state == false){

      // Stop Timer
      LETIMER_Enable(LETIMER0,false);
  }

}

/* Function to print LETIMER0 counter value
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Timer_Printcount(){


  //uint32_t temp;

  //Log Timer Count for debug purpose
  //temp=LETIMER_CounterGet(LETIMER0);
  //LOG_INFO("\nCurrent Timer Count is %d",(int32_t)temp);


}

/* Function to enable particular interrupts of LETIMER0
 * PARAMETERS : NONE
 * RETURNS    : NONE
 *
 */
void Timer_InterruptEnable(){

  // Enable Underflow Interrupt
  LETIMER_IntEnable(LETIMER0,LETIMER_IEN_UF);

}

void timerWaitUs(uint32_t us_wait){

  uint32_t compare0_value = (LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000;

  uint32_t total_ticks;

  // To avoid unsigned 32 bit overflow for too large delays
  if(us_wait >= MICROSEC_PER_SEC ){

      uint32_t sec_wait=us_wait/MICROSEC_PER_SEC;

      total_ticks = (sec_wait*(ACTUAL_CLK_FREQ));

  }

  else {

      total_ticks = ((us_wait*(ACTUAL_CLK_FREQ))/MICROSEC_PER_SEC);

  }


  uint32_t now_count;
  uint32_t current_count;

  // Range Check for total ticks
  if(total_ticks > (COMPARE0_VALUE)){

      //LOG_ERROR("Out of Range Value\r");
  }

  now_count=LETIMER_CounterGet(LETIMER0);


  // Roll over logic
  if(now_count < total_ticks){


      // Let timer count till underflow
      // while((current_count=LETIMER_CounterGet(LETIMER0)) < now_count);
      while((current_count=LETIMER_CounterGet(LETIMER0)) != 0);


      // Continue counting for remainin ticks after underflow
      while((current_count=LETIMER_CounterGet(LETIMER0)) >= ((compare0_value)-(total_ticks-now_count)));


  }

  else {

      // Continue counting till the calculated ticks
      while((current_count=LETIMER_CounterGet(LETIMER0)) >= (now_count-total_ticks));

  }

}

void timerWaitUs_irq(uint32_t us_wait){

  uint32_t compare0_value = (LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000;

   uint32_t total_ticks;

   // To avoid unsigned 32 bit overflow for too large delays
   if(us_wait >= MICROSEC_PER_SEC ){

       uint32_t sec_wait=us_wait/MICROSEC_PER_SEC;

       total_ticks = (sec_wait*(ACTUAL_CLK_FREQ));

   }

   else {

       total_ticks = ((us_wait*(ACTUAL_CLK_FREQ))/MICROSEC_PER_SEC);

   }


   uint32_t now_count;
   uint32_t set_count;

   now_count=LETIMER_CounterGet(LETIMER0);

   if (total_ticks > now_count){

       set_count = compare0_value - (total_ticks-now_count);

       LETIMER_IntClear(LETIMER0,LETIMER_IFC_COMP1);

       LETIMER_CompareSet(LETIMER0,1,set_count);

       //Enable comp1 Interrupt
       LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);



   }

   else {

       set_count =  now_count - total_ticks;
       LETIMER_IntClear(LETIMER0,LETIMER_IFC_COMP1);

       LETIMER_CompareSet(LETIMER0,1,set_count);

       //Enable comp1 Interrupt
       LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);


   }


}

