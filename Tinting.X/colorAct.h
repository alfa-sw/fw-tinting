/* 
 * File:   colorAct.h
 * Author: michele.abelli
 *
 * Created on 22 maggio 2019, 14.27
 */

#ifndef _COLOR_ACT_H_
#define _COLOR_ACT_H_

#include "serialCom.h"
//#include "define.h"
/**
 * WARNING: these definitions must match *exactly* the ones in
 * colorActuators/color.h. Any mismatch can (and will) cause wildly
 * unpredictable results!
 ******************************************************************************/
/**
 * commands
 */
#define CMD_IDLE          0x00
#define CMD_STOP          0x01
#define CMD_HOME          0x02
#define CMD_SUPPLY        0x04
#define CMD_RECIRC        0x08
#define CMD_RESH          0x10
#define CMD_RESH_RECIRC   0x18
#define CMD_SET_EV        0x20
#define CMD_INTR          0x80

enum {
  /*  0 */   COLOR_INIT_ST,
  /*  1 */   COLOR_READY_ST,
  /*  2 */   COLOR_SEARCH_HOMING_ST,
  /*  3 */   COLOR_GO_HOMING_ST,
  /*  4 */   COLOR_HOMING_ST,
  /*  5 */   COLOR_STANDBY_RUN_ST,
  /*  6 */   COLOR_SUPPLY_RUN_ST,
  /*  7 */   COLOR_STANDBY_END_ST,
  /*  8 */   COLOR_SUPPLY_END_ST,

  /** Errors */
  /*  9 */   COLOR_TOUT_ERROR_ST,
  /* 10 */   COLOR_SOFTWARE_ERROR_ST,
  /* 11 */   COLOR_HOMING_ERROR_ST,
  /* 12 */   COLOR_RESET_ERROR_ST,
  /* 13 */   COLOR_SUPPLY_CALC_ERROR_ST,
  /* 14 */   COLOR_OVERCURRENT_ERROR_ST,
  /* 15 */   COLOR_HOMING_BACK_ERROR_ST,
  /* 16 */   COLOR_POS0_READ_LIGHT_ERROR_ST,
  /* 17 */   COLOR_END_STROKE_READ_DARK_ERROR_ST,
  /* 18 */   COLOR_DRV_OVER_CURR_TEMP_ERROR_ST,
  /* 19 */   COLOR_OPEN_LOAD_ERROR_ST,                   
  /* Other Events */
  /* 20 */ 	 COLOR_JUMP_TO_BOOT_ST,
};

/**
 * FUNCTION-LIKE MACROS
 ******************************************************************************/
#define startSupplyColor(x)                                             \
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_COLORE, x);                      \
      colorAct[x].command.cmd = CMD_SUPPLY;                             \
      colorAct[x].n_cycles = colorAct[x].n_cycles_supply;               \
      colorAct[x].n_step_cycle = colorAct[x].n_step_cycle_supply;       \
      colorAct[x].speed_cycle = colorAct[x].speed_cycle_supply;         \
  } while (0)

#define stopColorAct(x)                                 \
  do {                                                  \
      setColorActMessage(CONTROLLO_PRESENZA, x);        \
      colorAct[x].command.cmd = CMD_STOP;               \
                                                        \
      procGUI.recirc_status &= ~(1L << x);              \
      procGUI.stirring_status &= ~(1L << x);            \
  } while (0)

#define startSet_EV(x)                                  \
  do {                                                  \
    setColorActMessage(SET_EV_DISPENSAZIONE, x);        \
    colorAct[x].command.cmd = CMD_SET_EV;               \
  } while (0)

#define intrColorAct(x)                                 \
  do {                                                  \
    setColorActMessage(CONTROLLO_PRESENZA, (x));        \
    colorAct[(x)].command.cmd = CMD_INTR;               \
  } while (0)

#define posHomingColorAct(x)                    \
  do {                                          \
    setColorActMessage(POS_HOMING, (x));        \
    colorAct[(x)].command.cmd = CMD_HOME;       \
  } while (0)

#define checkHomingColorAct(x)                  \
  do {                                          \
    setColorActMessage(CONTROLLO_PRESENZA, x);  \
    colorAct[x].command.cmd = CMD_IDLE;         \
  } while (0)

/**
 * FUNCTION-LIKE MACROS
 *****************************************/
// Find a Double Group with even and odd index 'x'

#define startSupplyColorDoubleGroupMasterSlave(x) 						\
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_GRUPPO_DOPPIO, x);               \
      colorAct[x].command.cmd = CMD_SUPPLY;                             \
      colorAct[x].algorithm = ALG_DOUBLE_GROUP;						    \
	  colorAct[x].speed_cycle_channel_A = colorAct[x].speed_cycle_supply;	\
	  colorAct[x].n_step_cycle_channel_A = colorAct[x].n_step_cycle_supply;	\
	  colorAct[x].n_cycles_channel_A = colorAct[x].n_cycles_supply; 		\
	  colorAct[x].speed_cycle_channel_B	 = colorAct[x+1].speed_cycle_supply; 	\
	  colorAct[x].n_step_cycle_channel_B = colorAct[x+1].n_step_cycle_supply; 	\
	  colorAct[x].n_cycles_channel_B = colorAct[x+1].n_cycles_supply; 			\
} while (0)
 
// Find a Double Group with even index 'x' (0,2,4,6)
#define startSupplyColorDoubleGroupMaster(x)                            \
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_GRUPPO_DOPPIO, x);               \
      colorAct[x].command.cmd = CMD_SUPPLY;                             \
      colorAct[x].algorithm = ALG_DOUBLE_GROUP;						    \
	  colorAct[x].speed_cycle_channel_A = colorAct[x].speed_cycle_supply;	\
	  colorAct[x].n_step_cycle_channel_A = colorAct[x].n_step_cycle_supply;	\
	  colorAct[x].n_cycles_channel_A = colorAct[x].n_cycles_supply; 	\
	  colorAct[x].speed_cycle_channel_B	 = colorAct[x].speed_suction; 	\
	  colorAct[x].n_step_cycle_channel_B = 0; 							\
	  colorAct[x].n_cycles_channel_B = 0; 								\
} while (0)

// Find a Double Group with odd index 'x' (1,3,5,7)
#define startSupplyColorDoubleGroupSlave(x)                             \
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_GRUPPO_DOPPIO, x-1);             \
      colorAct[x-1].command.cmd = CMD_SUPPLY;                           \
	  colorAct[x-1].algorithm = ALG_DOUBLE_GROUP;						\
	  colorAct[x-1].n_step_stroke = colorAct[x].n_step_stroke;         	\
	  colorAct[x-1].speed_cycle_channel_A  = colorAct[x].speed_suction; \
	  colorAct[x-1].n_step_cycle_channel_A = 0;                         \
	  colorAct[x-1].n_cycles_channel_A = 0;                             \
	  colorAct[x-1].speed_cycle_channel_B  = colorAct[x].speed_cycle_supply; \
	  colorAct[x-1].n_step_cycle_channel_B = colorAct[x].n_step_cycle_supply; \
	  colorAct[x-1].n_cycles_channel_B = colorAct[x].n_cycles_supply;   \
} while (0)
	
// Double Group Erogation
#define startSupplyColorDoubleGroup(x)                                  \
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_GRUPPO_DOPPIO, x);               \
      colorAct[x].command.cmd = CMD_SUPPLY;                             \
	  colorAct[x].algorithm = ALG_DOUBLE_GROUP;						    \
} while (0)	
// --------------------------------------------------------------
// Continuous Erogation: Find a Double Group with even and odd index 'x'
#define startSupplyColorContinuousDoubleGroupMasterSlave(x) 			\
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_COLORE_CONT_GRUPPO_DOPPIO , x);  \
      colorAct[x].command.cmd = CMD_SUPPLY;                             \
	  colorAct[x].posStart_A = colorAct[x].posStart; 					\
	  colorAct[x].posStop_A  = colorAct[x].posStop; 					\
	  colorAct[x].speedContinous_A = colorAct[x].speed_cycle_supply; 	\
	  colorAct[x].numCicliDosaggio_A = colorAct[x].numCicliDosaggio; 	\
	  colorAct[x].n_step_cycle_channel_A = colorAct[x].n_step_cycle; 	\
	  colorAct[x].speed_cycle_channel_A = colorAct[x].speed_cycle; 		\
	  colorAct[x].n_cycles_channel_A = colorAct[x].n_cycles; 			\
	  colorAct[x].posStart_B = colorAct[x+1].posStart; 					\
	  colorAct[x].posStop_B  = colorAct[x+1].posStop; 					\
	  colorAct[x].speedContinous_B = colorAct[x+1].speed_cycle_supply; 	\
	  colorAct[x].numCicliDosaggio_B = colorAct[x+1].numCicliDosaggio; 	\
	  colorAct[x].n_step_cycle_channel_B = colorAct[x+1].n_step_cycle; 	\
	  colorAct[x].speed_cycle_channel_B = colorAct[x+1].speed_cycle; 	\
	  colorAct[x].n_cycles_channel_B = colorAct[x+1].n_cycles; 			\
} while (0)
 
// Continuous Erogation: Find a Double Group with even index 'x' (0,2,4,6)
#define startSupplyColorContinuousDoubleGroupMaster(x)                  \
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_COLORE_CONT_GRUPPO_DOPPIO , x);  \
      colorAct[x].command.cmd = CMD_SUPPLY;                             \
	  colorAct[x].posStart_A = colorAct[x].posStart; 					\
	  colorAct[x].posStop_A  = colorAct[x].posStop; 					\
	  colorAct[x].speedContinous_A = colorAct[x].speed_cycle_supply; 	\
	  colorAct[x].numCicliDosaggio_A = colorAct[x].numCicliDosaggio; 	\
	  colorAct[x].n_step_cycle_channel_A = colorAct[x].n_step_cycle; 	\
	  colorAct[x].speed_cycle_channel_A = colorAct[x].speed_cycle; 		\
	  colorAct[x].n_cycles_channel_A = colorAct[x].n_cycles; 			\
	  colorAct[x].posStart_B = 0; 										\
	  colorAct[x].posStop_B  = 0; 										\
	  colorAct[x].speedContinous_B = colorAct[x].speed_suction; 		\
	  colorAct[x].numCicliDosaggio_B = 0; 								\
	  colorAct[x].n_step_cycle_channel_B = 0; 							\
	  colorAct[x].speed_cycle_channel_B = colorAct[x].speed_suction; 	\
	  colorAct[x].n_cycles_channel_B = 0; 								\
} while (0)

// Continuous Erogation: Find a Double Group with odd index 'x' (1,3,5,7)
#define startSupplyColorContinuousDoubleGroupSlave(x)                   \
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_COLORE_CONT_GRUPPO_DOPPIO , x-1);\
      colorAct[x-1].command.cmd = CMD_SUPPLY;                           \
	  colorAct[x-1].n_step_stroke = colorAct[x].n_step_stroke;         	\
	  colorAct[x-1].posStart_A = 0; 									\
	  colorAct[x-1].posStop_A  = 0; 									\
	  colorAct[x-1].speedContinous_A = colorAct[x].speed_suction; 		\
	  colorAct[x-1].numCicliDosaggio_A = 0; 					 		\
	  colorAct[x-1].n_step_cycle_channel_A = 0; 						\
	  colorAct[x-1].speed_cycle_channel_A = colorAct[x].speed_suction; 	\
	  colorAct[x-1].n_cycles_channel_A = 0; 							\
	  colorAct[x-1].posStart_B = colorAct[x].posStart; 					\
	  colorAct[x-1].posStop_B  = colorAct[x].posStop; 					\
	  colorAct[x-1].speedContinous_B = colorAct[x].speed_cycle_supply; 	\
	  colorAct[x-1].numCicliDosaggio_B = colorAct[x].numCicliDosaggio; 	\
	  colorAct[x-1].n_step_cycle_channel_B = colorAct[x].n_step_cycle; 	\
	  colorAct[x-1].speed_cycle_channel_B = colorAct[x].speed_cycle; 	\
	  colorAct[x-1].n_cycles_channel_B = colorAct[x].n_cycles; 			\
} while (0)

// Continuous Double Group Erogation:	
#define startSupplyColorContinuousDoubleGroup(x)						\
  do {                                                                  \
      setColorActMessage(DISPENSAZIONE_COLORE_CONT_GRUPPO_DOPPIO , x);  \
      colorAct[x].command.cmd = CMD_SUPPLY;                             \
	  colorAct[x].algorithm = ALG_DOUBLE_GROUP_CONTINUOUS;			    \
} while (0)

#define JumpToBoot_ColorAct(x)	          		\
  do {                                          \
	setColorActMessage(JUMP_TO_BOOT, x); 	    \
	colorAct[x].command.cmd = CMD_IDLE;         \
  } while (0)

/*
#define SetDoubleGroupStirring(group, stirr_sts)	\
  do {												\
	if ( (group == 0) && (stirr_sts == 0) )			\
		WATER_PUMP_OFF();        					\
	else if ( (group == 0) && (stirr_sts == 1) )	\
		WATER_PUMP_ON();                        	\
	else if ( (group == 1) && (stirr_sts == 0) ) 	\
		OUT24V_OFF();                               \
	else if ( (group == 1) && (stirr_sts == 1) )	\
		OUT24V_ON();                                \
  } while (0)
*/

#define SetDoubleGroupStirring(group, stirr_sts)	\
  do {												\
	if (stirr_sts == 1)                             \
        DoubleGoup_Stirring_st = 1;                 \
    else                                            \
        DoubleGoup_Stirring_st = 0;                 \
    if ( (group == 0) && (stirr_sts == 0) )			\
		WATER_PUMP_OFF();        					\
	else if ( (group == 0) && (stirr_sts == 1) )	\
		WATER_PUMP_ON();                        	\
	else if ( (group == 1) && (stirr_sts == 0) ) 	\
		WATER_PUMP_OFF();                           \
	else if ( (group == 1) && (stirr_sts == 1) )	\
		WATER_PUMP_ON();                            \
  } while (0)
/*
#define SetDoubleGroupStirring(group, stirr_sts)	\
  do {												\
	if (stirr_sts == 1)                             \
        DoubleGoup_Stirring_st = 1;                 \
    else                                            \
        DoubleGoup_Stirring_st = 0;                 \
    if ( (group == 0) && (stirr_sts == 0) )			\
		impostaDutyStirring(0);        				\
	else if ( (group == 0) && (stirr_sts == 1) )	\
		impostaDutyStirring(4);                     \
	else if ( (group == 1) && (stirr_sts == 0) ) 	\
		impostaDutyStirring(0);                     \
	else if ( (group == 1) && (stirr_sts == 1) )	\
		impostaDutyStirring(4);                     \
  } while (0)
*/
/**
 * PREDICATES
 ******************************************************************************/

/** Is this actuator a BASE pump? */
#define isBaseCircuit(x)                        \
  (x <= B8_BASE_IDX)

/** .. or a COLORANT? */
#define isColorCircuit(x)                       \
  (x > B8_BASE_IDX && x <= (C24_COLOR_ID-1))

#define isColorantActEnabled(x)                 \
  (isSlaveCircuitEn(x))

#define isResetRecircPending(x)                 \
  (colorAct[x].command.cmd == CMD_IDLE)

/** Act is DISPENSING */
#define isColorActSupplyRun(x)                  \
  (colorAct[x].colorFlags.supply_run)
/** Act is done DISPENSING */
#define isColorActSupplyEnd(x)                  \
  (colorAct[x].colorFlags.supply_end)

/** Act is RECIRCULATING */
#define isColorActRecircRun(x)                  \
  (colorAct[x].colorFlags.recirc_run)
/** Act is done RECIRCULATING */
#define isColorActRecircEnd(x)                  \
  (colorAct[x].colorFlags.recirc_end)

/** Act is in ERROR state */
#define isColorActError(x)                                       \
  (colorAct[x].colorFlags.homing_pos_error           ||          \
   colorAct[x].colorFlags.tout_error                 ||          \
   colorAct[x].colorFlags.reset_error                ||          \
   colorAct[x].colorFlags.supply_calc_error          ||          \
   colorAct[x].colorFlags.software_error             ||          \
   colorAct[x].colorFlags.overcurrent_error          ||          \
   colorAct[x].colorFlags.homing_back_error          ||          \
   colorAct[x].colorFlags.pos0_read_light_error      ||          \
   colorAct[x].colorFlags.end_stroke_read_dark_error ||          \
   colorAct[x].colorFlags.drv_over_curr_temp_error   ||          \
   colorAct[x].colorFlags.open_load_error)

/** Actuator has been told to go HOME */
#define isColorCmdHoming(x)                     \
  (colorAct[(x)].command.cmd == CMD_HOME)

/** Actuator has been told to RECIRCULATE */
#define isColorCmdRec(x)                     \
  (colorAct[(x)].command.cmd == CMD_RECIRC)

/** Actuator is READY */
#define isColorActReady(x)                      \
  (colorAct[x].colorFlags.ready)

/** Actuator has come HOME */
#define isColorActHoming(x)                     \
  (colorAct[x].colorFlags.homing)

/** Actuator is stopped */
#define isColorActStopped(x)                    \
  (colorAct[x].colorFlags.stopped)

#define isRecircBeforeSupplyDone(x)                     \
  (procGUI.recirc_before_supply_status & (1L << x))

#define isSimultaneousColorDispensing(x)                \
  (procGUI.simultaneous_colors_status & (1L << x))

/**
 * FUNCTION PROTOTYPES
 ******************************************************************************/
extern void makeColorActMessage(uartBuffer_t*, unsigned char);
extern void decodeColorActMessage(uartBuffer_t*, unsigned char);
extern void setColorActMessage(unsigned char, unsigned char);

extern void setColorRecirc(void);
extern void setColorSupply(void);
extern void setColorSupplyBasesColorant(unsigned short Bases_Vol, unsigned char Colorants);

extern unsigned char isAllCircuitsSupplyHome(void);
extern unsigned char isAllCircuitsSupplyRun(void);
extern unsigned char isAllCircuitsSupplyEnd(void);

extern unsigned char isCircuitsReady(unsigned char first_id, unsigned char last_id);
extern unsigned char isCircuitsHoming(unsigned char first_id, unsigned char last_id);
extern unsigned char isCircuitsStartedHoming(unsigned char first_id, unsigned char last_id);
extern unsigned char isBasesMotorCircuitsRunning(void);
extern unsigned char isAllCircuitsStopped(void);

extern void stopAllCircuitsAct(void);
extern void DiagColorSupply(void);

extern void DiagColorReshuffle(void);
extern void DiagColorRecirc(void);
extern void DiagColorSet_EV(void);
extern void DiagColorClean(void);

// Diagnostic processes 
extern void initColorDiagProcesses();

// Periodic processes (STANDBY, ALARM) 
extern void initColorStandbyProcesses(void);
extern void standbyProcesses(void);
extern void resetStandbyProcesses(void);

extern void checkDiagColorSupplyEnd(void);
extern void checkDiagColorRecircEnd(void);

extern void intrCircuitAct(unsigned char first_id, unsigned char last_id);
extern void resetCircuitAct(unsigned char first_id, unsigned char last_id);

extern void controlRecircStirringColor(unsigned char id, unsigned char type_cmd);
extern void stopColorActs(unsigned char from, unsigned char to);

extern void calcSupplyPar(unsigned long vol_t,
                          unsigned long vol_mu,
                          unsigned long vol_mc,
                          unsigned char id);

extern void checkCircuitsDispensationAct(unsigned char canDispensing);

extern unsigned char isAllBasesRecircBeforeFilling(void);
extern void recircResetCircuitAct(unsigned char first_id, unsigned char last_id);
extern unsigned char isCircuitsRecircEnd(unsigned char first_id, unsigned char last_id);
extern unsigned char isAllCircuitsHome(void);
extern unsigned char checkAllRequiredCircuitsEnabled();
extern void DoubleGroupContinuousManagement(void);
extern unsigned char isFormulaColorants(void);
extern unsigned char isFormulaBases(void);
extern void checkBasesDispensationAct(unsigned char canDispensing);
extern void checkColorantDispensationAct(unsigned char canDispensing);
extern unsigned char isAllBasesSupplyEnd(void);
extern unsigned char isDiagColorCircuitEn(unsigned char circuitID);
extern unsigned char isAllBasesSupplyHome(void);
extern unsigned char isAllColorantsSupplyHome(void);
extern unsigned char getAttuatoreAttivo(unsigned char attuatore);
extern void impostaDutyStirring(char val);
extern void countDoubleGroupAct(void);
extern void impostaDutyStiring(char val);


#endif /* _COLOR_ACT_H_ */
