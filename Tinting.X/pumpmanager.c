/* 
 * File:   statusmanager.h
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

#include <xc.h>

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
  TintingAct.Passi_Appoggio_Soffietto = PASSI_APPOGGIO_SOFFIETTO;
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
            // New Erogation Command Request
            if (Status.level == TINTING_SUPPLY_RUN_ST)
                Pump.level = PUMP_SETUP; 
            // New Ricirculation Command Received
            else if (Status.level == TINTING_STANDBY_RUN_ST)
                Pump.level = PUMP_SETUP; 
            // New Pump Homing Command Received
            else if (Status.level == TINTING_PUMP_SEARCH_HOMING_ST) {
                Pump.level = PUMP_HOMING;
                Pump.step = STEP_0;
            }
            // New Valve Homing Command Received
            else if (Status.level == TINTING_VALVE_SEARCH_HOMING_ST) {
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
            else if ( (Status.level == TINTING_SUPPLY_RUN_ST) && (TintingAct.Algorithm == ALG_SYMMETRIC_CONTINUOUS) ) {
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
                if (SingleStrokeColorSupply() == PROC_OK)
                    Pump.level = PUMP_END;
                else if (SingleStrokeColorSupply() == PROC_FAIL)
                    Pump.level = PUMP_ERROR;                    
            } 
            else if (TintingAct.Algorithm == HIGH_RES_STROKE) {     
                if (HighResColorSupply() == PROC_OK)
                    Pump.level = PUMP_END;
                else if (HighResColorSupply() == PROC_FAIL)
                    Pump.level = PUMP_ERROR;                                
            }
            else if (TintingAct.Algorithm == ALG_SYMMETRIC_CONTINUOUS) {     
                if (ContinuousColorSupply() == PROC_OK)
                    Pump.level = PUMP_END;
                else if (ContinuousColorSupply() == PROC_FAIL)
                    Pump.level = PUMP_ERROR;                                
            }            
            else
                Pump.level = PUMP_ERROR;                                                
        break;
        
        case PUMP_HOMING:
            if (PumpHomingColorSupply() == PROC_OK)
                Pump.level = PUMP_END;
            else if (PumpHomingColorSupply() == PROC_FAIL)
               Pump.level = PUMP_ERROR;                           
        break;

        case VALVE_HOMING:
            if (ValveOpenClose() == PROC_OK)
                Pump.level = PUMP_END;
            else if (ValveOpenClose() == PROC_FAIL)
               Pump.level = PUMP_ERROR;                           
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
            if (ValveHomingColorSupply() == PROC_OK)
                Pump.level = PUMP_END;
            else if (ValveHomingColorSupply() == PROC_FAIL)
               Pump.level = PUMP_ERROR;                           
        break;
                            
        case PUMP_RICIRCULATION:
            if (RicirculationColorSupply() == PROC_OK)
                Pump.level = PUMP_END;
            else if (RicirculationColorSupply() == PROC_FAIL)
               Pump.level = PUMP_ERROR;                           
        break;
            
        case PUMP_END:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_SETUP_OUTPUT_VALVE_ST) )
                Pump.level = PUMP_START; 
        break;

        case PUMP_ERROR:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST)         && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_SETUP_OUTPUT_VALVE_ST) )
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
    if (( (TintingAct.Speed_cycle > MAX_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step > MAX_SPEED) ) )) {
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
    if (TintingAct.Algorithm != ALG_SYMMETRIC_CONTINUOUS) {
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
    if (( (TintingAct.Speed_cycle > MAX_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step > MAX_SPEED) ) )) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }   
    if (( (TintingAct.Speed_cycle_supply > MAX_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step > MAX_SPEED) ) )) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }       
    if (TintingAct.Speed_suction > MAX_SPEED) {
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
    if (TintingAct.N_step_stroke > TintingAct.N_step_full_stroke) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
    if (TintingAct.Speed_cycle == 0) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }        
    if (TintingAct.Speed_cycle > MAX_SPEED) {
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
//        Pump.errorCode = TINTING_BAD_PAR_PUMP_ERROR;    
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
  static long Steps_Todo;

  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// CHECK HOME PHOTOCELL STATUS      
// -----------------------------------------------------------------------------      
    case STEP_0:
        SetStepperHomePosition(MOTOR_VALVE); 	
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
            Pump.step++;
        }
        else {
            // Move motor Pump till Home Photocell transition DARK-LIGHT
            StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, DARK_LIGHT, HOME_PHOTOCELL, 0); 
            Pump.step += 3; 
        }    
    break;
// -----------------------------------------------------------------------------
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_1:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if (GetStepperPosition(MOTOR_PUMP) <= -MAX_STEP_PUMP_HOMING) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
        }
	break;

	//  Check if position required is reached    
    case STEP_2:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step +=3;
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION 
    case STEP_3:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        } 
        else if (GetStepperPosition(MOTOR_PUMP) >= MAX_STEP_PUMP_HOMING) {
            Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
            return FALSE;
        }        
    break;

	// Check if position required is reached    
    case STEP_4:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_5:
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
  static long Steps_Todo;

  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// CHECK HOME PHOTOCELL STATUS      
// -----------------------------------------------------------------------------      
    case STEP_0:
        SetStepperHomePosition(MOTOR_VALVE); 	
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            // Rotate Valve motor till Home Photocell transition LIGHT-DARK
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
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        }
        // No LIGHT-DARK transition in CCW direction
        else if (GetStepperPosition(MOTOR_VALVE) <= -MAX_STEP_VALVE_HOMING) {
            // Try the same movemente in CW direction
            SetStepperHomePosition(MOTOR_VALVE);
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);	
            Pump.step += 2 ;
        }
	break;

	//  Check if position required is reached    
    case STEP_2:
        if (GetStepperPosition(MOTOR_VALVE) <= Steps_Todo)
            Pump.step +=5;
    break;
    
	//  Check if position required is reached    
    case STEP_3:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        }
        // No LIGHT-DARK transition in CW direction
        else if (GetStepperPosition(MOTOR_VALVE) >= MAX_STEP_VALVE_HOMING) {
            StopStepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return FALSE;
        }    
    break;
    
	//  Check if position required is reached    
    case STEP_4:
        if (GetStepperPosition(MOTOR_VALVE) >= Steps_Todo)
            Pump.step +=3;
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION 
    case STEP_5:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step ++ ;
        } 
        else if (GetStepperPosition(MOTOR_VALVE) <= (-MAX_STEP_VALVE_HOMING) ) {
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return FALSE;
        }        
    break;

	// Check if position required is reached    
    case STEP_6:
        if (GetStepperPosition(MOTOR_VALVE) >= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_7:
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Recirculation process OLD
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char OldRicirculationColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static unsigned char count;
  static long Steps_Todo;

  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {            
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

	//  Move towards Coupling Photocell (6.5mm))
	case STEP_1:
        Steps_Todo = TintingAct.Step_Accopp;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);
		Pump.step ++ ;
	break; 

	//  Check if position required is reached and Coupling Photocell is DARK
	case STEP_2:
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo) {
            if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {
                StopStepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;
            }
            else
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Accopp) )
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm))
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------        
// RICIRCULATION    
	//  Start Ricrculation
	case STEP_7:
		SetStepperHomePosition(MOTOR_PUMP); 	
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_8:
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo) {
			Durata[T_PAUSE_RECIRC] = (unsigned short)(TintingAct.Recirc_pause) * CONV_SEC_COUNT;
			// Start Ricirculation Pause
            StartTimer(T_PAUSE_RECIRC);
            Pump.step ++;
        }    
	break; 	

	// Wait Ricirculation Pause
	case STEP_9:
		if (StatusTimer(T_PAUSE_RECIRC) == T_ELAPSED) {
			Pump.step ++ ;
		}		
	break; 	
// -----------------------------------------------------------------------------  
// CHECK Cycles Number    
    case STEP_10:
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
        if (GetStepperPosition(MOTOR_PUMP) <= 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return FALSE;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;               
            }
            else
                Pump.step = STEP_7;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_13:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_14:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_15:
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
  static unsigned char count;
  static long Steps_Todo;
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {            
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
        if ( (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) {
            StopStepper(MOTOR_PUMP);
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
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo)
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------        
// RICIRCULATION    
	//  Start Ricrculation
	case STEP_7:
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_8:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
			Durata[T_PAUSE_RECIRC] = (unsigned short)(TintingAct.Recirc_pause) * CONV_SEC_COUNT;
			// Start Ricirculation Pause
            StartTimer(T_PAUSE_RECIRC);
            Pump.step ++;
        }    
	break; 	

	// Wait Ricirculation Pause
	case STEP_9:
		if (StatusTimer(T_PAUSE_RECIRC) == T_ELAPSED) {
			Pump.step ++ ;
		}		
	break; 	
// -----------------------------------------------------------------------------  
// CHECK Cycles Number    
    case STEP_10:
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
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return FALSE;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;               
            }
            else
                Pump.step = STEP_7;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_13:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_14:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_15:
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief HIGH RES STROKE dispensation algorithm OLD
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char OldHighResColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static long Steps_Todo;

  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {            
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

	//  Move towards Coupling Photocell (6.5mm))
	case STEP_1:
        Steps_Todo = TintingAct.Step_Accopp;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);
		Pump.step ++ ;
	break; 

	//  Check if position required is reached and Coupling Photocell is DARK
	case STEP_2:
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo) {
            if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {
                StopStepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;
            }
            else
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Accopp) )
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm))
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// GO to HIGH RES Erogation Starting Point
	//  Bellows head support movement (12.0mm))
	case STEP_7:
        Steps_Todo = TintingAct.Passi_Appoggio_Soffietto;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);
		Pump.step++;
	break;
	
	//  Check if position required is reached and Home Photocell is LIGHT
    case STEP_8:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp + TintingAct.Step_Recup) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                StopStepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return FALSE;
            }
            else
                Pump.step ++;
		}
	break;
// -----------------------------------------------------------------------------    
// BACKSTEP execution  
	// Check if Valve Photocell is DARK
    case STEP_9:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else            
            Pump.step ++;     	
    break;

	//  Valve towards Backstep Small hole (0.8mm)        
    case STEP_10:
        Steps_Todo = - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_SMALL_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT){
            SetStepperHomePosition(MOTOR_VALVE);
            Pump.step ++ ;            
        }
        else if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ) {
            StopStepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return FALSE;
        }    
    break;

	//  Check if position required is reached: Valve in BACKSTEP position
	case STEP_12:            
        if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ){
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_13:
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp + 
                                               TintingAct.Step_Recup + TintingAct.Passi_Appoggio_Soffietto) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Small hole (0.8mm)        
    case STEP_14:
        Steps_Todo = -TintingAct.Step_Valve_Backstep;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:            
        if (GetStepperPosition(MOTOR_VALVE) <= (Steps_Todo - TintingAct.Step_Valve_Backstep) ) {
            // Set Maximum Ramp Acceleration / Deceleration to Pump Motor
            ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                          MAX_ACC_RATE_PUMP, MAX_DEC_RATE_PUMP, ALARMS_PUMP);
            Pump.step ++;
        }                
	break; 
// -----------------------------------------------------------------------------    
// EROGATION    
	//  Start Erogation
	case STEP_16:
		SetStepperHomePosition(MOTOR_PUMP); 	
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_17:
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo)
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
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.N_step_stroke) ) {
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
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_23:
        if (GetStepperPosition(MOTOR_VALVE) >= Steps_Todo) {
            // Set Normal Ramp Acceleration / Deceleration to Pump Motor
            ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                          ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);
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
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_26:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_27:
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
  static long Steps_Todo;
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {
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
        if ( (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) {
            StopStepper(MOTOR_PUMP);
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
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo )
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm))
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// GO to HIGH RES Erogation Starting Point
	//  Bellows head support movement (12.0mm))
	case STEP_7:
        Steps_Todo = TintingAct.Passi_Appoggio_Soffietto;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);
		Pump.step++;
	break;
	
	//  Check if position required is reached and Home Photocell is LIGHT
    case STEP_8:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                StopStepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return FALSE;
            }
            else {
                // Start Backstep compensation movement
                Steps_Todo = TintingAct.N_step_back_step; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);                
                Pump.step ++;
            }    
		}
	break;
// -----------------------------------------------------------------------------    
// BACKSTEP execution  
	// Check if Valve Photocell is DARK at the end of Backstep compensation movement
    case STEP_9:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {		
            if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return FALSE;
            }
            else            
                Pump.step ++;
            }     	
    break;

	//  Valve towards Backstep Small hole (0.8mm)        
    case STEP_10:
        Steps_Todo = - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_SMALL_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT){
            SetStepperHomePosition(MOTOR_VALVE);
            Pump.step ++ ;            
        }
        else if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ) {
            StopStepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return FALSE;
        }    
    break;

	//  Check if position required is reached: Valve in BACKSTEP position
	case STEP_12:            
        if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ){
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_13:
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Recup + TintingAct.Step_Ingr) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Small hole (0.8mm)        
    case STEP_14:
        Steps_Todo = -TintingAct.Step_Valve_Backstep;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:            
        if (GetStepperPosition(MOTOR_VALVE) <= (Steps_Todo - TintingAct.Step_Valve_Backstep) ) {
            // Set Maximum Ramp Acceleration / Deceleration to Pump Motor
            ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                          MAX_ACC_RATE_PUMP, MAX_DEC_RATE_PUMP, ALARMS_PUMP);
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Recup +  TintingAct.Step_Ingr) )
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
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.N_step_stroke + TintingAct.Passi_Appoggio_Soffietto + TintingAct.Step_Recup +  TintingAct.Step_Ingr) ) {
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
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_23:
        if (GetStepperPosition(MOTOR_VALVE) >= Steps_Todo) {
            // Set Normal Ramp Acceleration / Deceleration to Pump Motor
            ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                          ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);
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
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_26:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_27:
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}

/*
*//*=====================================================================*//**
**      @brief SINGLE STROKE dispensation algorithm (OLD)
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char OldSingleStrokeColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned char count;
  unsigned short Motor_alarm;
  static long Steps_Todo;

  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }

  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {            
            count = 0;
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

	//  Move towards Coupling Photocell (6.5mm))
	case STEP_1:
        Steps_Todo = TintingAct.Step_Accopp;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);
		Pump.step ++ ;
	break; 

	//  Check if position required is reached and Coupling Photocell is DARK
	case STEP_2:
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo) {
            if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {
                StopStepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;
            }
            else
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Accopp) )
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm))
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// BACKSTEP execution  
	// Check if Valve Photocell is DARK
    case STEP_7:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else            
//            Pump.step ++;     	
            Pump.step +=5;     	
    break;

	//  Valve towards Backstep Small hole (0.8mm)        
    case STEP_8:
        Steps_Todo = - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_SMALL_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_9:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT){
            SetStepperHomePosition(MOTOR_VALVE);
            Pump.step ++ ;            
        }
        else if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ) {
            StopStepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return FALSE;
        }    
    break;

	//  Check if position required is reached: Valve in BACKSTEP position
	case STEP_10:            
        if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ){
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_11:
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp + 
                                               TintingAct.Step_Recup + TintingAct.Passi_Appoggio_Soffietto) )
    		SetStepperHomePosition(MOTOR_PUMP); 	
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_12:
//        Steps_Todo = TintingAct.Step_Valve_Backstep + TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE + STEP_PHOTO_VALVE_SMALL_HOLE;
        Steps_Todo = TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_13:            
        if (GetStepperPosition(MOTOR_VALVE) <= (TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE + STEP_PHOTO_VALVE_SMALL_HOLE) )
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
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo)
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
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.N_step_stroke) ) {
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
        // Rotate motor Valve till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_20:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_21:
        if (GetStepperPosition(MOTOR_VALVE) <= Steps_Todo)
            Pump.step ++;
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
        if (GetStepperPosition(MOTOR_PUMP) <= 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return FALSE;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;               
            }
            else
                Pump.step = STEP_12;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_25:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_26:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_27:
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}
// ******************************************************************************************************************************************
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
  static long Steps_Todo;
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }

  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {            
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

	//  Check if position required is reached and Coupling Photocell is DARK
	case STEP_2:
        if ( (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) {
            StopStepper(MOTOR_PUMP);
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
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo)
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm))
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// BACKSTEP execution  
	// Check if Valve Photocell is DARK
    case STEP_7:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {
            // Start Backstep compensation movement
            Steps_Todo = TintingAct.N_step_back_step; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);                              	
            Pump.step ++;     	
		}            
    break;

    case STEP_8:
    case STEP_9:
	case STEP_10:            
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_11:
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Recup + TintingAct.Step_Ingr) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_12:
//        Steps_Todo = TintingAct.Step_Valve_Backstep + TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE + STEP_PHOTO_VALVE_SMALL_HOLE;
        Steps_Todo = TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_13:            
        if (GetStepperPosition(MOTOR_VALVE) <= (TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE) )
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr) )
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
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.N_step_stroke + TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
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
        // Rotate motor Valve till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_20:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_21:
        if (GetStepperPosition(MOTOR_VALVE) <= Steps_Todo)
            Pump.step ++;
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
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Recup + TintingAct.Step_Ingr)) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return FALSE;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;               
            }
            else
                Pump.step = STEP_12;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_25:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_26:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_27:
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}


/*
*//*=====================================================================*//**
**      @brief CONTINUOUS dispensation algorithm OLD
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
  static long Steps_Todo;

  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }

  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {            
            count_single = 0;
            count_continuous = TintingAct.N_CicliDosaggio;
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

	//  Move towards Coupling Photocell (6.5mm))
	case STEP_1:
        Steps_Todo = TintingAct.Step_Accopp;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);
		Pump.step ++ ;
	break; 

	//  Check if position required is reached and Coupling Photocell is DARK
	case STEP_2:
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo) {
            if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {
                StopStepper(MOTOR_PUMP);
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;
            }
            else
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Accopp) )
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm))
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp) )
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr + TintingAct.Step_Accopp + TintingAct.Step_Recup) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------        
// BACKSTEP execution  
	// Check if Valve Photocell is DARK
    case STEP_9:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else            
//            Pump.step ++;     	
            Pump.step +=5;     	            
    break;

	//  Valve towards Backstep Small hole (0.8mm)        
    case STEP_10:
        Steps_Todo = - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_SMALL_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT){
            SetStepperHomePosition(MOTOR_VALVE);
            Pump.step ++ ;            
        }
        else if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ) {
            StopStepper(MOTOR_VALVE);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return FALSE;
        }    
    break;

	//  Check if position required is reached: Valve in BACKSTEP position
	case STEP_12:            
        if (GetStepperPosition(MOTOR_VALVE) <= (- TintingAct.Step_Valve_Backstep) ){
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_13:
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.PosStart + TintingAct.Step_Ingr + TintingAct.Step_Accopp + 
                                               TintingAct.Step_Recup + TintingAct.Passi_Appoggio_Soffietto) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_14:
//        Steps_Todo = TintingAct.Step_Valve_Backstep + TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE + STEP_PHOTO_VALVE_SMALL_HOLE;
        Steps_Todo = TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:            
        if (GetStepperPosition(MOTOR_VALVE) <= (TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE + STEP_PHOTO_VALVE_SMALL_HOLE) ) {
    		SetStepperHomePosition(MOTOR_PUMP); 	
            Pump.step ++;
        }    
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
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo)
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
        // Valve NOT CLOSED
//            Pump.step +=3;
            Pump.step = STEP_24;            
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
        if (GetStepperPosition(MOTOR_PUMP) <= 0)
            // GO to Erogation
            Pump.step = STEP_16;
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
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_23:
        if (GetStepperPosition(MOTOR_VALVE) <= Steps_Todo)
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
        Steps_Todo = -(TintingAct.PosStop - TintingAct.PosStart);
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        Pump.step ++;        
	break; 

	//  Check if position required is reached
	case STEP_26:
        if (GetStepperPosition(MOTOR_PUMP) <= 0) {
            SetStepperHomePosition(MOTOR_PUMP);    
            // Go to Single Stroke Erogation
            // NO More Valve Open
            //Pump.step++;
            Pump.step = STEP_29;
        }    
	break; 	
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_27:
        Steps_Todo = TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_28:            
        if (GetStepperPosition(MOTOR_VALVE) <= (TintingAct.Step_Valve_Open) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// EROGATION IN SINGLE STROKE    
	//  Start Erogation
	case STEP_29:
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_30:
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo)
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

	//  Check if position required is reached
	case STEP_32:
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.N_step_stroke) ) {
			StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);			
            Pump.step ++;
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
        // Rotate motor Valve till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_35:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_36:
        if (GetStepperPosition(MOTOR_VALVE) <= Steps_Todo)
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
        if (GetStepperPosition(MOTOR_PUMP) <= 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return FALSE;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;               
            }
            else
                Pump.step = STEP_27;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_40:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_41:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_42:
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
  unsigned short Motor_alarm;
  static long Steps_Todo;
  //----------------------------------------------------------------------------
  // Check for Motor Pump Error
  ReadStepperError(MOTOR_PUMP,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_PUMP);
       Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }

  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// COUPLING with bellow
// -----------------------------------------------------------------------------     
	//  Check if Home Photocell is Dark 
	case STEP_0:
		if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
		}
        else {            
            count_single = 0;
            count_continuous = TintingAct.N_CicliDosaggio;
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
		Pump.step ++ ;
	break; 

	//  Check if position required is reached and Coupling Photocell is DARK
	case STEP_2:
        if ( (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopStepper(MOTOR_PUMP);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return FALSE;
        }                
        else if ( (GetStepperPosition(MOTOR_PUMP) >= (TintingAct.Step_Accopp - TOLL_ACCOPP) ) && 
                  (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Accopp + TOLL_ACCOPP) ) && 
                  (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) {
            StopStepper(MOTOR_PUMP);
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
        if (GetStepperPosition(MOTOR_PUMP) >= Steps_Todo)
            Pump.step ++;
	break;

	//  Handling for games recovery (1.0mm))
	case STEP_5:
        Steps_Todo = TintingAct.Step_Recup;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_6:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Ingr) )
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr) )
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------        
// BACKSTEP execution  
	// Check if Valve Photocell is DARK
    case STEP_9:
		if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return FALSE;
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
            Steps_Todo = -TintingAct.N_step_back_step; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step);
            Pump.step ++;
        }                
	break;
    
	// Motor Backstep done      
    case STEP_11:
    case STEP_12:
    case STEP_13:
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Recup + TintingAct.Step_Ingr) )
            Pump.step ++ ;
	break;

// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_14:
        Steps_Todo = TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE;
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:            
        if (GetStepperPosition(MOTOR_VALVE) <= (TintingAct.Step_Valve_Open + STEP_PHOTO_VALVE_BIG_HOLE) ) {
            Pump.step ++;
        }    
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
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.PosStart + TintingAct.Step_Recup + TintingAct.Step_Ingr))
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
            Pump.step = STEP_24;            
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
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.PosStart + TintingAct.Step_Recup + TintingAct.Step_Ingr) )
            // GO to Erogation
            Pump.step = STEP_16;
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
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_23:
        if (GetStepperPosition(MOTOR_VALVE) <= Steps_Todo)
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
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
            SetStepperHomePosition(MOTOR_PUMP);    
            // Go to Single Stroke Erogation
            // NO More Valve Open
            Pump.step++;
        }    
	break; 	
// -----------------------------------------------------------------------------    
// EROGATION IN SINGLE STROKE    
    case STEP_27:
    case STEP_28:
    //  Start Erogation
	case STEP_29:
        Steps_Todo = TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_30:
        if (GetStepperPosition(MOTOR_PUMP) >= (Steps_Todo + TintingAct.Step_Recup + TintingAct.Step_Ingr) )
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

	//  Check if position required is reached
	case STEP_32:
        if (GetStepperPosition(MOTOR_PUMP) <= (Steps_Todo + TintingAct.N_step_stroke + TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
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
        // Rotate motor Valve till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_35:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_cycle);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_36:
        if (GetStepperPosition(MOTOR_VALVE) <= Steps_Todo)
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
        if (GetStepperPosition(MOTOR_PUMP) <= (TintingAct.Step_Recup + TintingAct.Step_Ingr) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
               StopStepper(MOTOR_PUMP);
               Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
               return FALSE;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                StopStepper(MOTOR_PUMP);    
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return FALSE;               
            }
            else
                Pump.step = STEP_27;    
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_40:
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
            SetStepperHomePosition(MOTOR_PUMP); 	
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (12.35mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
	break;

	//  Check if position required is reached    
    case STEP_41:
        if (GetStepperPosition(MOTOR_PUMP) <= Steps_Todo)
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_42:
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
  static long Steps_Todo, Steps_Position;

  // Check for Motor Valve Error
  ReadStepperError(MOTOR_VALVE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_VALVE);
       Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE OPEN OR CLOSE    
    case STEP_0:
        // Motor Valve Open Big Hole (3mm)    
        if (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) {
            if (PeripheralAct.Action == OUTPUT_ON) 
                TintingAct.OpenValve_BigHole_state = ON;    
            else
                TintingAct.OpenValve_BigHole_state = OFF;  
        } 
        // Motor Valve Open Small Hole (0.8mm)    
        else if (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) { 
            if (PeripheralAct.Action == OUTPUT_ON) 
                TintingAct.OpenValve_SmallHole_state = ON;
            else
                TintingAct.OpenValve_SmallHole_state = OFF;
        }
        
        // Starting point:
        Steps_Position = GetStepperPosition(MOTOR_VALVE);
        // Valve OPEN to BIG HOLE
        if (TintingAct.OpenValve_BigHole_state == ON) { 
            Steps_Todo = TintingAct.Step_Valve_Open - Steps_Position;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++ ;
        }
        // Valve OPEN to SMALL HOLE
        else if (TintingAct.OpenValve_SmallHole_state == ON) {
            Steps_Todo = -TintingAct.Step_Valve_Open - Steps_Position;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++ ;            
        }
        // Valve CLOSE
        else { 
            if (Steps_Position > 0)
                // Rotate motor Valve CCW till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            else
                // Rotate motor Valve CW till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            Pump.step += 2 ;
        }
    break;    
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE OPEN    
	//  Check if position required is reached: Valve OPEN
	case STEP_1:            
        // Valve OPEN on BIG HOLE
        if ( (TintingAct.OpenValve_BigHole_state == ON) && (GetStepperPosition(MOTOR_VALVE) >= (Steps_Todo + Steps_Position) ) )
            Pump.step +=3;
        // Valve OPEN on SMALL HOLE        
        else if ( (TintingAct.OpenValve_BigHole_state == ON) && (GetStepperPosition(MOTOR_VALVE) <= (Steps_Todo + Steps_Position) ) )
            Pump.step +=3;
	break; 
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE CLOSE        
	// Wait for Valve Photocell DARK
    case STEP_2:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) {
            if (Steps_Position > 0)
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            else
                Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }        
    break;

	//  Check if position required is reached    
    case STEP_3:
        if ( (Steps_Position > 0) && (GetStepperPosition(MOTOR_VALVE) <= 0) )
            Pump.step ++;
        else if ( (Steps_Position <= 0) && (GetStepperPosition(MOTOR_VALVE) >= 0) )
            Pump.step ++;            
    break;    
// -----------------------------------------------------------------------------    
    case STEP_4:
		ret = PROC_OK;
    break; 
    
    default:
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
  }
  return ret;
}
