/* 
 * File:   ram.h
 * Author: michele.abelli
 *
 * Created on 12 marzo 2020, 11.51
 */

#include "typedef.h"
#include <xc.h>
#ifdef RAM_EXTERN_DISABLE
#   define RAM_EXTERN
#else
#   define RAM_EXTERN extern
#endif
/*****************************************************************************/

/* ***************************** ADDRESS SETUP ***************************** */
#if ! defined RAM_EXTERN_DISABLE
/* decl */ extern unsigned char slave_id;

/* Assigning a fixed ID to begin with */
#else
/* def */  unsigned char slave_id

#    if defined COLOR_1
= B1_BASE_ID;

#  elif defined COLOR_2
= B2_BASE_ID;

#  elif defined COLOR_3
= B3_BASE_ID;

#  elif defined COLOR_4
= B4_BASE_ID;

#  elif defined COLOR_5
= B5_BASE_ID;

#  elif defined COLOR_6
= B6_BASE_ID;

#  elif defined COLOR_7
= B7_BASE_ID;

#  elif defined COLOR_8
= B8_BASE_ID;

#  elif defined COLOR_9
= C1_COLOR_ID;

#  elif defined COLOR_10
= C2_COLOR_ID;

#  elif defined COLOR_11
= C3_COLOR_ID;

#  elif defined COLOR_12
= C4_COLOR_ID;

#  elif defined COLOR_13
= C5_COLOR_ID;

#  elif defined COLOR_14
= C6_COLOR_ID;

#  elif defined COLOR_15
= C7_COLOR_ID;

#  elif defined COLOR_16
= C8_COLOR_ID;

#  elif defined COLOR_17
= C9_COLOR_ID;

#  elif defined COLOR_18
= C10_COLOR_ID;

#  elif defined COLOR_19
= C11_COLOR_ID;

#  elif defined COLOR_20
= C12_COLOR_ID;

#  elif defined COLOR_21
= C13_COLOR_ID;

#  elif defined COLOR_22
= C14_COLOR_ID;

#  elif defined COLOR_23
= C15_COLOR_ID;

#  elif defined COLOR_24
= C16_COLOR_ID;

#  elif defined AXIS_X
= MOVE_X_AXIS_ID;

#  elif defined AXIS_Y
= MOVE_Y_AXIS_ID;

#  elif defined CONTAINER_1
= STORAGE_CONTAINER1_ID;

#  elif defined CONTAINER_2
= STORAGE_CONTAINER2_ID;

#  elif defined CONTAINER_3
= STORAGE_CONTAINER3_ID;

#  elif defined CONTAINER_4
= STORAGE_CONTAINER4_ID;

#  elif defined COVER_1
= PLUG_COVER_1_ID;

#  elif defined COVER_2
= PLUG_COVER_2_ID;

#  elif (defined AUTOCAP_LIFTER || defined AUTOCAP_NOLIFTER)
= AUTOCAP_ID;

# elif defined _SGABELLO
= SGABELLO;

# elif defined _HUMIDIFIER
= HUMIDIFIER;

# elif defined _TINTING
= TINTING;

#  else
//#  error Universal address not yet supported.
= UNIVERSAL_ID; /* 0 is the universal address, the actuator will take
                 * the address from the dip-switches in R1. R0 does
                 * not support universal addresses.*/
#  endif

#endif /* ! defined RAM_EXTERN_DISABLE */

RAM_EXTERN Stepper_Status Status_Board_Mixer,Status_Board_Door, Status_Board_Autocap;
RAM_EXTERN status_t Status,Mixer,Humidifier,Autocap;
RAM_EXTERN status_t NextStatus,NextMixer,NextHumidifier;
RAM_EXTERN TintingAct_t TintingAct;
RAM_EXTERN PeripheralAct_t PeripheralAct;
RAM_EXTERN unsigned short Start_Jump_Boot;
// Humidifier
RAM_EXTERN unsigned int Status_I2C;
RAM_EXTERN unsigned char Start_New_Measurement;
RAM_EXTERN unsigned char Sensor_Measurement_Error;
RAM_EXTERN unsigned char Start_New_Temp_Measurement;
RAM_EXTERN unsigned char Sensor_Temp_Measurement_Error;
RAM_EXTERN unsigned char dutyPWM;
RAM_EXTERN unsigned char contaDuty;
RAM_EXTERN unsigned char Check_Presence;
RAM_EXTERN unsigned char Dos_Temperature_Count_Disable_Err;

RAM_EXTERN unsigned long SHT31_Temperature;
RAM_EXTERN unsigned long SHT31_Humidity;
RAM_EXTERN unsigned long TC72_Temperature;

RAM_EXTERN unsigned long Process_Period;

RAM_EXTERN unsigned char Humidifier_Enable;

RAM_EXTERN unsigned char Dos_Temperature_Enable;
RAM_EXTERN unsigned char Reference;
RAM_EXTERN unsigned char Set_Home_pos;
RAM_EXTERN unsigned char Tr_Light_Dark_1;
RAM_EXTERN unsigned char Old_Photocell_sts_1;
RAM_EXTERN DigInStatusType OutputFilter;

RAM_EXTERN unsigned char Mixer_Motor_High_Current_Setted;
RAM_EXTERN unsigned char indicator;   

RAM_EXTERN unsigned char Autocap_Enabled;

/**
 * EEPROM management
 */

RAM_EXTERN unsigned char eeprom_write_result;
RAM_EXTERN unsigned char eeprom_byte;
RAM_EXTERN unsigned char eeprom_read_result;
RAM_EXTERN unsigned char eeprom_retries;

RAM_EXTERN unsigned short offset;
RAM_EXTERN unsigned short startAddress;

RAM_EXTERN unsigned long Timer_Old, Timer_New, Cycle_Duration, MAX_Cycle_Duration;

RAM_EXTERN union {
  unsigned char byte;
  struct {
    unsigned char unused : 8;	
	};
} InitFlags;
        
RAM_EXTERN signed long pippo, pippo1, pippo2, pippo3, pippo4, pippo5, pippo6, pippo7, pippo8, pippo9, pippo10;
RAM_EXTERN unsigned long pippo11, pippo12;
