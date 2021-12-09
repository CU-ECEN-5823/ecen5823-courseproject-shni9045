/*
 * ble.c
 * Attributes - soc_thermometer.c
 *              Prof.David Sluiter Lecture 10 & 11
 *
 * Author @shrikant nimhan shni9045@colorado.edu & yogesh tripathi
 *
 */

#include "ble.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG (1)

#include "src/log.h"

#define SOFTTIMER_PERIOD                   (8192)

#define SL_BT_HT_TEMPERATURE_TYPE          (2)         ///SL_BT_HT_TEMPERATURE_TYPE_BODY

#define MEASUREMENT_INTERVAL               (2)

#define PASSIVE_SCANNING                   (0)

#define SCANNING_INTERVAL                  (80)

#define SCANNING_WINDOW                    (40)

// Flags for button status
uint8_t buttonstatus;
uint8_t buttonvalue;

uint8_t buttonvalue_buffer[2] = {0};

uint8_t button_sequence[3] = {0};                      // Array to track button sequence changes


#if (DEVICE_IS_BLE_SERVER == 0)
// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

static const uint8_t dutycycle_service[16] = {0xed,0xc4,0x01,0x6a,0x79,0x83,0x9e,0xb7,0x9d,0x4e,0xcd,0x36,0x6d,0xd2,0xa3,0x75};

static const uint8_t dutycycle_char[16] =  {0xed,0xc4,0x01,0x6a,0x79,0x83,0x9e,0xb7,0x9d,0x4e,0xcd,0x36,0x6e,0xd2,0xa3,0x75};

static const uint8_t ambient_service[16] = {0x10,0x9d,0x3a,0xc5,0x77,0xd6,0x6b,0x87,0xe9,0x49,0x6a,0xc6,0x39,0x0f,0x6d,0x85};

static const uint8_t ambient_char[16] = {0x10,0x9d,0x3a,0xc5,0x77,0xd6,0x6b,0x87,0xe9,0x49,0x6a,0xc6,0x40,0x0f,0x6d,0x85};


#endif

// BLE private data
ble_data_struct_t ble_data = {.temp_measure_indication_status=false,.temp_type_indication_status=false,.temp_interval_indication_status=false,.enable_measurement=false,.temp_measure_inflight_status=false,. temp_type_inflight_status=false,.temp_interval_inflight_status=false,.indications_enabled=true};


ble_data_struct_t* getBleDataPtr(){

  return (&ble_data);

}

#if (DEVICE_IS_BLE_SERVER == 0)
// -----------------------------------------------
// Private function, original from Dan Walkes. I fixed a sign extension bug.
// We'll need this for Client A7 assignment to convert health thermometer
// indications back to an integer. Convert IEEE-11073 32-bit float to signed integer.
// -----------------------------------------------
static int32_t FLOAT_TO_INT32(const uint8_t *value_start_little_endian)
{
    uint8_t signByte = 0;
    int32_t mantissa;
    // input data format is:
    // [0] = flags byte
    // [3][2][1] = mantissa (2's complement)
    // [4] = exponent (2's complement)
    // BT value_start_little_endian[0] has the flags byte
    int8_t exponent = (int8_t)value_start_little_endian[4];
    // sign extend the mantissa value if the mantissa is negative
    if (value_start_little_endian[3] & 0x80) { // msb of [3] is the sign of the mantissa
    signByte = 0xFF;
    }
    mantissa = (int32_t) (value_start_little_endian[1] << 0) |
    (value_start_little_endian[2] << 8) |
    (value_start_little_endian[3] << 16) |
    (signByte << 24) ;
    // value = 10^exponent * mantissa, pow() returns a double type
    return (int32_t) (pow(10, exponent) * mantissa);
} // FLOAT_TO_INT32
#endif

void transmit_ambientdata(){

  sl_status_t sc;

  // Update our local GATT DB and send indication if enabled for the characteristic// -------------------------------------------------------------------
  uint8_t   htm_ambient_buffer[2];

  htm_ambient_buffer[1] = ambilight;

  ble_data_struct_t* data = getBleDataPtr();


  if (data->connection_status && data->ambient_indication_status){

      //Display ALS value on LCD
      displayPrintf(7,"LIGHT = %d",ambilight);

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_Light_State,0,1,(uint8_t*)&htm_ambient_buffer[1]);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
        }


      sc = sl_bt_gatt_server_send_indication(data->bleconnection,gattdb_Light_State,2,(uint8_t*)&htm_ambient_buffer[0]);

      if (sc != SL_STATUS_OK)
      {
         LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
      }

      data->ambient_inflight_status = true;


  }

}



void dutycycle_indications()
{
  sl_status_t sc;
  uint8_t   htm_dutycycle_buffer[2];

  uint8_t datamod;

  htm_dutycycle_buffer[1] = dutycycle;

  if (buttonstatus == 1 && ble_data.bond_status == true ){

       if ( !ble_data.temp_measure_inflight_status  && !ble_data.pushbutton_inflight_status){
           buttonstatus = 0;

           if (ble_data.pushbutton_indication_status == true){

               datamod = 100 - dutycycle;

               //Display Temperature on LCD
               displayPrintf(8,"Duty Cycle = %d",datamod);

           sc = sl_bt_gatt_server_write_attribute_value(gattdb_dutycycle_state, 0, 1, (uint8_t *)&htm_dutycycle_buffer[1]);

           sc = sl_bt_gatt_server_send_indication(ble_data.bleconnection, gattdb_dutycycle_state,2,(uint8_t*)&(htm_dutycycle_buffer[0]));
           if (sc != SL_STATUS_OK) {
               LOG_ERROR("Errors are while sending indication\n\r");

            }
             ble_data.pushbutton_inflight_status = true;

           }
       }

       else {

          buttonstatus = 0;

          indication temp;
          temp.charHandle = gattdb_dutycycle_state;
          temp.bufferlength = 1;
          temp.buffer[0] = htm_dutycycle_buffer[0];
          temp.buffer[1] = htm_dutycycle_buffer[1];

          cbfifo_enqueue(temp, 1);
          LOG_INFO("ENQUEUED2:\n\r");
       }
  }
}

void transmit_tempdata(){

  sl_status_t sc;

  // Update our local GATT DB and send indication if enabled for the characteristic// -------------------------------------------------------------------
  uint8_t   htm_temperature_buffer[5];   // Stores the temperature data in the Health Thermometer (HTM) format.// format of the buffer is: flags_byte + 4-bytes of IEEE-11073 32-bit float
  uint8_t   *p = htm_temperature_buffer; // Pointer to HTM temperature buffer needed for converting values to bitstream.
  uint32_t  htm_temperature_flt;         // Stores the temperature data read from the sensor in the IEEE-11073 32-bit float format
  uint8_t   flags = 0x00;                // HTM flags set as 0 for Celsius, no time stamp and no temperature type.// "bitstream" refers to the order of bytes and bits sent. byte[0] is sent first, followed by byte[1]...
  UINT8_TO_BITSTREAM(p, flags); // put the flags byte in first, "convert" is a strong word, it places the byte into the buffer// Convert sensor data to IEEE-11073 32-bit floating point format.
  htm_temperature_flt = UINT32_TO_FLOAT(read_data*1000, -3); // Convert temperature to bitstream and place it in the htm_temperature_buffer
  UINT32_TO_BITSTREAM(p, htm_temperature_flt);

  ble_data_struct_t* data = getBleDataPtr();

  if(data->temp_measure_inflight_status || data->temp_type_inflight_status || data->temp_interval_inflight_status || data->pushbutton_inflight_status){

      indication temp;
      temp.charHandle = gattdb_temperature_measurement;
      temp.bufferlength = 5;
      memcpy(temp.buffer, htm_temperature_buffer,sizeof(htm_temperature_buffer));
      cbfifo_enqueue(temp, 1);
      LOG_INFO("-------ENQUEUED-----\n\r");

      return;

  }

  if (data->connection_status && data->temp_measure_indication_status){

      //Display Temperature on LCD
      displayPrintf(DISPLAY_ROW_TEMPVALUE,"Temp=%d",read_data);

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_temperature_measurement,0,5,&htm_temperature_buffer[0]);
      if (sc != SL_STATUS_OK)
        {
          LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
        }


      sc = sl_bt_gatt_server_send_indication(data->bleconnection,gattdb_temperature_measurement,5,&htm_temperature_buffer[0]);

      if (sc != SL_STATUS_OK)
      {
         LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int) sc);
      }

      data->temp_measure_inflight_status = true;


  }

}





void handle_ble_event(sl_bt_msg_t *evt){

  sl_status_t sc;

  switch(SL_BT_MSG_ID(evt->header)){

    case sl_bt_evt_system_boot_id:{

#if (DEVICE_IS_BLE_SERVER == 1)

      char string_buffer[50];

      // Initialize LCD Display
      displayInit();

      displayPrintf(DISPLAY_ROW_NAME,"SERVER");

      sc = sl_bt_system_set_soft_timer(SOFTTIMER_PERIOD,1,false);
      if (sc != SL_STATUS_OK) {
          LOG_ERROR("Error in setting softtimer\n\r");
          break;
      }


      // Extract unique ID from BT Address.
      sc = sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);
      if (sc != SL_STATUS_OK) {
          LOG_ERROR("Error in getting identity\n\r");
          break;
      }

      sprintf(string_buffer,"%x:%x:%x:%x:%x:%x",ble_data.myAddress.addr[0],ble_data.myAddress.addr[1],ble_data.myAddress.addr[2],ble_data.myAddress.addr[3],ble_data.myAddress.addr[4],ble_data.myAddress.addr[5]);
      displayPrintf(DISPLAY_ROW_BTADDR,"%s", string_buffer);

      displayPrintf(DISPLAY_ROW_ASSIGNMENT,"IOT PROJECT");

      //LOG_INFO("\nIdentity Retrieved");
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&ble_data.advertisingSetHandle);
      if (sc != SL_STATUS_OK) {
          LOG_ERROR("\nError in advertising\n\r");
          break;
      }

      // Set advertising interval to 100ms.
       sc = sl_bt_advertiser_set_timing(
             ble_data.advertisingSetHandle, // advertising set handle
              400, // min. adv. interval (milliseconds * 1.6)
              400, // max. adv. interval (milliseconds * 1.6)
              0,   // adv. duration
              0);  // max. num. adv. events
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in setting advertising time\n\r");
           break;
       }

       /*Security Manager*/


       sc = sl_bt_sm_delete_bondings();
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in deleting bonds\n\r");
           break;
       }


       sc = sl_bt_sm_configure(0x0f,sl_bt_sm_io_capability_displayyesno);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in configuring security manager\n\r");
           break;
       }



       sc = sl_bt_advertiser_start(
              ble_data.advertisingSetHandle,
              sl_bt_advertiser_general_discoverable,
              sl_bt_advertiser_connectable_scannable);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in starting advertising\n\r");
           break;
       }

       //DISPLAY_ROW_CONNECTION
       displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");



#endif

      // CLient Implememntation

#if (DEVICE_IS_BLE_SERVER == 0)

       char string_buffer[50];


       // Initialize LCD Display
       displayInit();

       displayPrintf(DISPLAY_ROW_NAME,"CLIENT");


       // Extract unique ID from BT Address.
       sc = sl_bt_system_get_identity_address(&ble_data.myAddress, &ble_data.myAddressType);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in getting identity\n\r");
           break;
       }

       sprintf(string_buffer,"%x:%x:%x:%x:%x:%x",ble_data.myAddress.addr[0],ble_data.myAddress.addr[1],ble_data.myAddress.addr[2],ble_data.myAddress.addr[3],ble_data.myAddress.addr[4],ble_data.myAddress.addr[5]);
       displayPrintf(DISPLAY_ROW_BTADDR,"%s", string_buffer);

       displayPrintf(DISPLAY_ROW_ASSIGNMENT,"IOT PROJECT");




       // Configure for passive scanning
       sc = sl_bt_scanner_set_mode(sl_bt_gap_1m_phy, PASSIVE_SCANNING);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in setting scanning mode\n\r");
           break;
       }

       sc = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy,SCANNING_INTERVAL, SCANNING_WINDOW);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in setting scanning timing\n\r");
           break;
       }


       sc = sl_bt_connection_set_default_parameters(60,60,4,300,0,0xffff);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in setting parameters\n\r");
           break;
       }

       sc = sl_bt_scanner_start(sl_bt_gap_1m_phy,sl_bt_scanner_discover_observation);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in starting scanning\n\r");
           break;
       }

       sc = sl_bt_sm_configure(0x0f,sl_bt_sm_io_capability_displayyesno);
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in configuring security manager\n\r");
           break;
       }

       /*Security Manager*/


       sc = sl_bt_sm_delete_bondings();
       if (sc != SL_STATUS_OK) {
           LOG_ERROR("Error in deleting bonds\n\r");
           break;
       }





       //DISPLAY_ROW_CONNECTION
       displayPrintf(DISPLAY_ROW_CONNECTION,"Discovering");

#endif
      break;
    }

    case sl_bt_evt_connection_opened_id:{

#if (DEVICE_IS_BLE_SERVER == 1)

      ble_data_struct_t* data = getBleDataPtr();
      data->enable_measurement = true;

      displayPrintf(DISPLAY_ROW_CONNECTION,"Connected");

      //LOG_INFO("Connection Open\n\r");

      sc =  sl_bt_advertiser_stop(ble_data.advertisingSetHandle);
      if (sc != SL_STATUS_OK) {
          LOG_ERROR("Error in stopping advertising\n\r");
          break;
      }

      sc = sl_bt_connection_set_parameters ( (evt->data.evt_connection_opened.connection),
      60,
      60,
      4,
      300,
      0,
      0xffff);

      if (sc != SL_STATUS_OK) {

          //LOG_ERROR("Error in setting connection parameters\n\r");
      }

      data->connection_status  = true;
      data->bleconnection  = evt->data.evt_connection_opened.connection;

#endif

#if (DEVICE_IS_BLE_SERVER == 0)

      ble_data_struct_t* data = getBleDataPtr();

      //DISPLAY_ROW_CONNECTION
      displayPrintf(DISPLAY_ROW_CONNECTION,"Connected");

      ble_data.new_connection = evt->data.evt_connection_opened.connection;
      ble_data.newconnection_status =  true;

      // Set event
      schedulerSetConnection_OpenedEvent();

      data->connection_status  = true;

#endif
      break;
    }


    case sl_bt_evt_connection_closed_id:{

#if (DEVICE_IS_BLE_SERVER == 1)

      ble_data_struct_t* data = getBleDataPtr();

      // Disable temperature measurement by switching off temperarture state machine
      data->enable_measurement = false;

      data->connection_status  = false;

      data->bond_status = false;

      gpioLed0SetOff();
      gpioLed1SetOff();


      //LOG_INFO("Connection Closed\n\r");

      displayPrintf(DISPLAY_ROW_CONNECTION,"");
      displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
      displayPrintf(DISPLAY_ROW_PASSKEY,"");
      displayPrintf(DISPLAY_ROW_ACTION,"");
      displayPrintf(DISPLAY_ROW_9,"");
      displayPrintf(DISPLAY_ROW_CONNECTION,"Advertising");

      sc = sl_bt_advertiser_start(
                    ble_data.advertisingSetHandle,
                    sl_bt_advertiser_general_discoverable,
                    sl_bt_advertiser_connectable_scannable);
             if (sc != SL_STATUS_OK) {
                 LOG_ERROR("Error in starting advertising\n\r");
                 break;
             }

      sc = sl_bt_sm_delete_bondings();
              if (sc != SL_STATUS_OK) {
                  LOG_ERROR("Error in deleting bonds\n\r");
                  break;
              }

#endif

#if (DEVICE_IS_BLE_SERVER == 0)

     ble_data_struct_t* data = getBleDataPtr();
     data->connection_status  = false;

     // Set event
     schedulerSetConnection_ClosedEvent();


     displayPrintf(DISPLAY_ROW_CONNECTION,"");
     displayPrintf(DISPLAY_ROW_TEMPVALUE,"");
     displayPrintf(DISPLAY_ROW_BTADDR2,"");
     displayPrintf(DISPLAY_ROW_ACTION,"");
     displayPrintf(DISPLAY_ROW_9,"");
     displayPrintf(DISPLAY_ROW_CONNECTION,"Discovering");

     sc = sl_bt_sm_delete_bondings();
     if (sc != SL_STATUS_OK) {
         LOG_ERROR("Error in deleting bonds\n\r");
         break;
     }


     sc = sl_bt_scanner_start(sl_bt_gap_1m_phy,sl_bt_scanner_discover_generic);
     if (sc != SL_STATUS_OK) {
        LOG_ERROR("Error in starting scanning\n\r");
        break;
     }



#endif

      break;
    }

#if (DEVICE_IS_BLE_SERVER == 1)
    case  sl_bt_evt_connection_parameters_id:{

      /*
      LOG_INFO("Connection params: connection=%d, interval=%d, latency=%d, timeout=%d, securitymode=%d",
               (int) (evt->data.evt_connection_parameters.connection),
               (int) (evt->data.evt_connection_parameters.interval*1.25),
               (int) (evt->data.evt_connection_parameters.latency),
               (int) (evt->data.evt_connection_parameters.timeout*10),
               (int) (evt->data.evt_connection_parameters.security_mode) );
       */

      break;
    }

    case sl_bt_evt_gatt_server_characteristic_status_id:{

      if  (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_Light_State){



               // Check if notifications and indications are not disabled
               if(((sl_bt_gatt_client_config_flag_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags) != sl_bt_gatt_disable){

                   // client characteristic configuration changed by remote GATT client
                   if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                       if (sl_bt_gatt_server_indication == (sl_bt_gatt_server_client_configuration_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags){

                           ble_data_struct_t* data = getBleDataPtr();
                           data->ambient_indication_status = true;
                           gpioLed1SetOn();

                       }

                   }

               }

               else {

                   ble_data_struct_t* data = getBleDataPtr();
                   data->ambient_indication_status = false;
                   gpioLed1SetOff();

                   displayPrintf(DISPLAY_ROW_TEMPVALUE,"");

               }

               if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                        ble_data_struct_t* data = getBleDataPtr();
                        data->ambient_inflight_status = false;

               }
           }



      if  (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement){



          // Check if notifications and indications are not disabled
          if(((sl_bt_gatt_client_config_flag_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags) != sl_bt_gatt_disable){

              // client characteristic configuration changed by remote GATT client
              if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                  if (sl_bt_gatt_server_indication == (sl_bt_gatt_server_client_configuration_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags){

                      ble_data_struct_t* data = getBleDataPtr();
                      data->temp_measure_indication_status = true;
//                      gpioLed0SetOn();

                  }

              }

          }

          else {

              ble_data_struct_t* data = getBleDataPtr();
              data->temp_measure_indication_status = false;
//              gpioLed0SetOff();

              displayPrintf(DISPLAY_ROW_TEMPVALUE,"");

          }

          if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                   ble_data_struct_t* data = getBleDataPtr();
                   data->temp_measure_inflight_status = false;

          }
      }

      if  (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_dutycycle_state){

          // Check if notifications and indications are not disabled
                  if(((sl_bt_gatt_client_config_flag_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags) != sl_bt_gatt_disable){

                      // client characteristic configuration changed by remote GATT client
                      if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                          if (sl_bt_gatt_server_indication == (sl_bt_gatt_server_client_configuration_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags){

                              ble_data_struct_t* data = getBleDataPtr();
                              data->pushbutton_indication_status = true;
//                              gpioLed1SetOn();

                          }

                      }

                  }

                  else {

                      ble_data_struct_t* data = getBleDataPtr();
                      data->pushbutton_indication_status = false;
//                      gpioLed1SetOff();

                      displayPrintf(DISPLAY_ROW_9,"");

                  }

                  if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                           ble_data_struct_t* data = getBleDataPtr();
                           data->pushbutton_inflight_status = false;

                  }
      }

      if  (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_type ){

          // Check if notifications and indications are not disabled
         if(((sl_bt_gatt_client_config_flag_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags) != sl_bt_gatt_disable){

             // client characteristic configuration changed by remote GATT client
             if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                 if (sl_bt_gatt_server_indication == (sl_bt_gatt_server_client_configuration_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags){

                     ble_data_struct_t* data = getBleDataPtr();
                     data->temp_type_indication_status = true;

                 }

             }

         }

         else {

             ble_data_struct_t* data = getBleDataPtr();
             data->temp_type_indication_status = false;

         }

         if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                  ble_data_struct_t* data = getBleDataPtr();
                  data->temp_type_inflight_status = false;

         }

      }

      if  (evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_measurement_interval){

          // Check if notifications and indications are not disabled
         if(((sl_bt_gatt_client_config_flag_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags) != sl_bt_gatt_disable){

             // client characteristic configuration changed by remote GATT client
             if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                 if (sl_bt_gatt_server_indication == (sl_bt_gatt_server_client_configuration_t)evt->data.evt_gatt_server_characteristic_status.client_config_flags){

                     ble_data_struct_t* data = getBleDataPtr();
                     data->temp_interval_indication_status = true;

                 }

             }

         }

         else {

             ble_data_struct_t* data = getBleDataPtr();
             data->temp_interval_indication_status = false;

         }

         if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)evt->data.evt_gatt_server_characteristic_status.status_flags) {

                  ble_data_struct_t* data = getBleDataPtr();
                  data->temp_interval_inflight_status = false;

         }

      }
      break;
    }


    case sl_bt_evt_gatt_server_indication_timeout_id:{

      ble_data_struct_t* data = getBleDataPtr();
      if (data->temp_measure_inflight_status == true){

          LOG_ERROR("Error in indication\n\r");
      }

      if (data->temp_type_inflight_status == true){

          LOG_ERROR("Error in indication\n\r");
      }
      else if (data->temp_interval_inflight_status == true){

           LOG_ERROR("Error in indication\n\r");
       }


      break;
    }

    case sl_bt_evt_system_soft_timer_id:{

      ble_data_struct_t* data = getBleDataPtr();

      if((data->connection_status == true) && evt->data.evt_system_soft_timer.handle == 1){

          if(ble_data.pushbutton_indication_status  && !ble_data.pushbutton_inflight_status)
           {
              dutycycle_indications();
           }

          if(data->temp_measure_inflight_status == false && data->pushbutton_inflight_status == false  && (cbfifo_length() != 0)){

              indication extracted;
              cbfifo_dequeue(&extracted, 1);

              LOG_INFO("DEQUED:%d ,%d\n\r",extracted.charHandle, extracted.bufferlength);

              sc = sl_bt_gatt_server_send_indication(data->bleconnection, extracted.charHandle, extracted.bufferlength, &(extracted.buffer[0]));
              if (sc != SL_STATUS_OK) {
                 LOG_ERROR("Error in sending indication - soft timer\n\r");
                 break;
              }

              else {

                  if (extracted.charHandle == gattdb_dutycycle_state){

                      data->pushbutton_inflight_status = true;
                  }

                  else if (extracted.charHandle == gattdb_temperature_measurement){

                      data->temp_measure_inflight_status = true;
                  }
              }
          }
      }

      else {

          displayUpdate();

      }

      break;

    }

    case sl_bt_evt_sm_confirm_bonding_id:{

      sc = sl_bt_sm_bonding_confirm(evt->data.evt_sm_confirm_bonding.connection,0x01);

      if (sc != SL_STATUS_OK) {
         LOG_ERROR("Error in confirming bonding\n\r");
         break;
      }

      ble_data.bond_connection = evt->data.evt_sm_confirm_bonding.connection;

      break;
    }

    case sl_bt_evt_sm_confirm_passkey_id:{

      displayPrintf(DISPLAY_ROW_PASSKEY,"%d",evt->data.evt_sm_confirm_passkey.passkey);
      displayPrintf(DISPLAY_ROW_ACTION,"Confirm with PB0");


      break;
    }

    case sl_bt_evt_system_external_signal_id:{


      if( evt->data.evt_system_external_signal.extsignals == evtPushbuttonPB0PressEvent){

          displayPrintf(DISPLAY_ROW_9,"Button Pressed");
          buttonstatus = 1;
          buttonvalue=1;
          buttonvalue_buffer[1] = 1;

          if (ble_data.bond_status == false){
            sc = sl_bt_sm_passkey_confirm(ble_data.bond_connection,1);
            if (sc != SL_STATUS_OK) {
               LOG_ERROR("Error in confirming passkey\n\r");
               break;
            }
          }

          sc = sl_bt_gatt_server_write_attribute_value(gattdb_dutycycle_state, 0, 1, (uint8_t *)&buttonvalue_buffer[1]);

      }

      else if( evt->data.evt_system_external_signal.extsignals == evtPushbuttonPB0ReleaseEvent){

          displayPrintf(DISPLAY_ROW_9,"Button Released");
          buttonstatus = 1;
          buttonvalue=0;
          buttonvalue_buffer[1] = 0;

          sc = sl_bt_gatt_server_write_attribute_value(gattdb_dutycycle_state, 0, 1, (uint8_t *)&buttonvalue_buffer[1]);


      }


      break;
    }

    case sl_bt_evt_sm_bonded_id:{

      displayPrintf(DISPLAY_ROW_CONNECTION,"Bonded");
      displayPrintf(DISPLAY_ROW_ACTION,"");
      displayPrintf(DISPLAY_ROW_9,"");
      displayPrintf(DISPLAY_ROW_PASSKEY,"");
      ble_data.bond_status = true;

      break;
    }

    case sl_bt_evt_sm_bonding_failed_id:{

      LOG_ERROR("Error in bonding\n\r");
      displayPrintf(DISPLAY_ROW_PASSKEY,"");
      displayPrintf(DISPLAY_ROW_ACTION,"");
      displayPrintf(DISPLAY_ROW_CONNECTION, "bonding failed");

      ble_data.bond_status = false;

    }

#endif


//
#if (DEVICE_IS_BLE_SERVER == 0)

    case sl_bt_evt_scanner_scan_report_id:{

      char server_string[50];

      bd_addr server_addr = SERVER_BT_ADDRESS;
      ble_data.serverAddress = server_addr;

      uint8_t compare_check = 0;
      for(int i=0;i<6;i++){

          if (ble_data.serverAddress.addr[i] == evt->data.evt_scanner_scan_report.address.addr[i]){

              compare_check++;

          }

      }

      if (compare_check == 6){

          sc =  sl_bt_scanner_stop();

          if (sc != SL_STATUS_OK) {
              LOG_ERROR("Error in stopping scanner\n\r");
              break;
          }



          sprintf(server_string,"%x:%x:%x:%x:%x:%x",ble_data.serverAddress.addr[0],ble_data.serverAddress.addr[1],ble_data.serverAddress.addr[2],ble_data.serverAddress.addr[3],ble_data.serverAddress.addr[4],ble_data.serverAddress.addr[5]);
          displayPrintf(DISPLAY_ROW_BTADDR2,"%s", server_string);

          // address
          sc = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                                evt->data.evt_scanner_scan_report.address_type,
                                                sl_bt_gap_1m_phy,
                                                NULL);

          if (sc != SL_STATUS_OK) {
              LOG_ERROR("Error in opening connection\n\r");
              break;
          }

      }

      break;
    }

      case sl_bt_evt_sm_confirm_bonding_id:{

         sc = sl_bt_sm_bonding_confirm(evt->data.evt_sm_confirm_bonding.connection,0x01);

         if (sc != SL_STATUS_OK) {
            LOG_ERROR("Error in confirming bonding\n\r");
            break;
         }

         ble_data.bond_connection = evt->data.evt_sm_confirm_bonding.connection;

         break;
       }

     case sl_bt_evt_sm_confirm_passkey_id:{

       displayPrintf(DISPLAY_ROW_PASSKEY,"%d",evt->data.evt_sm_confirm_passkey.passkey);
       displayPrintf(DISPLAY_ROW_ACTION,"Confirm with PB0");


       break;
     }



     case sl_bt_evt_sm_bonded_id:{

       displayPrintf(DISPLAY_ROW_CONNECTION,"Bonded");
       displayPrintf(DISPLAY_ROW_ACTION,"");
       displayPrintf(DISPLAY_ROW_9,"");
       displayPrintf(DISPLAY_ROW_PASSKEY,"");
       ble_data.bond_status = true;

       break;
     }

     case sl_bt_evt_sm_bonding_failed_id:{

       LOG_ERROR("Error in bonding\n\r");
       displayPrintf(DISPLAY_ROW_PASSKEY,"");
       displayPrintf(DISPLAY_ROW_ACTION,"");
       displayPrintf(DISPLAY_ROW_CONNECTION, "bonding failed");

       ble_data.bond_status = false;

       break;

     }


    case  sl_bt_evt_gatt_service_id:{

      if(evt->data.evt_gatt_service.uuid.data[0] ==  thermo_service[0] && evt->data.evt_gatt_service.uuid.data[1] ==  thermo_service[1]  ){

         ble_data.temperature_service = evt->data.evt_gatt_service.service;

      }


     uint8_t compare_check = 0;


     for(int i=0;i<16;i++){

         if ((evt->data.evt_gatt_service.uuid.data[i]) == dutycycle_service[i] ){

             compare_check++;

         }
     }


      if (compare_check == 16)  ble_data.button_service = evt->data.evt_gatt_service.service;

      uint8_t ambient_check = 0;

      for(int i=0;i<16;i++){

          if ((evt->data.evt_gatt_service.uuid.data[i]) == ambient_service[i] ){

              ambient_check++;

          }
      }


       if (ambient_check == 16)  ble_data.ambient_service = evt->data.evt_gatt_service.service;



      break;
    }


    case  sl_bt_evt_gatt_characteristic_id:{

      if(evt->data.evt_gatt_characteristic.uuid.data[0] ==  thermo_char[0] && evt->data.evt_gatt_characteristic.uuid.data[1] ==  thermo_char[1]  ){

          ble_data.temperature_characteristic = evt->data.evt_gatt_characteristic.characteristic;

      }

      uint8_t compare_check = 0;


      for(int i=0;i<16;i++){

          if ((evt->data.evt_gatt_characteristic.uuid.data[i]) == dutycycle_char[i] ){

                  compare_check++;

              }
        }


       if (compare_check == 16) ble_data.button_characteristic = evt->data.evt_gatt_characteristic.characteristic;


       uint8_t ambient_check = 0;


       for(int i=0;i<16;i++){

           if ((evt->data.evt_gatt_characteristic.uuid.data[i]) == ambient_char[i] ){

                   ambient_check++;

               }
         }


        if (ambient_check == 16) ble_data.ambient_characteristic = evt->data.evt_gatt_characteristic.characteristic;


      break;
    }


    case sl_bt_evt_gatt_procedure_completed_id:{

      ble_data.procedure_completion = evt->data.evt_gatt_procedure_completed.connection;



      // error in bonding
      if(evt->data.evt_gatt_procedure_completed.result == 0x110f ){

          sc = sl_bt_sm_increase_security(ble_data.new_connection);

          if (sc != SL_STATUS_OK) {
              LOG_ERROR("Error in opening connection\n\r");
              break;
          }
      }

      else {


          // Set Event
          schedulerSetProcedure_CompletedEvent();

      }

      break;
    }


    case sl_bt_evt_gatt_characteristic_value_id:{

      if ( evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication && evt->data.evt_gatt_characteristic_value.characteristic == ble_data.temperature_characteristic){

          uint8_t* data = &(evt->data.evt_gatt_characteristic_value.value.data[0]);

          int32_t temperature = FLOAT_TO_INT32(data);
          LOG_INFO("TEMP====%d",temperature);

          //Display Temperature on LCD
          displayPrintf(DISPLAY_ROW_TEMPVALUE,"Temp=%d",temperature);

          sc = sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
          if (sc != SL_STATUS_OK) {
              LOG_ERROR("Error in sending confirmation\n\r");
              break;
          }

      }

      if ( evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response && evt->data.evt_gatt_characteristic_value.characteristic == ble_data.button_characteristic){

          if ( evt->data.evt_gatt_characteristic_value.value.data[0]) displayPrintf(DISPLAY_ROW_9,"Button Pressed");

          else  displayPrintf(DISPLAY_ROW_9,"Button Released");


      }

      if ( evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication && evt->data.evt_gatt_characteristic_value.characteristic == ble_data.button_characteristic){

          uint8_t datamod = 100 - evt->data.evt_gatt_characteristic_value.value.data[1];

          displayPrintf(DISPLAY_ROW_9, "Duty Cycle: %d",datamod);

          if(evt->data.evt_gatt_characteristic_value.value.data[1] == 10)
            {
              //compareset for duty cycle 10
              LETIMER_CompareSet(LETIMER0, 1,
                     CMU_ClockFreqGet(cmuClock_LETIMER0) * 10 / (1 * 100));
            }
          else if(evt->data.evt_gatt_characteristic_value.value.data[1] == 80)
          {
              //compare set for duty cycle 80
              LETIMER_CompareSet(LETIMER0, 1,
                     CMU_ClockFreqGet(cmuClock_LETIMER0) * 80 / (1 * 100));
          }

          sc = sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
          if (sc != SL_STATUS_OK) {
              LOG_ERROR("Error in sending confirmation\n\r");
              break;
          }

      }

      if ( evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication && evt->data.evt_gatt_characteristic_value.characteristic == ble_data.ambient_characteristic){

          // LOG_INFO("received light\n\r");
          int light = (evt->data.evt_gatt_characteristic_value.value.data[1]);


          //Display Temperature on LCD
          displayPrintf(DISPLAY_ROW_TEMPVALUE,"LIGHT=%d",light);


          sc = sl_bt_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
          if (sc != SL_STATUS_OK) {
              LOG_ERROR("Error in sending confirmation\n\r");
              break;
          }

      }



      break;
    }

    case sl_bt_evt_system_external_signal_id:{

      if ( evt->data.evt_system_external_signal.extsignals == evtPushbuttonPB0PressEvent){


          if(!ble_data.bond_status){

              sc = sl_bt_sm_passkey_confirm(ble_data.new_connection,1);
              if (sc != SL_STATUS_OK) {
                 LOG_ERROR("Error in confirming passkey\n\r");
                 break;
              }

              displayPrintf(DISPLAY_ROW_PASSKEY,"");
              displayPrintf(DISPLAY_ROW_ACTION,"");

          }

              else{

                  ble_data.bond_status = true;
                  button_sequence[0] =  1;

              }

       }

      else  if ( evt->data.evt_system_external_signal.extsignals == evtPushbuttonPB0ReleaseEvent){

          if (button_sequence[0] == 1 && button_sequence[1] == 1 && button_sequence[2] == 1){

              button_sequence[0] = 0;
              button_sequence[1] = 0;
              button_sequence[2] = 0;

              if (!ble_data.indications_enabled){

                  sc = sl_bt_gatt_set_characteristic_notification(ble_data.new_connection,ble_data.button_characteristic,2);
                  if (sc != SL_STATUS_OK) {
                      LOG_ERROR("Error in sending confirmation\n\r");
                      break;
                  }

                  ble_data.indications_enabled = !ble_data.indications_enabled;

                  }

              else {

                  sc = sl_bt_gatt_set_characteristic_notification(ble_data.new_connection,ble_data.button_characteristic,0);
                  if (sc != SL_STATUS_OK) {
                      LOG_ERROR("Error in sending confirmation\n\r");
                      break;
                  }

                  ble_data.indications_enabled = !ble_data.indications_enabled;

                  }

          }

      }


    else if ( evt->data.evt_system_external_signal.extsignals == evtPushbuttonPB1PressEvent) {

        if ( button_sequence[0] ==  1){
            button_sequence[1] = 1;
            break;
        }


         sc  = sl_bt_gatt_read_characteristic_value(ble_data.new_connection,ble_data.button_characteristic);
         if (sc != SL_STATUS_OK) {
             LOG_ERROR("Error in reading characteristic value\n\r");
             break;
         }


    }


    else if ( evt->data.evt_system_external_signal.extsignals == evtPushbuttonPB1ReleaseEvent) {

        if (button_sequence[0] == 1 && button_sequence[1] == 1 ) button_sequence[2] = 1;

    }

      break;
    }

#endif
  }

}
