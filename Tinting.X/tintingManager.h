/* 
 * File:   tintingManager.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 15.03
 */

#ifndef TINTINGMANAGER_H
#define	TINTINGMANAGER_H

#include "stepperParameters.h"

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
	/* 40 */  TINTING_PHOTO_LIGHT_VALVE_SEARCH_HOMING_ST,
	/* 41 */  TINTING_LIGHT_VALVE_PUMP_SEARCH_HOMING_ST,
	/* 42 */  TINTING_PHOTO_LIGHT_VALVE_GO_TABLE_NOT_ENGAGED_ST,
	/* 43 */  TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST,
	/* 44 */  TINTING_PHOTO_LIGHT_VALVE_GO_VALVE_HOMING_ST,
	/* 45 */  TINTING_PHOTO_LIGHT_VALVE_SEARCH_PUMP_HOMING_ST,
	/* 46 */  TINTING_PHOTO_LIGHT_VALVE_GO_PUMP_HOMING_ST,
	/* 47 */  TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_HOMING_ST,
	/* 48 */  TINTING_PHOTO_LIGHT_VALVE_GO_TABLE_HOMING_ST,
	/* 49 */  TINTING_LIGHT_VALVE_PUMP_GO_HOMING_ST,
	/* 50 */  TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_NOT_ENGAGED_ST,
	/* 51 */  TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_TABLE_NOT_ENGAGED_ST,
	/* 52 */  TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_VALVE_HOMING_ST,
	/* 53 */  TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_VALVE_HOMING_ST,
	/* 54 */  TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_TABLE_HOMING_ST,
	/* 55 */  TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_TABLE_HOMING_ST,
	/* 56 */  TINTING_WAIT_SETUP_OUTPUT_TABLE_ST,
    /* 57 */  TINTING_SETUP_OUTPUT_TABLE_ST,
    /* 58 */  TINTING_TABLE_FIND_REFERENCE_ST,
    /* 59 */  TINTING_PHOTO_LIGHT_VALVE_NEW_SEARCH_VALVE_HOMING_ST,
    /* 60 */  TINTING_PHOTO_LIGHT_VALVE_NEW_GO_VALVE_HOMING_ST,

  /** Errors */
    /* 65 */  TINTING_TIMEOUT_ERROR_ST = 65, // Not used
    /* 66 */  TINTING_TIMEOUT_SELF_LEARNING_PROCEDURE_ERROR_ST, // Not used
    /* 67 */  TINTING_TIMEOUT_TABLE_MOVE_ERROR_ST, // Not used
    /* 68 */  TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST,
    /* 69 */  TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST,
    /* 70 */  TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST,
    /* 71 */  TINTING_PUMP_HOMING_ERROR_ST, // Not used
    /* 72 */  TINTING_PUMP_HOMING_BACK_ERROR_ST, // Not used
    /* 73 */  TINTING_VALVE_HOMING_ERROR_ST, 
    /* 74 */  TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST,  
    /* 75 */  TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST,     
    /* 76 */  TINTING_VALVE_HOMING_BACK_ERROR_ST, // Not used
    /* 77 */  TINTING_PUMP_RESET_ERROR_ST,
    /* 78 */  TINTING_VALVE_RESET_ERROR_ST, 
    /* 79 */  TINTING_TABLE_RESET_ERROR_ST,     
    /* 80 */  TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST,
    /* 81 */  TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST,
    /* 82 */  TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST,
    /* 83 */  TINTING_LACK_CIRCUITS_POSITION_ERROR_ST, 
    /* 84 */  TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST,
    /* 85 */  TINTING_TABLE_MOVE_ERROR_ST,
    /* 86 */  TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST,            
    /* 87 */  TINTING_TABLE_MISMATCH_POSITION_ERROR_ST,
    /* 88 */  TINTING_TABLE_TEST_ERROR_ST,     
    /* 89 */  TINTING_RH_ERROR_ST,
    /* 90 */  TINTING_TEMPERATURE_ERROR_ST,     
    /* 91 */  TINTING_PUMP_SOFTWARE_ERROR_ST,
    /* 92 */  TINTING_TABLE_SOFTWARE_ERROR_ST,
    /* 93 */  TINTING_BAD_PAR_HUMIDIFIER_ERROR_ST,
    /* 94 */  TINTING_BAD_PAR_PUMP_ERROR_ST,
    /* 95 */  TINTING_BAD_PAR_TABLE_ERROR_ST,
    /* 96 */  TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST, 
    /* 97 */  TINTING_TABLE_HOMING_ERROR_ST,
    /* 98 */  TINTING_BRUSH_OPEN_LOAD_ERROR_ST,
    /* 99 */  TINTING_BRUSH_OVERCURRENT_THERMAL_ERROR_ST,
    /* 100 */  TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST,
    /* 101 */  TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST,
    /* 102 */  TINTING_PUMP_OPEN_LOAD_ERROR_ST,
    /* 103 */  TINTING_PUMP_OVERCURRENT_THERMAL_ERROR_ST,
    /* 104 */  TINTING_RELE_OPEN_LOAD_ERROR_ST,
    /* 105 */  TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST, 
    /* 106 */  TINTING_GENERIC24V_OPEN_LOAD_ERROR_ST,
    /* 107 */  TINTING_GENERIC24V_OVERCURRENT_THERMAL_ERROR_ST,              
    /* 108 */  TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST,
    /* 109 */  TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST,
    /* 110 */  TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST,
    /* 111 */  TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST,
    /* 112 */  TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST,
    /* 113 */  TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST,
    /* 114 */  TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST,
    /* 115 */  TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST,
    /* 116 */  TINTING_VALVE_OPEN_READ_DARK_ERROR_ST,
    /* 117 */  TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST,     
    /* 118 */  TINTING_BASES_CARRIAGE_ERROR_ST,     
    /* 119 */  TINTING_PANEL_TABLE_ERROR_ST, 
    /* 120 */  TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST,	
    /* 121 */  TINTING_BRUSH_READ_LIGHT_ERROR_ST,
    /* 122 */  TINTING_BAD_PAR_CLEAN_ERROR_ST,               
};

enum {
    CMD_TINTING_IDLE   = 0x00,
    CMD_TINTING_STOP   = 0x01,
    CMD_TINTING_HOME   = 0x02,
    CMD_TINTING_SUPPLY = 0x04,
    CMD_TINTING_RECIRC = 0x08,
    CMD_TINTING_SETUP_PARAM   = 0x10,
    CMD_TINTING_SETUP_OUTPUT  = 0x20,
    CMD_TINTING_STOP_PROCESS  = 0x40, 
    CMD_TINTING_INTR  = 0x80,
  };
  
// Comandi MMT <-> Attuatori
enum {
  NO_MESSAGE = 0x00,  
  CONTROLLO_PRESENZA = 0x01,
  POS_HOMING = 0x02,
  DISPENSAZIONE_BASE = 0x03,
  RICIRCOLO_PITTURA = 0x04,
  AGITAZIONE_PITTURA = 0x05,
  DISPENSAZIONE_COLORE = 0x06,
  RICIRCOLO_COLORE= 0x07,
  AGITAZIONE_COLORE = 0x08,
  AGITAZIONE_RICIRCOLO_COLORE = 0x09,
  PRELIEVO_CONTENITORE = 0x0A,
  MOVIMENTAZIONE_ASSE = 0x0B,
  TAPPATURA_COPERCHIO = 0x0C,
  POSIZIONA_AUTOCAP = 0x0D,
  SET_EV_DISPENSAZIONE = 0x0E,
  DISPENSAZIONE_COLORE_CONT=0x0F,
  MUOVI_SGABELLO = 0x10,
  SETUP_PARAMETRI_UMIDIFICATORE_10 = 0x11,
  CAMBIA_SPEED_SGABELLO = 0x12,
  SETUP_PARAMETRI_UMIDIFICATORE = 0x13,
  IMPOSTA_USCITE_UMIDIFICATORE = 0x14,
  JUMP_TO_BOOT = 0x15,
  DISPENSAZIONE_GRUPPO_DOPPIO = 0x16,
  SETUP_PARAMETRI_POMPA = 0x17,
  SETUP_PARAMETRI_TAVOLA = 0x18,
  TEST_FUNZIONAMENTO_TAVOLA_ROTANTE = 0x19,
  AUTOAPPRENDIMENTO_TAVOLA_ROTANTE = 0x1A,
  ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE = 0x1B,
  RICERCA_RIFERIMENTO_TAVOLA_ROTANTE = 0x1C,
  SETUP_PARAMETRI_PULIZIA = 0x1D,
  POSIZIONAMENTO_TAVOLA_ROTANTE = 0x1E,
  IMPOSTA_USCITE_TAVOLA_ROTANTE = 0x1F,
  POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE = 0x20,    
  DISPENSAZIONE_COLORE_CONT_GRUPPO_DOPPIO = 0x21,	  
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
  STEP_50, STEP_51, STEP_52, STEP_53, STEP_54,
};

/**
 * Main FSM definition (states, phases, steps)
 */
enum {
  /* 0 */ POWER_OFF_ST,
  /* 1 */ INIT_ST,
  /* 2 */ IDLE_ST,
  /* 3 */ RESET_ST,
  /* 4 */ COLOR_RECIRC_ST,
  /* 5 */ COLOR_SUPPLY_ST,
  /* 6 */ ALARM_ST,
  /* 7 */ DIAGNOSTIC_ST,
  /* 8 */ POSITIONING_ST,
  /* 9 */ JUMP_TO_BOOT_ST,
  /* 10 */ ROTATING_ST,
  /* 11 */ AUTOTEST_ST,  
  /* 12 */ N_STATUS
};

enum {
  /* 0 */ CLEAN_INIT_ST,            
  /* 1 */ CLEAN_PAUSE_ST,
  /* 2 */ CLEAN_START_ST,
  /* 3 */ CLEAN_ACTIVE_ST,
  /* 4 */ CLEAN_STOP_ST,
  /* 5 */ CLEAN_DISABLE_ST,         
  /* 6 */ N_CLEAN_STATUS,
};

enum {
  ENTRY_PH,
  RUN_PH,
  EXIT_PH
};

/**
 * Function-like macros (statements)
 */
#define posHomingTintingAct()                     \
	do {                                          \
		setTintingActMessage(POS_HOMING);         \
		TintingAct.command.cmd = CMD_TINTING_HOME;\
	} while (0)

#define intrTintingAct()                          \
	do {                                          \
		setTintingActMessage(CONTROLLO_PRESENZA); \
		TintingAct.command.cmd = CMD_TINTING_INTR;\
	} while (0)

#define checkHomingTintingAct()                   \
	do {                                          \
		setTintingActMessage(CONTROLLO_PRESENZA); \
		TintingAct.command.cmd = CMD_TINTING_IDLE;\
	} while (0)

#define idleTintingAct()                          \
	do {                                          \
		setTintingActMessage(CONTROLLO_PRESENZA); \
		TintingAct.command.cmd = CMD_TINTING_IDLE;\
	} while (0)	

#define TintingPosizionamentoTavola()                               \
	do {                                                            \
		setTintingActMessage(POSIZIONAMENTO_TAVOLA_ROTANTE);	 	\
        TintingAct.command.cmd = CMD_TINTING_IDLE;                  \
        TintingAct.Color_Id = TintingAct.NextColor_Id;              \
        PositioningCmd = 1;                                         \
	} while (0)

#define TintingStop()			          		                    \
	do {                                                            \
		setTintingActMessage(CONTROLLO_PRESENZA); 	 		   		\
		TintingAct.command.cmd = CMD_TINTING_STOP;              	\
        procGUI.recirc_status &= ~(0xFFFF00);                       \
	} while (0)

#define TintingNoCmd()			          		                    \
	do {                                                            \
		TintingAct.command.cmd = CMD_TINTING_IDLE;                  \
	} while (0)

#define TintingStopProcess()            	          		        \
	do {                                                            \
		setTintingActMessage(CONTROLLO_PRESENZA); 	 		   		\
		TintingAct.command.cmd = CMD_TINTING_STOP_PROCESS;          \
        procGUI.recirc_status &= ~(0xFFFF00);                       \
	} while (0)

#define TintingSetPeripheralAct()                                   \
	do {                                                            \
		setTintingActMessage(IMPOSTA_USCITE_TAVOLA_ROTANTE); 	    \
		TintingAct.command.cmd = CMD_TINTING_SETUP_OUTPUT;          \
	} while (0)

#define	startSupplyContinuousTinting(x)                                \
	do {                                                               \
		setTintingActMessage(DISPENSAZIONE_COLORE_CONT);			   \
		TintingAct.command.cmd = CMD_TINTING_SUPPLY;				   \
		TintingAct.Color_Id = x - COLORANT_ID_OFFSET;                  \
        TintingAct.PosStart = colorAct[x].posStart * (unsigned long)CORRECTION_PUMP_STEP_RES; \
        TintingAct.PosStop  = colorAct[x].posStop * (unsigned long)CORRECTION_PUMP_STEP_RES;  \
        TintingAct.Speed_cycle_supply = colorAct[x].speed_cycle_supply; \
        TintingAct.N_CicliDosaggio = colorAct[x].numCicliDosaggio;      \
        TintingAct.N_step_full_stroke = TintingPump.N_steps_stroke * (unsigned long)CORRECTION_PUMP_STEP_RES; \
        TintingAct.N_step_stroke = colorAct[x].n_step_cycle * (unsigned long)CORRECTION_PUMP_STEP_RES;        \
        TintingAct.Speed_cycle = colorAct[x].speed_cycle_supply;               \
        TintingAct.N_cycles = colorAct[x].n_cycles;                     \
        TintingAct.Algorithm = ALG_ASYMMETRIC_CONTINUOUS;               \
        TintingAct.En_back_step = colorAct[x].en_back_step;             \
        TintingAct.N_step_back_step_2 = colorAct[x].n_step_back_step * (unsigned long)CORRECTION_PUMP_STEP_RES;   \
        TintingAct.Speed_back_step_2 = colorAct[x].speed_back_step;     \
        TintingAct.N_step_backlash = colorAct[x].n_step_backlash * (unsigned long)CORRECTION_PUMP_STEP_RES;       \
        TintingAct.Delay_EV_off = colorAct[x].delay_EV_off;             \
        TintingAct.Speed_suction = colorAct[x].speed_suction;           \
        TintingAct.Delay_resh_after_supply = 0;                         \
        TintingAct.N_step_back_step_Big_Hole = color_supply_par[x].reshuffle_window * (unsigned long)CORRECTION_PUMP_STEP_RES;   \
        TintingAct.Speed_back_step_Big_Hole =  color_supply_par[x].reshuffle_duration; \
	} while (0)

#define startSupplyTinting(x)                                          \
	do {                                                               \
		setTintingActMessage(DISPENSAZIONE_COLORE);                    \
		TintingAct.command.cmd = CMD_TINTING_SUPPLY;                   \
		TintingAct.Color_Id =  x - COLORANT_ID_OFFSET;                 \
        PositioningCmd = 0;                                            \
        TintingAct.N_step_full_stroke = TintingPump.N_steps_stroke * (unsigned long)CORRECTION_PUMP_STEP_RES; \
        TintingAct.N_step_stroke = colorAct[x].n_step_cycle_supply * (unsigned long)CORRECTION_PUMP_STEP_RES; \
        TintingAct.Speed_cycle = colorAct[x].speed_cycle_supply;                                              \
        TintingAct.N_cycles = colorAct[x].n_cycles_supply;                                                    \
        TintingAct.Algorithm = colorAct[x].algorithm;                                                         \
        TintingAct.En_back_step = colorAct[x].en_back_step;                                                   \
        TintingAct.N_step_back_step_2 = colorAct[x].n_step_back_step * (unsigned long)CORRECTION_PUMP_STEP_RES; \
        TintingAct.Speed_back_step_2  = colorAct[x].speed_back_step;                                            \
        TintingAct.N_step_backlash = colorAct[x].n_step_backlash;     \
        TintingAct.Delay_EV_off = colorAct[x].delay_EV_off;           \
        TintingAct.Speed_suction = colorAct[x].speed_suction;         \
        TintingAct.Delay_resh_after_supply = colorAct[x].delay_resh_after_supply; \
        TintingAct.N_step_back_step_Small_Hole = color_supply_par[x].reshuffle_pwm_pct * (unsigned long)CORRECTION_PUMP_STEP_RES; \
        TintingAct.Speed_back_step_Small_Hole = color_supply_par[x].reshuffle_duration; \
        TintingAct.N_step_back_step_Big_Hole = color_supply_par[x].reshuffle_window * (unsigned long)CORRECTION_PUMP_STEP_RES; \
        TintingAct.Speed_back_step_Big_Hole = color_supply_par[x].reshuffle_duration; \
        if (colorAct[x].SingleStrokeType > SINGLE_STROKE_CLEVER)    \
            colorAct[x].SingleStrokeType = SINGLE_STROKE_CLEVER;    \
        TintingAct.SingleStrokeType = colorAct[x].SingleStrokeType; \
	} while (0)
        
// Fatto
#define TintingPosizionamentoPassiTavola()                    		\
	do {                                                            \
		setTintingActMessage(POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE);  \
		TintingAct.command.cmd = CMD_TINTING_IDLE;            	    \
        PositioningCmd = 1;                                         \
	} while (0)


#define TintingRicercaRiferimentoTavola()                 		    \
	do {                                                            \
		setTintingActMessage(RICERCA_RIFERIMENTO_TAVOLA_ROTANTE); 	\
		TintingAct.command.cmd = CMD_TINTING_IDLE;                  \
	} while (0)

#define TintingTestFunzionamnetoTavola()                 		    \
	do {                                                            \
		setTintingActMessage(TEST_FUNZIONAMENTO_TAVOLA_ROTANTE); 	\
		TintingAct.command.cmd = CMD_TINTING_IDLE;              	\
	} while (0)

#define TintingPuliziaTavola()                           		    \
	do {                                                            \
		setTintingActMessage(ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE); 	\
		TintingAct.command.cmd = CMD_TINTING_IDLE;              	\
	} while (0)

#define TintingAutoapprendimentoTavola()                            \
	do {                                                            \
		setTintingActMessage(AUTOAPPRENDIMENTO_TAVOLA_ROTANTE); 	\
		TintingAct.command.cmd = CMD_TINTING_IDLE;                  \
	} while (0)

#define TintingStartRecirc(x)                           \
	do {                                                \
		setTintingActMessage(RICIRCOLO_COLORE);       	\
		TintingAct.command.cmd = CMD_TINTING_RECIRC;    \
		TintingAct.Color_Id = x - COLORANT_ID_OFFSET;   \
        PositioningCmd = 0;                             \
        TintingAct.N_step_stroke = TintingPump.N_steps_stroke * (unsigned long)CORRECTION_PUMP_STEP_RES;  \
        TintingAct.Speed_cycle   = colorAct[x].speed_cycle;     \
        TintingAct.N_cycles = colorAct[x].n_cycles;             \
        TintingAct.Recirc_pause = colorAct[x].recirc_pause;     \
	} while (0)

#define TintingStartRecircBeforeSupply(x)                               \
	do {                                                            	\
		if (color_supply_par[x].recirc_before_dispensation_n_cycles) {	\
			setTintingActMessage(RICIRCOLO_COLORE);       				\
			TintingAct.command.cmd = CMD_TINTING_RECIRC;           		\
			TintingAct.Color_Id = x - COLORANT_ID_OFFSET;               \
            TintingAct.N_step_stroke = TintingPump.N_steps_stroke * (unsigned long)CORRECTION_PUMP_STEP_RES;  \
			TintingAct.Speed_cycle = color_supply_par[x].speed_recirc;	\
			TintingAct.N_cycles = color_supply_par[x].recirc_before_dispensation_n_cycles;	\
			TintingAct.Recirc_pause = 0; \
		} \
		else { \
			procGUI.recirc_before_supply_status |= (1L << x); \
		}	\
	} while (0)

#define TintingStartStirring()                          \
	do {                                                \
		setTintingActMessage(AGITAZIONE_COLORE);       	\
		TintingAct.command.cmd = CMD_TINTING_IDLE;      \
	} while (0)

// -----------------------------------------------------------------------------
#define isTintingReady()  				     \
	(TintingAct.TintingFlags.tinting_ready)

#define isTintingStopped()                   \
	(TintingAct.TintingFlags.tinting_stopped)

#define isTintingHoming()                    \
	(TintingAct.TintingFlags.tinting_homing)

#define isTinting_RH_error()                 \
	(TintingAct.TintingFlags.tinting_RH_error)
	
#define isTinting_Temperature_error()        \
	(TintingAct.TintingFlags.tinting_Temperature_error)

#define isTintingRicircRun()  			    \
	(TintingAct.TintingFlags.tinting_recirc_run)

#define isTintingRicircEnd()  			    \
	(TintingAct.TintingFlags.tinting_recirc_end)

#define isTintingStirringRun()  			    \
    (TintingAct.TintingFlags_2.tinting_stirring_run)

#define isTintingSupplyEnd()  			    \
	(TintingAct.TintingFlags.tinting_supply_end)

#define isTintingSetPeripheralAct()                           \
    (PeripheralAct.Peripheral_Types.RotatingTable 		||	  \
	 PeripheralAct.Peripheral_Types.Cleaner             ||	  \
	 PeripheralAct.Peripheral_Types.WaterPump 			||	  \
	 PeripheralAct.Peripheral_Types.Nebulizer_Heater	||	  \
	 PeripheralAct.Peripheral_Types.HeaterResistance	||	  \
	 PeripheralAct.Peripheral_Types.OpenValve_BigHole	||	  \
	 PeripheralAct.Peripheral_Types.OpenValve_SmallHole	||	  \
	 PeripheralAct.Peripheral_Types.Rotating_Valve	)

#define isTintingSetHumidifierHeaterAct()                     \
	(PeripheralAct.Peripheral_Types.Nebulizer_Heater	||	  \
	 PeripheralAct.Peripheral_Types.HeaterResistance    ||	  \
	 PeripheralAct.Peripheral_Types.Cleaner             ||	  \
	 PeripheralAct.Peripheral_Types.WaterPump)

#define isTintingSetPumpTableAct()                            \
    (PeripheralAct.Peripheral_Types.RotatingTable 		||	  \
	 PeripheralAct.Peripheral_Types.Cleaner             ||	  \
	 PeripheralAct.Peripheral_Types.OpenValve_BigHole	||	  \
	 PeripheralAct.Peripheral_Types.OpenValve_SmallHole	||	  \
	 PeripheralAct.Peripheral_Types.Rotating_Valve	)

/** Act is in ERROR state */
#define isTintingActError()                                                     \
	   (TintingAct.TintingFlags.tinting_bad_humidifier_par_error	||          \
	   TintingAct.TintingFlags.tinting_tout_error                 	||          \
	   TintingAct.TintingFlags.tinting_table_software_error         ||          \
	   TintingAct.TintingFlags.tinting_pump_homing_pos_error        ||          \
	   TintingAct.TintingFlags.tinting_valve_homing_pos_error       ||          \
	   TintingAct.TintingFlags.tinting_table_homing_pos_error      	||          \
	   TintingAct.TintingFlags.tinting_pump_reset_error 			||          \
	   TintingAct.TintingFlags.tinting_valve_reset_error   			||          \
	   TintingAct.TintingFlags.tinting_pump_reset_error             ||          \
	   TintingAct.TintingFlags.tinting_valve_reset_error            ||          \
	   TintingAct.TintingFlags.tinting_table_reset_error			||          \
	   TintingAct.TintingFlags.tinting_pump_supply_calc_error       ||          \
	   TintingAct.TintingFlags.tinting_pos0_read_light_error        ||          \
	   TintingAct.TintingFlags.tinting_end_stroke_read_dark_error   ||          \
	   TintingAct.TintingFlags.tinting_pump_motor_open_load_error   ||          \
	   TintingAct.TintingFlags.tinting_valve_motor_open_load_error  ||          \
	   TintingAct.TintingFlags.tinting_table_motor_open_load_error  ||          \
	   TintingAct.TintingFlags.tinting_pump_motor_overcurrent_error ||          \
	   TintingAct.TintingFlags.tinting_valve_motor_overcurrent_error||          \
	   TintingAct.TintingFlags.tinting_table_motor_overcurrent_error||          \
	   TintingAct.TintingFlags.tinting_pump_motor_home_back_error   ||          \
	   TintingAct.TintingFlags.tinting_valve_motor_home_back_error  ||          \
	   TintingAct.TintingFlags_1.tinting_timeout_table_move_error   ||          \
	   TintingAct.TintingFlags_1.tinting_table_search_position_reference_error  ||          \
	   TintingAct.TintingFlags_1.tinting_lack_circuits_position_error  			||          \
	   TintingAct.TintingFlags_1.tinting_timeout_self_learning_procedure_error  ||          \
	   TintingAct.TintingFlags_1.tinting_self_learning_procedure_error 			||          \
	   TintingAct.TintingFlags_1.tinting_table_move_error  						||          \
	   TintingAct.TintingFlags_1.tinting_table_mismatch_position_error  		||          \
	   TintingAct.TintingFlags_1.tinting_bad_pump_param_error  					||          \
	   TintingAct.TintingFlags_1.tinting_pump_photo_ingr_read_light_error 		||          \
	   TintingAct.TintingFlags_1.tinting_pump_photo_home_read_dark_error		||          \
	   TintingAct.TintingFlags_1.tinting_valve_pos0_read_light_error			||          \
	   TintingAct.TintingFlags_1.tinting_pump_software_error					||          \
	   TintingAct.TintingFlags_1.tinting_table_test_error						||          \
	   TintingAct.TintingFlags_1.tinting_brush_open_load_error				    ||          \
	   TintingAct.TintingFlags_1.tinting_brush_overcurrent_thermal_error		||          \
	   TintingAct.TintingFlags_1.tinting_generic24v_open_load_error		        ||          \
	   TintingAct.TintingFlags_1.tinting_generic24v_overcurrent_thermal_error   ||          \
	   TintingAct.TintingFlags_2.tinting_pump_motor_thermal_shutdown_error      ||          \
	   TintingAct.TintingFlags_2.tinting_valve_motor_thermal_shutdown_error     ||          \
	   TintingAct.TintingFlags_2.tinting_table_motor_thermal_shutdown_error     ||          \
	   TintingAct.TintingFlags_2.tinting_pump_motor_under_voltage_error         ||          \
	   TintingAct.TintingFlags_2.tinting_valve_motor_under_voltage_error        ||          \
	   TintingAct.TintingFlags_2.tinting_table_motor_under_voltage_error	   	||          \
	   TintingAct.TintingFlags_2.tinting_eeprom_colorants_steps_position_crc_error ||       \
	   TintingAct.TintingFlags_2.tinting_valve_open_read_dark_error             ||          \
	   TintingAct.TintingFlags_2.tinting_valve_open_read_light_error			||          \
	   TintingAct.TintingFlags_2.tinting_pump_photo_ingr_read_dark_error		||          \
	   TintingAct.TintingFlags_2.tinting_panel_table_error                      || \
       TintingAct.TintingFlags_2.tinting_carriage_bases_error )

extern void TintingManager(void);
extern void initStatusManager(void);
extern void initParam(void);
extern void jump_to_boot(void);
extern unsigned char isTintingEnabled(void);
extern void resetStandbyProcessesSingle(unsigned char id);
extern void resetStandbyProcessesTinting(void);
extern unsigned char isColorReadyTintingModule(unsigned char color_id);
extern unsigned char isColorTintingModule(unsigned char color_id);
extern unsigned char isColorSupllyEndTintingModule(void);
extern void NEW_Calculates_Tinting_Colorants_Order(void);
extern void NEW_Calculates_Cleaning_Tinting_Colorants_Order(void);
extern void setTintingActMessage(unsigned char packet_type);
extern void Cleaning_Manager(void);

#endif	/* TINTINGMANAGER_H */

