/* 
 * File:   pumpmanager.c
 * Author: michele.abelli
 * Description: Pump Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "pumpManager.h"
#include "statusManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "stepper.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "typedef.h"
#include "humidifierManager.h"
#include "stepperParameters.h"
#include "stepper.h"
#include "spi.h"
#include "L6482H.h"
#include <xc.h>

static cSPIN_RegsStruct_TypeDef  cSPIN_RegsStruct3 = {0};  //to set

/*
*//*=====================================================================*//**
**      @brief Initialization status
**
**      @param void
**
                                                                              * 
**      @retval void
**
*//*=====================================================================*//**
*/
void initPumpStatusManager(void)
{
	Pump.level = PUMP_IDLE;
}

/*
*//*=====================================================================*//**
**      @brief Pump Initialization parameters
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void initPumpParam(void)
{
  // Passi da fotocellula madrevite coperta a fotocellula ingranamento coperta
  TintingAct.Step_Accopp = STEP_ACCOPP;
  // Passi a fotoellula ingranamento coperta per ingaggio circuito
  TintingAct.Step_Ingr = STEP_INGR;
  // Passi per recupero giochi
  TintingAct.Step_Recup = STEP_RECUP;
  // Passi a fotocellula madrevite coperta per posizione di home
  TintingAct.Passi_Madrevite = PASSI_MADREVITE;
  // Passi per raggiungere la posizione di start ergoazione in alta risoluzione
  TintingAct.Passi_Appoggio_Soffietto = (unsigned long)TOT_PASSI_APPOGGIO_SOFFIETTO;
  // Velocità da fotocellula madrevite coperta a fotocellula ingranamento coperta
  TintingAct.V_Accopp = V_ACCOPP;
  // Velocità a fotoellula ingranamento coperta per ingaggio circuito
  TintingAct.V_Ingr = V_INGR;
  // Velocità per raggiungere la posizione di start ergoazione in alta risoluzione
  TintingAct.V_Appoggio_Soffietto = V_APPOGGIO_SOFFIETTO;
  // Passi da posizione di home/ricircolo (valvola chiusa) a posizone di valvola aperta su fori grande (3mm) e piccolo(0.8mm))
  TintingAct.Step_Valve_Open = STEP_VALVE_OPEN;
  // Passi da posizione di home/ricircolo (valvola chiusa) a posizone di backstep (0.8mm))
  TintingAct.Step_Valve_Backstep = STEP_VALVE_BACKSTEP;
  // Velocità di apertura/chiusura valvola
  TintingAct.Speed_Valve = SPEED_VALVE;
  // N. steps in una corsa intera
  TintingAct.N_steps_stroke = N_STEPS_STROKE; 
  // Back step N. before to Open valve
  TintingAct.N_step_back_step = PUMP_STEP_BACKSTEP; 
  // Back Step Speed (rpm) before to Open Valve
  TintingAct.Speed_back_step = PUMP_SPEED_BACKSTEP;

  // No peripheral selected
  PeripheralAct.Peripheral_Types.bytePeripheral = 0;
  TintingAct.Output_Act = OUTPUT_OFF;
}    
/*
*//*=====================================================================*//**
**      @brief Updates Pump status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void PumpManager(void)
{
    switch(Pump.level)
    {
        case PUMP_IDLE:
            if (Status.level == TINTING_WAIT_PUMP_PARAMETERS_ST) {
                if ( AnalyzePumpParameters() == TRUE) {
                    Pump.level = PUMP_PAR_RX;
                    NextPump.level = PUMP_START;
                }
                else
                    Pump.level = PUMP_PAR_ERROR;
            } 
        break;

		case PUMP_PAR_RX:
			if ( (Status.level != TINTING_WAIT_PUMP_PARAMETERS_ST) &&
                 (Status.level != TINTING_WAIT_SETUP_OUTPUT_VALVE_ST) )
                Pump.level = NextPump.level;
			// STOP PROCESS command received
            else if (Status.level == TINTING_STOP_ST) { 
                Pump.level = PUMP_IDLE;
			}                        
        break;
        
        case PUMP_START:
            if (Status.level == TINTING_WAIT_PUMP_PARAMETERS_ST) {
                if ( AnalyzePumpParameters() == TRUE) {
                    Pump.level = PUMP_PAR_RX;
                    NextPump.level = PUMP_START;
                }
                else
                    Pump.level = PUMP_PAR_ERROR;
            }
            // New Erogation Command Request
            else if (Status.level == TINTING_SUPPLY_RUN_ST)
                Pump.level = PUMP_SETUP; 
            // New Ricirculation Command Received
            else if (Status.level == TINTING_STANDBY_RUN_ST)
                Pump.level = PUMP_SETUP; 
            // New Pump Homing Command Received
            else if ( (Status.level == TINTING_PUMP_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST) ) {
                Pump.level = PUMP_HOMING;
                Pump.step = STEP_0;
            }
            // New Valve Homing Command Received
            else if ( (Status.level == TINTING_VALVE_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST) ) {
                Pump.level = VALVE_HOMING;
                Pump.step = STEP_0;
            }
            // New Valve Open/Close Command Received
            else if (Status.level == TINTING_WAIT_SETUP_OUTPUT_VALVE_ST) {
                Pump.level = PUMP_SETUP_OUTPUT;
                Pump.step = STEP_0;
            }                
        break;
            
        case PUMP_SETUP:
            if ( (Status.level == TINTING_SUPPLY_RUN_ST) && ( (TintingAct.Algorithm == ALG_SINGLE_STROKE) || (TintingAct.Algorithm == HIGH_RES_STROKE) ) ) {
                // Analyze Formula
                if (AnalyzeFormula() == TRUE)  {
                    Pump.level = PUMP_RUNNING;
                    Pump.step = STEP_0;
                } 
                else
                    Pump.level = PUMP_ERROR;                 
            }
            else if ( (Status.level == TINTING_SUPPLY_RUN_ST) && (TintingAct.Algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) {
                // Analyze Formula
                if (AnalyzeContinuousFormula() == TRUE)  {
                    Pump.level = PUMP_RUNNING;
                    Pump.step = STEP_0;
                } 
                else
                    Pump.level = PUMP_ERROR;                 
            }
            else if (Status.level == TINTING_STANDBY_RUN_ST) {
                // Analyze Ricirculation Command
                if (AnalyzeRicirculationCommand() == TRUE)  {
                    Pump.level = PUMP_RICIRCULATION;
                    Pump.step = STEP_0;
                } 
                else
                    Pump.level = PUMP_ERROR;                 
            }       
        break;

        case PUMP_RUNNING:
            
            if (TintingAct.Algorithm == ALG_SINGLE_STROKE) {
//Pump.level = PUMP_END;                
                if (SingleStrokeColorSupply() == PROC_OK)  {
                    Pump.level = PUMP_END;
                }    
                else if (SingleStrokeColorSupply() == PROC_FAIL)  {
                    Pump.level = PUMP_ERROR; 
                }                   
            } 
            else if (TintingAct.Algorithm == HIGH_RES_STROKE) {     
//Pump.level = PUMP_END;
                if (HighResColorSupply() == PROC_OK) {
                    Pump.level = PUMP_END;
                }
                else if (HighResColorSupply() == PROC_FAIL) {
                    Pump.level = PUMP_ERROR; 
                }               
            }
            else if (TintingAct.Algorithm == ALG_ASYMMETRIC_CONTINUOUS) {     
//Pump.level = PUMP_END;                
                if (ContinuousColorSupply() == PROC_OK) {
                    Pump.level = PUMP_END;
                }
                else if (ContinuousColorSupply() == PROC_FAIL) {
                    Pump.level = PUMP_ERROR;
                }                
            }            
            else
                Pump.level = PUMP_ERROR;                                                
        break;
        
        case PUMP_HOMING:
//Pump.level = PUMP_END;                        
            if (PumpHomingColorSupply() == PROC_OK) {
                Pump.level = PUMP_END;
            }    
            else if (PumpHomingColorSupply() == PROC_FAIL) {
               Pump.level = PUMP_ERROR; 
            }             
        break;

        case VALVE_HOMING:            
//Pump.level = PUMP_END;            
            if (ValveHomingColorSupply() == PROC_OK) {
                Pump.level = PUMP_END;
            }    
            else if (ValveHomingColorSupply() == PROC_FAIL) {
               Pump.level = PUMP_ERROR;
            }    
        break;
                                    
        case PUMP_SETUP_OUTPUT:
            if ( AnalyzeSetupOutputs() == TRUE) {
                Pump.level = PUMP_PAR_RX;
                NextPump.level = PUMP_VALVE_OPEN_CLOSE;
            }
            else
                Pump.level = PUMP_ERROR;
        break;

        case PUMP_VALVE_OPEN_CLOSE:
//Pump.level = PUMP_END;
            if (ValveOpenClose() == PROC_OK)
                Pump.level = PUMP_END;
            else if (ValveOpenClose() == PROC_FAIL)
               Pump.level = PUMP_ERROR;                           
        break;
        
        case PUMP_RICIRCULATION:
//Pump.level = PUMP_END;
            if (RicirculationColorSupply() == PROC_OK) {
                Pump.level = PUMP_END;
            }
            else if (RicirculationColorSupply() == PROC_FAIL) {
                Pump.level = PUMP_ERROR;  
            }              
        break;
            
        case PUMP_END:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_SETUP_OUTPUT_VALVE_ST) && (Status.level != TINTING_PAR_RX) && 
                 (Status.level != TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST) )
                Pump.level = PUMP_START; 
        break;

        case PUMP_ERROR:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST)         && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_SETUP_OUTPUT_VALVE_ST) && (Status.level != TINTING_PAR_RX) &&
                 (Status.level != TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST) )
                Pump.level = PUMP_START; 
        break;
        
        case PUMP_PAR_ERROR:
            if (Status.level == TINTING_WAIT_PUMP_PARAMETERS_ST) {
                if ( AnalyzePumpParameters() == TRUE) {
                    Pump.level = PUMP_PAR_RX;
                    NextPump.level = PUMP_START;
                }
                else
                    Pump.level = PUMP_PAR_ERROR;
            } 
        break;
            
        default:
            Pump.level = PUMP_IDLE;             
        break;            
    }        
}
/*
*//*=====================================================================*//**
**      @brief Analyze Dispensation parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char AnalyzeFormula(void)
{
    if ( (TintingAct.Algorithm != ALG_SINGLE_STROKE) && (TintingAct.Algorithm != HIGH_RES_STROKE) ) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
    if (TintingAct.N_step_stroke > TintingAct.N_step_full_stroke) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
    if ( (TintingAct.Speed_cycle == 0) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step == 0) ) ) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }        
    if (( (TintingAct.Speed_cycle > MAXIMUM_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step > MAXIMUM_SPEED) ) )) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }            
    return TRUE;
}

/*
*//*=====================================================================*//**
**      @brief Analyze Continuous Dispensation parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char AnalyzeContinuousFormula(void)
{
    if (TintingAct.Algorithm != ALG_ASYMMETRIC_CONTINUOUS) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
    if (TintingAct.N_step_stroke > TintingAct.N_step_full_stroke) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
    if (TintingAct.PosStart > TintingAct.N_step_full_stroke) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
    if (TintingAct.PosStop > TintingAct.N_step_full_stroke) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }    
    if ( (TintingAct.Speed_cycle == 0) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step == 0) ) ) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }        
    if (( (TintingAct.Speed_cycle > MAXIMUM_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step > MAXIMUM_SPEED) ) )) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }   
    if (( (TintingAct.Speed_cycle_supply > MAXIMUM_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step > MAXIMUM_SPEED) ) )) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }       
    if (TintingAct.Speed_suction > MAXIMUM_SPEED) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }     
    if (TintingAct.N_step_backlash >= TintingAct.N_step_full_stroke) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }  
    return TRUE;
}

/*
*//*=====================================================================*//**
**      @brief Analyze Ricirculation parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char AnalyzeRicirculationCommand(void)
{
    if (TintingAct.N_step_stroke > TintingAct.N_steps_stroke) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
    if (TintingAct.Speed_cycle == 0) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }        
    if (TintingAct.Speed_cycle > MAXIMUM_SPEED) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }        
    return TRUE;
}

/*
*//*=====================================================================*//**
**      @brief Analyze Pump parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char AnalyzePumpParameters(void)
{
    if ((TintingAct.Passi_Appoggio_Soffietto + TintingAct.N_step_back_step) > MAX_PASSI_APPOGGIO_SOFFIETTO)
        return FALSE;
    else
        return TRUE;
}

/*
*//*=====================================================================*//**
**      @brief Pump Homing process
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char PumpHomingColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static signed long Steps_Todo;

  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
/*  +-++
 * 
 * 
 * 
 *  \\- -+
 * +
 * 9
  if 0( ((Motor_alarm & MOT_STS) > 0) && ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) ) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Table.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & MOT_STS) > 0) && (((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE)) )  {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Table.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
 * 
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & MOT_STS) > 0) && (((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0)) ) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Table.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/    
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Pump.step = STEP_5;
  }
/*
  if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
*/  
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);  
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// CHECK HOME PHOTOCELL STATUS      
// -----------------------------------------------------------------------------      
    case STEP_0: 
//TintingAct.V_Accopp = 400;            
        SetStepperHomePosition(MOTOR_PUMP); 
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
            Pump.step++;
        }
        else {
            SetStepperHomePosition(MOTOR_PUMP); 	            
            // Move motor Pump till Home Photocell transition DARK-LIGHT
            StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, DARK_LIGHT, HOME_PHOTOCELL, 0); 
            Pump.step += 3; 
        } 
    break;
// -----------------------------------------------------------------------------
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_1:        
//        if ( (PhotocellStatus(HOME_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if ((signed long)GetStepperPosition(MOTOR_PUMP) >= MAX_STEP_PUMP_HOMING) {
            HardHiZ_Stepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }        
	break;

	//  Check if position required is reached    
    case STEP_2:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step +=3;
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION 
    case STEP_3:
//        if ((PhotocellStatus(HOME_PHOTOCELL, NO_FILTER) == LIGHT) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {          
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm)
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
        else if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -MAX_STEP_PUMP_HOMING) {
            HardHiZ_Stepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }        
    break;

	// Check if position required is reached    
    case STEP_4:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;            
    break;
// -----------------------------------------------------------------------------        
    case STEP_5:
        HardHiZ_Stepper(MOTOR_PUMP);
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Valve Homing process
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char ValveHomingColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static signed long Steps_Position, Steps_Todo;
  
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
/*  
  if ( ((Motor_alarm & MOT_STS) > 0) && ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) ) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Table.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & MOT_STS) > 0) && (((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE)) )  {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Table.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & MOT_STS) > 0) && (((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0)) ) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Table.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/    
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.step = STEP_11;
  }
/*
  if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_VALVE);                                    
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
*/  
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// CHECK HOME PHOTOCELL STATUS      
// -----------------------------------------------------------------------------      
    case STEP_0:
//TintingAct.Speed_Valve = 20;  
        SetStepperHomePosition(MOTOR_VALVE); 	
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Steps_Position = GetStepperPosition(MOTOR_VALVE);
            if (Steps_Position >= 0)
                // Rotate motor Valve CW till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            else if (Steps_Position < 0)
                // Rotate motor Valve CCW till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            Pump.step++;
        }
        else {
            // Rotate Valve motor till Home Photocell transition DARK-LIGHT
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	
            Pump.step += 5; 
        }    
    break;
// -----------------------------------------------------------------------------
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_1:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){        
            SetStepperHomePosition(MOTOR_VALVE); 	
            // CCW rotation
            if (Steps_Position < 0)
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            // CW rotation
            else
                Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
                
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        }
        // No LIGHT-DARK transition in CCW direction
        else if ( (Steps_Position < 0) && ((signed long)GetStepperPosition(MOTOR_VALVE)>= MAX_STEP_VALVE_HOMING) ) {
            // Try the same movemente in CW direction
            SetStepperHomePosition(MOTOR_VALVE);
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);	
            Pump.step += 2 ;
        }
        // No LIGHT-DARK transition in CW direction
        else if ( (Steps_Position >= 0) && ((signed long)GetStepperPosition(MOTOR_VALVE)<= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) ) {
            // Try the same movemente in CCW direction
            SetStepperHomePosition(MOTOR_VALVE);
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);	
            Pump.step += 2 ;
        }  
	break;

	//  Check if position required is reached    
    case STEP_2:
/*        
        if ( (Steps_Position >= 0) && ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) 
            Pump.step +=9;
        else if ( (Steps_Position < 0) && (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) 
            Pump.step +=9;
 */
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)        
            Pump.step +=9;        
    break;
    
	//  Check if position required is reached    
    case STEP_3:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_VALVE); 	
            // CW rotation
            if (Steps_Position < 0)
                Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            // CCW rotation
            else
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        }
/*        
        // No LIGHT-DARK transition in CCW direction
        else if ( (Steps_Position >= 0) && ((signed long)GetStepperPosition(MOTOR_VALVE)>= MAX_STEP_VALVE_HOMING) ) {
            HardHiZ_Stepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }
        // No LIGHT-DARK transition in CW direction
        else if ( (Steps_Position < 0) && ((signed long)GetStepperPosition(MOTOR_VALVE)<= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) ){
            HardHiZ_Stepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }         
*/        
        // No LIGHT-DARK transition in CCW direction
        else if ( (Steps_Position >= 0) && ((signed long)GetStepperPosition(MOTOR_VALVE)>= MAX_STEP_VALVE_HOMING) ) {
            // Try the same movemente in CW direction
            SetStepperHomePosition(MOTOR_VALVE);
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);	
            Pump.step += 4 ;
        }
        // No LIGHT-DARK transition in CW direction
        else if ( (Steps_Position < 0) && ((signed long)GetStepperPosition(MOTOR_VALVE)<= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) ){
            // Try the same movemente in CCW direction
            SetStepperHomePosition(MOTOR_VALVE);
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);	
            Pump.step += 6 ;
        }                
    break;
    
	//  Check if position required is reached    
    case STEP_4:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)        
            Pump.step +=7;
/*        
        if ( (Steps_Position < 0) && ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) 
            Pump.step +=7;
        else if ( (Steps_Position >= 0) && (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) 
            Pump.step +=7;
*/
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION 
    case STEP_5: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        } 
        else if ((signed long)GetStepperPosition(MOTOR_VALVE) <= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) {
            HardHiZ_Stepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }        
    break;

	// Check if position required is reached    
    case STEP_6:
//        if ( ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )                
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)        
            Pump.step += 5;
    break;

// GO TO HOME POSITION 
    case STEP_7: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        } 
        else if ((signed long)GetStepperPosition(MOTOR_VALVE) >= MAX_STEP_VALVE_HOMING) {
            HardHiZ_Stepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }        
    break;

	// Check if position required is reached    
    case STEP_8:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)        
//        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )                
        Pump.step += 3;
    break;    

    case STEP_9: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        } 
        else if ((signed long)GetStepperPosition(MOTOR_VALVE) <= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) {
            HardHiZ_Stepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }        
    break;

	// Check if position required is reached    
    case STEP_10:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)                
//        if ( ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )                
        Pump.step ++;
    break;    
// -----------------------------------------------------------------------------        
    case STEP_11:
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);                        
        TintingAct.OpenValve_BigHole_state = OFF;  
        TintingAct.OpenValve_SmallHole_state = OFF;          
        SetStepperHomePosition(MOTOR_VALVE); 		
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Recirculation process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char  RicirculationColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static unsigned short count;
  static signed long Steps_Todo;
  unsigned char currentReg;
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/ 
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Pump.step = STEP_15;
  }
 
  if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
/*  
  if (isColorCmdStop() ) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Pump.step = STEP_15;
  }
*/  
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with Bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else {
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            //ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                          
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);
            count = 0;
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == DARK) )  {
            HardHiZ_Stepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == LIGHT) ) {
            HardHiZ_Stepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            SetStepperHomePosition(MOTOR_PUMP);
            Pump.step ++ ;
        }        
	break;

	//  Gear Movement (1.5mm))
	case STEP_3:
        Steps_Todo = TintingAct.Step_Ingr;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_4:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break;

	//  Handling for games recovery (1.075mm)
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------        
// RICIRCULATION    
	//  Start Ricrculation
	case STEP_7:
if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
    pippo = 1;
}    
    
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_8:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Recup)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
			Durata[T_PAUSE_RECIRC] = (unsigned short)(TintingAct.Recirc_pause) * CONV_SEC_COUNT;
			// Start Ricirculation Pause
            StartTimer(T_PAUSE_RECIRC);
            HardHiZ_Stepper(MOTOR_PUMP);                    
            Pump.step ++;
        }    
	break; 	

	// Wait Ricirculation Pause
	case STEP_9:
        if (isColorCmdStop() )
			Pump.step ++ ;
		else if (StatusTimer(T_PAUSE_RECIRC) == T_ELAPSED) {
            StopTimer(T_PAUSE_RECIRC);
			Pump.step ++ ;
		}		
	break; 	
// -----------------------------------------------------------------------------  
// CHECK Cycles Number    
    case STEP_10:
        if (isColorCmdStop() )
            count = TintingAct.N_cycles;
        else
            count++;
            
		if (count < TintingAct.N_cycles) {
			// Got to SUCTION
			Pump.step++; 
		}
		else {
            // Start decoupling
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.Speed_cycle, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);            
			Pump.step += 3;  
		}
    break;
// -----------------------------------------------------------------------------        
// SUCTION
    // Start Suction
    case STEP_11:
        Steps_Todo = -TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);        
        Pump.step++;
    break;
    
	// Check if position required is reached    
    case STEP_12:
        if ((-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                HardHiZ_Stepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else
                Pump.step = STEP_7;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_13:
//        if ( (PhotocellStatus(HOME_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_14:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_15:
        HardHiZ_Stepper(MOTOR_PUMP);        
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief HIGH RES STROKE dispensation algorithm
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char HighResColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static unsigned char Wait_Before_BackStep;
  static signed long Steps_Todo;
  unsigned char currentReg;
//  unsigned short accentReg = 0; 
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }  
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/ 
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.step = STEP_27;
  }

  if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    HardHiZ_Stepper(MOTOR_VALVE);        
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }

  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else {
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
//            ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                                      
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == DARK) )  {
            HardHiZ_Stepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == LIGHT) ) {
            HardHiZ_Stepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            SetStepperHomePosition(MOTOR_PUMP);
            Pump.step ++ ;
        }        
	break;

	//  Gear Movement (1.5mm))
	case STEP_3:
        Steps_Todo = TintingAct.Step_Ingr;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_4:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break;

	//  Handling for games recovery (1.075mm)
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// GO to HIGH RES Erogation Starting Point
	//  Bellows head support movement (15.335mm))
    case STEP_7:
        Steps_Todo = (TintingAct.Passi_Appoggio_Soffietto - TintingAct.Step_Recup);
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);
		Pump.step++;
	break;
	
	//  Check if position required is reached and Home Photocell is LIGHT
    case STEP_8:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                Steps_Todo = TintingAct.N_step_back_step; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
                StopTimer(T_DELAY_BEFORE_VALVE_BACKSTEP);
                Wait_Before_BackStep = 0;
                Pump.step ++;
            }    
		}
	break;
// -----------------------------------------------------------------------------    
// BACKSTEP execution  
	// Check if Valve Photocell is DARK at the end of Backstep compensation movement
    case STEP_9:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Ingr) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {		
            if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                if (Wait_Before_BackStep == 0) {
                    Durata[T_DELAY_BEFORE_VALVE_BACKSTEP] = TintingAct.Delay_Before_Valve_Backstep / T_BASE;
                    StartTimer(T_DELAY_BEFORE_VALVE_BACKSTEP);
                    Wait_Before_BackStep = 1;
                }
                else if (StatusTimer(T_DELAY_BEFORE_VALVE_BACKSTEP)==T_ELAPSED)   
                    Pump.step ++;
            }    
        }     	
    break;

	//  Valve towards Backstep Small hole (0.8mm)        
    case STEP_10:
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	                    
        Pump.step ++ ;
	break;

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_VALVE);
            // Set Maximum Ramp Acceleration / Deceleration to Valve Motor
            ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
                          MAX_ACC_RATE_VALVE, MAX_DEC_RATE_VALVE, ALARMS_VALVE);    
            
/*
            accentReg = TABLE_ACC_DEC[MAX_ACC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.ACC = accentReg;          
            cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
            accentReg = TABLE_ACC_DEC[MAX_DEC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.DEC = accentReg;          
            cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);
*/
            Steps_Todo = - TintingAct.Step_Valve_Backstep;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);        
            Pump.step ++ ;            
        }
        else if ( (GetStepperPosition(MOTOR_VALVE) >= ((signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            HardHiZ_Stepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }    
    break;

	//  Check if position required is reached: Valve in BACKSTEP position
	case STEP_12:            
//        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= ((signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            Steps_Todo = -(TintingAct.N_step_back_step); 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_13:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Small hole (0.8mm)        
    case STEP_14:
        ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
            ACC_RATE_VALVE, DEC_RATE_VALVE, ALARMS_VALVE);   
/*        
        accentReg = TABLE_ACC_DEC[ACC_RATE_VALVE/1000];
        cSPIN_RegsStruct3.ACC = accentReg;          
        cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
        accentReg = TABLE_ACC_DEC[DEC_RATE_VALVE/1000];
        cSPIN_RegsStruct3.DEC = accentReg;          
        cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);  
*/        
        Steps_Todo = -(TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep);
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:
//        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)(Steps_Todo - TintingAct.Step_Valve_Backstep + STEP_PHOTO_VALVE_SMALL_HOLE + VALVE_STEPS_TOLERENCE)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )  {
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
        // Set Maximum Ramp Acceleration / Deceleration to Pump Motor
            ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                          MAX_ACC_RATE_PUMP, MAX_DEC_RATE_PUMP, ALARMS_PUMP);
/*        
            accentReg = TABLE_ACC_DEC[MAX_ACC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.ACC = accentReg;          
            cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_PUMP);
            accentReg = TABLE_ACC_DEC[MAX_DEC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.DEC = accentReg;          
            cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_PUMP);  
*/                        
            Pump.step ++;
        }                
	break; 
// -----------------------------------------------------------------------------    
// EROGATION    
	//  Start Erogation
	case STEP_16:
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_17:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Ingr) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 	

	//  Start Backstep if present
	case STEP_18:
        if (TintingAct.En_back_step) {
            Steps_Todo = -TintingAct.N_step_back_step_2; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
			Pump.step ++ ;
		}
		else {
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step +=2;
            }
            else
                Pump.step +=3;
		}
	break;

	//  Check if position required is reached
	case STEP_19:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(Steps_Todo + TintingAct.N_step_stroke + TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Ingr)) &&
             (Status_Board_Pump.Bit.MOT_STATUS == 0) ){
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else
                Pump.step +=2;
        }    
	break; 
// -----------------------------------------------------------------------------    
// VALVE CLOSE execution    
    //  Wait before to Close Valve 	    
	case STEP_20:
	  if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
		StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
		Pump.step ++ ;		
	  }		
	break;
    
	// Start Valve Close     
	case STEP_21:
        // Rotate motor Valve till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_22:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_23:
//        if ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)Steps_Todo) {
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_VALVE); 		
			// Set Normal Ramp Acceleration / Deceleration to Pump Motor
            ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                          ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);            
/*            
            accentReg = TABLE_ACC_DEC[ACC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.ACC = accentReg;          
            cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_PUMP);
            accentReg = TABLE_ACC_DEC[DEC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.DEC = accentReg;          
            cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_PUMP);  
*/          
            Pump.step ++;
        }
    break;
// -----------------------------------------------------------------------------    
// BELLOW DECOUPLING            
	//  Start Decoupling
	case STEP_24:
        // Move motor Pump till Home Photocell transition LIGHT-DARK
        StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
        Pump.step ++ ;
	break;
// -----------------------------------------------------------------------------    
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_25:
//        if ( (PhotocellStatus(HOME_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
		if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_26:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_27:
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);                        
        HardHiZ_Stepper(MOTOR_PUMP);        
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}

/*
*//*=====================================================================*//**
**      @brief SINGLE STROKE dispensation algorithm
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char SingleStrokeColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned char count;
  unsigned short Motor_alarm;
  static signed long Steps_Todo;
  unsigned char currentReg;
//  unsigned short accentReg = 0; 
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }  
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.step = STEP_27;
  }

  if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    HardHiZ_Stepper(MOTOR_VALVE);        
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
	//  Check if Home Photocell is Dark 
	case STEP_0:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else {
            count = 0;
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            //ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                                                  
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);
            
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == DARK) )  {
            HardHiZ_Stepper(MOTOR_PUMP);        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == LIGHT) ) {
            HardHiZ_Stepper(MOTOR_PUMP);        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        } 
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            SetStepperHomePosition(MOTOR_PUMP);
            Pump.step ++ ;
        }        
	break;

	//  Gear Movement (1.5mm))
	case STEP_3:
        Steps_Todo = TintingAct.Step_Ingr;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_4:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break;

	//  Handling for games recovery (1.075mm)
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break;       
// -----------------------------------------------------------------------------     
// BACKSTEP execution  
	// Check if Valve Photocell is DARK
    case STEP_7:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else {
            // Start Backstep compensation movement
            Steps_Todo = TintingAct.N_step_back_step; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);                              	
            Pump.step ++;     	
		}            
    break;

	case STEP_8:            
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
        	if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else if (Status_Board_Valve.Bit.MOT_STATUS == 0) {          
                // Valve towards Backstep Small hole (0.8mm)                 
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	            
                Pump.step ++ ;                
            }
        }    
	break; 

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_9:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            // Set Maximum Ramp Acceleration / Deceleration to Valve Motor
            ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
                          MAX_ACC_RATE_VALVE, MAX_DEC_RATE_VALVE, ALARMS_VALVE);               
/*            
            accentReg = TABLE_ACC_DEC[MAX_ACC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.ACC = accentReg;          
            cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
            accentReg = TABLE_ACC_DEC[MAX_DEC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.DEC = accentReg;          
            cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);  
*/            
            Steps_Todo = TintingAct.Step_Valve_Backstep;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;            
        }
        else if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }    
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position
    case STEP_10:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
//        if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                        
    break;
    
	//  Check if position required is reached: BACKSTEP executed
	case STEP_11:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            Steps_Todo = TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep;
            ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
                ACC_RATE_VALVE, DEC_RATE_VALVE, ALARMS_VALVE);                     
/*            
            accentReg = TABLE_ACC_DEC[MAX_ACC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.ACC = accentReg;          
            cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
            accentReg = TABLE_ACC_DEC[MAX_DEC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.DEC = accentReg;          
            cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);  
*/                        
            Pump.step ++;
        }                                    
	break; 
    
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_12:
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_13:            
//        if ( ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)(TintingAct.Step_Valve_Open)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) 
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// EROGATION    
	//  Start Erogation
	case STEP_14:
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_15:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 	

	//  Start Backstep if present
	case STEP_16:
        if (TintingAct.En_back_step) {
            Steps_Todo = -TintingAct.N_step_back_step_2; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
			Pump.step ++ ;
		}
		else {
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step +=2;
            }
            else
                Pump.step +=3;
		}
	break;

	//  Check if position required is reached
	case STEP_17:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(Steps_Todo + TintingAct.N_step_stroke + TintingAct.Step_Recup +  TintingAct.Step_Ingr)) &&
             (Status_Board_Pump.Bit.MOT_STATUS == 0) ){                
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else
                Pump.step +=2;
        }    
	break; 
// -----------------------------------------------------------------------------    
// VALVE CLOSE execution    
    //  Wait before to Close Valve 	    
	case STEP_18:
	  if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
		StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
		Pump.step ++ ;		
	  }		
	break;
	// Start Valve Close     
	case STEP_19:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            Pump.step ++ ;
	  }		            
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_20:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_21:
//        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ){
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            Pump.step ++;
        }    
    break;
// -----------------------------------------------------------------------------    
// CHECK Cycles Number    
    case STEP_22:
        count++;
		if (count < TintingAct.N_cycles) {
			// Got to SUCTION
			Pump.step++; 
		}
		else {
            // Start decoupling
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);            
			Pump.step += 3;              
		}            
    break;
// -----------------------------------------------------------------------------        
// SUCTION
    // Start Suction
    case STEP_23:
        if (TintingAct.En_back_step) {
            Steps_Todo = -(TintingAct.N_step_stroke - TintingAct.N_step_back_step_2);
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);
        }
        else  {
            Steps_Todo = -TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        }             
        Pump.step++;
    break;
    
	// Check if position required is reached    
    case STEP_24:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               HardHiZ_Stepper(MOTOR_PUMP);        
               Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                HardHiZ_Stepper(MOTOR_PUMP);        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else {
                Steps_Todo = TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE;
                Pump.step = STEP_12;   
            }                
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_25:
//        if ( (PhotocellStatus(HOME_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
		if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_26:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_27:
        HardHiZ_Stepper(MOTOR_PUMP);
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);                        
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}

/*
*//*=====================================================================*//**
**      @brief OLD CONTINUOUS dispensation algorithm
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char OldContinuousColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned char count_single, count_continuous;
  unsigned short Motor_alarm;
  static signed long Steps_Todo;
//  unsigned short accentReg = 0;   
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    StopStepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    StopStepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    StopStepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }  
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    StopStepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    StopStepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    StopStepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  } 
*/ 
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.step = STEP_42;
  }
  
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
	//  Check if Home Photocell is Dark 
	case STEP_0:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else {
            count_continuous = TintingAct.N_CicliDosaggio;
            count_single = 0;            
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == DARK) )  {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == LIGHT) ) {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        } 
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            SetStepperHomePosition(MOTOR_PUMP);
            Pump.step ++ ;
        }        
	break;

	//  Gear Movement (1.5mm))
	case STEP_3:
        Steps_Todo = TintingAct.Step_Ingr;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_4:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break;

	//  Handling for games recovery (1.075mm)
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break;       
// -----------------------------------------------------------------------------    
// GO to CONTINUOUS START EROGATION position
    case STEP_7:
        Steps_Todo = TintingAct.PosStart;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle_supply);
		Pump.step ++ ;
    break;

	//  Check if position required is reached
	case STEP_8:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------        
// BACKSTEP execution  
	// Check if Valve Photocell is DARK
    case STEP_9:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else {
            // Start Backstep compensation movement
            Steps_Todo = TintingAct.N_step_back_step; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);                            
            Pump.step ++;     	
		}            
    break;

    // Start Backstep movement
    case STEP_10:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {        
        	if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else if (Status_Board_Valve.Bit.MOT_STATUS == 0) {          
                // Valve towards Backstep Small hole (0.8mm)                 
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	            
                Pump.step ++ ;                
            }
        }                
	break;
    
    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
//		if ( (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ){
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            // Set Maximum Ramp Acceleration / Deceleration to Valve Motor
            ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
                          MAX_ACC_RATE_VALVE, MAX_DEC_RATE_VALVE, ALARMS_VALVE);    
/*            
            accentReg = TABLE_ACC_DEC[MAX_ACC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.ACC = accentReg;          
            cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
            accentReg = TABLE_ACC_DEC[MAX_DEC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.DEC = accentReg;          
            cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);  
*/            
            Steps_Todo = TintingAct.Step_Valve_Backstep;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                        
            Pump.step ++ ;            
        }
        else if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }    
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position    
    case STEP_12:
        if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                                        
    break;

	//  Check if position required is reached    
    case STEP_13:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.PosStart + TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break;

// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_14:
        ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
            ACC_RATE_VALVE, DEC_RATE_VALVE, ALARMS_VALVE);                 
/*        
        accentReg = TABLE_ACC_DEC[ACC_RATE_VALVE/1000];
        cSPIN_RegsStruct3.ACC = accentReg;          
        cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
        accentReg = TABLE_ACC_DEC[DEC_RATE_VALVE/1000];
        cSPIN_RegsStruct3.DEC = accentReg;          
        cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);  
*/                
        Steps_Todo = TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:
        if ( ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)(TintingAct.Step_Valve_Open)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// EROGATION IN CONTINUOUS   
	//  Start Erogation
	case STEP_16:
        Steps_Todo = TintingAct.PosStop - TintingAct.PosStart;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle_supply);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_17:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.PosStart + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break; 	
// -----------------------------------------------------------------------------    
// CHECK Continuous Cycles Number
	case STEP_18:            
        count_continuous--;
        // Continuous Cycles Number NOT terminated
        if (count_continuous)
            Pump.step ++;
        // Continuous Cycles Number terminated
        else
        // Da capire se occorre chiudere la Valvola in questa fase    
//            Pump.step = STEP_24;            
            Pump.step = STEP_21;                        
	break; 
// -----------------------------------------------------------------------------    
// SUCTION IN CONTINUOUS
    // Start Suction
    case STEP_19:
        Steps_Todo = -(TintingAct.PosStop - TintingAct.PosStart);
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        Pump.step ++;        
	break; 

	//  Check if position required is reached
	case STEP_20:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.PosStart + TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               HardHiZ_Stepper(MOTOR_PUMP);        
               Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                HardHiZ_Stepper(MOTOR_PUMP);        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else
                // GO to Erogation
                Pump.step = STEP_16;                
        }
	break; 	
// -----------------------------------------------------------------------------    
    // CONTINUOUS TERMINATED --> VALVE CLOSE    
	// Start Valve Close     
	case STEP_21:
        // Rotate motor Valve till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_22:
//        if ( (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_23:
        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------    
// CHECK IF ALSO SINGLE STROKE IS REQUESTED
    case STEP_24:
        if (TintingAct.N_cycles > 0)
            Pump.step ++;
        else {
            // Start decoupling
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);            
            Pump.step += 16;
        }    
    break;
// -----------------------------------------------------------------------------    
// SUCTION FOR STARTING SINGLE STROKE
    // Start Suction
    case STEP_25:
        Steps_Todo = -TintingAct.PosStop;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        Pump.step ++;        
	break; 

	//  Check if position required is reached
	case STEP_26:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               HardHiZ_Stepper(MOTOR_PUMP);        
               Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                HardHiZ_Stepper(MOTOR_PUMP);        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else {
            // SetStepperHomePosition(MOTOR_PUMP);    
            // Valve towards Big hole (3.0mm)                 
            StartStepper(MOTOR_PUMP, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	            
            Pump.step++;
            }
        }    
	break; 	
// -----------------------------------------------------------------------------        
// VALVE OPEN execution          
	//  Valve Open towards Big hole (3.0mm)            
    case STEP_27:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            Steps_Todo = TintingAct.Step_Valve_Open;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++ ;        
        }
        else if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }            
	break;
	//  Check if position required is reached: Valve OPEN        
    case STEP_28:
        if ( ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)(Steps_Todo)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
            Pump.step ++;        
    break;    
// -----------------------------------------------------------------------------        
    //  Start Erogation        
	case STEP_29:
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_30:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break; 	

	//  Start Backstep if present
	case STEP_31:
        if (TintingAct.En_back_step) {
            Steps_Todo = -TintingAct.N_step_back_step_2; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
			Pump.step ++ ;
		}
		else {
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step +=2;
            }
            else
                Pump.step +=3;
		}
	break;

	// Check if position required is reached
	case STEP_32:
//        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(Steps_Todo + TintingAct.N_step_stroke + TintingAct.Step_Recup +  TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ){                
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){                
            if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else
                Pump.step +=2;
        }    
	break; 
// -----------------------------------------------------------------------------    
// SINGLE STROKE TERMINATED --> VALVE CLOSE      
    // Wait before to Close Valve 	    
	case STEP_33:
	  if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
		StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
		Pump.step ++ ;		
	  }		
	break;
    
	// Start Valve Close     
	case STEP_34:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            Pump.step ++ ;
        }		                    
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_35:
//        if ( (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            if (Status_Board_Valve.Bit.MOT_STATUS == 0){        
                SetStepperHomePosition(MOTOR_VALVE); 	
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
                MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
                Pump.step ++ ;
            }        
    break;

	//  Check if position required is reached    
    case STEP_36:
        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
            Pump.step ++;
    break;    
// -----------------------------------------------------------------------------    
// CHECK Cycles Number    
    case STEP_37:
        count_single++;
TintingAct.N_cycles = 2;                    
		if (count_single < TintingAct.N_cycles) {
			// Got to SUCTION
			Pump.step++; 
		}
		else {
            // Start decoupling
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);            
			Pump.step += 3;              
		}            
    break;
// -----------------------------------------------------------------------------        
// SUCTION
    // Start Suction
    case STEP_38:
        if (TintingAct.En_back_step) {
            Steps_Todo = -(TintingAct.N_step_stroke - TintingAct.N_step_back_step_2);
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);
        }
        else  {
            Steps_Todo = -TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        }             
        Pump.step++;
    break;
    
	// Check if position required is reached    
    case STEP_39:
//        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
          if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else
                Pump.step = STEP_27;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_40:
//        if ( (PhotocellStatus(HOME_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_41:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_42:
        HardHiZ_Stepper(MOTOR_PUMP);
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);                        
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}

/*
*//*=====================================================================*//**
**      @brief CONTINUOUS dispensation algorithm
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char ContinuousColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned char count_single, count_continuous;
  static unsigned char Wait_Before_BackStep;  
  unsigned short Motor_alarm;
  static signed long Steps_Todo;
  unsigned char currentReg;
//  unsigned short accentReg = 0;     
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    StopStepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    StopStepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    StopStepper(MOTOR_PUMP);
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }  
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    StopStepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    StopStepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    StopStepper(MOTOR_VALVE);
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  } 
*/  
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.step = STEP_42;
  }

  if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    HardHiZ_Stepper(MOTOR_VALVE);        
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
	//  Check if Home Photocell is Dark 
	case STEP_0:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else {
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            //ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                                                  
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);           
            count_continuous = TintingAct.N_CicliDosaggio;
            count_single = 0;            
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == DARK) )  {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == LIGHT) ) {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        } 
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            SetStepperHomePosition(MOTOR_PUMP);
            Pump.step ++ ;
        }        
	break;

	//  Gear Movement (1.5mm))
	case STEP_3:
        Steps_Todo = TintingAct.Step_Ingr;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_4:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm)
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
            Pump.step ++;
	break;       
// -----------------------------------------------------------------------------    
// GO to CONTINUOUS START EROGATION position
    case STEP_7:
        Steps_Todo = TintingAct.PosStart;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle_supply);
		Pump.step ++ ;
    break;

// -----------------------------------------------------------------------------        
// BACKSTEP execution  
    case STEP_8:
    	//  Check if position required is reached
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
        	// Check if Valve Photocell is DARK
            if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                Steps_Todo = TintingAct.N_step_back_step; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
                StopTimer(T_DELAY_BEFORE_VALVE_BACKSTEP);
                Wait_Before_BackStep = 0;            
                Pump.step ++;     	
            }            
        }            
    break;

    // Start Backstep movement
    case STEP_9:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
        	if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            if (Wait_Before_BackStep == 0) {
                Durata[T_DELAY_BEFORE_VALVE_BACKSTEP] = TintingAct.Delay_Before_Valve_Backstep / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_BACKSTEP);
                Wait_Before_BackStep = 1;
            }
            else if (StatusTimer(T_DELAY_BEFORE_VALVE_BACKSTEP)==T_ELAPSED)   
                Pump.step ++;
        }                
	break;

	//  Valve towards Backstep Small hole (0.8mm)    
    case STEP_10:
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	            
        Pump.step ++ ;                        
    break;
        
    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            // Set Maximum Ramp Acceleration / Deceleration to Valve Motor
            ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
                          MAX_ACC_RATE_VALVE, MAX_DEC_RATE_VALVE, ALARMS_VALVE);    
/*            
            accentReg = TABLE_ACC_DEC[MAX_ACC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.ACC = accentReg;          
            cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
            accentReg = TABLE_ACC_DEC[MAX_DEC_RATE_PUMP/1000];
            cSPIN_RegsStruct3.DEC = accentReg;          
            cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);  
*/                        
            Steps_Todo = TintingAct.Step_Valve_Backstep;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                        
            Pump.step ++ ;            
        }
        else if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }    
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position    
    case STEP_12:
//        if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                                        
    break;

	//  Check if position required is reached    
    case STEP_13:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.PosStart + TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break;

// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_14:
        ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, 
            ACC_RATE_VALVE, DEC_RATE_VALVE, ALARMS_VALVE);                 
/*        
        accentReg = TABLE_ACC_DEC[ACC_RATE_VALVE/1000];
        cSPIN_RegsStruct3.ACC = accentReg;          
        cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct3.ACC, MOTOR_VALVE);
        accentReg = TABLE_ACC_DEC[DEC_RATE_VALVE/1000];
        cSPIN_RegsStruct3.DEC = accentReg;          
        cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct3.DEC, MOTOR_VALVE);  
*/        
        Steps_Todo = TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:
//        if ( ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)(TintingAct.Step_Valve_Open)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            Steps_Todo = TintingAct.N_steps_stroke - TintingAct.PosStart;
            Pump.step ++;
        }    
	break; 
// -----------------------------------------------------------------------------    
// EROGATION IN CONTINUOUS   
	//  Start Erogation
	case STEP_16:
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle_supply);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_17:
//        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.PosStart + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step ++;
	break; 	
// -----------------------------------------------------------------------------    
// CHECK Continuous Cycles Number
	case STEP_18:            
        count_continuous--;
        // Continuous Cycles Number NOT terminated
        if (count_continuous)
            Pump.step ++;
        // Continuous Cycles Number terminated: no residual in Single Stroke
        else {
            // No Valve Close
            if (TintingAct.N_cycles > 0)
                Pump.step = STEP_24; 
            // Valve Close
            else {
                if (TintingAct.En_back_step) {
                    Steps_Todo = -TintingAct.N_step_back_step_2; 
                    MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
                    Pump.step += 3 ;
                }
                else {
                    if (TintingAct.Delay_EV_off > 0)  {
                        Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                        StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                        Pump.step +=4;
                    }
                    else {
                        // Start Valve Close     
                        // Rotate motor Valve till Photocell transition LIGHT-DARK
                        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);                        
                        Pump.step += 5;       
                    }                        
                }
            }                    
        }        
	break; 
// -----------------------------------------------------------------------------    
// SUCTION IN CONTINUOUS
    // Start Suction
    case STEP_19:
        Steps_Todo = -TintingAct.N_steps_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        Pump.step ++;        
	break; 

	//  Check if position required is reached
	case STEP_20:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               HardHiZ_Stepper(MOTOR_PUMP);        
               Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                HardHiZ_Stepper(MOTOR_PUMP);        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else {
                count_continuous--;
                // GO to Erogation
                if (count_continuous > 1) {
                    Steps_Todo = TintingAct.N_steps_stroke;
                    Pump.step = STEP_16; 
                }
                else {
                    Steps_Todo = TintingAct.PosStop;
                    Pump.step = STEP_16; 
                }                    
            }
        }
	break; 	
// -----------------------------------------------------------------------------    
    // CONTINUOUS TERMINATED
    case STEP_21:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){                
            if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else {
                // Start Valve Close     
                // Rotate motor Valve till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                Pump.step += 2 ;                        
            }    
        }            
    break; 
    
    case STEP_22:
	  if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
		StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
    	// Start Valve Close     
        // Rotate motor Valve till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        Pump.step ++ ;        
	  }		        
    break; 
            
	// Wait for Valve Photocell DARK
    case STEP_23:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;
    
// -----------------------------------------------------------------------------    
// CHECK IF ALSO SINGLE STROKE IS REQUESTED
    case STEP_24:
        // Position required is reached 
//        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )        
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if (TintingAct.N_cycles > 0)
                Pump.step ++;
            else {
                // Start decoupling
                // Move motor Pump till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);            
                Pump.step += 16;
            }
        }            
    break;
// -----------------------------------------------------------------------------    
// SUCTION FOR STARTING SINGLE STROKE
    // Start Suction
    case STEP_25:
        Steps_Todo = -TintingAct.PosStop;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        Pump.step ++;        
	break; 

	//  Check if position required is reached
	case STEP_26:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               HardHiZ_Stepper(MOTOR_PUMP);        
               Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                HardHiZ_Stepper(MOTOR_PUMP);        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else {
                // SetStepperHomePosition(MOTOR_PUMP);    
                // Valve towards Big hole (3.0mm)                 
                //StartStepper(MOTOR_PUMP, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	            
                // Valve is already Open
                Pump.step = STEP_29;
            }
        }    
	break; 	
// -----------------------------------------------------------------------------        
// VALVE OPEN execution          
	//  Valve Open towards Big hole (3.0mm)            
    case STEP_27:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            Steps_Todo = TintingAct.Step_Valve_Open;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++ ;        
        }
        else if ( (GetStepperPosition(MOTOR_VALVE) <= (-(signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }            
	break;
	//  Check if position required is reached: Valve OPEN        
    case STEP_28:
//        if ( ((signed long)GetStepperPosition(MOTOR_VALVE) <= -(signed int)(Steps_Todo)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
		if (Status_Board_Valve.Bit.MOT_STATUS == 0)
            Pump.step ++;        
    break;    
// -----------------------------------------------------------------------------        
    //  Start Erogation        
	case STEP_29:
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_30:
        if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Recup ) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
	break; 	

	//  Start Backstep if present
	case STEP_31:
        if (TintingAct.En_back_step) {
            Steps_Todo = -TintingAct.N_step_back_step_2; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
			Pump.step ++ ;
		}
		else {
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step +=2;
            }
            else {          
                if ( ((count_single + 1) == TintingAct.N_cycles) || (TintingAct.N_cycles == 0) )       
                    Pump.step += 3 ;		
                else
                    Pump.step += 6; 
    		}        
		}
	break;

	// Check if position required is reached
	case STEP_32:
//        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(Steps_Todo + TintingAct.N_step_stroke + TintingAct.Step_Recup +  TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ){                
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){                
            if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else {
                // Last Erogation cycle --> Close valve
                if ( ((count_single + 1) == TintingAct.N_cycles) || (TintingAct.N_cycles == 0) )       
                    Pump.step += 2;		
                else
                    Pump.step += 5; 
            }    
        }    
	break; 
// -----------------------------------------------------------------------------    
// SINGLE STROKE TERMINATED --> VALVE CLOSE      
    // Wait before to Close Valve 	    
	case STEP_33:
	  if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
		StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
        // Last Erogation cycle --> Close valve
        if ( ((count_single + 1) == TintingAct.N_cycles) || (TintingAct.N_cycles == 0) )       
    		Pump.step ++ ;		
        else
            Pump.step += 4; 
	  }		
	break;
    
	// Start Valve Close     
	case STEP_34:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            Pump.step ++ ;
        }		                    
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_35:
            if (Status_Board_Valve.Bit.MOT_STATUS == 0){        
                SetStepperHomePosition(MOTOR_VALVE); 	
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
                MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
                Pump.step ++ ;
            }        
    break;

	//  Check if position required is reached    
    case STEP_36:
        if ( (-(signed long)GetStepperPosition(MOTOR_VALVE) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
            Pump.step ++;
    break;    
// -----------------------------------------------------------------------------    
// CHECK Cycles Number    
    case STEP_37:
        count_single++;
		if (count_single < TintingAct.N_cycles) {
			// Got to SUCTION
			Pump.step++; 
		}
		else {
            // Start decoupling
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);            
			Pump.step += 3;              
		}            
    break;
// -----------------------------------------------------------------------------        
// SUCTION
    // Start Suction
    case STEP_38:
        if (TintingAct.En_back_step) {
            Steps_Todo = -(TintingAct.N_step_stroke - TintingAct.N_step_back_step_2);
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);
        }
        else  {
            Steps_Todo = -TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        }             
        Pump.step++;
    break;
    
	// Check if position required is reached    
    case STEP_39:
//        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)(TintingAct.Step_Recup + TintingAct.Step_Ingr)) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else
//                Pump.step = STEP_27;    
                Pump.step = STEP_29;    
       }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_40:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_41:
        if ( (-(signed long)GetStepperPosition(MOTOR_PUMP) <= (signed int)Steps_Todo) && (Status_Board_Pump.Bit.MOT_STATUS == 0) )        
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_42:
        HardHiZ_Stepper(MOTOR_PUMP);
//        HardHiZ_Stepper(MOTOR_VALVE);
        StopStepper(MOTOR_VALVE);                                
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}

/*
*//*=====================================================================*//**
**      @brief Valve Open/Close Process
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char ValveOpenClose(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static signed long Steps_Todo, Steps_Position;
  unsigned char currentReg;
  
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/ 
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Pump.step = STEP_6;
  }

  if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_VALVE);        
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE OPEN OR CLOSE    
    case STEP_0:
        // Motor Valve Open Big Hole (3mm)    
        if (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) {
            if (TintingAct.Output_Act == OUTPUT_ON) {
                if (TintingAct.OpenValve_BigHole_state == OFF)
                    TintingAct.OpenValve_BigHole_state = ON;
                else
                    return PROC_OK;
            }
            else {
                if (TintingAct.OpenValve_BigHole_state == ON)
                    TintingAct.OpenValve_BigHole_state = OFF;
//                else
//                    return PROC_OK;
            }                
        } 
        // Motor Valve Open Small Hole (0.8mm)    
        else if (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) {
            if (TintingAct.Output_Act == OUTPUT_ON) {
                if (TintingAct.OpenValve_SmallHole_state == OFF)
                    TintingAct.OpenValve_SmallHole_state = ON;
                else
                    return PROC_OK;
            }
            else {
                if (TintingAct.OpenValve_SmallHole_state == ON)
                    TintingAct.OpenValve_SmallHole_state = OFF;
//                else
//                    return PROC_OK;
            }                
        } 
        // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
        //ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                                      
        currentReg = HOLDING_CURRENT_TABLE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);
        
        // Starting point:
        Steps_Position = (signed long)GetStepperPosition(MOTOR_VALVE);
        // Valve OPEN to BIG HOLE
        if (TintingAct.OpenValve_BigHole_state == ON) { 
			if ( (Steps_Position == 0) && (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) ) {
				Steps_Todo = TintingAct.Step_Valve_Open - Steps_Position;
				MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
				Pump.step ++ ;
			}
			else
                return PROC_OK;				
        }
        // Valve OPEN to SMALL HOLE
        else if (TintingAct.OpenValve_SmallHole_state == ON) {
			if ( (Steps_Position == 0) && (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) ) {
				Steps_Todo = -TintingAct.Step_Valve_Open + Steps_Position;
				MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
				Pump.step ++ ;
			}
			else
                return PROC_OK;				
        }
        // Valve CLOSE
        else { 
//			if (Steps_Position == 0)
//                return PROC_OK;
            if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
                // Rotate Valve motor till Home Photocell transition DARK-LIGHT
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	
                Pump.step += 4 ;
			}            
            else if (Steps_Position > 0) {
                SetStepperHomePosition(MOTOR_VALVE);
                // Rotate motor Valve CW till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                Pump.step += 2 ;
			}                
            else if (Steps_Position < 0) {
                SetStepperHomePosition(MOTOR_VALVE);
                // Rotate motor Valve CCW till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                Pump.step += 2 ;
			}                                
        }
    break;    
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE OPEN    
	//  Check if position required is reached: Valve OPEN
	case STEP_1: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)        
            Pump.step +=5;
/*        
        // Valve OPEN on BIG HOLE
        if ( (TintingAct.OpenValve_BigHole_state == ON) && ((signed long)(GetStepperPosition(MOTOR_VALVE)) <= -(signed int)(Steps_Todo - Steps_Position)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
            Pump.step +=5;
        // Valve OPEN on SMALL HOLE        
        else if ( (TintingAct.OpenValve_SmallHole_state == ON) && ((signed long)(-GetStepperPosition(MOTOR_VALVE)) <= (signed int)(Steps_Todo - Steps_Position)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) )
            Pump.step +=5;
*/
	break; 
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE CLOSE        
	// Wait for Valve Photocell DARK
    case STEP_2:
//        if ( (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == DARK) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)  {
            SetStepperHomePosition(MOTOR_VALVE);
            // CCW Rotation
            if (Steps_Position < 0)
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            // CW Rotation
            else
                Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        // No LIGHT-DARK transition in CCW direction
        else if ( (Steps_Position < 0) && ((signed long)(GetStepperPosition(MOTOR_VALVE)) >= MAX_STEP_VALVE_HOMING)  ) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }
        // No LIGHT-DARK transition in CW direction
        else if ( (Steps_Position > 0) && ((signed long)(GetStepperPosition(MOTOR_VALVE)) <= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) ) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }        
    break;

	//  Check if position required is reached    
    case STEP_3:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)        
            Pump.step += 3;
/*        
        if ( (Steps_Position < 0) && ((signed long)(-GetStepperPosition(MOTOR_VALVE)) <= (signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {            
            SetStepperHomePosition(MOTOR_VALVE); 
            Pump.step += 3;
        }
        else if ( (Steps_Position >= 0) && ((signed long)(GetStepperPosition(MOTOR_VALVE)) <= -(signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            SetStepperHomePosition(MOTOR_VALVE); 
            Pump.step += 3;
        }            
*/
    break;    

    case STEP_4:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)  {        
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        } 
        else if ((signed long)(GetStepperPosition(MOTOR_VALVE)) >= MAX_STEP_VALVE_HOMING) {
            HardHiZ_Stepper(MOTOR_VALVE);        
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }        
    break;

	// Check if position required is reached    
    case STEP_5:
//        if ( ((signed long)(GetStepperPosition(MOTOR_VALVE)) <= -(signed int)Steps_Todo) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {                
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {         
            SetStepperHomePosition(MOTOR_VALVE); 	            
            Pump.step ++;
        }                    
    break;
// -----------------------------------------------------------------------------          
    case STEP_6:
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);                        
		ret = PROC_OK;
    break; 
    
    default:
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}
