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
static cSPIN_RegsStruct_TypeDef  cSPIN_RegsStruct2 = {0};  //to set
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
  TintingAct.Step_Valve_Open = (unsigned long)STEP_VALVE_OPEN + (unsigned long)STEP_VALVE_OFFSET;
  // Passi da posizione di home/ricircolo (valvola chiusa) a posizone di backstep (0.8mm))
  TintingAct.Step_Valve_Backstep = STEP_VALVE_BACKSTEP;
  // Velocità di apertura/chiusura valvola
  TintingAct.Speed_Valve = SPEED_VALVE;
  // N. steps in una corsa intera
  TintingAct.N_steps_stroke = N_STEPS_STROKE; 
  // Tipo di Foro nell'Algoritmo Single Stroke Empty Room
  TintingAct.Free_param_2 = BIG_HOLE;
  // No peripheral selected
  PeripheralAct.Peripheral_Types.bytePeripheral = 0;
  TintingAct.Output_Act = OUTPUT_OFF;
  Pump_Valve_Motors = OFF;    
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
    unsigned char ret_proc;
    unsigned char currentReg;    
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
            else if ( (Status.level == TINTING_PUMP_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST) ||
                      (Status.level == TINTING_PHOTO_LIGHT_VALVE_SEARCH_PUMP_HOMING_ST) || (Status.level == TINTING_LIGHT_VALVE_PUMP_SEARCH_HOMING_ST) ) {
                Pump.level = PUMP_HOMING;
                Pump.step = STEP_0;
                Pump_Valve_Motors = ON;                
            }
            // New Valve Homing Command Received
            else if ( (Status.level == TINTING_VALVE_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST) ||
                      (Status.level == TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST) || (Status.level == TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_VALVE_HOMING_ST) ) {
                Pump.level = VALVE_HOMING;
                Pump.step = STEP_0;
                Pump_Valve_Motors = ON;                                
            }
            // Valve New Homing Command Received
            else if (Status.level == TINTING_PHOTO_LIGHT_VALVE_NEW_SEARCH_VALVE_HOMING_ST){
                Pump.level = VALVE_NEW_HOMING;
                Pump.step = STEP_0;
                Pump_Valve_Motors = ON;                
            }
            
            // New Valve Open/Close Command Received
            else if (Status.level == TINTING_WAIT_SETUP_OUTPUT_VALVE_ST) {
                Pump.level = PUMP_SETUP_OUTPUT;
                Pump.step = STEP_0;
                Pump_Valve_Motors = ON;                
            }                
        break;
            
        case PUMP_SETUP:
            if ( (Status.level == TINTING_SUPPLY_RUN_ST) && ( (TintingAct.Algorithm == ALG_SINGLE_STROKE) || (TintingAct.Algorithm == HIGH_RES_STROKE) ) ) {
                // Analyze Formula
                if (AnalyzeFormula() == TRUE)  {
                    Pump.level = PUMP_RUNNING;
                    Pump.step = STEP_0;
                    Pump_Valve_Motors = ON;
#if defined NOPUMP
    StartTimer(T_WAIT_DISPENSING);                    
#endif    
                } 
                else
                    Pump.level = PUMP_ERROR;                 
            }
            else if ( (Status.level == TINTING_SUPPLY_RUN_ST) && (TintingAct.Algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) {
                // Analyze Formula
                if (AnalyzeContinuousFormula() == TRUE)  {
                    Valve_open = FALSE;
                    Pump.level = PUMP_RUNNING;
                    Pump.step = STEP_0;
                    Pump_Valve_Motors = ON;                                        
#if defined NOPUMP
    StartTimer(T_WAIT_DISPENSING);                    
#endif    
                } 
                else
                    Pump.level = PUMP_ERROR;                 
            }
            else if (Status.level == TINTING_STANDBY_RUN_ST) {
                // Analyze Ricirculation Command
                if (AnalyzeRicirculationCommand() == TRUE)  {
                    Pump.level = PUMP_RICIRCULATION;
                    Pump.step = STEP_0;
                    Pump_Valve_Motors = ON;                    
                } 
                else
                    Pump.level = PUMP_ERROR;                 
            }       
        break;

        case PUMP_RUNNING:            
            if (TintingAct.Algorithm == ALG_SINGLE_STROKE) {
#if defined NOPUMP
if (StatusTimer(T_WAIT_DISPENSING)==T_ELAPSED) {                
    Pump.level = PUMP_END;
    StopTimer(T_WAIT_DISPENSING);
}
#else
                if (TintingAct.EnableDuckbill == DUCKBILL_ENABLED)
                    ret_proc = SingleStrokeColorSupplyDuckbill();                
                else                
                    ret_proc = SingleStrokeColorSupply();

                if (ret_proc == PROC_OK)  {
                    Valve_open = FALSE;
                    Pump.level = PUMP_END;
                }    
                else if (ret_proc == PROC_FAIL)  {
                    if (valve_dir == 1)
                        Dir_Valve_Close = CW;                        
                    else    
                        Dir_Valve_Close = CCW;
                    Valve_open = FALSE;
                    Pump.step = 0;
                    Pump.level = PUMP_CLOSE_VALVE; 
                } 
#endif                   
            } 
            else if (TintingAct.Algorithm == HIGH_RES_STROKE) {     
#if defined NOPUMP
if (StatusTimer(T_WAIT_DISPENSING)==T_ELAPSED) {                
    Pump.level = PUMP_END;
    StopTimer(T_WAIT_DISPENSING);
}
#else
                if (TintingAct.EnableDuckbill == DUCKBILL_ENABLED)
                    ret_proc = HighResColorSupplyDuckbill();                
                else
                    ret_proc = HighResColorSupply();                
                    
                if (ret_proc == PROC_OK) {
                    Valve_open = FALSE;
                    Pump.level = PUMP_END;
                }
                else if (ret_proc == PROC_FAIL) {
                    if (valve_dir == 1)
                        Dir_Valve_Close = CCW;                        
                    else    
                        Dir_Valve_Close = CW;                    
                    Valve_open = FALSE;
                    Pump.step = 0;                    
                    Pump.level = PUMP_CLOSE_VALVE; 
                }               
#endif                                   
            }
            else if (TintingAct.Algorithm == ALG_ASYMMETRIC_CONTINUOUS) {     
#if defined NOPUMP
if (StatusTimer(T_WAIT_DISPENSING)==T_ELAPSED) {                
    Pump.level = PUMP_END;
    StopTimer(T_WAIT_DISPENSING);
}
#else
                if (TintingAct.EnableDuckbill == DUCKBILL_ENABLED)
                    ret_proc = ContinuousColorSupplyDuckbill();                
                else                                
                    ret_proc = ContinuousColorSupply();
                
                if (ret_proc == PROC_OK) {                    
                    Valve_open = FALSE;
                    Pump.level = PUMP_END;
                }
                else if (ret_proc == PROC_FAIL) {
                    if (valve_dir == 1)
                        Dir_Valve_Close = CW;                        
                    else    
                        Dir_Valve_Close = CCW;                    
                    Valve_open = FALSE;
                    Pump.step = 0;                    
                    Pump.level = PUMP_CLOSE_VALVE;
                }             
#endif                                                   
            }            
            else
                Pump.level = PUMP_ERROR;                                                
        break;
        
        case PUMP_CLOSE_VALVE: 
            ret_proc = ValveClosingColorSupply(Dir_Valve_Close);
            if (ret_proc == PROC_OK) {
                Pump.step = 0;                                    
                Pump.level = PUMP_GO_HOME;
            }    
            else if (ret_proc == PROC_FAIL) {
               // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
               currentReg = HOLDING_CURRENT_TABLE * 100 /156;
               cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
               cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                                                                    
               Pump.level = PUMP_ERROR; 
            }              
        break;
            
        case PUMP_GO_HOME:
            ret_proc = PumpHomingColorSupply();
            if (ret_proc == PROC_OK) {
                 Pump.level = PUMP_ERROR;;
            }    
            else if (ret_proc == PROC_FAIL) {
               // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
               currentReg = HOLDING_CURRENT_TABLE * 100 /156;
               cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
               cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                                                                    
               Pump.level = PUMP_ERROR; 
            }                              
        break;
            
        case PUMP_HOMING:
//Pump.level = PUMP_END;            
            ret_proc = PumpHomingColorSupply();
            if (ret_proc == PROC_OK) {
                Pump.level = PUMP_END;
            }    
            else if (ret_proc == PROC_FAIL) {
               Pump.level = PUMP_ERROR; 
            }              
        break;

        case VALVE_HOMING:            
//Pump.level = PUMP_END;            
            ret_proc = ValveHomingColorSupply();            
            if (ret_proc == PROC_OK) {
                Pump.level = PUMP_END;
            }    
            else if (ret_proc == PROC_FAIL) {
               // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
               currentReg = HOLDING_CURRENT_TABLE * 100 /156;
               cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
               cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                                                                                    
               Pump.level = PUMP_ERROR;
            }    
        break;

        case VALVE_NEW_HOMING:            
//Pump.level = PUMP_END;            
            ret_proc = NEWValveHomingColorSupply();
            if (ret_proc == PROC_OK) {
                Pump.level = PUMP_END;
            }    
            else if (ret_proc == PROC_FAIL) {
               // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
               currentReg = HOLDING_CURRENT_TABLE * 100 /156;
               cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
               cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                                                                                    
               Pump.level = PUMP_ERROR;
            }    
        break;
        
        case PUMP_SETUP_OUTPUT:
            if ( AnalyzeSetupOutputs() == TRUE) {
                Pump.level = PUMP_PAR_RX;
                if (PeripheralAct.Peripheral_Types.Rotating_Valve == ON)
                    NextPump.level = PUMP_VALVE_ROTATING;                
                else
                    NextPump.level = PUMP_VALVE_OPEN_CLOSE;
            }
            else
                Pump.level = PUMP_ERROR;
        break;

        case PUMP_VALVE_ROTATING:
//Pump.level = PUMP_END;
            ret_proc = ValveRotating();
            if (ret_proc == PROC_OK)
                Pump.level = PUMP_END;
            else if (ret_proc == PROC_FAIL) {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                                                                                    
                Pump.level = PUMP_ERROR;
            }                
        break;
            
        case PUMP_VALVE_OPEN_CLOSE:
//Pump.level = PUMP_END;
            ret_proc = ValveOpenClose();
            if (ret_proc == PROC_OK)
                Pump.level = PUMP_END;
            else if (ret_proc == PROC_FAIL) {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                                                                                    
                Pump.level = PUMP_ERROR;
            }    
        break;
        
        case PUMP_RICIRCULATION:
#if defined NOPUMP
Pump.level = PUMP_END;
#else                                         
            ret_proc = RicirculationColorSupply();
            if (ret_proc == PROC_OK) {
                Pump.level = PUMP_END;
            }
            else if (ret_proc == PROC_FAIL) {
                Pump.step = 0;                                    
                Pump.level = PUMP_GO_HOME;                        
            }                                
#endif
        break;
            
        case PUMP_END:
            Pump_Valve_Motors = OFF;
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_SETUP_OUTPUT_VALVE_ST) && (Status.level != TINTING_PAR_RX) && 
                 (Status.level != TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST) && (Status.level != TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_VALVE_HOMING_ST) &&
                 (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_PUMP_HOMING_ST)  && (Status.level != TINTING_LIGHT_VALVE_PUMP_SEARCH_HOMING_ST) && 
                 (Status.level != TINTING_PHOTO_LIGHT_VALVE_NEW_SEARCH_VALVE_HOMING_ST) )
                Pump.level = PUMP_START; 
        break;

        case PUMP_ERROR:
            Pump_Valve_Motors = OFF;
            if ( (Status.level != TINTING_SUPPLY_RUN_ST)         && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_SETUP_OUTPUT_VALVE_ST) && (Status.level != TINTING_PAR_RX) &&
                 (Status.level != TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST) && (Status.level != TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST) &&
                 (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST) && (Status.level != TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_VALVE_HOMING_ST) &&
                 (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_PUMP_HOMING_ST)  && (Status.level != TINTING_LIGHT_VALVE_PUMP_SEARCH_HOMING_ST) && 
                 (Status.level != TINTING_PHOTO_LIGHT_VALVE_NEW_SEARCH_VALVE_HOMING_ST) )
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
    if ( (TintingAct.Speed_cycle == 0) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step_Small_Hole == 0) ) ) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }        
    if (( (TintingAct.Speed_cycle > MAXIMUM_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step_Small_Hole > MAXIMUM_SPEED) ) )) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }
/*    
    TintingAct.StrokeType = TintingAct.SingleStrokeType;
    if (TintingAct.StrokeType == SINGLE_STROKE_CLEVER) {
        if (TintingAct.N_cycles <= 1)
            TintingAct.StrokeType = SINGLE_STROKE_EMPTY_ROOM;
        else
            TintingAct.StrokeType = SINGLE_STROKE_FULL_ROOM;            
    }
*/   
    TintingAct.StrokeType = SINGLE_STROKE_EMPTY_ROOM;    
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
    if ( (TintingAct.Speed_cycle == 0) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step_Big_Hole == 0) ) ) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }        
    if (( (TintingAct.Speed_cycle > MAXIMUM_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step_Big_Hole > MAXIMUM_SPEED) ) )) {
        Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        return FALSE;
    }   
    if (( (TintingAct.Speed_cycle_supply > MAXIMUM_SPEED) || ( (TintingAct.En_back_step == 1) && (TintingAct.Speed_back_step_Big_Hole > MAXIMUM_SPEED) ) )) {
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
    if ((TintingAct.Passi_Appoggio_Soffietto) > MAX_PASSI_APPOGGIO_SOFFIETTO)
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
  static signed long Steps_Todo;
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
//    Table.step = STEP_7; 
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }  
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }  
/*
 else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
*/    
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
      
// CHECK HOME PHOTOCELL STATUS      
// -----------------------------------------------------------------------------      
    case STEP_0: 
        SetStepperHomePosition(MOTOR_PUMP); 
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
            StartTimer(T_MOTOR_WAITING_TIME); 
            Pump.step++;
        } 
        else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
            // Move motor Pump till Ingr Photocell transition DARK-LIGHT
            StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_SUCTION, DARK_LIGHT, COUPLING_PHOTOCELL, 0);
            StartTimer(T_MOTOR_WAITING_TIME); 
            Pump.step+=5;
        }    
        else {
            // Move motor Pump till Home Photocell transition DARK-LIGHT
            StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, DARK_LIGHT, HOME_PHOTOCELL, 0);
            StartTimer(T_MOTOR_WAITING_TIME); 
            Pump.step += 3; 
        } 
    break;
// -----------------------------------------------------------------------------
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_1:        
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);            
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if ((signed long)GetStepperPosition(MOTOR_PUMP) >= (signed long)MAX_STEP_PUMP_HOMING_BACKWARD) {
            StopTimer(T_MOTOR_WAITING_TIME);            
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }          
	break;

	//  Check if position required is reached    
    case STEP_2:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step +=5;
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION 
    case STEP_3:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_PUMP); 
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm)
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }        
        else if ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed long)MAX_STEP_PUMP_HOMING_FORWARD) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                  
    break;

	// Check if position required is reached    
    case STEP_4:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step +=3;            
    break;
// -----------------------------------------------------------------------------        
    case STEP_5:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){
            StopTimer(T_MOTOR_WAITING_TIME);                                
            Steps_Todo = - STEP_ACCOPP;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
            Pump.step ++;                        
        } 
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                          
    break;
      
    case STEP_6:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
            return PROC_FAIL;  
        }    
    break;
      
    case STEP_7:
        HardHiZ_Stepper(MOTOR_PUMP);
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
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
  static signed long Steps_Todo;
  unsigned char currentReg;
//------------------------------------------------------------------------------  
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_VALVE);
//    Table.step = STEP_24;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  } 
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }  
/*  
 else if (TintingAct.Circuit_Engaged != 0) {
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_HOMING_ERROR_ST;
    return PROC_FAIL;
  }      

 else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    StopTimer(T_VALVE_WAITING_TIME);                                        
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
*/      
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// CHECK HOME PHOTOCELL STATUS      
// -----------------------------------------------------------------------------      
    case STEP_0:
        SetStepperHomePosition(MOTOR_VALVE);
        
        currentReg = HOMING_RAMP_PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_ACC = currentReg ;            
        cSPIN_Set_Param(cSPIN_TVAL_ACC, cSPIN_RegsStruct3.TVAL_ACC, MOTOR_VALVE);       
        cSPIN_RegsStruct3.TVAL_DEC = currentReg;            
        cSPIN_Set_Param(cSPIN_TVAL_DEC, cSPIN_RegsStruct3.TVAL_DEC, MOTOR_VALVE);

        currentReg = HOMING_PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_RUN = currentReg ;               
        cSPIN_Set_Param(cSPIN_TVAL_RUN, cSPIN_RegsStruct3.TVAL_RUN, MOTOR_VALVE);

        // Set Maximum Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
                
        currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        // Se voglio che l'impostazione della Holding Current abbia effetto devo inviare almeno un comando di movimentazione alla Tavola (?????))
        MoveStepper(MOTOR_TABLE, 0, 0);  

        // Valve position NOT determined
		if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Steps_Todo = MAX_STEP_VALVE_HOMING;
            // Move in CW
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);     
            Pump.step +=15;
        }
        // Valve Open on Small Hole
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0); 	                                    
            StartTimer(T_VALVE_WAITING_TIME);
            Pump.step +=7; 
        }
        // Valve Open on Big Hole
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step +=11;             
        }        
        // Valve in HOME position (both Photocell DARK))
        else {
            // Rotate Valve motor till Home Photocell transition DARK-LIGHT
//            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	
            Steps_Todo = STEP_CLOSE_VALVE;
            // Move in CW
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                 
            Pump.step ++; 
        }    
    break;
// -----------------------------------------------------------------------------
// VALVE HOME PHOTOCELL AND VALVE OPEN PHOTOCELL --> DARK 
    case STEP_1:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) )  {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                                       
            }
            else {
                // Rotate Valve motor till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                StartTimer(T_VALVE_WAITING_TIME);
                Pump.step ++; 
            }                
        }                
    break;            
    
    case STEP_2: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);                
            Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);     
            Pump.step ++ ;
        } 
        else if ((signed long)GetStepperPosition(MOTOR_VALVE) <= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) {
            StopTimer(T_VALVE_WAITING_TIME); 
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                                                          
    break;

	// Check if position required is reached    
    case STEP_3:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            Steps_Todo = -STEP_CLOSE_VALVE;
            // Move in CCW
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                 
            Pump.step ++; 
        } 
    break; 
    
    case STEP_4:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            // Rotate Valve motor till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);
            Pump.step ++; 
        }                        
    break;         

    case STEP_5: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);                
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);     
            Pump.step ++ ;
        } 
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }   
    break;
    
    case STEP_6:         
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)
            Pump.step +=18;         
    break;
// -----------------------------------------------------------------------------
// VALVE OPEN ON SMALL HOLE -> CLOSE VALVE 
	// Wait for Valve Photocell DARK
    case STEP_7:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE + STEP_CLOSE_VALVE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }        
    break;
    
    case STEP_8:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++ ;
        }                  
    break;      

    case STEP_9:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                          
    break;      

    case STEP_10:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                            
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {                
                // Valve CLOSE in Home position
                Pump.step += 14;            
            }
        }          
    break;      
    
// -----------------------------------------------------------------------------
// VALVE OPEN ON BIG HOLE -> CLOSE VALVE 
	// Wait for Valve Photocell DARK
    case STEP_11:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
    break;      

    case STEP_12:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);                                    
            Pump.step ++ ;
        }
    break;      

    case STEP_13:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;      

    case STEP_14:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                            
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {                
                // Valve CLOSE in Home position
                Pump.step += 10;            
            }
        }          
    break;      
// -----------------------------------------------------------------------------
// VALVE POSITION NOT DETERMINED 
    case STEP_15:
        // Home Position correct
        if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            StopStepper(MOTOR_VALVE); 
            Pump.step ++;
        }
        // Home Position rotated 180°
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
            StopStepper(MOTOR_VALVE); 
            Pump.step +=7;
        }        
        else if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_HOMING_ERROR_ST;
            return PROC_FAIL;                                       
        }            
    break;      

    // Home Position correct, do further "STEP_CLOSE_VALVE" for DARK both the Photocels    
    case STEP_16:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){        
            Steps_Todo = STEP_CLOSE_VALVE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++;            
        }        
    break;

    // Home Position correct, do further "STEP_CLOSE_VALVE" steps for centering valve    
    case STEP_17:
        if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
            StopStepper(MOTOR_VALVE); 
            Pump.step ++;
        }
        else if (Status_Board_Valve.Bit.MOT_STATUS == 0){  
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_HOMING_ERROR_ST;
            return PROC_FAIL;                                                                       
        }            
    break;

    case STEP_18:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){        
            Steps_Todo = STEP_CLOSE_VALVE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                            
            Pump.step ++;            
        }            
    break;
        
    case STEP_19:                         
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){        
            // Rotate Valve motor till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++;            
        }        
    break;

    case STEP_20:                         
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            StopTimer(T_VALVE_WAITING_TIME);
            SetStepperHomePosition(MOTOR_VALVE); 	            
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);
            Pump.step ++;            
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                
    break;
 
    case STEP_21:                         
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
                Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;                           
            }
            else {
                // Valve CLOSE in Home position
                SetStepperHomePosition(MOTOR_VALVE); 	            
                Pump.step += 3;            
            }
        }                                
    break;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++            
    case STEP_22:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){        
            Steps_Todo = -MAX_STEP_VALVE_HOMING -MAX_STEP_VALVE_HOMING;
            // Move in CCW
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++;            
        }          
    break;      
    
    case STEP_23:
        // Find Home Position
        if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
            StopStepper(MOTOR_VALVE); 
            Pump.step -= 6;
        }
        else if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_HOMING_ERROR_ST;
            return PROC_FAIL;                                                                       
        }    
    break;          
// -----------------------------------------------------------------------------        
    case STEP_24:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
        
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);
        SetStepperHomePosition(MOTOR_VALVE);
        TintingAct.OpenValve_BigHole_state = OFF;  
        TintingAct.OpenValve_SmallHole_state = OFF;          
        
        currentReg = RAMP_PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_ACC = currentReg ;            
        cSPIN_Set_Param(cSPIN_TVAL_ACC, cSPIN_RegsStruct3.TVAL_ACC, MOTOR_VALVE);       
        cSPIN_RegsStruct3.TVAL_DEC = currentReg;            
        cSPIN_Set_Param(cSPIN_TVAL_DEC, cSPIN_RegsStruct3.TVAL_DEC, MOTOR_VALVE);

        currentReg = PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_RUN = currentReg ;               
        cSPIN_Set_Param(cSPIN_TVAL_RUN, cSPIN_RegsStruct3.TVAL_RUN, MOTOR_VALVE);

        // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
        currentReg = HOLDING_CURRENT_TABLE * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        
		ret = PROC_OK;
    break; 
    
	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
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
unsigned char NEWValveHomingColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  static signed long Steps_Todo;
  unsigned char currentReg;
  static char Valve_Open_Photo, Valve_Photo;            
//------------------------------------------------------------------------------  
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_VALVE);
//    Table.step = STEP_10;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }  
/*  
 else if (TintingAct.Circuit_Engaged != 0) {
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_HOMING_ERROR_ST;
    return PROC_FAIL;
  }      

 else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    StopTimer(T_VALVE_WAITING_TIME);                                        
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
*/      
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// CHECK HOME PHOTOCELL STATUS      
// -----------------------------------------------------------------------------      
    case STEP_0:
        SetStepperHomePosition(MOTOR_VALVE);
        
        currentReg = HOMING_RAMP_PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_ACC = currentReg ;            
        cSPIN_Set_Param(cSPIN_TVAL_ACC, cSPIN_RegsStruct3.TVAL_ACC, MOTOR_VALVE);       
        cSPIN_RegsStruct3.TVAL_DEC = currentReg;            
        cSPIN_Set_Param(cSPIN_TVAL_DEC, cSPIN_RegsStruct3.TVAL_DEC, MOTOR_VALVE);

        currentReg = HOMING_PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_RUN = currentReg ;               
        cSPIN_Set_Param(cSPIN_TVAL_RUN, cSPIN_RegsStruct3.TVAL_RUN, MOTOR_VALVE);

        // Set Maximum Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
                
        currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        // Se voglio che l'impostazione della Holding Current abbia effetto devo inviare almeno un comando di movimentazione alla Tavola (?????))
        MoveStepper(MOTOR_TABLE, 0, 0);  

        Steps_Todo = MAX_STEP_VALVE_HOMING_STUPID;
        // Move in CW
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        StartTimer(T_VALVE_MOVING_TIME);
        Pump.step +=7;
    break;
// -----------------------------------------------------------------------------
// VALVE HOME PHOTOCELL AND VALVE OPEN PHOTOCELL --> DARK 
    case STEP_1:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) )  {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                                       
            }
            else {
                // Rotate Valve motor till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                StartTimer(T_VALVE_WAITING_TIME);
                Pump.step ++; 
            }                
        }                
    break;            
    
    case STEP_2: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);                
            Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);     
            Pump.step ++ ;
        } 
        else if ((signed long)GetStepperPosition(MOTOR_VALVE) <= (-(signed long)(MAX_STEP_VALVE_HOMING)) ) {
            StopTimer(T_VALVE_WAITING_TIME); 
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                                                          
    break;

	// Check if position required is reached    
    case STEP_3:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            Steps_Todo = -STEP_CLOSE_VALVE;
            // Move in CCW
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                 
            Pump.step ++; 
        } 
    break; 
    
    case STEP_4:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            // Rotate Valve motor till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);
            Pump.step ++; 
        }                        
    break;         

    case STEP_5: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);                
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;            
            // Move with Photocell DARK to reach Valve HOME position
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);     
            Pump.step ++ ;
        } 
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }   
    break;
    
    case STEP_6:         
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)
            Pump.step +=4;         
    break;
// -----------------------------------------------------------------------------    
// VALVE POSITION NOT DETERMINED 
    case STEP_7:
        // Problem in CW direction: try the same movement in CCW direction
        if ( (Status_Board_Valve.Bit.MOT_STATUS == 0) || (StatusTimer(T_VALVE_MOVING_TIME)==T_ELAPSED) ) {
            StopTimer(T_VALVE_MOVING_TIME);
            Steps_Todo = -MAX_STEP_VALVE_HOMING;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                         
            Pump.step ++;            
        }
    break;      
      
    case STEP_8:
        // No problem in CCW direction: presence of an obstacle in CW direction
        if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
            StopTimer(T_VALVE_MOVING_TIME);
            StopStepper(MOTOR_VALVE);
            if ( (Valve_Photo == FALSE) && (Valve_Open_Photo == FALSE) )
                // Move the motor in CCW direction to reach correct position
                Steps_Todo = -STEP_VALVE_HOMING_OBSTACLE_CCW;            
            else
                // Move the motor in CW direction to reach correct position
                Steps_Todo = STEP_VALVE_HOMING_OBSTACLE_CW;            
                
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                        
            Pump.step ++;
        }
        // Problem also in CCW direction: Homing Stop with Error
        else if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
            Pump.errorCode = TINTING_VALVE_HOMING_ERROR_ST;
            return PROC_FAIL;                                                   
        }    
    break;      
    
    case STEP_9:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                SetStepperHomePosition(MOTOR_VALVE);
                Steps_Todo = STEP_CLOSE_VALVE;
                MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                 
                Pump.step = STEP_1;
            }
            else {
                // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
                currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
                Pump.errorCode = TINTING_VALVE_HOMING_ERROR_ST;
                return PROC_FAIL;                                                                   
            }
        }    
    break;        
// -----------------------------------------------------------------------------        
    case STEP_10:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
        
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);
        SetStepperHomePosition(MOTOR_VALVE);
        TintingAct.OpenValve_BigHole_state = OFF;  
        TintingAct.OpenValve_SmallHole_state = OFF;          
                
        currentReg = RAMP_PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_ACC = currentReg ;            
        cSPIN_Set_Param(cSPIN_TVAL_ACC, cSPIN_RegsStruct3.TVAL_ACC, MOTOR_VALVE);       
        cSPIN_RegsStruct3.TVAL_DEC = currentReg;            
        cSPIN_Set_Param(cSPIN_TVAL_DEC, cSPIN_RegsStruct3.TVAL_DEC, MOTOR_VALVE);

        currentReg = PHASE_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_RUN = currentReg ;               
        cSPIN_Set_Param(cSPIN_TVAL_RUN, cSPIN_RegsStruct3.TVAL_RUN, MOTOR_VALVE);

        // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
        currentReg = HOLDING_CURRENT_TABLE * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        
		ret = PROC_OK;
    break; 
    
	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Valve Closing process
**
**      @param unsigned char direction: CW / CCW
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char ValveClosingColorSupply(unsigned char direction)
{
  unsigned char ret = PROC_RUN;
  unsigned char currentReg;
  static signed long Steps_Todo;  
//------------------------------------------------------------------------------      
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
  // No Motor Valve Error Check here!
/*  
  // Check for Valve Pump Error 
  if (Status_Board_Valve.Bit.OCD == 0) {
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
*/         
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
	// Start Valve Close only if a Circuit is Engaged AND Valve is not in Home position     
	case STEP_0:
        if ( (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && 
             ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) ){
            if (direction == CW)
                // Rotate motor Valve till Valve Open Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            else
                // Rotate motor Valve till Valve Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            
            StartTimer(T_VALVE_WAITING_TIME);  
            Pump.step ++ ;
        }
        else 
            Pump.step +=4;
    break;
        
	// Wait for transition LIGHT-DARK
    case STEP_1:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            if (direction == CW)
                Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE + STEP_CLOSE_VALVE;
            else    
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE;            

            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            HardHiZ_Stepper(MOTOR_VALVE);            
            Pump.step +=3;
        }
    break;
          
    case STEP_2:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            if (direction == CW)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            else    
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++ ;
        }        
    break;      

    case STEP_3:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            if (direction == CW)
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;    
            else    
                Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            HardHiZ_Stepper(MOTOR_VALVE);            
            Pump.step ++;
        }                
    break;      
            
	//  Check if position required is reached    
    case STEP_4:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {            
            SetStepperHomePosition(MOTOR_VALVE);
            StopStepper(MOTOR_VALVE);
            TintingAct.OpenValve_BigHole_state = OFF;  
            TintingAct.OpenValve_SmallHole_state = OFF;          

            currentReg = RAMP_PHASE_CURRENT_VALVE * 100 /156;
            cSPIN_RegsStruct3.TVAL_ACC = currentReg ;            
            cSPIN_Set_Param(cSPIN_TVAL_ACC, cSPIN_RegsStruct3.TVAL_ACC, MOTOR_VALVE);       
            cSPIN_RegsStruct3.TVAL_DEC = currentReg;            
            cSPIN_Set_Param(cSPIN_TVAL_DEC, cSPIN_RegsStruct3.TVAL_DEC, MOTOR_VALVE);

            currentReg = PHASE_CURRENT_VALVE * 100 /156;
            cSPIN_RegsStruct3.TVAL_RUN = currentReg ;               
            cSPIN_Set_Param(cSPIN_TVAL_RUN, cSPIN_RegsStruct3.TVAL_RUN, MOTOR_VALVE);

            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
                        
            // Set Minimum Holding Current on Motor Valve 
            currentReg = HOLDING_CURRENT_VALVE * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);            
            ret = PROC_OK;
        }
    break;
    
	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
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
  static unsigned short count;
  static signed long Steps_Todo;
  //unsigned char currentReg;
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
   // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
//    Table.step = STEP_15;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }  
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }       
  else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
/*  
  if (isColorCmdStop() || isColorCmdStopProcess()) {
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
        if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
		}
        else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
            
		}
        else {
            // TABLE Motor with 1.2A
//            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
/*
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);
*/
            count = 0;
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }                
        else if  (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)       
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
/*
            // TABLE Motor with 0.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_MOVING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);      
*/      
            Pump.step ++;
        }    
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
			Durata[T_PAUSE_RECIRC] = (unsigned short)(TintingAct.Recirc_pause) * CONV_SEC_COUNT;
			// Start Ricirculation Pause
            StartTimer(T_PAUSE_RECIRC);
            HardHiZ_Stepper(MOTOR_PUMP);                    
            Pump.step ++;
        }    
	break; 	

	// Wait Ricirculation Pause
	case STEP_9:
        if (isColorCmdStop() || isColorCmdStopProcess()) 
			Pump.step ++ ;
		else if (StatusTimer(T_PAUSE_RECIRC) == T_ELAPSED) {
            StopTimer(T_PAUSE_RECIRC);
			Pump.step ++ ;
		}		
	break; 	
// -----------------------------------------------------------------------------  
// CHECK Cycles Number    
    case STEP_10:
        if (isColorCmdStop() || isColorCmdStopProcess())
            count = TintingAct.N_cycles;
        else
            count++;
            
		if (count < TintingAct.N_cycles) {
			// Got to SUCTION
			Pump.step++; 
		}
		else {
            // TABLE Motor with 1.2A
/*            
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);
*/            
            // Start decoupling
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.Speed_cycle, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0); 
            StartTimer(T_MOTOR_WAITING_TIME); 
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
               Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
               return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP); 
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                          
	break;

	//  Check if position required is reached    
    case STEP_14:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)      
            Pump.step ++;
    break;
// -----------------------------------------------------------------------------        
    case STEP_15:
/*        
        // TABLE Motor with 0.8A
        currentReg = HOLDING_CURRENT_TABLE  * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_TABLE);    
*/        
        HardHiZ_Stepper(MOTOR_PUMP);        
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;      
}

//------------------------------------------------------------------------------
//
//                      DUCKBILL
//-----------------------------------------------------------------------------
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
unsigned char HighResColorSupplyDuckbill(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned char Wait_Before_BackStep;
  static signed long Steps_Todo;
  unsigned char currentReg;
//  unsigned short accentReg = 0; 
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);  

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    StopTimer(T_MOTOR_WAITING_TIME); 
    StopTimer(T_VALVE_WAITING_TIME);       
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);                
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    //Table.step = STEP_29;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }  
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }       
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }    
  else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;                
  }
  if (isColorCmdStop() || isColorCmdStopProcess()) {
    HardHiZ_Stepper(MOTOR_VALVE);  
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
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
            return PROC_FAIL;
		}
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else {
            valve_dir = 0;
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
*/
            SetStepperHomePosition(MOTOR_PUMP);
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (Status_Board_Pump.Bit.MOT_STATUS == 0) {    
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)        
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step ++;
	break; 
// -----------------------------------------------------------------------------    
// GO to HIGH RES Erogation Starting Point
	//  Bellows head support movement (15.335mm))
    case STEP_7:
        // High Res "Normal"
        if (TintingAct.En_back_step == TRUE)
            Steps_Todo = (TintingAct.Passi_Appoggio_Soffietto - TintingAct.Step_Recup);
        // High Res modified that uses Big Hole 
        else
            Steps_Todo = (TintingAct.Passi_Appoggio_Soffietto - TintingAct.Free_param_1 - TintingAct.Step_Recup);        
            
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);
		Pump.step++;
	break;
	
	//  Check if position required is reached and Home Photocell is LIGHT
    case STEP_8:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                // High Res "Normal"
                if (TintingAct.En_back_step == TRUE)
                    Steps_Todo = TintingAct.N_step_back_step_Small_Hole; 
                // High Res modified that uses Big Hole
                else
                    Steps_Todo = TintingAct.N_step_back_step_Big_Hole;
                
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {		            
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
//            ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE_FINAL, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                
        currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	                    
        StartTimer(T_VALVE_WAITING_TIME);
        Pump.step ++ ;
	break;

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = - TintingAct.Step_Valve_Backstep;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);        
            Pump.step ++ ;            
        }
/*        
        else if ( (GetStepperPosition(MOTOR_VALVE) >= ((signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }         
*/
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;

	//  Check if position required is reached: Valve in BACKSTEP position
	case STEP_12:            
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            // High Res "Normal"
            if (TintingAct.En_back_step == TRUE)
                Steps_Todo = -(TintingAct.N_step_back_step_Small_Hole); 
            // High Res modified that uses Big Hole
            else
                Steps_Todo = -(TintingAct.N_step_back_step_Big_Hole);
                        
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Small_Hole);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_13:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);            
            Pump.step ++;
        }    
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Small hole (0.8mm)        
    case STEP_14:
//        Steps_Todo = -(TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_BIG_HOLE);
//        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0); 	                    
        StartTimer(T_VALVE_WAITING_TIME);        
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            StopTimer(T_VALVE_WAITING_TIME);            
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
            Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
	break; 
// -----------------------------------------------------------------------------    
// EROGATION    
	//  Start Erogation
	case STEP_16:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Valve not NOT open!
            if ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == DARK) || (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_OPEN_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }                        
            else
                Valve_open = TRUE;
            Steps_Todo = TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
            Pump.step ++ ;
        }                            
	break;

	//  Check if position required is reached
	case STEP_17:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            Pump.step ++;
        }                            
	break; 	

	//  Start Backstep if present
	case STEP_18:
        if (TintingAct.N_step_back_step_2 > 0) {
            Steps_Todo = -TintingAct.N_step_back_step_2; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
			Pump.step ++ ;
		}
		else {
/*
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step +=2;
            }
            else
*/
                Pump.step +=3;
		}
	break;

	//  Check if position required is reached
	case STEP_19:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
/*            
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else
*/
                Pump.step +=2;
        }    
	break; 
// -----------------------------------------------------------------------------    
// VALVE CLOSE execution    
    //  Wait before to Close Valve 	    
	case STEP_20:
/*        
	  if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
		StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
		Pump.step ++ ;		
	  }		
*/
        Pump.step ++ ;		        
	break;
    
	// Start Valve Close     
	case STEP_21:
        Valve_open = FALSE;
        // Rotate motor Valve till Photocell transition LIGHT-DARK towards Small Hole (0.8mm))
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0); 	                                    
        StartTimer(T_VALVE_WAITING_TIME);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_22:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE + STEP_CLOSE_VALVE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }        
    break;

    case STEP_23:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT)
            valve_dir = 1;            
        
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++ ;
        }        
    break;
                        
    case STEP_24:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;

	//  Check if position required is reached    
    case STEP_25:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
*/        	
            valve_dir = 0;
            if ( (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {                
                SetStepperHomePosition(MOTOR_VALVE); 		
                // Set Normal Ramp Acceleration / Deceleration to Pump Motor
                ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                              ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);            
                Pump.step ++;
            }    
        }
    break;
// -----------------------------------------------------------------------------    
// BELLOW DECOUPLING            
	//  Start Decoupling
	case STEP_26:        
        // Move motor Pump till Home Photocell transition LIGHT-DARK
        StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
        Pump.step ++ ;
	break;
// -----------------------------------------------------------------------------    
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_27:
		if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        } 
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                                          
	break;

	//  Check if position required is reached    
    case STEP_28:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {       
        	// TABLE Motor with 1.2A
        	currentReg = HOLDING_CURRENT_TABLE  * 100 /156;
        	cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        	cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            Pump.step ++;
        }                                                                      
    break;
// -----------------------------------------------------------------------------        
    case STEP_29:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);
        StopStepper(MOTOR_VALVE);                        
        HardHiZ_Stepper(MOTOR_PUMP);        
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;
}

/*
*//*=====================================================================*//**
**      @brief SINGLE STROKE dispensation algorithm with DUCKBILL
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char SingleStrokeColorSupplyDuckbill(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned char count;
  static signed long Steps_Todo;
  unsigned char currentReg;
//  unsigned short accentReg = 0; 
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    StopTimer(T_MOTOR_WAITING_TIME);  
    StopTimer(T_VALVE_WAITING_TIME);    
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);                
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);        
    //Table.step = STEP_29; 
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }         
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }  
  else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;                
  }
  if (isColorCmdStop() || isColorCmdStopProcess()) {
    HardHiZ_Stepper(MOTOR_VALVE);  
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }   
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
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }        
        else {
            valve_dir = 0;
            count = 0;
            StopTimer(T_MOTOR_WAITING_TIME); 
            StopTimer(T_VALVE_WAITING_TIME);  
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
*/
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        } 
        else if  (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)        
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            // Valve Open Position is not Fixed. Valve Close position is fixed at the end of Full Stroke
            Steps_Todo = TintingAct.N_step_full_stroke - TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);            
            Pump.step ++;
		}            
	break;       
// -----------------------------------------------------------------------------     
// BACKSTEP execution  
    case STEP_7:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
          	// Check if Valve Photocell is LIGHT or Valve Open Photocell is LIGHT
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                if (TintingAct.Free_param_2 == BIG_HOLE) {
                    Steps_Todo = TintingAct.N_step_back_step_Big_Hole; 
                    MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);                              	                    
                }                            
                else {
                    Steps_Todo = TintingAct.N_step_back_step_Small_Hole;     
                    MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Small_Hole);                              	
                }            
                Pump.step ++;     	
            }
        }            
    break;

	case STEP_8:            
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
//          ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE_FINAL, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                
            currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);

            if (TintingAct.Free_param_2 == BIG_HOLE)            
                // Valve towards Backstep Big hole (3.0mm)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	                    
            else
                // Valve towards Backstep Small hole (0.8mm)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_OPEN_PHOTOCELL, 0);
            
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++ ;                
        }    
	break; 

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_9:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);            
            if (TintingAct.Free_param_2 == BIG_HOLE)            
                // Valve towards Backstep Big hole (3.0mm)
                Steps_Todo = TintingAct.Step_Valve_Backstep;
            else
                // Valve towards Backstep Small hole (0.8mm)            
                Steps_Todo = - TintingAct.Step_Valve_Backstep;
            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;            
        } 
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }        
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position
    case STEP_10:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            if (TintingAct.Free_param_2 == BIG_HOLE) {
                Steps_Todo = -TintingAct.N_step_back_step_Big_Hole; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);                              	                    
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0); 	                                 
            }                            
            else {
                Steps_Todo = -TintingAct.N_step_back_step_Small_Hole;     
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Small_Hole);                              	
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0); 	                                                    
            }
            StartTimer(T_VALVE_WAITING_TIME);                    
            currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);                        
            Pump.step +=3;
        }                        
    break;
    
	//  Check if position required is reached: BACKSTEP executed
	case STEP_11:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            // VALVE OPEN execution                
            if (TintingAct.Free_param_2 == BIG_HOLE)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0); 	                                                    
            else
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0); 	                                    
            Pump.step += 2;
            StartTimer(T_VALVE_WAITING_TIME);                    
        }                                    
	break; 
    
// -----------------------------------------------------------------------------    
    case STEP_12:
	//  Check if position required is reached: Valve OPEN AND Pump BACKSTEP executed
	case STEP_13:            
		if ( (Status_Board_Valve.Bit.MOT_STATUS == 0) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            StopTimer(T_VALVE_WAITING_TIME);
            // Foro Grande
            if (TintingAct.Free_param_2 == BIG_HOLE)
                Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;                
            // Foro Piccolo
            else
                Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;
                    
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            if (TintingAct.Free_param_2 == BIG_HOLE)
                Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            else
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;                
            return PROC_FAIL;                           
        }                        
	break; 
// -----------------------------------------------------------------------------    
// EROGATION    
	//  Start Erogation
	case STEP_14:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Valve not NOT open!
            if ( (TintingAct.Free_param_2 == BIG_HOLE) && ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == DARK) ) ){
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }
            else if ( (TintingAct.Free_param_2 == SMALL_HOLE) && ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == DARK) || (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) ) ){
                Pump.errorCode = TINTING_VALVE_OPEN_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }                
            else
                Valve_open = TRUE;            
            Steps_Todo = TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
            Pump.step ++ ;
        }             
	break;

	//  Check if position required is reached
	case STEP_15:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step ++;
	break; 	

	//  Start Backstep if present
	case STEP_16:        
// With Duckbill NO backstep
/*        
        if (TintingAct.En_back_step) {
            Steps_Todo = -TintingAct.N_step_back_step_2; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
			Pump.step ++ ;
		}
		else {
*/
            // Last cycle
            if ((count+1) == TintingAct.N_cycles) {            
                // Start decoupling
                // Move motor Pump till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0); 
                StartTimer(T_MOTOR_WAITING_TIME); 
            }
            // Not Last Cycle
            else {
                Steps_Todo = -TintingAct.N_step_full_stroke - TintingAct.Step_Recup;
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
            }                
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step +=2;
            }
            else
                Pump.step +=3;
//		}
	break;

	//  Check if position required is reached
	case STEP_17:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {                
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
        Valve_open = FALSE;            
        if (TintingAct.Free_param_2 == BIG_HOLE)
            // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        else
            // Rotate motor Valve till Photocell transition LIGHT-DARK towards Small Hole (0.8mm))
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);                
        StartTimer(T_VALVE_WAITING_TIME);                        
        Pump.step ++ ;            
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_20:
        if ( (Status_Board_Valve.Bit.MOT_STATUS == 0) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ){
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            // Last Cycle: Valve Close in the good way
            if ((count+1) == TintingAct.N_cycles) {
                if (TintingAct.Free_param_2 == BIG_HOLE)
                    // Big Hole (3.0mm))
                    Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE;            
                else
                    // Small Hole (0.8mm))
                    Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE + STEP_CLOSE_VALVE;            
                    
                Pump.step ++ ;            
            }
            // Not Last cycle: Valve Close fastly 
            else {
                if (TintingAct.Free_param_2 == BIG_HOLE)                
                    // Big Hole (3.0mm))
                    Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
                else
                    // Small Hole (0.8mm))
                    Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
                
                Pump.step +=3 ;                            
            }                
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
    break;

    case STEP_21:
        if (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)
            valve_dir = 1;            
        
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            if (TintingAct.Free_param_2 == BIG_HOLE)                
                // Big Hole (3.0mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            else
                // Small Hole (0.8mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                
            StartTimer(T_VALVE_WAITING_TIME);                                    
            Pump.step ++ ;
        }
    break;
                        
    case STEP_22:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            if (TintingAct.Free_param_2 == BIG_HOLE)                
                // Big Hole (3.0mm))            
                Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
//                Steps_Todo = 64;
            else
                // Small Hole (0.8mm)
                Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;
//               Steps_Todo = -64;
                 
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;
    
	//  Check if position required is reached    
    case STEP_23:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
/*            
            // TABLE Motor with 0.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_MOVING * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
*/
            valve_dir = 0;            
            if ( (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }            
            Pump.step ++;
        }    
    break;
// -----------------------------------------------------------------------------    
// CHECK Cycles Number    
    case STEP_24:
        count++;
		if (count < TintingAct.N_cycles) {
			// Got to SUCTION
			Pump.step++; 
		}
		else {
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
*/
            Pump.step += 3;              
		}            
    break;
// -----------------------------------------------------------------------------        
// SUCTION
    // Start Suction
    case STEP_25:
// With Duckbill NO backstep
/*
        if (TintingAct.En_back_step) {
            Steps_Todo = -(TintingAct.N_step_full_stroke - TintingAct.N_step_back_step_2 + TintingAct.Step_Recup);
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);
        }
*/
/*        
        else  {
            Steps_Todo = -TintingAct.N_step_full_stroke - TintingAct.Step_Recup;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        }             
*/
        Pump.step++;
    break;
        
	// Check if position required is reached    
    case STEP_26:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else
//                Pump.step = STEP_6;                              
                Pump.step = STEP_4;                              
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_27:
        if ( (Status_Board_Pump.Bit.MOT_STATUS == 0) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {      
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                                                          
	break;

	//  Check if position required is reached    
    case STEP_28:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
        	// TABLE Motor with 1.2A
        	currentReg = HOLDING_CURRENT_TABLE  * 100 /156;
        	cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        	cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);    
            Pump.step ++;
        }    
    break;
// -----------------------------------------------------------------------------        
    case STEP_29:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
        HardHiZ_Stepper(MOTOR_PUMP);
        StopStepper(MOTOR_VALVE); 
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;
}
    
/*
*//*=====================================================================*//**
**      @brief CONTINUOUS dispensation algorithm with Duckbill valve integrated 
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char ContinuousColorSupplyDuckbill(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned char count_single, count_continuous;
  static signed long Steps_Todo, Steps_Todo_Suction;
  unsigned char currentReg;
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);                
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    //Table.step = STEP_49;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }         
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    StopTimer(T_MOTOR_WAITING_TIME);  
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }    
  else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  if (isColorCmdStop() || isColorCmdStopProcess()) {
    HardHiZ_Stepper(MOTOR_VALVE);  
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    StopTimer(T_MOTOR_WAITING_TIME);  
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }    
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
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }        
        else {
            valve_dir = 0;
            count_continuous = TintingAct.N_CicliDosaggio;
            count_single = TintingAct.N_cycles;            
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}               
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        } 
        else if  (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                                                                                  
	break;

	// Gear Movement (1.5mm))
	case STEP_3:
        Steps_Todo = TintingAct.Step_Ingr;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Ingr);
		Pump.step ++ ;
	break;

	//  Check if position required is reached
	case STEP_4:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)        
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
          	// Check if Valve Photocell is LIGHT or Valve Open Photocell is LIGHT
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                Steps_Todo = TintingAct.N_step_back_step_Big_Hole; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);
                Pump.step ++;     	
            }            
        }            
    break;

    // Start Backstep movement
    case STEP_9:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)        
            Pump.step ++;
	break;

	//  Valve backstep    
    case STEP_10:      
        // Valve towards Backstep Big hole (3.0mm)
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0);
        StartTimer(T_VALVE_WAITING_TIME);                    
        Pump.step ++ ;                        
    break;
        
    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);            
            // Valve towards Backstep Big hole (3.0mm)
            Steps_Todo = TintingAct.Step_Valve_Backstep;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;            
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                        
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position    
    case STEP_12:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            Steps_Todo = -TintingAct.N_step_back_step_Big_Hole; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);
            Pump.step ++;
        }                                        
    break;

	//  Check if position required is reached    
    case STEP_13:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {       
            currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);                        
            Pump.step ++;
        }                                                    
	break;

// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_14:
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        } 
	break; 
// -----------------------------------------------------------------------------    
// EROGATION IN CONTINUOUS   
	//  Start Erogation
	case STEP_16:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Valve not NOT open!            
            if ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }
            else
                Valve_open = TRUE;
            if (count_continuous == TintingAct.N_CicliDosaggio)
                Steps_Todo = TintingAct.N_step_full_stroke - TintingAct.PosStart;
            else
                Steps_Todo = TintingAct.N_step_full_stroke;
            
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle_supply);
            Pump.step ++ ;
        }    
	break;

	//  Check if position required is reached
	case STEP_17:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step ++;
	break;

    // Pump Backstep only on the last continuous cycle if no residual in single stroke is present 
	case STEP_18:        
        if ( (count_continuous == 3) && (count_single == 0) ) {
/*
            if (TintingAct.En_back_step) {
                Steps_Todo = -TintingAct.N_step_back_step_2; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
                Pump.step ++ ;
            }
            else {
*/
                // Move motor Pump till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
                StartTimer(T_MOTOR_WAITING_TIME);                                                     
                if (TintingAct.Delay_EV_off > 0)  {
                    Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                    StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                    Pump.step += 2;
                }
                else 
                    Pump.step += 3;
//            }
        }
        else
            Pump.step += 3;            
	break;

    // Pump Back Step Execution    
    case STEP_19:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){                
            if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else 
                Pump.step += 2;
        }  
    break;      

    // Delay    
    case STEP_20:
        if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
            StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
            Pump.step ++;            
        }
    break;      

    case STEP_21:
    case STEP_22:        
        Pump.step ++;    
    break;    
// -----------------------------------------------------------------------------    
// CHECK Continuous Cycles Number
	case STEP_23:            
        count_continuous--;
        // Continuous Cycles Number NOT terminated
        if (count_continuous > 2)
            Pump.step ++;
        // Continuous Cycles Number terminated
        else {
            if (count_single) {                
                Valve_open = OPEN;
                Steps_Todo_Suction = -TintingAct.N_step_stroke;            
                Pump.step +=7;
            }
            // Erogation terminated
            else {                       
                // Start Valve Close     
                // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                StartTimer(T_VALVE_WAITING_TIME);  
                // Start decoupling
                Pump.step+=3; 
            }    
        }
	break; 
// -----------------------------------------------------------------------------    
// SUCTION IN CONTINUOUS
    // Start Suction
    case STEP_24:
        Steps_Todo = -TintingAct.N_step_full_stroke;                                
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);
        Pump.step ++;        
	break; 

	// Check if Continuous Cycle is finished
	case STEP_25:
        // Valve Close Steps
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }            
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
                
            }
            else {
                count_continuous--;
                if (count_continuous == 1) { 
                    count_continuous = 0;
                    Steps_Todo_Suction = 0;
                    // Continuous terminated and Valve already Closed   
                    // Erogation proceed with residual in Single Stroke
                    if (count_single)
                        Pump.step +=5;
                    // Erogation terminated
                    else {                       
                        // Start Valve Close     
                        // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
                        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                        StartTimer(T_VALVE_WAITING_TIME);
                        // Start decoupling
                        // Move motor Pump till Home Photocell transition LIGHT-DARK
                        StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
                        StartTimer(T_MOTOR_WAITING_TIME);                                     
                        Pump.step++; 
                    }    
                }                    
                // New Erogation cycle 
                else       
                    Pump.step -=9;    
            }                
        }
	break; 	
// ----------------------------------------------------------------------------- 
// CLOSE VALVE
	// Wait for Valve Photocell DARK
	case STEP_26:
        if ( (Status_Board_Pump.Bit.MOT_STATUS == 0) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {            
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++ ;                        
        }        
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
	break; 	

    case STEP_27:
        if (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)
            valve_dir = 1;            
        
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);                                    
            Pump.step ++ ;
        }
    break;
                        
    case STEP_28:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;
    
	//  Check if position required is reached    
    case STEP_29:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            valve_dir = 0;            
            if ( (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            // Erogation proceed with residual in Single Stroke
            else if (count_single)
                Pump.step ++; 
            // Erogation terminated
            else
                Pump.step += 17;
        }    
    break;
// -----------------------------------------------------------------------------    
// SUCTION FOR STARTING SINGLE STROKE
    // Start Suction 
    case STEP_30:
        MoveStepper(MOTOR_PUMP, Steps_Todo_Suction, TintingAct.Speed_suction);        
        Pump.step ++;                
    break;

    case STEP_31:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
/*            
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
*/
            if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else { 
                if (Valve_open == CLOSE)
                    Pump.step++;
                else
                    // Go to Start Erogation
                    Pump.step += 9;
            }                
        }        
    break;
// -----------------------------------------------------------------------------    
    // GO TOWARDS START EROGATION POINT IN SINGLE STROKE EMPTY ROOM    
    case STEP_32:
        Pump.step++; 
    break;

// -----------------------------------------------------------------------------        
// BACKSTEP EXECUTION for SINGLE STROKE EMPTY ROOM  
    case STEP_33:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
          	// Check if Valve Photocell is LIGHT or Valve Open Photocell is LIGHT
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                Steps_Todo = TintingAct.N_step_back_step_Big_Hole; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);                              	
                Pump.step ++;     	
            }
        }            
    break;

	// Valve backstep    
    case STEP_34:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {  
            currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            // Valve towards Backstep Big hole (3.0mm)
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	                    
            StartTimer(T_VALVE_WAITING_TIME);                        
            Pump.step ++ ;                        
        }                        
    break;
        
    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_35:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            StopTimer(T_VALVE_WAITING_TIME);                        
            SetStepperHomePosition(MOTOR_VALVE);
            // Valve towards Backstep Big hole (3.0mm)
            Steps_Todo = TintingAct.Step_Valve_Backstep;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;            
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                        
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position    
    case STEP_36:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            Steps_Todo = -TintingAct.N_step_back_step_Big_Hole; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);
        	//  Valve Open towards Big hole (3.0mm)        
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0); 
            StartTimer(T_VALVE_WAITING_TIME);                    
            currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);                        
            Pump.step +=3;
        }                                        
    break;

    case STEP_37:
    case STEP_38:
	//  Check if position required is reached: Valve OPEN
	case STEP_39:
		if ( (Status_Board_Valve.Bit.MOT_STATUS == 0) && (Status_Board_Pump.Bit.MOT_STATUS == 0) ) {
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
	break;    
// -----------------------------------------------------------------------------    
// EROGATION for SINGLE STROKE
	case STEP_40:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) { 
            // Valve not NOT open!
            if ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }            
            else
                Valve_open = TRUE;            
            Steps_Todo = TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
            Pump.step++;                
        }            
    break;

	//  Check if position required is reached    
	case STEP_41:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)       
            Pump.step ++;
    break;
    
	//  Start Backstep if present
	case STEP_42:
        if (TintingAct.En_back_step) {
            Steps_Todo = -TintingAct.N_step_back_step_2; 
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_2);
			Pump.step ++ ;
		}
		else {
            if ((count_single-1) == 0) {            
                // Start decoupling
                // Move motor Pump till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0); 
                StartTimer(T_MOTOR_WAITING_TIME); 
            }			
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
	case STEP_43:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {                
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else
                Pump.step +=2;
        }    
	break; 

    // Wait before to Close Valve 	    
	case STEP_44:    
        if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
            StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
            Pump.step ++ ;		
        }		
	break; 
// -----------------------------------------------------------------------------    
// SET and GO to SUCTION
	case STEP_45:    
        if (TintingAct.En_back_step)
            Steps_Todo_Suction = -(TintingAct.N_step_stroke + TintingAct.N_step_back_step_2);
        else
            Steps_Todo_Suction = -TintingAct.N_step_stroke; 
            
        count_single--;
        Valve_open = CLOSE;
        // Start Valve Close     
        // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        StartTimer(T_VALVE_WAITING_TIME);                                
        Pump.step -= 19 ;                    
	break; 
// ----------------------------------------------------------------------------- 
    // GO TO HOME POSITION            
    case STEP_46:
		Pump.step ++;               
    break;

	// Wait for Pump Home Photocell DARK
	case STEP_47:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                                                                                          
	break;

	//  Check if position required is reached    
    case STEP_48:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {       
        	// TABLE Motor with 1.2A
        	currentReg = HOLDING_CURRENT_TABLE  * 100 /156;
        	cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        	cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            Pump.step ++;
        }                                                                                                                      
    break;
    
    case STEP_49:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
        
        HardHiZ_Stepper(MOTOR_PUMP);
        StopStepper(MOTOR_VALVE);                                
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
// ----------------------------------------------------------------------------- 
  }
  return ret;
}

//------------------------------------------------------------------------------
//
//                                NO_DUCKBILL
//
//------------------------------------------------------------------------------

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
  static unsigned char Wait_Before_BackStep;
  static signed long Steps_Todo;
  unsigned char currentReg;
//  unsigned short accentReg = 0; 
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);  

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    StopTimer(T_MOTOR_WAITING_TIME); 
    StopTimer(T_VALVE_WAITING_TIME);       
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);                
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    //Table.step = STEP_29;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }  
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }       
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }    
  else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;                
  }
  else if ( ((PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT)) && (Valve_open == TRUE) ) {
    Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);     
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;                
  }
  if (isColorCmdStop() || isColorCmdStopProcess()) {
    HardHiZ_Stepper(MOTOR_VALVE);  
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    StopTimer(T_MOTOR_WAITING_TIME);        
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
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
            return PROC_FAIL;
		}
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else {
            valve_dir = 0;
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
*/
            SetStepperHomePosition(MOTOR_PUMP);
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)      
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                Steps_Todo = TintingAct.N_step_back_step_Small_Hole; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {		            
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
        currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	                    
        StartTimer(T_VALVE_WAITING_TIME);
        Pump.step ++ ;
	break;

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = - TintingAct.Step_Valve_Backstep;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);        
            Pump.step ++ ;            
        }
/*        
        else if ( (GetStepperPosition(MOTOR_VALVE) >= ((signed int)TintingAct.Step_Valve_Backstep)) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ) {
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }         
*/
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;

	//  Check if position required is reached: Valve in BACKSTEP position
	case STEP_12:            
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
                        
            Steps_Todo = -(TintingAct.N_step_back_step_Small_Hole); 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Small_Hole);
            Pump.step ++;
        }                
	break; 

	//  Check if position required is reached
	case STEP_13:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);            
            Pump.step ++;
        }    
	break; 
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Small hole (0.8mm)        
    case STEP_14:
//        Steps_Todo = -(TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_BIG_HOLE);
//        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0); 	                    
        StartTimer(T_VALVE_WAITING_TIME);        
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            StopTimer(T_VALVE_WAITING_TIME);            
            // Set Maximum Ramp Acceleration / Deceleration to Pump Motor
            ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                          MAX_ACC_RATE_PUMP, MAX_DEC_RATE_PUMP, ALARMS_PUMP);
            
            Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
	break; 
// -----------------------------------------------------------------------------    
// EROGATION    
	//  Start Erogation
	case STEP_16:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Valve not NOT open!
            if ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_OPEN_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }                        
            else
                Valve_open = TRUE;
            Steps_Todo = TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
            Pump.step ++ ;
        }                            
	break;

	//  Check if position required is reached
	case STEP_17:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            Pump.step ++;
        }                            
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
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
        Valve_open = FALSE;
        // Rotate motor Valve till Photocell transition LIGHT-DARK towards Small Hole (0.8mm))
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0); 	                                    
        StartTimer(T_VALVE_WAITING_TIME);
        Pump.step ++ ;
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_22:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE + STEP_CLOSE_VALVE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }        
    break;

    case STEP_23:
        if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT)
            valve_dir = 1;            
        
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            // Rotate motor Valve till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++ ;
        }        
    break;
                        
    case STEP_24:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;

	//  Check if position required is reached    
    case STEP_25:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
*/        	
            valve_dir = 0;
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
//            if (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {                
                SetStepperHomePosition(MOTOR_VALVE); 		
                // Set Normal Ramp Acceleration / Deceleration to Pump Motor
                ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, 
                              ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);            
                Pump.step ++;
            }    
        }
    break;
// -----------------------------------------------------------------------------    
// BELLOW DECOUPLING            
	//  Start Decoupling
	case STEP_26:
        // Move motor Pump till Home Photocell transition LIGHT-DARK
        StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
        Pump.step ++ ;
	break;
// -----------------------------------------------------------------------------    
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_27:
		if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        } 
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                                          
	break;

	//  Check if position required is reached    
    case STEP_28:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {       
        	// TABLE Motor with 0.8A
        	currentReg = HOLDING_CURRENT_TABLE  * 100 /156;
        	cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        	cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            Pump.step ++;
        }                                                                      
    break;
// -----------------------------------------------------------------------------        
    case STEP_29:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);
        StopStepper(MOTOR_VALVE);                        
        HardHiZ_Stepper(MOTOR_PUMP);        
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
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
  static signed long Steps_Todo;
  unsigned char currentReg;
//  unsigned short accentReg = 0; 
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    StopTimer(T_MOTOR_WAITING_TIME);  
    StopTimer(T_VALVE_WAITING_TIME);    
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);                
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);        
    //Table.step = STEP_29; 
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }         
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  }  
  else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;                
  }
  else if ( (TintingAct.Free_param_2 == BIG_HOLE) && ( ((PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK)) && (Valve_open == TRUE) ) ) {
    Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;                
  }
  else if ( (TintingAct.Free_param_2 == SMALL_HOLE) && ( ((PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT)) && (Valve_open == TRUE) ) ) {
    Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;                
  }   
  if (isColorCmdStop() || isColorCmdStopProcess()) {
    HardHiZ_Stepper(MOTOR_VALVE);  
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);        
    return PROC_FAIL;
  } 
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
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }        
        else {
            valve_dir = 0;
            count = 0;
            StopTimer(T_MOTOR_WAITING_TIME); 
            StopTimer(T_VALVE_WAITING_TIME);
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
*/ 
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}        
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        } 
        else if  (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)        
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            // Valve Open Position is not Fixed. Valve Close position is fixed at the end of Full Stroke
            Steps_Todo = TintingAct.N_step_full_stroke - TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);            
            Pump.step ++;
		}            
	break;       
// -----------------------------------------------------------------------------     
// BACKSTEP execution  
    case STEP_7:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
          	// Check if Valve Photocell is LIGHT or Valve Open Photocell is LIGHT
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                if (TintingAct.Free_param_2 == BIG_HOLE) {
                    Steps_Todo = TintingAct.N_step_back_step_Big_Hole; 
                    MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);                              	                    
                }                            
                else {
                    Steps_Todo = TintingAct.N_step_back_step_Small_Hole;     
                    MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Small_Hole);                              	
                }            
                Pump.step ++;     	
            }
        }            
    break;

	case STEP_8:            
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);

            if (TintingAct.Free_param_2 == BIG_HOLE)            
                // Valve towards Backstep Big hole (3.0mm)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	                    
            else
                // Valve towards Backstep Small hole (0.8mm)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_OPEN_PHOTOCELL, 0);
            
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++ ;                
        }    
	break; 

    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_9:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);            
            if (TintingAct.Free_param_2 == BIG_HOLE)            
                // Valve towards Backstep Big hole (3.0mm)
                Steps_Todo = TintingAct.Step_Valve_Backstep;
            else
                // Valve towards Backstep Small hole (0.8mm)            
                Steps_Todo = - TintingAct.Step_Valve_Backstep;
            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;            
        } 
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }        
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position
    case STEP_10:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {        
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            if (TintingAct.Free_param_2 == BIG_HOLE) {
                Steps_Todo = -TintingAct.N_step_back_step_Big_Hole; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);                              	                    
            }                            
            else {
                Steps_Todo = -TintingAct.N_step_back_step_Small_Hole;     
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Small_Hole);                              	
            }
            currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);                        
            Pump.step ++;
        }                        
    break;
    
	//  Check if position required is reached: BACKSTEP executed
	case STEP_11:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (TintingAct.Free_param_2 == BIG_HOLE)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0); 	                                                    
//                Steps_Todo = TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_BIG_HOLE;             
            else
//                Steps_Todo = -(TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_SMALL_HOLE);
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0); 	                                    
            Pump.step += 2;
            StartTimer(T_VALVE_WAITING_TIME);                    
        }                                    
	break; 
    
// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
    case STEP_12:
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_13:            
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            StopTimer(T_VALVE_WAITING_TIME);
            // Foro Grande
            if (TintingAct.Free_param_2 == BIG_HOLE)
                Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;                
            // Foro Piccolo
            else
                Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;
                    
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            if (TintingAct.Free_param_2 == BIG_HOLE)
                Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            else
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;                
            return PROC_FAIL;                           
        }                        
	break; 
// -----------------------------------------------------------------------------    
// EROGATION    
	//  Start Erogation
	case STEP_14:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Valve not NOT open!
            if ( (TintingAct.Free_param_2 == BIG_HOLE) && ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) ) ){
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }
            else if ( (TintingAct.Free_param_2 == SMALL_HOLE) && ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) ) ){
                Pump.errorCode = TINTING_VALVE_OPEN_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }                
            else
                Valve_open = TRUE;            
            Steps_Todo = TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
            Pump.step ++ ;
        }             
	break;

	//  Check if position required is reached
	case STEP_15:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {                
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
            // Last cycle --> Valve Close
/*
            if ((count+1) == TintingAct.N_cycles) {
                // Rotate motor Valve till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                Pump.step ++ ;
            }
            // NOT last cycle --> Valve in Backstep Position
            else {
                Steps_Todo = TintingAct.Step_Valve_Backstep;
                MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve); 
                Pump.step += 2;                
            }              
*/
            Valve_open = FALSE;            
            if (TintingAct.Free_param_2 == BIG_HOLE)
                // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            else
                // Rotate motor Valve till Photocell transition LIGHT-DARK towards Small Hole (0.8mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);                
            StartTimer(T_VALVE_WAITING_TIME);                        
            Pump.step ++ ;            
        }		            
    break;
        
	// Wait for Valve Photocell DARK
    case STEP_20:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            // Last Cycle: Valve Close in the good way
            if ((count+1) == TintingAct.N_cycles) {
                if (TintingAct.Free_param_2 == BIG_HOLE)
                    // Big Hole (3.0mm))
                    Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE;            
                else
                    // Small Hole (0.8mm))
                    Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE + STEP_CLOSE_VALVE;                                
                Pump.step ++ ;            
                MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                                            
            }
            // Not Last cycle: Valve Close fastly 
            else {
                if (TintingAct.Free_param_2 == BIG_HOLE)                
                    // Big Hole (3.0mm))
                    Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
                else
                    // Small Hole (0.8mm))
                    Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
                
                Pump.step +=3 ;
                MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);                            
            }                
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
    break;

    case STEP_21:
        if (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)
            valve_dir = 1;            
        
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            if (TintingAct.Free_param_2 == BIG_HOLE)                
                // Big Hole (3.0mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            else
                // Small Hole (0.8mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                
            StartTimer(T_VALVE_WAITING_TIME);                                    
            Pump.step ++ ;
        }
    break;
                        
    case STEP_22:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            if (TintingAct.Free_param_2 == BIG_HOLE)                
                // Big Hole (3.0mm))            
                Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            else
                // Small Hole (0.8mm)
                Steps_Todo = -STEP_PHOTO_VALVE_SMALL_HOLE;
                
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;
    
	//  Check if position required is reached    
    case STEP_23:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
/*            
            // TABLE Motor with 0.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_MOVING * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
*/                        
            valve_dir = 0;
            
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
//            if (PhotocellStatus(VALVE_PHOTOCELL, NO_FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }            
            Pump.step ++;
        }    
    break;
// -----------------------------------------------------------------------------    
// CHECK Cycles Number    
    case STEP_24:
        count++;
		if (count < TintingAct.N_cycles) {
			// Got to SUCTION
			Pump.step++; 
		}
		else {
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
*/
            // Start decoupling
            // Move motor Pump till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0); 
            StartTimer(T_MOTOR_WAITING_TIME); 
			Pump.step += 3;              
		}            
    break;
// -----------------------------------------------------------------------------        
// SUCTION
    // Start Suction
    case STEP_25:
        if (TintingAct.En_back_step) {
            Steps_Todo = -(TintingAct.N_step_full_stroke - TintingAct.N_step_back_step_2 + TintingAct.Step_Recup);
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);
        }
        else  {
            Steps_Todo = -TintingAct.N_step_full_stroke - TintingAct.Step_Recup;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);        
        }             
        Pump.step++;
    break;
    
	// Check if position required is reached    
    case STEP_26:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else
//                Pump.step = STEP_6;                              
                Pump.step = STEP_4;                              
        }    
    break;
// -----------------------------------------------------------------------------        
// GO TO HOME POSITION            
	// Wait for Pump Home Photocell DARK
	case STEP_27:
		if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                                                          
	break;

	//  Check if position required is reached    
    case STEP_28:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
        	// TABLE Motor with 0.8A
        	currentReg = HOLDING_CURRENT_TABLE  * 100 /156;
        	cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        	cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);    
            Pump.step ++;
        }    
    break;
// -----------------------------------------------------------------------------        
    case STEP_29:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
        HardHiZ_Stepper(MOTOR_PUMP);
        StopStepper(MOTOR_VALVE); 
		ret = PROC_OK;
    break; 

	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
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
  static signed long Steps_Todo, Steps_Todo_Suction, Start_Valve_Close;
  unsigned char currentReg;
//  unsigned short accentReg = 0;     
  //----------------------------------------------------------------------------
  Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);        
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    HardHiZ_Stepper(MOTOR_PUMP);        
    HardHiZ_Stepper(MOTOR_VALVE);                
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    //Table.step = STEP_49;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }
  // Check for Motor Pump Error
  else if (Status_Board_Pump.Bit.OCD == 0) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Pump.Bit.UVLO == 0) { //|| (Status_Board_Pump.Bit.UVLO_ADC == 0) ) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Pump.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Pump.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }         
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    StopTimer(T_MOTOR_WAITING_TIME);  
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    StopTimer(T_MOTOR_WAITING_TIME);    
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }    
  else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
    StopTimer(T_MOTOR_WAITING_TIME);
    StopTimer(T_VALVE_WAITING_TIME);            
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if ( ((PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK)) && (Valve_open == TRUE) ) {
    Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    StopTimer(T_MOTOR_WAITING_TIME);     
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;                
  }
  if (isColorCmdStop() || isColorCmdStopProcess()) {
    HardHiZ_Stepper(MOTOR_VALVE);  
    HardHiZ_Stepper(MOTOR_PUMP);                                    
    StopTimer(T_MOTOR_WAITING_TIME);  
    StopTimer(T_VALVE_WAITING_TIME);            
    return PROC_FAIL;
  }  
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
        else if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }        
        else {
/*            
            // TABLE Motor with 1.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
*/            
            valve_dir = 0;
            count_continuous = TintingAct.N_CicliDosaggio;
            count_single     = TintingAct.N_cycles;            
            SetStepperHomePosition(MOTOR_PUMP);    
            Pump.step ++;
		}               
	break;

    // Move motor Pump till Coupling Photocell transition LIGHT-DARK
	case STEP_1:
        StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_EROG, LIGHT_DARK, COUPLING_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME); 
		Pump.step ++ ;
	break; 

	//  Check when Coupling Photocell is DARK: ZERO Erogation point 
	case STEP_2:
        if ( ((signed long)GetStepperPosition(MOTOR_PUMP) >= -(signed int)(TintingAct.Step_Accopp - TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) )  {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }                
        else if ( ((signed long)GetStepperPosition(MOTOR_PUMP) <= -(signed int)(TintingAct.Step_Accopp + TOLL_ACCOPP) ) && (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) {
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        } 
        else if  (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_PUMP);
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)        
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
/*            
            // TABLE Motor with 0.2A
            currentReg = HOLDING_CURRENT_TABLE_PUMP_MOVING * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                        
*/
            Pump.step ++;
        }            
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
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
          	// Check if Valve Photocell is LIGHT or Valve Open Photocell is LIGHT
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                // Start Backstep compensation movement
                Steps_Todo = TintingAct.N_step_back_step_Big_Hole; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);
                Pump.step ++;     	
            }            
        }            
    break;

    // Start Backstep movement
    case STEP_9:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)        
            Pump.step ++;
	break;

	//  Valve backstep    
    case STEP_10:        
        // Valve towards Backstep Big hole (3.0mm)
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0);
        StartTimer(T_VALVE_WAITING_TIME);                    
        Pump.step ++ ;                        
    break;
        
    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_11:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);            
            // Valve towards Backstep Big hole (3.0mm)
            Steps_Todo = TintingAct.Step_Valve_Backstep;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;            
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                        
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position    
    case STEP_12:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            Steps_Todo = -TintingAct.N_step_back_step_Big_Hole; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);
            Pump.step ++;
        }                                        
    break;

	//  Check if position required is reached    
    case STEP_13:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {       
            currentReg = HOLDING_CURRENT_VALVE_DOSING * 100 /156;
            cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);                        
            Pump.step ++;
        }                                                    
	break;

// -----------------------------------------------------------------------------    
// VALVE OPEN execution    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_14:
//        Steps_Todo = TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_BIG_HOLE;                     
//        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_15:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        } 
	break; 
// -----------------------------------------------------------------------------    
// EROGATION IN CONTINUOUS   
	//  Start Erogation
	case STEP_16:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // Valve not NOT open!            
            if ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }
            else
                Valve_open = TRUE;
                
            if (count_continuous == TintingAct.N_CicliDosaggio)
                Steps_Todo = TintingAct.N_step_full_stroke - TintingAct.PosStart;
            else if (count_continuous == 1) 
                Steps_Todo = TintingAct.PosStop;
            else
                Steps_Todo = TintingAct.N_step_full_stroke;
            
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle_supply);
            Pump.step ++ ;
        }    
	break;

	//  Check if position required is reached
	case STEP_17:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step ++;
	break;

    // Pump Back Step at the End of the Stroke
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
                Pump.step += 2;
            }
            else 
                Pump.step += 3;
        }    
	break;

    // Pump Back Step Execution    
    case STEP_19:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0){                
            if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else 
                Pump.step += 2;
        }  
    break;      

    // Delay Before Valve Close    
    case STEP_20:
        if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
            StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
            Pump.step ++;            
        }
    break;      

    // Valve Close towards BackStep Big Hole
    case STEP_21:
        Valve_open = CLOSE;
        Steps_Todo = -(TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_BIG_HOLE);                     
        MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        Pump.step ++;                      
    break;
    
    // Valve Backstep Close execution 
    case STEP_22:        
		if (Status_Board_Valve.Bit.MOT_STATUS == 0)        
            Pump.step ++;    
    break;    
// -----------------------------------------------------------------------------    
// CHECK Continuous Cycles Number
	case STEP_23:            
        count_continuous--;
        // Continuous Cycles Number NOT terminated
        if (count_continuous)
            Pump.step ++;
        // Continuous Cycles Number terminated: Close Valve
        else {
            if (TintingAct.En_back_step)
                Steps_Todo_Suction = -(TintingAct.PosStop - TintingAct.N_step_back_step_2);            
            else
                Steps_Todo_Suction = -TintingAct.PosStop;            

            // Start Valve Close     
            // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);                        
            Pump.step +=3;       
        }
	break; 
// -----------------------------------------------------------------------------    
// SUCTION IN CONTINUOUS
    // Start Suction AND Valve Closing
    case STEP_24:
        if (TintingAct.En_back_step)
            Steps_Todo = -(TintingAct.N_step_full_stroke - TintingAct.N_step_back_step_2);            
        else
            Steps_Todo = -TintingAct.N_step_full_stroke;                        
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_suction);
        
        Start_Valve_Close = CLOSING_STEP0;        
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        StartTimer(T_VALVE_WAITING_TIME);   
        
        Pump.step ++;        
	break; 

	//  Check if Valve is Closed and after Check if Continuous Cycle is finished
	case STEP_25:
        // Valve Close Steps
        if ( (Status_Board_Valve.Bit.MOT_STATUS == 0) && (Start_Valve_Close == CLOSING_STEP0) ) {
            Start_Valve_Close = CLOSING_STEP1;
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
        }
        else if (Start_Valve_Close == CLOSING_STEP1) {
            if (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)
                valve_dir = 1;            

            if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
                Start_Valve_Close = CLOSING_STEP2;
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
                StartTimer(T_VALVE_WAITING_TIME);                                    
            }            
        }
        else if ( (Status_Board_Valve.Bit.MOT_STATUS == 0) && (Start_Valve_Close == CLOSING_STEP2) ) {
            Start_Valve_Close = CLOSING_STEP3;
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
        }
        else if ( (Status_Board_Valve.Bit.MOT_STATUS == 0) && (Start_Valve_Close == CLOSING_STEP3) ) {
            Start_Valve_Close = CLOSING_STEP4;
            valve_dir = 0;            
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
//            if (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }            
        }            
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }        
        // Valve Close Steps
        if ( (Status_Board_Pump.Bit.MOT_STATUS == 0) && (Start_Valve_Close == CLOSING_STEP4) ) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }            
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
                
            }
            else {
                count_continuous--;
                if ( (count_continuous == 1) && (TintingAct.PosStop == 0) ) { 
                    count_continuous = 0;
                    Steps_Todo_Suction = 0;
                    // Continuous terminated and Valve already Closed   
                    // Erogation proceed with residual in Single Stroke
                    if (count_single)
                        Pump.step +=5;
                    // Erogation terminated
                    else
                        Pump.step += 21;
                }                    
                else {       
                    Steps_Todo = -STEPS_TO_OPEN_CHECK_VALVE;            
                    MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);        
                    Pump.step += 25;
                }    
            }                
        }
	break; 	
// ----------------------------------------------------------------------------- 
// CLOSE VALVE
	// Wait for Valve Photocell DARK
	case STEP_26:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
            Pump.step ++ ;                        
        }        
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }        
	break; 	

    case STEP_27:
        if (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)
            valve_dir = 1;            
        
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);                                    
            Pump.step ++ ;
        }
    break;
                        
    case STEP_28:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                
    break;
    
	//  Check if position required is reached    
    case STEP_29:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            valve_dir = 0;            
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
//            if (PhotocellStatus(VALVE_OPEN_PHOTOCELL, NO_FILTER) == LIGHT) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            // Erogation proceed with residual in Single Stroke
            else if (count_single)
                Pump.step ++; 
            // Erogation terminated
            else
                Pump.step += 17;
        }    
    break;
// -----------------------------------------------------------------------------    
// SUCTION FOR STARTING SINGLE STROKE
    // Start Suction 
    case STEP_30:
        MoveStepper(MOTOR_PUMP, Steps_Todo_Suction, TintingAct.Speed_suction);        
        Pump.step ++;                
    break;

    case STEP_31:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) {
                Pump.errorCode = TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) {        
                Pump.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;               
            }
            else 
                Pump.step++; 
        }        
    break;
// -----------------------------------------------------------------------------    
// GO TOWARDS START EROGATION POINT IN SINGLE STROKE EMPTY ROOM    
    case STEP_32:
        Steps_Todo = TintingAct.N_step_full_stroke - TintingAct.N_step_stroke;
        MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Appoggio_Soffietto);
        Pump.step++; 
    break;

// -----------------------------------------------------------------------------        
// BACKSTEP EXECUTION for SINGLE STROKE EMPTY ROOM  
    case STEP_33:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {
          	// Check if Valve Photocell is LIGHT or Valve Open Photocell is LIGHT
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                Steps_Todo = TintingAct.N_step_back_step_Big_Hole; 
                MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);                              	
                Pump.step ++;     	
            }
        }            
    break;

	// Valve backstep    
    case STEP_34:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            // Valve towards Backstep Big hole (3.0mm)
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	                    
            StartTimer(T_VALVE_WAITING_TIME);                        
            Pump.step ++ ;                        
        }                        
    break;
        
    // When Valve Photocell is LIGHT set ZERO position counter
    case STEP_35:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0){
            StopTimer(T_VALVE_WAITING_TIME);                        
            SetStepperHomePosition(MOTOR_VALVE);
            // Valve towards Backstep Big hole (3.0mm)
            Steps_Todo = TintingAct.Step_Valve_Backstep;            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;            
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                        
    break;
    
	//  Check if position required is reached: Valve in BACKSTEP position    
    case STEP_36:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;
            }
            Steps_Todo = -TintingAct.N_step_back_step_Big_Hole; 
            // Start Backstep
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);
            Pump.step ++;
        }                                        
    break;

	// Check if position required is reached    
    case STEP_37:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {       
            Pump.step ++;
        }                                                    
	break;
// -----------------------------------------------------------------------------    
// VALVE OPEN execution for SINGLE STROKE EMPTY ROOM    
	//  Valve Open towards Big hole (3.0mm)        
    case STEP_38:
        //Steps_Todo = TintingAct.Step_Valve_Open - TintingAct.Step_Valve_Backstep - STEP_PHOTO_VALVE_BIG_HOLE;                     
        //MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);        
        StartTimer(T_VALVE_WAITING_TIME);        
        Pump.step ++ ;
	break;

	//  Check if position required is reached: Valve OPEN
	case STEP_39:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            StopTimer(T_VALVE_WAITING_TIME);
            Steps_Todo = STEP_PHOTO_VALVE_BIG_HOLE;
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);
            Pump.step ++;
        }
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
	break;    
// -----------------------------------------------------------------------------    
// EROGATION for SINGLE STROKE
	case STEP_40:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) { 
            // Valve not NOT open!
            if ( (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }            
            else
                Valve_open = TRUE;            
            Steps_Todo = TintingAct.N_step_stroke;
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_cycle);
            Pump.step++;                
        }            
    break;

	//  Check if position required is reached    
	case STEP_41:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0)       
            Pump.step ++;
    break;
    
	//  Start Backstep if present
	case STEP_42:
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
	case STEP_43:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {                
			if (TintingAct.Delay_EV_off > 0)  {
                Durata[T_DELAY_BEFORE_VALVE_CLOSE] = TintingAct.Delay_EV_off / T_BASE;
                StartTimer(T_DELAY_BEFORE_VALVE_CLOSE);
                Pump.step ++;
            }
            else
                Pump.step +=2;
        }    
	break; 

    // Wait before to Close Valve 	    
	case STEP_44:    
        if (StatusTimer(T_DELAY_BEFORE_VALVE_CLOSE)==T_ELAPSED) {
            StopTimer(T_DELAY_BEFORE_VALVE_CLOSE);
            Pump.step ++ ;		
        }		
	break; 
// -----------------------------------------------------------------------------    
// SET and GO to SUCTION
	case STEP_45:    
        if (TintingAct.En_back_step)
            Steps_Todo_Suction = -(TintingAct.N_step_stroke + TintingAct.N_step_back_step_2);
        else
            Steps_Todo_Suction = -TintingAct.N_step_stroke;
            
        count_single--;
        Valve_open = CLOSE;
        // Start Valve Close     
        // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
        StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
        StartTimer(T_VALVE_WAITING_TIME);                                
        Pump.step -= 19 ;                    
	break; 
// ----------------------------------------------------------------------------- 
    // GO TO HOME POSITION            
    case STEP_46:
/*        
        // TABLE Motor with 1.2A
        currentReg = HOLDING_CURRENT_TABLE_PUMP_ENGAGE * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);        
*/
        // Start decoupling
        // Move motor Pump till Home Photocell transition LIGHT-DARK
        StartStepper(MOTOR_PUMP, TintingAct.Speed_suction, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0);
        StartTimer(T_MOTOR_WAITING_TIME);             
		Pump.step ++;               
    break;

	// Wait for Pump Home Photocell DARK
	case STEP_47:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_PUMP); 
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = -TintingAct.Passi_Madrevite;
            // Move with Photocell DARK to reach HOME position (7.40mm))
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.V_Accopp);            
            Pump.step ++ ;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            Pump.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                                                                                                          
	break;

	//  Check if position required is reached    
    case STEP_48:
        if (Status_Board_Pump.Bit.MOT_STATUS == 0) {       
        	// TABLE Motor with 0.8A
        	currentReg = HOLDING_CURRENT_TABLE  * 100 /156;
        	cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        	cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            Pump.step ++;
        }                                                                                                                      
    break;
    
    case STEP_49:
        // Set Minimim Holding Current on  Motor Valve 
        currentReg = HOLDING_CURRENT_VALVE * 100 /156;
        cSPIN_RegsStruct3.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct3.TVAL_HOLD, MOTOR_VALVE);        
        StopStepper(MOTOR_VALVE);                                        
        HardHiZ_Stepper(MOTOR_PUMP);
		ret = PROC_OK;
    break; 

    case STEP_50:
        if ( (Status_Board_Pump.Bit.MOT_STATUS == 0) && (Status_Board_Valve.Bit.MOT_STATUS == 0) ){            
            Steps_Todo = STEPS_TO_OPEN_CHECK_VALVE;            
            MoveStepper(MOTOR_PUMP, Steps_Todo, TintingAct.Speed_back_step_Big_Hole);        
            Pump.step++;
        }    
    break; 

    case STEP_51:    
		if (Status_Board_Pump.Bit.MOT_STATUS == 0)
            Pump.step = STEP_8;
    break;
    
	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
// ----------------------------------------------------------------------------- 
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
  static signed long Steps_Todo, Steps_Position;
  unsigned char currentReg;

//------------------------------------------------------------------------------  
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    StopTimer(T_VALVE_WAITING_TIME);    
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    HardHiZ_Stepper(MOTOR_VALVE);
    StopTimer(T_VALVE_WAITING_TIME);    
    //Table.step = STEP_7;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }  
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    StopTimer(T_VALVE_WAITING_TIME);
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    StopTimer(T_VALVE_WAITING_TIME);
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    StopTimer(T_VALVE_WAITING_TIME);
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }   
/*
  // Moving Valve ONLY if a Circuit is Engaged
 else if ( (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) || (TintingAct.Circuit_Engaged == 0) ) { 
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    StopTimer(T_VALVE_WAITING_TIME);
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
*/  

  if (isColorCmdStopProcess()) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    HardHiZ_Stepper(MOTOR_VALVE);        
    StopTimer(T_VALVE_WAITING_TIME);
    Table.step = STEP_7;      
  }    
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
        // Starting point:
        Steps_Position = (signed long)GetStepperPosition(MOTOR_VALVE);
        // Valve OPEN to BIG HOLE
        if (TintingAct.OpenValve_BigHole_state == ON) { 
			if ( (Steps_Position == 0) && (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
                
				Steps_Todo = TintingAct.Step_Valve_Open - Steps_Position;
				MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
				Pump.step ++ ;
			}
			else
                return PROC_OK;				
        }
        // Valve OPEN to SMALL HOLE
        else if (TintingAct.OpenValve_SmallHole_state == ON) {
			if ( (Steps_Position == 0) && (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
                cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
                
				Steps_Todo = -TintingAct.Step_Valve_Open + Steps_Position;
				MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);
				Pump.step ++ ;
			}
			else
                return PROC_OK;				
        }
        // Valve CLOSE        
        else {
            currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);            
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) {
                // Rotate Valve motor till Home Photocell transition DARK-LIGHT
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, DARK_LIGHT, VALVE_PHOTOCELL, 0); 	
                StopTimer(T_VALVE_WAITING_TIME);                
                Pump.step += 2 ;
			}            
            else if (Steps_Position > 0) {
                SetStepperHomePosition(MOTOR_VALVE);
                // Rotate motor Valve till Photocell transition LIGHT-DARK towards Small Hole (0.8mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
                StartTimer(T_VALVE_WAITING_TIME);                
                Pump.step += 3 ;
			}                
            else if (Steps_Position < 0) {
                SetStepperHomePosition(MOTOR_VALVE);
                // Rotate motor Valve till Photocell transition LIGHT-DARK towards Big Hole (3.0mm))
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
                StartTimer(T_VALVE_WAITING_TIME);
                Pump.step += 3 ;
			}                                
        }
    break;    
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE OPEN    
	//  Check if position required is reached: Valve OPEN
	case STEP_1: 
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            // Valve not NOT open!
            if ( (TintingAct.OpenValve_BigHole_state == ON) && ((PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK)) ) {
                Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                
            }
            else if ( (TintingAct.OpenValve_SmallHole_state == ON) && ((PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT)) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;                
            }
            else
                Pump.step +=6;
        }    
	break; 
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE CLOSE        
	// Wait for Valve Photocell LIGHT
    case STEP_2:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)  {
            StopTimer(T_VALVE_WAITING_TIME);         
            // Steps_Position > 0
            // Rotate motor Valve till Photocell transition LIGHT-DARK towards Small Hole (0.8mm))
            StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            StartTimer(T_VALVE_WAITING_TIME);                     
            Pump.step ++ ;
        }
        // No DARK-LIGHT transition
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            
            StopTimer(T_VALVE_WAITING_TIME);
            Pump.errorCode = TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }        
	break; 
        
	// Wait for Valve Photocell DARK
    case STEP_3:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0)  {
            SetStepperHomePosition(MOTOR_VALVE);
            StopTimer(T_VALVE_WAITING_TIME);         
            // CCW Rotation
            if (Steps_Position < 0)
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE - STEP_CLOSE_VALVE; 
            // CW Rotation
            else
                Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE + STEP_CLOSE_VALVE;
            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);            
            Pump.step ++ ;
        }
        // No LIGHT-DARK transition
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                

            StopTimer(T_VALVE_WAITING_TIME);
            if (Steps_Position < 0)
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            else
                Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
                
            return PROC_FAIL;                           
        }        
    break;

    case STEP_4:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {           
            // CCW Rotation
            if (Steps_Position < 0)
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CW, LIGHT_DARK, VALVE_OPEN_PHOTOCELL, 0);
            // CW Rotation
            else
                StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);
            
            StartTimer(T_VALVE_WAITING_TIME);            
            Pump.step ++ ;
        }                
    break;

	//  Check if position required is reached    
    case STEP_5:
        if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_VALVE); 	
            StopTimer(T_VALVE_WAITING_TIME);
            if (Steps_Position < 0)
                Steps_Todo = STEP_PHOTO_VALVE_SMALL_HOLE;
            else
                Steps_Todo = -STEP_PHOTO_VALVE_BIG_HOLE;
            
            MoveStepper(MOTOR_VALVE, Steps_Todo, (unsigned char)SPEED_VALVE_PHOTOCELL);
            Pump.step ++ ;
        }
        // No LIGHT-DARK transition
        else if (StatusTimer(T_VALVE_WAITING_TIME)==T_ELAPSED) {
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                

            StopTimer(T_VALVE_WAITING_TIME);
            if (Steps_Position < 0)
                Pump.errorCode = TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST;
            else
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                
            return PROC_FAIL;                           
        }                
    break;    

    case STEP_6:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            
            if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) {
                Pump.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            SetStepperHomePosition(MOTOR_VALVE); 	            
            Pump.step ++;
        }    
    break;

// -----------------------------------------------------------------------------          
    case STEP_7:
//        HardHiZ_Stepper(MOTOR_VALVE);                        
        StopStepper(MOTOR_VALVE);                        
		ret = PROC_OK;
    break; 
    
	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;
}

/*
*//*=====================================================================*//**
**      @brief Valve Rotating Process
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char ValveRotating(void)
{
  unsigned char ret = PROC_RUN;
  unsigned char currentReg;
  signed long Steps_Todo;
//------------------------------------------------------------------------------  
  Status_Board_Valve.word = GetStatus(MOTOR_VALVE);

  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    Pump.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
    return PROC_FAIL;                
  }
  // Bases Carriage Open      
  else if (TintingAct.BasesCarriageOpen == OPEN) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    HardHiZ_Stepper(MOTOR_VALVE);
    //Table.step = STEP_7;
    Pump.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;       
    return PROC_FAIL;                    
  }  
  // Check for Valve Pump Error
  else if (Status_Board_Valve.Bit.OCD == 0) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    Pump.errorCode = TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Valve.Bit.UVLO == 0) { // || (Status_Board_Valve.Bit.UVLO_ADC == 0) ) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    Pump.errorCode = TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Valve.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Valve.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    Pump.errorCode = TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }   
/*
  // Moving Valve ONLY if a Circuit is Engaged
 else if ( (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) || (TintingAct.Circuit_Engaged == 0) ) { 
    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
    cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
    Pump.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
*/  

  if (isColorCmdStopProcess() )
    Table.step = STEP_1;         
  //----------------------------------------------------------------------------
  switch(Pump.step)
  {
// -----------------------------------------------------------------------------    
// CHECK FOR VALVE ROTATING    
    case STEP_0:
        if (PeripheralAct.Peripheral_Types.Rotating_Valve == ON) {
            if (TintingAct.Output_Act == ROTATING_CW)
                Steps_Todo = STEP_VALVE_HOMING_OBSTACLE_CW;            
            else
                Steps_Todo = -STEP_VALVE_HOMING_OBSTACLE_CCW;
                            
            MoveStepper(MOTOR_VALVE, Steps_Todo, TintingAct.Speed_Valve);                        
            Pump.step ++;
        }
        else 
            return PROC_OK;
    break;    
// -----------------------------------------------------------------------------    
    case STEP_1:
		if (Status_Board_Valve.Bit.MOT_STATUS == 0) {
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            currentReg = HOLDING_CURRENT_TABLE * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);                
            StopStepper(MOTOR_VALVE);                        
            ret = PROC_OK;
        }    
    break;
    
	default:
		Pump.errorCode = TINTING_PUMP_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;
}
