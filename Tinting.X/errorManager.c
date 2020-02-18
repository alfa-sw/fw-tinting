/* 
 * File:   errorManager.c
 * Author: michele.abelli
 * Description: Errors  management
 * Created on 21 Maggio 2019, 9.36
 */

#include "errorManager.h"
#include "serialCom.h"
#include "tintingmanager.h"
#include "p24FJ256GB110.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ram.h"
#include "define.h"
#include "gestIO.h"
#include "timerMg.h"
#include "mem.h"
#include "typedef.h"
#include "stepperParameters.h"
#include "tintingManager.h"
#include "serialCom_GUI.h"
#include "autocapAct.h"

/* Module data */
static unsigned short manual_error_code;
static unsigned short alarm_error_code;

void checkMMTTimeoutConditions()
{
    int i;
    /**
    * Check alarm conditions from TIMEOUT
    */
    for (i = 0; i < (N_SLAVES-1); ++ i) {
        /* if actuator is not enabled, skip it */
        if (! isColorantActEnabled(i))
            continue;
        if (isSlaveTimeout(i))
            setAlarm(TIMEOUT_COM_MAB_ACT + i);
    }
}

void checkColorantAlarmConditions()
{
    int i;    
    /**
    * Check alarm conditions from COLORANT acts
    */		
    for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
        /* if actuator is not enabled, skip it */
        if (! isColorantActEnabled(i))
            continue;

    // A local copy 
    union colorFlags_t flags = colorAct[i].colorFlags;

    if (flags.homing_pos_error) {
      setAlarm(B1_COLOR_HOME_POS_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.tout_error) {
      setAlarm(B1_BASE_TOUT_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.reset_error) {
      setAlarm(B1_BASE_RESET_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.software_error) {
      setAlarm(B1_SOFTWARE_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.overcurrent_error) {
      setAlarm(B1_OVERCURRENT_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.homing_back_error) {
      setAlarm(B1_COLOR_HOME_BACK_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.pos0_read_light_error) {
      setAlarm(B1_COLOR_POS0_READ_LIGHT_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.end_stroke_read_dark_error) {
      setAlarm(B1_COLOR_END_STROKE_READ_DARK_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.open_load_error) {
      setAlarm(B1_COLOR_OPEN_LOAD_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.drv_over_curr_temp_error) {
      setAlarm(B1_COLOR_DRV_OVER_CURR_TEMP_ERROR + i);
      resetStandbyProcessesSingle(i);
    }
    else if (flags.supply_calc_error) {
      setAlarm(B1_SUPPLY_CALC_ERROR + i);
      /* just a software error, no need to */
      /* resetStandbyProcessesSingle(i); */
    }
  }
}

void updateIndicators()
/**/
/*==========================================================================*/
/**
**   @brief  Update reserve information received from Actuators
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  int i;
  // Colorant reserve indicators (32 bits)
  procGUI.Color_res = 0L;
  for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) 
  {
  	// 13-07-2017 - Used from Testing Bench
  	if (MachineStatus.level == DIAGNOSTIC_ST)
  	{
	    if (colorAct[i].usw_reserveDiag)
	      procGUI.Color_res |= (1L << i);
  	}
	else
	{
	    if (colorAct[i].usw_reserve)
	      procGUI.Color_res |= (1L << i);
	}
  }
} // updateIndicators()

unsigned short alarm(void)
/**/
/*==========================================================================*/
/**
**   @brief  Gets alarm error code
**
**   @return alarm error code
**/
/*==========================================================================*/
/**/
{
  return alarm_error_code;
}

void setNextStatus(unsigned char status)
/**/
/*==========================================================================*/
/**
**   @brief  External interface for status change
**
**   @param  status - next status for toplevel FSM
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  nextStatus = status;
}

void CheckAlarmCondition()
/**/
/*===========================================================================*/
/**
**   @brief  Check alarm conditions
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  if (isAlarmEvaluable()) {

    /**
     * User interrupt?
     */
    if (isHaltButtonPressed()){
	  Stop_Process = TRUE;	
      setAlarm(USER_INTERRUPT);
	}
    if (isGUIAbortCmd()){
	  Stop_Process = TRUE;			
      setAlarm(USER_INTERRUPT);
	}
    // Timeouts
    checkMMTTimeoutConditions();
    checkColorantAlarmConditions();
    checkAutocapAlarmConditions();
    checkTintingAlarmConditions();

  } // if (isAlarmEvaluable())
} // CheckAlarmCondition() 

unsigned short checkAlarm(unsigned short allarme)
{
	unsigned short ris=allarme;
	unsigned short attuatore;
	
	if ((allarme < TIMEOUT_COM_MAB_ACT) || (allarme >= TIMEOUT_COM_MAB_MGB)) 
		// We decide if reset the alarm or not only in case of a actuator timeout
		return allarme;

	attuatore = allarme - TIMEOUT_COM_MAB_ACT;
	switch (MachineStatus.level) {
        case DIAGNOSTIC_ST:
            if (getNumErroriSerial(attuatore) >= NUM_MAX_ERROR_TIMEOUT)
                return allarme;
                
            if (attuatoreAttivo[attuatore])
                // I've lost the communication with an actuator in movement - alarm
                return allarme;
            else
                // The actuator is currently in standby - I don't signal any alarm
                return NO_ALARM;

        break;
        
        case COLOR_RECIRC_ST:
            if (getNumErroriSerial(attuatore) >= NUM_MAX_ERROR_TIMEOUT)
                return allarme;
            
            if (attuatoreAttivo[attuatore])
                // I've lost the communication with an actuator in movement - alarm
                return allarme;
            else
                // The actuator is currently in standby - I don't signal any alarm
                return NO_ALARM;

        break;
        
        case COLOR_SUPPLY_ST:
            if (getNumErroriSerial(attuatore) >= NUM_MAX_ERROR_TIMEOUT)
                return allarme;
            
            if (attuatoreAttivo[attuatore])
                // I've lost the communication with an actuator in movement - alarm
                return allarme;
            else
                // The actuator is currently in standby - I don't signal any alarm
                return NO_ALARM;
        break;
        
        default:
            if (getNumErroriSerial(attuatore) >= NUM_MAX_ERROR_TIMEOUT)
                return allarme;

            return NO_ALARM;
        break;
	}
	return ris;
}

void setAlarm(unsigned short error_code)
/**/
/*==========================================================================*/
/**
**   @brief  Set the fatal error code and write the log
**
**   @param  error_code code of fatal error
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  if (alarm_error_code == NO_ALARM) {
    alarm_error_code = error_code;

    if (error_code >= MANUAL_INTERVENTION_REQUEST0 &&
        error_code <= MANUAL_INTERVENTION_REQUEST7) {
      manual_error_code = error_code;
    }

	alarm_error_code=checkAlarm(alarm_error_code);

	if (alarm_error_code!=NO_ALARM)
	{
		setNextStatus(ALARM_ST);
	}
  }
}

void resetAlarm(void)
/**/
/*===========================================================================*/
/**
**   @brief Reset alarm code
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  alarm_error_code = NO_ALARM;
}

void resetManualAlarm(void)
/**/
/*===========================================================================*/
/**
**   @brief Reset alarm code
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  manual_error_code = NO_ALARM;
}

void forceAlarm(unsigned short error_code)
/**/
/*===========================================================================*/
/**
**   @brief Forces given alarm code, overriding any existing alarm
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/

{
  resetAlarm();
  setAlarm(error_code);
}

int isAlarmEvaluable(void)
/**/
/*===========================================================================*/
/**
**   @brief Returns true if alarm condition can be evaluated
**
**   @param void
**
**   @return TRUE/FALSE
**/
/*===========================================================================*/
/**/
{
	
  if (MachineStatus.level == INIT_ST ||
      MachineStatus.level == IDLE_ST ||
      MachineStatus.level == RESET_ST ||
      MachineStatus.level == AUTOTEST_ST)
    return FALSE;

  return MachineStatus.phase == RUN_PH;
}

void checkAutocapAlarmConditions()
{
  /**
   * Check alarm conditions from AUTOCAP act
   */
  if (isAutocapActEnabled()) {
    /* a local copy */
    union autocapFlags_t flags =
      autocapAct.autocapFlags;

    if (flags.homing_error)
      setAlarm(AUTOCAP_HOMING_ERROR);
    else if (flags.packing_error)
      setAlarm(AUTOCAP_PACK_POS_ERROR);
    else if (flags.home_pos_error)
      setAlarm(AUTOCAP_HOME_POS_ERROR);
    else if (flags.tout_error)
      setAlarm(AUTOCAP_TOUT_ERROR);
    else if (flags.lifter_error)
      setAlarm(AUTOCAP_PACK_POS_ERROR);
    else if (flags.software_error)
      setAlarm(AUTOCAP_SOFTWARE_ERROR);
    else if (flags.open_load_error)
      setAlarm(AUTOCAP_OPEN_LOAD_ERR);
    else if (flags.over_curr_temp_error)
      setAlarm(AUTOCAP_DRV_OVER_CURR_TEMP_ERR);  
  }
}
void checkTintingAlarmConditions()
{
  /**
   * Check alarm conditions from TINTING and HUMIDIFIER act
   */
// -----------------------------------------------------------------------------    
	if (Status.level == TINTING_TABLE_SOFTWARE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_SOFTWARE_ERROR);					
	}
	else if (Status.level == TINTING_PUMP_HOMING_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_HOME_POS_ERROR);
	}
	else if (Status.level == TINTING_VALVE_HOMING_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_HOME_POS_ERROR);
	}
	else if (Status.level == TINTING_TABLE_HOMING_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_HOME_POS_ERROR);
	}
	else if (Status.level == TINTING_PUMP_RESET_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_RESET_ERROR);
	}
	else if (Status.level == TINTING_VALVE_RESET_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_RESET_ERROR);
	}
	else if (Status.level == TINTING_TABLE_RESET_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_RESET_ERROR);
	}
	else if (Status.level == TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_POS0_READ_LIGHT_ERROR);
	}
	else if (Status.level == TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_OVERCURRENT_ERROR);
	}
	else if (Status.level == TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_OVERCURRENT_ERROR);
	}
	else if (Status.level == TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_OVERCURRENT_ERROR);
	}
	else if (Status.level == TINTING_PUMP_HOMING_BACK_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_HOME_BACK_ERROR);
	}
	else if (Status.level == TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_HOME_BACK_ERROR);
	}
/*    
	else if (Status.level == TINTING_BAD_PAR_TABLE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_BAD_TABLE_PARAM_ERROR);	
	}
	else if (Status.level == TINTING_BAD_PAR_PUMP_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_BAD_PUMP_PARAM_ERROR);		
	}
*/
	else if (Status.level == TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_BAD_PERIPH_PARAM_ERROR);		
	}
	else if (Status.level == TINTING_TIMEOUT_TABLE_MOVE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TIMEOUT_TABLE_MOVE_ERROR);
	}
	else if (Status.level == TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR);			
	}
	else if (Status.level == TINTING_LACK_CIRCUITS_POSITION_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_LACK_OF_CIRCUITS_POSITION_ERROR);			
	}
	else if (Status.level == TINTING_TIMEOUT_SELF_LEARNING_PROCEDURE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TIMEOUT_SELF_LEARNING_PROCEDURE_ERROR);			
	}
	else if (Status.level == TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_SELF_LEARNING_PROCEDURE_ERROR);			
	}
	else if (Status.level == TINTING_TABLE_MOVE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		if (procGUI.id_color_circuit >= TINTING_COLORANT_OFFSET)
			setAlarm(C1_TURN_TABLE_MOVE_ERROR + procGUI.id_color_circuit - TINTING_COLORANT_OFFSET);
		else
			setAlarm(TINTING_TABLE_MOVE_ERROR);				
	}
	else if (Status.level == TINTING_TABLE_MISMATCH_POSITION_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(C1_TURN_TABLE_MISMATCH_POSITION_ERROR + Status.errorCode);			
	}
	else if (Status.level == TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR);			
	}
	else if (Status.level == TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR);			
	}
	else if (Status.level == TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_1_POS0_READ_LIGHT_ERROR);					
	}
	else if (Status.level == TINTING_PUMP_SOFTWARE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_SOFTWARE_ERROR);
	}
	else if (Status.level == TINTING_TABLE_TEST_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_TEST_ERROR);							
	}
	else if (Status.level == TINTING_GENERIC24V_OPEN_LOAD_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_BRUSH_OPEN_LOAD_ERROR);
	}
	else if (Status.level == TINTING_GENERIC24V_OVERCURRENT_THERMAL_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_BRUSH_OVERCURRENT_THERMAL_ERROR);
	}
	else if (Status.level == TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR);
	}
	else if (Status.level == TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR);
	}
	else if (Status.level == TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR);
	}
	else if (Status.level == TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR);
	}
	else if (Status.level == TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR);
	}
	else if (Status.level == TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR);
	}
	else if (Status.level == TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(EEPROM_TINTING_COLORANTS_STEPS_POSITION_CRC_FAULT);
	}
	else if (Status.level == TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_TABLE_PHOTO_READ_LIGHT_ERROR);		
	}
	else if (Status.level == TINTING_VALVE_OPEN_READ_DARK_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_2_READ_DARK_ERROR);		
	}
	else if (Status.level == TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_VALVE_2_READ_LIGHT_ERROR);		
	}		
	else if (Status.level == TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR);		
	}
	else if (Status.level == TINTING_PANEL_TABLE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_PANEL_TABLE_ERROR);		
	}
	else if (Status.level == TINTING_BRUSH_READ_LIGHT_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_BRUSH_READ_LIGHT_ERROR);		
	}
	else if (Status.level == TINTING_BASES_CARRIAGE_ERROR_ST) {
		resetStandbyProcessesTinting();			
		setAlarm(TINTING_BASES_CARRIAGE_ERROR);		
	}    
}

int isDeviceGlobalError(void)
/**/
/*===========================================================================*/
/**
**   @brief Returns true in case of device global error (i.e parameter error)
**
**   @param void
**
**   @return TRUE/FALSE
**/
/*===========================================================================*/
/**/
{
	     return
		InitFlags.CRCParamColorCircuitFailed ||
		InitFlags.CRCParamCalibCurvesFailed  ||
		InitFlags.CRCParamSlavesEnFailed     ||
		InitFlags.CRCParamCircuitPumpTypesFailed	||
		InitFlags.CRCParamHumidifier_paramFailed	||
		InitFlags.CRCParamTinting_Table_paramFailed ||
		InitFlags.CRCParamTinting_Pump_paramFailed  ||
		InitFlags.CRCParamTinting_Clean_paramFailed	
		;
}

int isSkipStandbyForError(int id)
/**/
/*===========================================================================*/
/**
**   @brief Return TRUE if standby on circuit id must be skipped
**
**   @param unsigned char id
**
**   @return TRUE/FALSE
**/
/*===========================================================================*/
/**/
{
    int ret = FALSE;
    // Slave errors
    if ((colorAct[id].colorFlags.homing_pos_error) || (colorAct[id].colorFlags.tout_error) ||
        (colorAct[id].colorFlags.reset_error) || (colorAct[id].colorFlags.supply_calc_error))
        ret = TRUE;

    // Skip acts declared in TIMEOUT 
    if ((unsigned short) id == (alarm_error_code - TIMEOUT_COM_MAB_ACT))
        ret = TRUE;
    
  return ret;
}

int isAlarmRecoverable()
/**/
/*===========================================================================*/
/**
**   @brief Return TRUE if current alarm is recoverable
**
**   @return TRUE/FALSE
**/
/*===========================================================================*/
/**/
{
    // trivial case
    if (! alarm_error_code)
        return TRUE;

    return
        (alarm_error_code >= B1_COLOR_HOME_POS_ERROR &&
        alarm_error_code <= C24_COLOR_HOME_POS_ERROR) ||

        (alarm_error_code >= B1_COLOR_HOME_BACK_ERROR &&
        alarm_error_code <= C24_COLOR_HOME_BACK_ERROR) ||

        (alarm_error_code >= B1_COLOR_POS0_READ_LIGHT_ERROR &&
        alarm_error_code <= C24_COLOR_POS0_READ_LIGHT_ERROR) ||

        (alarm_error_code >= B1_COLOR_END_STROKE_READ_DARK_ERROR &&
        alarm_error_code <= C24_COLOR_END_STROKE_READ_DARK_ERROR) ||

        (alarm_error_code >= B1_OVERCURRENT_ERROR &&
        alarm_error_code <= C24_OVERCURRENT_ERROR) ;
}

void monitorManager()
/**/
/*==========================================================================*/
/**
**   @brief  Sequencer of the module
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
   CheckAlarmCondition();
   updateIndicators();
}
