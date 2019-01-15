/* 
 * File:   statusManager.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 15.03
 */

#ifndef STATUSMANAGER_H
#define	STATUSMANAGER_H

enum {
	/* 0 */   TINTING_INIT_ST=0,
	/* 1 */   TINTING_READY_ST,
	/* 2 */   TINTING_PUMP_GO_HOMING_ST,
	/* 3 */   TINTING_PUMP_SEARCH_HOMING_ST,
	/* 4 */   TINTING_VALVE_GO_HOMING_ST,
	/* 5 */   TINTING_VALVE_SEARCH_HOMING_ST,
	/* 6 */   TINTING_TABLE_GO_HOMING_ST,
	/* 7 */   TINTING_TABLE_SEARCH_HOMING_ST,
	/* 8 */   TINTING_HOMING_ST,
	/* 9 */   TINTING_STANDBY_END_ST,
	/* 10 */  TINTING_SUPPLY_END_ST,
	/* 11 */  TINTING_STANDBY_RUN_ST,
	/* 12 */  TINTING_SUPPLY_RUN_ST,
	/* 13 */  TINTING_VALVE_OPEN_SMALL_ON_ST,
	/* 14 */  TINTING_VALVE_OPEN_BIG_ON_ST,
	/* 15 */  TINTING_SETUP_OUTPUT_ST,
	/* 16 */  TINTING_PAR_RX,
	/* 17 */  TINTING_JUMP_TO_BOOT, 
	/* 18 */  TINTING_CLEANING_ST,
	/* 19 */  TINTING_STOP_ST, 
	/* 20 */  TINTING_WAIT_PARAMETERS_ST,
	/* 21 */  TINTING_WAIT_PUMP_PARAMETERS_ST,
    /* 22 */  TINTING_WAIT_TABLE_PARAMETERS_ST,    
	/* 23 */  TINTING_WAIT_SETUP_OUTPUT_ST,  
	/* 24 */  TINTING_TABLE_SELF_RECOGNITION_ST,
	/* 25 */  TINTING_TABLE_POSITIONING_ST, 
	/* 26 */  TINTING_TABLE_CLEANING_ST, 
	/* 27 */  TINTING_TABLE_TEST_ST, 
	/* 28 */  TINTING_WAIT_SETUP_OUTPUT_VALVE_ST,
	/* 29 */  TINTING_SETUP_OUTPUT_VALVE_ST, 
	/* 30 */  TINTING_TABLE_STEPS_POSITIONING_ST,
	/* 31 */  TINTING_WAIT_COLORANT_TABLE_ST, 
	/* 32 */  TINTING_TABLE_GO_REFERENCE_ST, 
	/* 33 */  TINTING_TABLE_STIRRING_ST,
	/* 34 */  TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST,  
	/* 35 */  TINTING_PHOTO_DARK_TABLE_GO_HOMING_ST,  
	/* 36 */  TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST,  
	/* 37 */  TINTING_PHOTO_DARK_PUMP_GO_HOMING_ST,  
	/* 38 */  TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST,  
	/* 39 */  TINTING_PHOTO_DARK_VALVE_GO_HOMING_ST,  
  /** Errors */
    /* 45 */  TINTING_TIMEOUT_ERROR_ST = 45, // Not used
    /* 46 */  TINTING_TIMEOUT_SELF_LEARNING_PROCEDURE_ERROR_ST, // Not used
    /* 47 */  TINTING_TIMEOUT_TABLE_MOVE_ERROR_ST, // Not used
    /* 48 */  TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST,
    /* 49 */  TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST,
    /* 50 */  TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST,
    /* 51 */  TINTING_PUMP_HOMING_ERROR_ST, // Not used
    /* 52 */  TINTING_PUMP_HOMING_BACK_ERROR_ST, // Not used
    /* 53 */  TINTING_VALVE_HOMING_ERROR_ST, // Not used
    /* 54 */  TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST,  
    /* 55 */  TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST,     
    /* 56 */  TINTING_VALVE_HOMING_BACK_ERROR_ST, // Not used
    /* 57 */  TINTING_PUMP_RESET_ERROR_ST,
    /* 58 */  TINTING_VALVE_RESET_ERROR_ST, 
    /* 59 */  TINTING_TABLE_RESET_ERROR_ST,     
    /* 60 */  TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST,
    /* 61 */  TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST,
    /* 62 */  TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST,
    /* 63 */  TINTING_LACK_CIRCUITS_POSITION_ERROR_ST, 
    /* 64 */  TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST,
    /* 65 */  TINTING_TABLE_MOVE_ERROR_ST,
    /* 66 */  TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST,            
    /* 67 */  TINTING_TABLE_MISMATCH_POSITION_ERROR_ST,
    /* 68 */  TINTING_TABLE_TEST_ERROR_ST,     
    /* 69 */  TINTING_RH_ERROR_ST,
    /* 70 */  TINTING_TEMPERATURE_ERROR_ST,     
    /* 71 */  TINTING_PUMP_SOFTWARE_ERROR_ST,
    /* 72 */  TINTING_TABLE_SOFTWARE_ERROR_ST,
    /* 73 */  TINTING_BAD_PAR_HUMIDIFIER_ERROR_ST,
    /* 74 */  TINTING_BAD_PAR_PUMP_ERROR_ST,
    /* 75 */  TINTING_BAD_PAR_TABLE_ERROR_ST,
    /* 76 */  TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST, 
    /* 77 */  TINTING_TABLE_HOMING_ERROR_ST,
    /* 78 */  TINTING_BRUSH_OPEN_LOAD_ERROR_ST,
    /* 79 */  TINTING_BRUSH_OVERCURRENT_THERMAL_ERROR_ST,
    /* 80 */  TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST,
    /* 81 */  TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST,
    /* 82 */  TINTING_PUMP_OPEN_LOAD_ERROR_ST,
    /* 83 */  TINTING_PUMP_OVERCURRENT_THERMAL_ERROR_ST,
    /* 84 */  TINTING_RELE_OPEN_LOAD_ERROR_ST,
    /* 85 */  TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST, 
    /* 86 */  TINTING_GENERIC24V_OPEN_LOAD_ERROR_ST,
    /* 87 */  TINTING_GENERIC24V_OVERCURRENT_THERMAL_ERROR_ST,              
    /* 88 */  TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST,
    /* 89 */  TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST,
    /* 90 */  TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST,
    /* 91 */  TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST,
    /* 92 */  TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST,
    /* 93 */  TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST,
    /* 94 */  TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST,
    /* 95 */  TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST,              
};

enum {
    CMD_TINTING_IDLE   = 0x00,
    CMD_TINTING_STOP   = 0x01,
    CMD_TINTING_HOME   = 0x02,
    CMD_TINTING_CLEAN  = 0x03,    
    CMD_TINTING_SUPPLY = 0x04,
    CMD_TINTING_RECIRC = 0x08,
    CMD_TINTING_STIRR  = 0x05,    
    CMD_TINTING_SETUP_PARAM   = 0x10,
    CMD_TINTING_SETUP_OUTPUT  = 0x20,
    CMD_TINTING_SETUP_PROCESS = 0x40,
    CMD_TINTING_INTR  = 0x80,
  };
  
enum {
    CONTROLLO_PRESENZA = 0x01,
    POS_HOMING = 0x02,
    DISPENSAZIONE_COLORE = 0x06,
    RICIRCOLO_COLORE = 0x07,
    AGITAZIONE_COLORE = 0x08,    
    DISPENSAZIONE_COLORE_CONTINUOUS = 0x0F,
    SETUP_PARAMETRI_UMIDIFICATORE = 0x13,
    JUMP_TO_BOOT = 0x15,
    SETUP_PARAMETRI_POMPA  = 0x17,
    SETUP_PARAMETRI_TAVOLA = 0x18,
    TEST_FUNZIONAMENTO_TAVOLA_ROTANTE = 0x19,
    AUTOAPPRENDIMENTO_TAVOLA_ROTANTE = 0x1A,
    ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE = 0x1B,
    RICERCA_RIFERIMENTO_TAVOLA_ROTANTE = 0x1C,
    SETUP_PARAMETRI_PULIZIA = 0x1D,
    POSIZIONAMENTO_TAVOLA_ROTANTE = 0x1E,
    IMPOSTA_USCITE_TAVOLA_ROTANTE = 0x1F,
    POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE = 0x20,
    IMPOSTA_COLORANTI_TAVOLA = 0x21,
  };

enum {
  STEP_0,  STEP_1,  STEP_2,  STEP_3,  STEP_4,
  STEP_5,  STEP_6,  STEP_7,  STEP_8,  STEP_9,
  STEP_10, STEP_11, STEP_12, STEP_13, STEP_14,
  STEP_15, STEP_16, STEP_17, STEP_18, STEP_19,
  STEP_20, STEP_21, STEP_22, STEP_23, STEP_24,
  STEP_25, STEP_26, STEP_27, STEP_28, STEP_29,
  STEP_30, STEP_31, STEP_32, STEP_33, STEP_34,
  STEP_35, STEP_36, STEP_37, STEP_38, STEP_39,
  STEP_40, STEP_41, STEP_42, STEP_43, STEP_44,
  STEP_45, STEP_46, STEP_47, STEP_48, STEP_49,
};

extern void StatusManager(void);
extern void initStatusManager(void);
extern void initParam(void);
extern void jump_to_boot(void);

#endif	/* STATUSMANAGER_H */

