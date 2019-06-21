/* 
 * File:   ram.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 13.20
 */

#include "typedef.h"
#include <xc.h>
#include "define.h"
#include "serialCom.h"
#include "mem.h"
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

#ifndef SKIP_FAULT_1
RAM_EXTERN unsigned char fault_1_state;
#endif

/**
 * Versioning data
 */
RAM_EXTERN unsigned long slaves_sw_versions[N_SLAVES];
RAM_EXTERN unsigned long slaves_boot_versions[N_SLAVES];

RAM_EXTERN unsigned char RicirculationCmd, PositioningCmd, End_Table_Position, Stirring_Method, Last_Circ;
RAM_EXTERN Stepper_Status Status_Board_Pump,Status_Board_Valve, Status_Board_Table;
RAM_EXTERN status_t Status,Pump,Table,Humidifier, MachineStatus;
RAM_EXTERN status_t NextStatus,NextPump,NextTable,NextHumidifier;
RAM_EXTERN TintingAct_t TintingAct;
RAM_EXTERN CircStepPosAct_t CircStepPosAct;
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

RAM_EXTERN unsigned long SHT31_Temperature;
RAM_EXTERN unsigned long SHT31_Humidity;
RAM_EXTERN unsigned long TC72_Temperature;

RAM_EXTERN uint16_t SHT31_DeviceAddress;

RAM_EXTERN unsigned long Process_Period;

RAM_EXTERN unsigned char Humidifier_Enable;
RAM_EXTERN unsigned char Check_Neb_Error, Check_Neb_Timer;
RAM_EXTERN TintingHumidifier_t TintingHumidifierWrite, TintingHumidifier;
RAM_EXTERN TintingPump_t TintingPumpWrite, TintingPump;
RAM_EXTERN TintingTable_t TintingTableWrite, TintingTable;
RAM_EXTERN TintingCleaning_t TintingCleanWrite;

RAM_EXTERN unsigned char Dos_Temperature_Enable;
RAM_EXTERN unsigned char Dir_Valve_Close;
RAM_EXTERN unsigned char valve_dir;
RAM_EXTERN unsigned char Valve_open;
RAM_EXTERN unsigned char Valve_Position;
RAM_EXTERN unsigned char Table_Steps_Positioning_Photocell_Ctrl;
RAM_EXTERN unsigned char Reference;
RAM_EXTERN unsigned char Table_Error;
RAM_EXTERN unsigned char Set_Home_pos;
RAM_EXTERN unsigned char Tr_Light_Dark_1;
RAM_EXTERN unsigned char Old_Photocell_sts_1;

RAM_EXTERN unsigned char Total_circuit_n;
RAM_EXTERN unsigned char Table_circuits_pos;
RAM_EXTERN unsigned char EEprom_Crc_Error;
RAM_EXTERN signed long Circuit_step_tmp[16],Circuit_step_original_pos[16];
RAM_EXTERN unsigned short Num_Table_Error;

RAM_EXTERN unsigned char Clean_Activation;
RAM_EXTERN DigInStatusType OutputFilter;

RAM_EXTERN _procGUI_t procGUI;
RAM_EXTERN calib_curve_par_t  calib_curve_par_writing;
RAM_EXTERN calib_curve_par_t  calib_curve_par[N_CALIB_CURVE]; // 74*5*14 = 5180 bytes [REVIEW]
RAM_EXTERN color_supply_par_t color_supply_par[N_SLAVES_COLOR_ACT]; //24*14 =336 bytes
RAM_EXTERN color_supply_par_t color_supply_par_writing;
RAM_EXTERN unsigned char en_slaves_writing[7];
RAM_EXTERN unsigned short Run_Dispensing;

RAM_EXTERN unsigned char tinting_ricirc_active;
RAM_EXTERN unsigned short stirring_counter_tinting;
RAM_EXTERN unsigned long  stirring_act_fsm_tinting;
RAM_EXTERN unsigned long recirc_act_fsm[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned short recirc_counter[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned long stirring_act_fsm[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned short stirring_counter[N_SLAVES_COLOR_ACT];
//RAM_EXTERN unsigned long  cleaning_act_fsm[N_SLAVES_COLOR_ACT];
//RAM_EXTERN unsigned short cleaning_counter[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned char nextStatus;
RAM_EXTERN unsigned char numErroriSerial[N_SLAVES];
RAM_EXTERN unsigned char attuatoreAttivo[N_SLAVES];

/**
 * EEPROM management
 */
RAM_EXTERN unsigned char eeprom_write_result;
RAM_EXTERN unsigned char eeprom_byte;
RAM_EXTERN unsigned char eeprom_read_result;
RAM_EXTERN unsigned char eeprom_retries;
RAM_EXTERN unsigned char clearEEPROMInCorso;
RAM_EXTERN unsigned char eeprom_read_result;
RAM_EXTERN unsigned short eeprom_crc;
RAM_EXTERN unsigned char eeprom_i, eeprom_j;


RAM_EXTERN unsigned short offset;
RAM_EXTERN unsigned short startAddress;

RAM_EXTERN union {
  unsigned char byte;
  struct {
    unsigned char CRCCircuitStepsPosFailed      : 1;
    unsigned char CRCParamColorCircuitFailed    : 1;
    unsigned char CRCParamCalibCurvesFailed     : 1;
    unsigned char CRCParamSlavesEnFailed        : 1;    
	unsigned char CRCParamHumidifier_paramFailed     : 1;
	unsigned char CRCParamTinting_Pump_paramFailed   : 1;
	unsigned char CRCParamTinting_Table_paramFailed  : 1;		
//	unsigned char CRCParamTinting_Clean_paramFailed  : 1;			
	unsigned char CRCParamCircuitPumpTypesFailed : 1;    
	};
} InitFlags;
/**
 * Bases and colorants
 */
RAM_EXTERN colorAct_t colorAct[N_SLAVES_COLOR_ACT];
RAM_EXTERN dispensationAct_t dispensationAct[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned short Filling_step;

RAM_EXTERN unsigned long  Timer_Out_Supply_High;
RAM_EXTERN unsigned long  Timer_Out_Supply_Low;
RAM_EXTERN unsigned short Diag_Setup_Timer_Received;
RAM_EXTERN unsigned long Timer_Old, Timer_New, Cycle_Duration, MAX_Cycle_Duration;

RAM_EXTERN unsigned short Pump_Types_Circuit_writing[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned short Double_Group_0, Double_Group_1;
RAM_EXTERN unsigned short Erogation_Type[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned short Erogation_done[N_SLAVES_COLOR_ACT];
// Actuators bitmask for Diagnostic Recirculation and Stirring 
RAM_EXTERN unsigned long diag_recirc_act_fsm[N_SLAVES_COLOR_ACT];
RAM_EXTERN unsigned long diag_stirring_act_fsm[N_SLAVES_COLOR_ACT];


RAM_EXTERN unsigned short mismatch_circuit_id_error;
RAM_EXTERN unsigned short Stop_Process;			

RAM_EXTERN unsigned short Old_Panel_table_status;
RAM_EXTERN unsigned short New_Panel_table_status;
RAM_EXTERN unsigned short Panel_table_transition;
RAM_EXTERN unsigned short Old_Bases_Carriage_status;
RAM_EXTERN unsigned short New_Bases_Carriage_status;
RAM_EXTERN unsigned short Bases_Carriage_transition;

// COLD RESET override
RAM_EXTERN unsigned char force_cold_reset;
/**
 * Autocap
 */
RAM_EXTERN autocapAct_t autocapAct; // Movimento autocap

// This flag is TRUE to prevent looping in RESET 
RAM_EXTERN unsigned char inhibitReset;
RAM_EXTERN unsigned short New_Reset_Done;

RAM_EXTERN volatile unsigned short jump_to_boot_done __attribute__((space(data), address(__JMP_BOOT_ADDR)));

RAM_EXTERN signed long pippo, pippo1, pippo2, pippo3, pippo4, pippo5, pippo6, pippo7, pippo8, pippo9, pippo10;
RAM_EXTERN unsigned long pippo11, pippo12;
