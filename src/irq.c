/*
 * irq.c -  Source file for implementation of LETIMER0 IRQ Handler
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#include "irq.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG (1)

#include "src/log.h"

#if(LOWEST_ENERGY_MODE==3)
#define CLOCK_FREQ (1000)
// Macro definition for Clock Prescaler
#define PRESCALER (1)
#else
#define CLOCK_FREQ (32768)
// Macro definition for Clock Prescaler
#define PRESCALER (4)
#endif

#define ACTUAL_CLK_FREQ ((CLOCK_FREQ) / (PRESCALER))

// Volatile variable to track number of rollovers since system is up
volatile uint32_t rollover_count=0;



/* Function that Implements LETIMER0 IRQ Handler
 * PARAMETERS : NONE
 * RETURNS    : NONE
 */
void LETIMER0_IRQHandler(void){

  uint32_t flags = LETIMER_IntGetEnabled(LETIMER0);

  LETIMER_IntClear(LETIMER0,flags);

  // Check if COMP1 Interrupt flag is set
  if (flags == LETIMER_IEN_COMP1){

      // Set the appropriate event in scheduler
      schedulerSetCOMP1Event();

      /*Disable COMP1 Interrupt*/
      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);

  }

  // Check if Underflow Interrupt flag is set
 if (flags == LETIMER_IEN_UF){

      // Set the appropriate event in scheduler
      schedulerSetUFEvent();

      CORE_DECLARE_IRQ_STATE;                         // Save system interrupt state

      CORE_ENTER_CRITICAL();                          // Start critical section

      // Increament underflow count
      rollover_count++;

      CORE_EXIT_CRITICAL();                           // End critical section

  }


}

uint32_t letimerMilliseconds(){

  uint32_t compare0_value = (LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000;

  uint32_t milli_sec;
  uint32_t current_count = LETIMER_CounterGet(LETIMER0);

  // Add rollover count plus number of current counter ticks passed
  milli_sec = (rollover_count * LETIMER_PERIOD_MS) + (((compare0_value-current_count) *1000)/ACTUAL_CLK_FREQ);

  return milli_sec;

}




void I2C0_IRQHandler(void){

  I2C_TransferReturn_TypeDef check_transfer;

  // Continue the initiated I2C transfer
  check_transfer = I2C_Transfer(I2C0);

  // Check status of transfer
  if (check_transfer == i2cTransferDone) {

      schedulerSetI2CdoneEvent();


  }

  if (check_transfer < 0){

      switch(check_transfer){

         case i2cTransferNack:{
           LOG_ERROR("NACK Received\n\r");
           break;
         }

         case i2cTransferBusErr:{
           LOG_ERROR("Bus Error\n\r");
            break;
          }

         case i2cTransferArbLost:{

           LOG_ERROR("Arbitration lost\n\r");
             break;
           }

         case i2cTransferUsageFault:{

           LOG_ERROR("Usage Fault\n\r");
             break;
           }

         case i2cTransferSwFault:{

           LOG_ERROR("Sw Fault\n\r");
           break;
           }

         default:{

           break;
         }

       }


  }


}


void GPIO_EVEN_IRQHandler(void){

  uint32_t flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  if(!(GPIO_PinInGet(PUSHBUTTON_PB0port, PUSHBUTTON_PB0pin)))
  {
      //LOG_INFO("Button Pressed\n\r");
      schedulerSetPushbuttonPB0PressEvent();
  }
  else
  {
      //LOG_INFO("Button Released\n\r");
      schedulerSetPushbuttonPB0ReleaseEvent();
  }

}


void GPIO_ODD_IRQHandler(void){

  uint32_t flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  if(!(GPIO_PinInGet(PUSHBUTTON_PB1port, PUSHBUTTON_PB1pin)))
  {
      //LOG_INFO("Button Pressed\n\r");
      schedulerSetPushbuttonPB1PressEvent();
  }
  else
  {
      //LOG_INFO("Button Released\n\r");
      schedulerSetPushbuttonPB1ReleaseEvent();
  }

}





