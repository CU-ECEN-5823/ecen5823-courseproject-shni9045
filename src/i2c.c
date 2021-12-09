/*
 * i2c.c - Source file Containing implementation of I2C functionality
 * Attributes - Prof.David Sluiter IOT and Embedded Firmware Lecture 5 & Lecture 6
 *
 */

#include "i2c.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG (1)

#include "src/log.h"

// Si7021 I2C Address
#define SI70_I2C_ADDR (0x40)

// Measure temperature No Hold Master Mode
#define MEASURE_TEMP_CMD (0xF3)

// CCS811 I2C Address
#define CCS811_I2C_ADDR (0xB6)

#define CCS811_ADDR_ALG_RESULT_DATA (0x02)

#define CCS811_ADDR_APP_START (0xF4)

#define CCS811_ADDR_STATUS (0x00)

#define CCS811_ADDR_HW_ID (0x20)

#define CCS811_ADDR_MEASURE_MODE (0x01)

#define CCS811_HW_ID (0x81)

// VLM6030 I2C Address
#define VLM6030_I2C_ADDR (0x48)


// Master mode transfer message structure
I2C_TransferSeq_TypeDef transferSequence;

// Variable for temperature commands
uint8_t cmd_data;

// Variable for temperature in celsius
uint32_t read_data;

uint8_t ambilight;

// Buffer for storing read data
uint8_t *temp_buffer;

void I2C_init(void){

  // Assign values for I2C init
  I2CSPM_Init_TypeDef i2cspm = {

      .i2cClhr = i2cClockHLRStandard,
      .i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
      .i2cRefFreq = 0,
      .port = I2C0,
      .portLocationScl = 14u,
      .portLocationSda = 16u,
      .sclPin = 10u,
      .sclPort = gpioPortC,
      .sdaPin = 11u,
      .sdaPort = gpioPortC,

      };

  // Initialize I2C0
  I2CSPM_Init(&i2cspm);

  //i2c_freq=I2C_BusFreqGet(I2C0);


}


void I2C_Read_Si7021(void){

  I2C_TransferReturn_TypeDef check_transfer;

  // Allocate Memory for read buffer to store temperature data
  temp_buffer=(uint8_t*)malloc(sizeof(uint8_t)*2);


  I2C_init();

  transferSequence.addr = SI70_I2C_ADDR<<1;             // Si7021 I2C Address
  transferSequence.flags = I2C_FLAG_READ;               // Set transfer flas as Read
  transferSequence.buf[0].data = temp_buffer,           // Assign buffer to store read data
  transferSequence.buf[0].len = sizeof(temp_buffer),    // Assign size of the buffer

  // Enable I2C Interrupt
  NVIC_EnableIRQ(I2C0_IRQn);

  // Prepare and Start I2C Transfer
  check_transfer = I2C_TransferInit(I2C0, &transferSequence);


  // Map Return status of I2C transfer
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


void I2C_Write_Si7021(void){

  I2C_TransferReturn_TypeDef check_transfer;

  I2C_init();

  // Measure temperature No Hold Master Mode command
  cmd_data = MEASURE_TEMP_CMD;

  transferSequence.addr = SI70_I2C_ADDR<<1;                // Si7021 I2C Address
  transferSequence.flags = I2C_FLAG_WRITE;                 // Set transfer flas as write
  transferSequence.buf[0].data = &cmd_data;                // Assign address of Temperature command
  transferSequence.buf[0].len = sizeof(cmd_data);          // Assign size

  // Enable I2C Interrupt
  NVIC_EnableIRQ(I2C0_IRQn);

  // Prepare and Start I2C Transfer
  check_transfer = I2C_TransferInit(I2C0, &transferSequence);


  // Map Return status of I2C transfer
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


void Enable_si7021(bool state){

  // Set Enable Pin high
  if (state == true){

      GPIO_PinOutSet(gpioPortD,15);

  }

  // Set Enable Pin low
  else if (state == false){

      GPIO_PinOutClear(gpioPortD,15);

  }


}

void process_temp_si7021(void){


  uint16_t temp = 0;

  uint16_t celsius = 0;

  // Combine 8 bit words by left shiffting MSB by 8
  temp=(256*temp_buffer[0])+temp_buffer[1];

  // Convert Temperarure Code to degree Celsius
  celsius = ((175.72*(temp))/65535)  - 46.85;

  // Free allocated buffer
  free(temp_buffer);

  read_data=celsius;

  // LOG the temperature
  //LOG_INFO("Current Temperature : %d\n\r",(int32_t)read_data);


}


void Enable_CCS811(bool state){

  // Set Enable Pin high
  if (state == true){

      GPIO_PinOutSet(gpioPortA,3);

  }

  // Set Enable Pin low
  else if (state == false){

      GPIO_PinOutClear(gpioPortA,3);

  }


}

void Wake_CCS811(bool state){

  // Set Enable Pin high
  if (state == true){

      //GPIO_PinOutSet(gpioPortD,10);
      //GPIO_PinOutClear(gpioPortA,2);
      GPIO_PinOutSet(gpioPortA,2);

  }

  // Set Enable Pin low
  else if (state == false){

      //GPIO_PinOutClear(gpioPortD,10);
      //GPIO_PinOutSet(gpioPortA,2);
      GPIO_PinOutClear(gpioPortA,2);

  }


}



uint32_t init_CCS811(void){

  uint8_t id;
  uint32_t check;

  //Enable_CCS811(true);
  Wake_CCS811(true);


  // Wait for Power up time
  timerWaitUs(100000);


  check  =  readMailbox_CCS811(CCS811_ADDR_HW_ID, 1, &id);

  LOG_INFO("HARDWARE ID : %02X\r",id);

  if (check != 1 && id != CCS811_HW_ID){

      LOG_ERROR("Initialize CCS811 failed\r");
      return 0;

  }


  Wake_CCS811(false);

  return 1;


}

uint32_t readMailbox_CCS811(uint8_t id, uint8_t length, uint8_t *data){

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t write_data[1];


  Wake_CCS811(true);

  write_data[0] = id;


  seq.addr = CCS811_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE_READ;
  seq.buf[0].data = write_data;
  seq.buf[0].len = 1;


  seq.buf[1].data = data;
  seq.buf[1].len = length;


  ret = I2CSPM_Transfer(I2C0, &seq);

  if (ret != i2cTransferDone){

      LOG_ERROR("i2c transfer failed\r");
      return 0;

  }


  Wake_CCS811(false);

  return 1;

}

uint32_t setappmode_CCS811(void){

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t read_data[2];
  uint8_t write_data[1];

  Wake_CCS811(true);

  write_data[0] = CCS811_ADDR_APP_START;

  seq.addr = CCS811_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE;

  seq.buf[0].data = write_data;
  seq.buf[0].len = 1;

  seq.buf[1].data = read_data;
  seq.buf[1].len = 0;


  ret = I2CSPM_Transfer(I2C0, &seq);

   if (ret != i2cTransferDone){

       LOG_ERROR("i2c transfer failed\r");
       return 0;

   }


   Wake_CCS811(false);

   return 1;

}

uint32_t startapp_CCS811(void){

  uint32_t result;
  uint8_t status;

  result  = readMailbox_CCS811(CCS811_ADDR_STATUS, 1, &status);

  if ((status & 0x10 ) != 0x10){

    LOG_ERROR("Application Missing\r");
    return 0;

  }

  result += setappmode_CCS811();

  result = readMailbox_CCS811(CCS811_ADDR_STATUS,1,&status);

  if ((status & 0x90 ) != 0x90){

      LOG_ERROR("Error in setting Application Mode\r");
      return 0;
  }


  return 1;


}


uint32_t setMode_CCS811(uint8_t mode){

    I2C_TransferSeq_TypeDef seq;
    I2C_TransferReturn_TypeDef ret;

    uint8_t read_data[1];
    uint8_t write_data[2];

    Wake_CCS811(true);

    mode = (mode & 0x38);

    write_data[0] = CCS811_ADDR_MEASURE_MODE;

    write_data[1] = mode;

    seq.addr = CCS811_I2C_ADDR;
    seq.flags = I2C_FLAG_WRITE;

    seq.buf[0].data = write_data;
    seq.buf[0].len = 2;

    seq.buf[1].data = read_data;
    seq.buf[1].len = 0;


    ret = I2CSPM_Transfer(I2C0, &seq);

     if (ret != i2cTransferDone){

         LOG_ERROR("i2c transfer failed\r");
         return 0;

     }


     Wake_CCS811(false);

     return 1;


}



uint32_t measurequality_CCS811(uint16_t *eco2,uint16_t *tvoc){

  I2C_TransferSeq_TypeDef seq;
  I2C_TransferReturn_TypeDef ret;

  uint8_t read_data[4];
  uint8_t write_data[1];

  *eco2 = 0;
  *tvoc = 0;

  Wake_CCS811(true);

  write_data[0] = CCS811_ADDR_ALG_RESULT_DATA;

   seq.addr = CCS811_I2C_ADDR;
   seq.flags = I2C_FLAG_WRITE;

   seq.buf[0].data = write_data;
   seq.buf[0].len = 1;

   seq.buf[1].data = read_data;
   seq.buf[1].len = 4;

   ret = I2CSPM_Transfer(I2C0, &seq);

   if (ret != i2cTransferDone){

       LOG_ERROR("i2c transfer failed\r");
       return 0;

   }

   *eco2 = ((uint16_t ) read_data[0] << 8 ) + (uint16_t) read_data[1];
   *tvoc = ((uint16_t ) read_data[2] << 8 ) + (uint16_t) read_data[3];

   Wake_CCS811(false);

   return 1;


}

bool dataavailaible( void ){

  bool state = false;
  uint32_t status;
  uint8_t reg;

  uint32_t status2;
  uint8_t reg2;

  status = readMailbox_CCS811(CCS811_ADDR_STATUS, 1, &reg);

  if ((status == 1) && ((reg & 0x08) == 0x08)){

      state = true;

  }

  else if ((status == 1) && ((reg & 0x01) == 0x01)){

      LOG_ERROR("ERROR OCCURED in reading status register\r");

      status2 = readMailbox_CCS811(0xe0, 1, &reg2);

      if (status2){

          LOG_INFO("ERROR REGISTER : %02X\r",reg2);

      }

  }

  return state;

}

bool I2C_Write_VEML6030init(void){

  // Measure temperature No Hold Master Mode command
  uint8_t command;
  uint8_t commanddata[2];

  command = 0x00;
  commanddata[0] = 0x00;
  commanddata[1] = 0x20;

  I2C_TransferReturn_TypeDef check_transfer;

  // Assign address, set write flag  and pass command to buffer
  I2C_TransferSeq_TypeDef write ={

      .addr = VLM6030_I2C_ADDR<<1,
      .flags = I2C_FLAG_WRITE_WRITE,

      .buf[0].data = &command,
      .buf[0].len = 1,

      .buf[1].data = commanddata,
      .buf[1].len = 2,



  };

  // Perform I2C transfer
  check_transfer=I2CSPM_Transfer(I2C0,&write);

  // Map Return status of I2C transfer
  switch(check_transfer){

    case i2cTransferInProgress:{
      LOG_INFO("\n\rTransfer In Progress");
      break;
    }

    case i2cTransferDone:{

      return true;
      break;

    }

    case i2cTransferNack:{
      LOG_ERROR("\n\rNACK Received");
      break;
    }

    case i2cTransferBusErr:{
      LOG_ERROR("\n\rBus Error");
       break;
     }

    case i2cTransferArbLost:{

      LOG_ERROR("\n\rArbitration lost");
        break;
      }

    case i2cTransferUsageFault:{

      LOG_ERROR("\n\rUsage Fault");
        break;
      }

    case i2cTransferSwFault:{

      LOG_ERROR("\n\rSw Fault");
      break;
      }

    default:{

      break;
    }

  }

  return false;

}




uint16_t read_ALS_VLM6030(void){

    uint16_t light = 0;

    I2C_TransferSeq_TypeDef seq;
    I2C_TransferReturn_TypeDef ret;

    uint8_t read_data[2];
    uint8_t write_data[1];

    write_data[0] = 0x04;


    seq.addr = VLM6030_I2C_ADDR<<1;
    seq.flags = I2C_FLAG_WRITE_READ;
    seq.buf[0].data = write_data;
    seq.buf[0].len = 1;


    seq.buf[1].data = read_data;
    seq.buf[1].len = 2;


    ret = I2CSPM_Transfer(I2C0, &seq);

    while(ret == i2cTransferNack);

    if (ret != i2cTransferDone){

        LOG_ERROR("i2c transfer failed\r");
        return 0;

    }

    // Combine 8 bit words by left shiffting MSB by 8
    light =(read_data[0])+(256*read_data[1]);

    // reference app note : For Gain 1 & IT time 100ms factor is 0.0576
    ambilight = light*0.0576;

    // LOG the temperature
    LOG_INFO("Current LUX Level : %d\r",(int32_t)ambilight);

    schedulerSetAmbient_readdoneEvent();

    return 1;

}







