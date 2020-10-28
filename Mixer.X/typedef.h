/* 
 * File:   typedef.h
 * Author: michele.abelli
 *
 * Created on 12 marzo 2020, 10.42
 */

#ifndef TYPEDEF_H
#define	TYPEDEF_H

/*========================== DEFINIZIONI GENERALI DI TIPO ================== */
typedef union {
  unsigned char bytes[2];
  unsigned short word;

  struct {
    /* User-operated inputs */
    unsigned short  StatusType0       : 1;  //LEV_SENS
    unsigned short  StatusType1       : 1;  //FO_CPR = Jar Presence
    unsigned short  StatusType2       : 1;  //FO_VALV = Autocap Open
    unsigned short  StatusType3       : 1;  //FO_ACC = Autocap Lifter Down
    unsigned short  StatusType4       : 1;  //FO_BRD = Door Open
    unsigned short  StatusType5       : 1;  //FO_HOME = Mixer Home
    unsigned short  StatusType6       : 1;  //FO_GEN1 = Jar Presence
    unsigned short  StatusType7       : 1;  //FO_GEN2
    unsigned short  StatusType8       : 1;  //INT_CAR = Door Closed
    unsigned short  StatusType9       : 1;  //INT_PAN
    unsigned short  StatusType10      : 1;  //IO_GEN1
    unsigned short  StatusType11      : 1;  //FO_GEN2
    unsigned short  StatusType12      : 1;  //BUTTON
    unsigned short  StatusType13      : 1;
    unsigned short  StatusType14      : 1; 
    unsigned short  StatusType15      : 1; 
} Bit;

  struct {
    unsigned char low;
    unsigned char high;
  } byte;
} DigInStatusType;

/*!  Union   */
typedef union __attribute__ ((packed))
{
  unsigned short allflags;
  struct
  {
    unsigned short unused:16;
  } bit;
} statusFlags_t;


typedef union __attribute__ ((packed))
{
  unsigned short uword;
  signed short   sword;
  unsigned char  byte[2];
} unionWord_t;


typedef union __attribute__ ((packed))
{
  unsigned long  udword;
  signed long    sdword;
  unsigned short word[2];
  unsigned char  byte[4];
} unionDWord_t;

typedef struct
{
  unsigned char level;
  unsigned char phase;
  unsigned char step;
  unsigned char errorCode;
} status_t;

typedef union {
  unsigned char bytes[2];
  unsigned short word;

  struct {
    /* User-operated inputs */
    unsigned short  HiZ       : 1;      
    unsigned short  BUSY      : 1;  
    unsigned short  SW_F      : 1;  
    unsigned short  SW_EVN    : 1;  
    unsigned short  DIR       : 1;  
    unsigned short  MOT_STATUS: 2;  
    unsigned short  CMD_ERROR : 1;      
    unsigned short  STCK_MOD  : 1;  
    unsigned short  UVLO      : 1;  
    unsigned short  UVLO_ADC  : 1;
    unsigned short  TH_STATUS : 2; 
    unsigned short  OCD       : 1;     
    unsigned short  unused    : 2; 
  } Bit;

} Stepper_Status;

typedef struct
{
  unsigned char Action;
  union __attribute__ ((packed)) PeripheralTypes_t
  {
	unsigned char bytePeripheral;
	
	struct {
      unsigned char WaterPump		 : 1;
      unsigned char Nebulizer_Heater : 1;
      unsigned char HeaterResistance : 1;
      unsigned char MixerMotor       : 1;      
      unsigned char DoorMotor        : 1;      
      unsigned char Unused           : 3;      
	};	  
  } Peripheral_Types;
} PeripheralAct_t;

typedef struct
{
  unsigned char typeMessage;

  // tinting command 
  union __attribute__ ((packed))
  {
    unsigned char cmd;
    struct
    {
      unsigned char tinting_stop : 1;
      unsigned char tinting_intr : 1;
      unsigned char tinting_home : 1;
      unsigned char packing : 1;
      unsigned char open    : 1;
      unsigned char close   : 1;
      unsigned char extend  : 1;
      unsigned char retract : 1;
    };
  } command;


  union __attribute__ ((packed)) TintingFlags_t
  {
    unsigned long allFlags;
    unsigned char byteFlags[4];
    unsigned short wordFlags[2];
	
    struct {
      // octet 1 
      unsigned char tinting_stopped		: 1;
      unsigned char tinting_ready       : 1;
      unsigned char tinting_homing      : 1;
      unsigned char tinting_supply_run  : 1;
      unsigned char tinting_supply_end  : 1;
      unsigned char tinting_peripheral_on : 1;
      unsigned char tinting_mixer_setup_output : 1;
      unsigned char tinting_mixer_test  : 1;
      // octet 2 
	  unsigned char tinting_bad_humidifier_par_error  : 1;	  
	  unsigned char tinting_tout_error        : 1;	  
      unsigned char tinting_RH_error          : 1;
      unsigned char tinting_Temperature_error : 1;
      unsigned char tinting_mixer_software_error    : 1;
      unsigned char tinting_mixer_homing_pos_error	: 1;	  
	  unsigned char tinting_mixer_reset_error       : 1;
	  unsigned char tinting_mixer_jar_photo_read_light_error : 1;
      // octet 3 
	  unsigned char tinting_mixer_jar_photo_read_dark_error	: 1;
	  unsigned char tinting_mixer_photo_read_light_error	: 1;
      unsigned char tinting_mixer_photo_read_dark_error     : 1;
      unsigned char tinting_bad_mixer_param_error : 1;
      unsigned char tinting_mixer_test_error : 1;
	  unsigned char tinting_jump_to_boot    : 1;	  
	  unsigned char tinting_hum_par_rx      : 1; 
      unsigned char unused : 1;      	  
      // octet 4 
      unsigned char tinting_bad_peripheral_param_error : 1;
      unsigned char tinting_jar_motor_run : 1;
      unsigned char tinting_jar_motor_end : 1;
      unsigned char tinting_mixer_motor_thermal_shutdown_error : 1;
      unsigned char tinting_mixer_motor_under_voltage_error : 1;
      unsigned char tinting_mixer_motor_overcurrent_error : 1;      
      unsigned char tinting_mixer_motor_blocked_error : 1;            
      unsigned char tinting_door_motor_blocked_error : 1;            
   };
  } TintingFlags;
  
  union __attribute__ ((packed)) TintingFlags_1_t
  {
    unsigned long allFlags;
    unsigned char byteFlags[4];
    unsigned short wordFlags[2];
	
    struct {
      // octet 1
      unsigned char tinting_door_motor_thermal_shutdown_error : 1;
      unsigned char tinting_door_motor_under_voltage_error : 1;
      unsigned char tinting_door_motor_overcurrent_error : 1;      
      unsigned char tinting_mixer_door_microswitch_open_error: 1;
      unsigned char tinting_mixer_door_microswitch_close_error: 1;
      unsigned char tinting_mixer_door_open_photo_read_light_error: 1;
      unsigned char tinting_mixer_door_open_photo_read_dark_error: 1;
      unsigned char tinting_mixer_timeout_jar_presence_error: 1;
      // octet 2 
      unsigned char tinting_mixer_motor_high_current_not_setted: 1;
      unsigned char unused2 : 7;      
      // octet 3 
      unsigned char unused3 : 8;      
      // octet 4 
      unsigned char unused4 : 8;            
   };
  } TintingFlags_1;
  
  // Humidifier  
  // 0 --> Humidifier Disabled
  // 1 --> Humidifier Enabled
  unsigned char Humidifier_Enable;
  // Type of R/H Sensor: 
  // 0 --> Sensirion SHT30
  // 1 --> ..................
  unsigned char Humdifier_Type;
  // PWM value on HUMIDIFIER_TYPE_2
  unsigned char Humidifier_PWM;
  // Process Period (sec))
  unsigned long Humidifier_Period;
  // Multiplier Coefficient
  unsigned long Humidifier_Multiplier;
  // Nebulizer Duration with Autocap Open (sec)
  // 0 --> Nebulizer always OFF
  unsigned long AutocapOpen_Duration;
  // Nebulizer Period with Autocap Open (sec)
  // 0 --> Nebulizer always ON
  unsigned long AutocapOpen_Period;
  // 0 --> Humidifier Disabled
  // 1 --> Humidifier Enabled
  unsigned char Temp_Enable;
  // Type of R/H Sensor: 
  // 0 --> Sensirion SHT30
  // 1 --> ..................
  unsigned char Temp_Type;
  // Dosing Temperature Period (sec)  
  unsigned long Temp_Period;
  // Low Temperature Threshold (°C) 
  unsigned char Temp_T_LOW;
  // High Temperature Threshold (°C) 
  unsigned char Temp_T_HIGH; 
  // Heater Temperature Threshold (°C) 
  unsigned char Heater_Temp;
  // Hystersis Interval (°C)
  unsigned char Heater_Hysteresis;

  unsigned char Autocap_Status;
  unsigned char Machine_Motors;
  unsigned char Check_Jar_presence;
  unsigned char Check_Door_Open;
  
  unsigned long Temperature;
  unsigned long RH;  
  unsigned long Dosing_Temperature;
  unsigned char WaterLevel_state;
  
  // Peripherals State
  unsigned char DoorMotor_state;
  unsigned char MixerMotor_state;
  unsigned char WaterPump_state;
  unsigned char Nebulizer_Heater_state;
  unsigned char HeaterResistance_state;  
  
  // Home Photocell status
  unsigned char Home_photocell;
  // Jar Photocell status
  unsigned char Jar_photocell;
  // Door Microswitch status
  unsigned char Door_Microswitch;
  // Door Open Photocell status
  unsigned char DoorOpen_Photocell;
  // Jar presence = end mixing
  unsigned char Jar_presence;
  // SetHighCurrent + Mixing + DoorOpen State --> 0 = Not Actrive,  1 = Active 2 = END Successfully
  unsigned char SetHighCurrent_Mixing_DoorOpen_state;
  // Autocap Open status
  unsigned char AutocapOpen_Photocell;
  // Autocap Lifter Down status
  unsigned char AutocapLifterDown_Photocell;
  // Photocells status 
  unsigned char Photocells_state;  
  // Tipo di Uscita
  unsigned char Output_Type;
  // Enable/Disable Output
  unsigned char Output_Act;  

  unsigned long  Mixer_Homimg_Speed; 
  unsigned long  Mixer_Door_Homimg_Speed; 

  unsigned char  Homing_type;
 
  unsigned char  Autocap_Enable; 
  unsigned char  Motor_Enable;
  unsigned char  Door_Enable;
  unsigned long  Time_Door_Open;
  unsigned short Mixer_N_cycle;
  unsigned short Mixer_Speed[10];
  unsigned short Mixer_Duration[10];
  unsigned char  Mixer_Direction[10];  
  
  // Autocap parameters
  unsigned long n_step_homing;
  unsigned short n_step_move;
  unsigned short speed_move;
  unsigned long  Autocap_Homimg_Speed;

} TintingAct_t;

typedef struct
{
  unsigned char Humidifier_Enable;
  unsigned char Humdifier_Type;
  unsigned char Humidifier_PWM;
  unsigned long Humidifier_Period;
  unsigned long Humidifier_Multiplier;
  unsigned long AutocapOpen_Duration;
  unsigned long AutocapOpen_Period;
  unsigned char Temp_Enable;
  unsigned char Temp_Type;
  unsigned long Temp_Period;
  unsigned char Temp_T_LOW;
  unsigned char Temp_T_HIGH; 
  unsigned char Heater_Temp;
  unsigned char Heater_Hysteresis;
} Humidifier_t;

#endif	/* TYPEDEF_H */

