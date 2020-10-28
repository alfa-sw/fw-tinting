/* 
 * File:   mixermanager.c
 * Author: michele.abelli
 * Description: Mixer Processes management
 * Created on 11 marzo 2020, 15.2
 */

#include "p24FJ256GB110.h"
#include "mixermanager.h"
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

static cSPIN_RegsStruct_TypeDef  cSPIN_RegsStruct = {0};  //to set
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
void initMixerStatusManager(void)
{
	Mixer.level = MIXER_IDLE;
    Mixer.phase = MIXER_PHASE_IDLE;
}

/*
*//*=====================================================================*//**
**      @brief Mixer Initialization parameters
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void initMixerParam(void)
{
    unsigned short i;
    // No peripheral selected
    PeripheralAct.Peripheral_Types.bytePeripheral = 0;
    TintingAct.Output_Act = OUTPUT_OFF;
    TintingAct.Time_Door_Open  = MIXER_TIME_DOOR_OPEN;
    TintingAct.Mixer_N_cycle = MIXER_DEFAULT_N_CYCLE;
    // Step N.1
    TintingAct.Mixer_Direction[0]= CW;
    TintingAct.Mixer_Speed[0] = MIXER_DEFAULT_SPEED;
    TintingAct.Mixer_Duration[0] = MIXER_DEFAULT_DURATION;        
    // Step N.2
    TintingAct.Mixer_Direction[1]= CCW;
    TintingAct.Mixer_Speed[1] = MIXER_DEFAULT_SPEED;
    TintingAct.Mixer_Duration[1] = MIXER_DEFAULT_DURATION;        
        
    for (i = 2; i < MIXER_N_PROFILE; i++) {
        TintingAct.Mixer_Direction[i]= CW;
        TintingAct.Mixer_Speed[i] = MIXER_DEFAULT_SPEED;
        TintingAct.Mixer_Duration[i] = 0;        
    } 
    
    TintingAct.Mixer_Homimg_Speed = MIXER_DEFAULT_HOMING_SPEED;
    TintingAct.Mixer_Door_Homimg_Speed = MIXER_DOOR_DEFAULT_HOMING_SPEED;
}    
/*
*//*=====================================================================*//**
**      @brief Updates Mixer status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void MixerManager(void)
{
    unsigned char ret_proc;
    unsigned char currentReg; 
    switch(Mixer.level)
    {
        case MIXER_IDLE:
            if (Status.level == TINTING_WAIT_MIXER_PARAMETERS_ST) {
                if ( AnalyzeMixerParameters() == TRUE) {
                    Mixer.level = MIXER_PAR_RX;
                    NextMixer.level = MIXER_START;
                }
                else
                    Mixer.level = MIXER_PAR_ERROR;
            } 
        break;

		case MIXER_PAR_RX:
			if (Status.level != TINTING_WAIT_MIXER_PARAMETERS_ST)
                Mixer.level = NextMixer.level;
			// STOP PROCESS command received
            else if (Status.level == TINTING_STOP_ST) { 
                Mixer.level = MIXER_IDLE;
			}                        
        break;
        
        case MIXER_START:            
            if (Status.level == TINTING_WAIT_MIXER_PARAMETERS_ST) {
                if ( AnalyzeMixerParameters() == TRUE) {
                    Mixer.level = MIXER_PAR_RX;
                    NextMixer.level = MIXER_START;
                }
                else
                    Mixer.level = MIXER_PAR_ERROR;
            }
            // New Mixing Command Request
            else if (Status.level == TINTING_SUPPLY_RUN_ST)
                Mixer.level = MIXER_SETUP; 
            // New Mixer Homing Command Received
            else if (Status.level == TINTING_MIXER_SEARCH_HOMING_ST)  {
                Mixer.step = STEP_0;
                Mixer.level = MIXER_HOMING;
            }
            // New Jar Motor Command Received
            else if (Status.level == TINTING_JAR_MOTOR_RUN_ST)
                Mixer.level = MIXER_SETUP;   
            // Set Mixer Motor Holding High Current
            else if (Status.level == TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_RUN_ST)
                Mixer.level = MIXER_SETUP;     
            // New Automatic Set High Current + Mixing + Door Open Command Received
            else if (Status.level == TINTING_MIXING_OPEN_DOOR)
                Mixer.level = MIXER_SETUP;                     
        break;
            
        case MIXER_SETUP:
            Mixer.step = STEP_0;
            if (Status.level == TINTING_SUPPLY_RUN_ST)
                //Nothing to Analyze
                Mixer.level = MIXER_RUNNING;
            else if (Status.level == TINTING_JAR_MOTOR_RUN_ST)  {
                //Nothing to Analyze
                Mixer.level = JAR_MOTOR_RUNNING;            
            }
            else if (Status.level == TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_RUN_ST)
                //Nothing to Analyze
                Mixer.level = SET_MIXER_MOTOR_HIGH_CURRENT; 
            else if (Status.level == TINTING_MIXING_OPEN_DOOR) {
                //Nothing to Analyze
                Mixer.level = SET_MIXER_MOTOR_HIGH_CURRENT_MIXING_OPEN_DOOR; 
                Mixer.phase = MIXER_PHASE_SET_HIGH_CURRENT;
            }            
        break;

        case MIXER_RUNNING:            
                ret_proc = MixingColorSupply();                                    
                if (ret_proc == PROC_OK) {
                    Mixer.level = MIXER_END;
                }
                else if (ret_proc == PROC_FAIL) {
                    Mixer.level = MIXER_ERROR;
                }
        break;
                           
        case MIXER_HOMING:
            ret_proc = MixerHomingColorSupply();
            if (ret_proc == PROC_OK) {
                Mixer.level = MIXER_END;
            }    
            else if (ret_proc == PROC_FAIL) {
               Mixer.level = MIXER_ERROR; 
            }              
        break;
        
        case JAR_MOTOR_RUNNING:
            ret_proc = MixerJarMotorSupply();
            if (ret_proc == PROC_OK) {
                Mixer.step = STEP_6;
                StartTimer(T_WAIT_BEFORE_START_MIXER);
                Mixer.level = MIXER_HOMING;
            }    
            else if (ret_proc == PROC_FAIL) {
               Mixer.level = MIXER_ERROR; 
            }              
        break;

        case SET_MIXER_MOTOR_HIGH_CURRENT:
            if (TintingAct.Motor_Enable == TRUE) {
                // MIXER Motor with 1.2A
                currentReg = HOLDING_CURRENT_MIXER_ENGAGE  * 100 /156;
                cSPIN_RegsStruct.TVAL_HOLD = currentReg;          
                cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct.TVAL_HOLD, MOTOR_MIXER);   
            }            
            Mixer.level = MIXER_END;
        break;
        
        case SET_MIXER_MOTOR_HIGH_CURRENT_MIXING_OPEN_DOOR:
            switch(Mixer.phase)
            {                
                case MIXER_PHASE_SET_HIGH_CURRENT:
                    if (TintingAct.Motor_Enable == TRUE) {
                        // MIXER Motor with 1.2A
                        currentReg = HOLDING_CURRENT_MIXER_ENGAGE  * 100 /156;
                        cSPIN_RegsStruct.TVAL_HOLD = currentReg;          
                        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct.TVAL_HOLD, MOTOR_MIXER);   
                    } 
                    Mixer.step = STEP_0; 
                    TintingAct.Check_Jar_presence = FALSE;
                    StopTimer(T_WAIT_JAR_PRESENCE);
                    StartTimer(T_WAIT_JAR_PRESENCE);
                    Mixer.phase++;
                break;
                
                case MIXER_WAIT_JAR_PRESENCE:
                    if (PhotocellStatus(JAR_PHOTOCELL, FILTER) == DARK) {  
                        Mixer.phase++;
                    }
                    else if (StatusTimer(T_WAIT_JAR_PRESENCE) == T_ELAPSED) {
                            // MIXER Motor with 0.2A
                        currentReg = HOLDING_CURRENT_MIXER_MOVING  * 100 /156;
                        cSPIN_RegsStruct.TVAL_HOLD = currentReg;          
                        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct.TVAL_HOLD, MOTOR_MIXER);                    
                        Mixer.phase = MIXER_PHASE_END;
                    }
                break;
                
                case MIXER_PHASE_MIXING:
                    ret_proc = MixingColorSupply();                                    
                    if (ret_proc == PROC_OK) {
                        TintingAct.Check_Jar_presence = TRUE;
                        Mixer.step = STEP_0;
                        Mixer.phase++;
                    }
                    else if (ret_proc == PROC_FAIL)
                        Mixer.phase = MIXER_PHASE_ERROR;               
                break;
                
                case MIXER_PHASE_DOOR_OPEN:
                    ret_proc = MixerJarMotorSupply();
                    if (ret_proc == PROC_OK) {
                        if (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT)
                            TintingAct.Check_Jar_presence = FALSE;
                        else 
                            TintingAct.Check_Jar_presence = TRUE;
                            
                        Mixer.step = STEP_6;
                        StartTimer(T_WAIT_BEFORE_START_MIXER);
                        Mixer.phase++;
                    }    
                    else if (ret_proc == PROC_FAIL)
                       Mixer.phase = MIXER_PHASE_ERROR;                                   
                break;
                
                case MIXER_PHASE_HOMING:
                    ret_proc = MixerHomingColorSupply();
                    if (ret_proc == PROC_OK) {
                        if ( ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) ) || (StopCmd == TRUE) )
                            Mixer.phase = MIXER_PHASE_END;
                        else {
                            TintingAct.Check_Jar_presence = TRUE;                            
                            Mixer.step = STEP_2; 
                            Mixer.phase = MIXER_PHASE_DOOR_OPEN;                            
                        }
                    }    
                    else if (ret_proc == PROC_FAIL) {
                       Mixer.phase = MIXER_PHASE_ERROR; 
                    }                                  
                break;
                
                case MIXER_PHASE_END: 
                    Mixer.level = MIXER_END;    
                break; 
                        
                case MIXER_PHASE_ERROR:
                    Mixer.level = MIXER_ERROR;
                break; 
                        
                default:   
                    Mixer.level = MIXER_ERROR;                    
                break;                    
            }
        break;
            
        case MIXER_END:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_JAR_MOTOR_RUN_ST) &&
                 (Status.level != TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_RUN_ST) && (Status.level != TINTING_MIXING_OPEN_DOOR) &&                   
                 (Status.level != TINTING_MIXER_SEARCH_HOMING_ST) && (Status.level != TINTING_PAR_RX))
                Mixer.level = MIXER_START; 
        break;

        case MIXER_ERROR:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_JAR_MOTOR_RUN_ST) &&
                 (Status.level != TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_RUN_ST) && (Status.level != TINTING_MIXING_OPEN_DOOR) &&                       
                 (Status.level != TINTING_MIXER_SEARCH_HOMING_ST) && (Status.level != TINTING_PAR_RX) )
                Mixer.level = MIXER_START; 
        break;
        
        case MIXER_PAR_ERROR:
            if (Status.level == TINTING_WAIT_MIXER_PARAMETERS_ST) {
                if ( AnalyzeMixerParameters() == TRUE) {
                    Mixer.level = MIXER_PAR_RX;
                    NextMixer.level = MIXER_START;
                }
                else
                    Mixer.level = MIXER_PAR_ERROR;
            } 
        break;
            
        default:
            Mixer.level = MIXER_IDLE;             
        break;            
    }        
}

/*
*//*=====================================================================*//**
**      @brief Analyze Mixer parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char AnalyzeMixerParameters(void)
{
    unsigned short i;

    if (TintingAct.Time_Door_Open  > MIXER_MAX_DOOR_OPEN)
        return FALSE;
    else if (TintingAct.Mixer_N_cycle > MIXER_MAX_N_CYCLE)
        return FALSE;
    for (i = 0; i < MIXER_N_PROFILE; i++) {   
        if ( ((TintingAct.Mixer_Direction[i]) != CW) && ((TintingAct.Mixer_Direction[i]) != CCW) )
            return FALSE;            
        else if ((TintingAct.Mixer_Speed[i]) > MIXER_MAX_SPEED)
            return FALSE;
        else if ((TintingAct.Mixer_Duration[i]) > MIXER_MAX_DURATION)    
            return FALSE;    
    }
    if (TintingAct.Mixer_N_cycle == 0)
        TintingAct.Mixer_N_cycle = MIXER_MAX_N_CYCLE;
    return TRUE;
}

/*
*//*=====================================================================*//**
**      @brief Mixer Homing process
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char MixerHomingColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  static signed long Steps_Todo;
  unsigned char currentReg;  
  static char check_door_closed_microswitch = FALSE;
  static char change_sts; 
  static char error;
  static char jar_presence;
  static char door_closing_attempts;  
  //----------------------------------------------------------------------------
  Status_Board_Mixer.word = GetStatus(MOTOR_MIXER);
  Status_Board_Door.word  = GetStatus(MOTOR_DOOR);

  if ( (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) && (check_door_closed_microswitch == TRUE) && (Mixer.step >= STEP_5) ) {
    Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
    return PROC_FAIL;            
  }

  // Check for Motor Mixer Error
  if (Status_Board_Mixer.Bit.OCD == 0) {
    Mixer.errorCode = TINTING_MIXER_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }  
  if(Status_Board_Mixer.Bit.UVLO == 0) { //|| (Status_Board_Mixer.Bit.UVLO_ADC == 0) ) {
    Mixer.errorCode = TINTING_MIXER_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Mixer.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Mixer.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Mixer.errorCode = TINTING_MIXER_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }  
  // Check for Motor Door Error
  if (Status_Board_Door.Bit.OCD == 0) {
    Mixer.errorCode = TINTING_DOOR_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Door.Bit.UVLO == 0) { //|| (Status_Board_Door.Bit.UVLO_ADC == 0) ) {
    Mixer.errorCode = TINTING_DOOR_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Door.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Door.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Mixer.errorCode = TINTING_DOOR_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }     
  StopCmd = FALSE;
  if ( (isColorCmdStop()) && (TintingAct.Check_Door_Open != TRUE) ) {
    HardHiZ_Stepper(MOTOR_DOOR);
    HardHiZ_Stepper(MOTOR_MIXER);
    StopCmd = TRUE;    
    return PROC_OK;
  }

  if ( ((Mixer.step >= 0) && (Mixer.step <= 4)) || ((Mixer.step >= 11) && (Mixer.step <= 13)) ) {
        if ( (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) && (change_sts == 0)) {
            // When Port is Opened MIXER Motor Current Phases is 0
            HardHiZ_Stepper(MOTOR_MIXER);                        
            change_sts = 1;  
        }
        else if (change_sts == 1){
            // MIXER Motor with 0.2A
            currentReg = HOLDING_CURRENT_MIXER_MOVING  * 100 /156;
            cSPIN_RegsStruct.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct.TVAL_HOLD, MOTOR_MIXER);
            change_sts = 0;
        }    
  }    
 
  //----------------------------------------------------------------------------
  switch(Mixer.step)
  {      
    case STEP_0:     
        change_sts = 1;
        SetStepperHomePosition(MOTOR_DOOR);
        check_door_closed_microswitch = FALSE;
        door_closing_attempts = 0;
//TintingAct.Door_Enable = FALSE;        
        if (TintingAct.Door_Enable == FALSE) {
            StopTimer(T_WAIT_MICRO);
            StartTimer(T_WAIT_MICRO);            
            Mixer.step +=5;
        }        
        // Door Closed
        else if ( (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) ) {
            // Move motor Door till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_DOOR, TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
            StartTimer(T_WAIT_DOOR_OPENING); 
            Mixer.step ++;
        }        
        // Door NOT Closed and NOT Open
        else if ( (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) ) {
            // Move motor Door till Home Photocell transition LIGHT-DARK
            StartStepper(MOTOR_DOOR, TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
            StartTimer(T_WAIT_DOOR_OPENING); 
            Mixer.step ++;
        }
        // Door Opened
        else if ( (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) ) {
            // Move motor Door of fixed steps to reach Microswitch CLOSE position
            Steps_Todo = (signed long)STEP_DOOR_MOTOR_OPEN;
            indicator = LIGHT_PULSE_SLOW;    
            StopTimer(T_WAIT_DOOR_OPENING);
            MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);            
            Mixer.step +=2;
        }
        // Door Closed AND Opened!!!
        else if ( (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) ) {
            // JAR not present           
            if (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) {
                // Move motor Door of some steps to understand if the problem is the Microswitch or Photocell
                Steps_Todo = (signed long)STEP_DOOR_MOTOR_CHECK_MICRO;
            }
            // JAR present
            else {
                // Move to reach Door Close position
                Steps_Todo = (signed long)STEP_DOOR_MOTOR_OPEN;
                indicator = LIGHT_PULSE_SLOW;
            }
            MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);            
            Mixer.step +=3;             
        }
        // JAR Status
        if (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT)
            jar_presence = FALSE;
        else
            jar_presence = TRUE;            
    break;     

    case STEP_1:
        // Check JAR status transition
/*
        if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) && (jar_presence == TRUE) )  {
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;    
        }
*/        
        if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == DARK) && (jar_presence == FALSE) )  {
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;    
        }
        // Door Open now
        if (Status_Board_Door.Bit.MOT_STATUS == 0){
            // Check if Micro is OPEN
            if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) {
                SetStepperHomePosition(MOTOR_DOOR);
                StopTimer(T_WAIT_DOOR_OPENING);            
                Steps_Todo = (signed long)STEP_DOOR_MOTOR_OPEN;
                // Move to reach Door Close position
                indicator = LIGHT_PULSE_SLOW;
                MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);            
                Mixer.step ++ ;
            }
            // Micro is CLOSED!!!
            else {
                Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_CLOSE_ERROR_ST;
                return PROC_FAIL;                            
            }
        }
        // Door Open Photocell never DARK
        else if ((signed long)GetStepperPosition(MOTOR_DOOR) >= (signed long)MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL) {
            StopTimer(T_WAIT_DOOR_OPENING);            
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_WAIT_DOOR_OPENING)==T_ELAPSED) {
            StopTimer(T_WAIT_DOOR_OPENING);
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                            
    break;    
    
    case STEP_2:
/*
        // Check JAR status transition
        if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) && (jar_presence == TRUE) )  {
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;    
        }
*/        
        if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == DARK) && (jar_presence == FALSE) )  {
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;    
        }
        
        if ((Status_Board_Door.Bit.MOT_STATUS == 0) || (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) ) {
            // Check if Microswitch is CLOSED now
            if ( (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) && (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == LIGHT) )  {
                StopTimer(T_WAIT_MICRO);
                StartTimer(T_WAIT_MICRO);
                Mixer.step +=3 ;
            }
            // Micro is OPEN again!!!
            else if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) {
                door_closing_attempts++;
                if (door_closing_attempts < MAX_DOOR_CLOSING_ATTEMPTS) {
                    // Try to Open the Door again
                    StopStepper(MOTOR_DOOR);
                    StartTimer(T_WAIT_DOOR_OPENING); 
                    // Rotate CCW DOOR motor till Door Open Photocell transition LIGHT_DARK
                    StartStepper(MOTOR_DOOR,  TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
                    Mixer.step += 13;
                }    
                else {
                    // JAR not present, Homing terminates with error
                    if (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) {
                        Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
                        return PROC_FAIL;
                    }
                    // JAR present, Moving DOOR to OPEN before to finish with error
                    else {                    
                        error = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
                        // Move motor Door till Home Photocell transition LIGHT-DARK
                        StartStepper(MOTOR_DOOR, TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
                        StartTimer(T_WAIT_DOOR_OPENING); 
                        Mixer.step +=2 ;
                    }
                }                    
            }
            // Photocell is still DARK!!!
            else {
                // JAR not present, Homing terminates with error
                if (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) {
                    Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_DARK_ERROR_ST;
                    return PROC_FAIL;                                 
                }
                // JAR present, Moving DOOR to OPEN before to finish with error
                else {
                    error = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_DARK_ERROR_ST;
                    // Move motor Door till Home Photocell transition LIGHT-DARK
                    StartStepper(MOTOR_DOOR, TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
                    StartTimer(T_WAIT_DOOR_OPENING); 
                    Mixer.step +=2 ;
                }                
            }  
        }
        
    break;    
    
    case STEP_3:
        if (Status_Board_Door.Bit.MOT_STATUS == 0){
            // Microswitch is till CLOSED
            if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) {
                Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_CLOSE_ERROR_ST;
                return PROC_FAIL;                                            
            }
            // Problem is Photocell DARK!!!
            else {
                Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;                                            
            }  
        }  
    break;    
    
      case STEP_4:
        // Door Open now
        if (Status_Board_Door.Bit.MOT_STATUS == 0){
            StopTimer(T_WAIT_DOOR_OPENING);  
            Mixer.errorCode = error;
            return PROC_FAIL;            
        }
        // Door Open Photocell never DARK
        else if ((signed long)GetStepperPosition(MOTOR_DOOR) >= (signed long)MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL) {
            StopTimer(T_WAIT_DOOR_OPENING);            
            Mixer.errorCode = error;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_WAIT_DOOR_OPENING)==T_ELAPSED) {
            StopTimer(T_WAIT_DOOR_OPENING);
            Mixer.errorCode = error;
            return PROC_FAIL;                           
        }                                    
    break;

    case STEP_5:
        if (StatusTimer(T_WAIT_MICRO)==T_ELAPSED) {
            StopStepper(MOTOR_DOOR);
            StopTimer(T_WAIT_MICRO);
            StartTimer(T_WAIT_BEFORE_START_MIXER);
            Mixer.step ++;
        } 
    break;    
// MIXER HOMING ----------------------------------------------------------------    
    case STEP_6:
        if (StatusTimer(T_WAIT_BEFORE_START_MIXER)==T_ELAPSED) {
            StopTimer(T_WAIT_BEFORE_START_MIXER);
            indicator = LIGHT_STEADY; 
    //TintingAct.Motor_Enable = FALSE;        
            if (TintingAct.Motor_Enable == FALSE) {
                return PROC_OK; 
            }           
            SetStepperHomePosition(MOTOR_MIXER); 
            check_door_closed_microswitch = TRUE;
            Steps_Todo = (signed long)MIXER_HOMING_OFFSET;
            // Initial Movement without check Photocell
            MoveStepper(MOTOR_MIXER, Steps_Todo, (unsigned short)MIXER_LOW_HOMING_SPEED);            
            Mixer.step ++ ;
        }        
        
    break;
    
      case STEP_7:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0) {
            SetStepperHomePosition(MOTOR_MIXER); 
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
                // Move motor Mixer till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_MIXER, TintingAct.Mixer_Homimg_Speed, CW, LIGHT_DARK, HOME_PHOTOCELL, 0);
                StartTimer(T_MOTOR_WAITING_TIME); 
                Mixer.step ++;
            }
            else {
                // Move motor Mixer till Home Photocell transition DARK-LIGHT
                StartStepper(MOTOR_MIXER, (unsigned short)MIXER_LOW_HOMING_SPEED, CCW, DARK_LIGHT, HOME_PHOTOCELL, 0);
                StartTimer(T_MOTOR_WAITING_TIME); 
                Mixer.step += 3; 
            }
        }    
    break;
    
    // GO TO HOME POSITION            
	// Wait for Mixer Home Photocell DARK
	case STEP_8:        
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_MIXER);
            StopTimer(T_MOTOR_WAITING_TIME); 
            Steps_Todo = (signed long)MIXER_HOME_PHOTO_STEPS;
            // Move with Photocell DARK to reach HOME position
            MoveStepper(MOTOR_MIXER, Steps_Todo, (unsigned short)MIXER_LOW_HOMING_SPEED);            
            Mixer.step ++ ;
        }        
        // HOME Photocell never DARK
        else if ((signed long)GetStepperPosition(MOTOR_MIXER) <= -(signed long)MAX_STEP_MIXER_MOTOR_HOME_PHOTOCELL) {
            StopTimer(T_MOTOR_WAITING_TIME); 
            check_door_closed_microswitch = FALSE;
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {         
            StopTimer(T_MOTOR_WAITING_TIME);
            check_door_closed_microswitch = FALSE;
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }          
	break;

	// Check if position required is reached    
    case STEP_9:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0) {
            check_door_closed_microswitch = FALSE;
            Mixer.step +=3; 
        }   
    break;
    
    // GO TO HOME POSITION 
    case STEP_10:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_MIXER); 
            StopTimer(T_MOTOR_WAITING_TIME);                        
            Steps_Todo = (signed long)MIXER_HOME_PHOTO_STEPS;
            // Move with Photocell DARK to reach HOME position 
            MoveStepper(MOTOR_MIXER, Steps_Todo, (unsigned short)MIXER_LOW_HOMING_SPEED);            
            Mixer.step ++ ;
        }        
        else if ((signed long)GetStepperPosition(MOTOR_MIXER) >= (signed long)MAX_STEP_MIXER_MOTOR_HOME_PHOTOCELL) {
            StopTimer(T_MOTOR_WAITING_TIME); 
            check_door_closed_microswitch = FALSE;            
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {
            StopTimer(T_MOTOR_WAITING_TIME);
            check_door_closed_microswitch = FALSE;            
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;                           
        }                  
    break;

	// Check if position required is reached    
    case STEP_11:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0) {
            check_door_closed_microswitch = FALSE;
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
                Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;                           
                
            }
            else    
                Mixer.step ++; 
        }   
    break;
// -----------------------------------------------------------------------------        
    case STEP_12:        
        if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) {
            Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
            return PROC_FAIL;            
        }
        else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
        // Now Mixer Motor is in Home position
        else {    
            // Jar Photoell DARK! Can present
            if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == DARK) && (TintingAct.Check_Jar_presence == TRUE) ) {  
                TintingAct.Jar_presence = TRUE;
                // Move motor Door till Home Photocell transition LIGHT-DARK
                StartStepper(MOTOR_DOOR, TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
                StartTimer(T_WAIT_DOOR_OPENING); 
                jar_presence = TRUE;
                Mixer.step ++;
            }
            else {                 
                if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == DARK) )
                    TintingAct.Jar_presence = TRUE;
                else
                    TintingAct.Jar_presence = FALSE;
                    
                Mixer.step +=2;
            }
            
        }   
    break; 

    case STEP_13:
        // Check JAR status transition
/*        
        if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) && (jar_presence == TRUE) )  {
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;    
        }
*/                
        // Door Opened!
        if (Status_Board_Door.Bit.MOT_STATUS == 0){
            SetStepperHomePosition(MOTOR_DOOR);
            StopTimer(T_WAIT_DOOR_OPENING);
            Steps_Todo = -(signed long)MIXER_DOOR_HOME_PHOTO_STEPS;
            // Move to reach Photocell alignement position
            MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);  
            Mixer.step ++ ;
        }
        // Door Open Photocell never DARK
        else if ((signed long)GetStepperPosition(MOTOR_DOOR) >= (signed long)MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL) {
            StopTimer(T_WAIT_DOOR_OPENING);    
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (StatusTimer(T_WAIT_DOOR_OPENING)==T_ELAPSED) {
            StopTimer(T_WAIT_DOOR_OPENING);
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                    
    break; 

    case STEP_14:
        // Door Opened!
        if (Status_Board_Door.Bit.MOT_STATUS == 0)
            return PROC_OK;
    break; 
    
    //--------------------------------------------------------------------------
    // Opening Door after problem during Closure
    case STEP_15:
        if (Status_Board_Door.Bit.MOT_STATUS == 0) {          
            StopTimer(T_WAIT_DOOR_OPENING); 
            Mixer.step++; 
        }        
        // Door Open Photocell never DARK
        else if ((signed long)GetStepperPosition(MOTOR_DOOR) >= (signed long)MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL) {
            StopTimer(T_WAIT_DOOR_OPENING);            
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        // Timeout 8 sec elapsed without Door Open
        else if (StatusTimer(T_WAIT_DOOR_OPENING)==T_ELAPSED) {
            StopTimer(T_WAIT_DOOR_OPENING);
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                 
    break;        
           
    // Door Open, check Photocell
    case STEP_16:
        if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) {  
            Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_CLOSE_ERROR_ST;
            return PROC_FAIL;
        }
        else { 
            // Move motor Door of fixed steps to reach Microswitch CLOSE position
            Steps_Todo = (signed long)STEP_DOOR_MOTOR_OPEN;
            indicator = LIGHT_PULSE_SLOW;    
            SetStepperHomePosition(MOTOR_DOOR);   
            MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);                        
            Mixer.step = STEP_2; 
        }            
    //--------------------------------------------------------------------------
    break; 
    default:
		check_door_closed_microswitch = FALSE;
        Mixer.errorCode = TINTING_MIXER_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Mixing process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char  MixingColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  static unsigned short count, step_n;
  static signed long Steps_Todo;
  static unsigned char Home_Mixer_Photo, Check_Home_Mixer_Photo;
  static unsigned char direction, Photo_Mixer_Home;
  //----------------------------------------------------------------------------
  Status_Board_Mixer.word = GetStatus(MOTOR_MIXER);

  // Check if Home Mixer Photocell is DARKENED during Mixing
  if (Check_Home_Mixer_Photo == TRUE) {
        if ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) && (Home_Mixer_Photo == 0) )
            Home_Mixer_Photo = 1;
        else if ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) && (Home_Mixer_Photo == 1) )
            Home_Mixer_Photo = 4;

        else if ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) && (Home_Mixer_Photo == 0) )
            Home_Mixer_Photo = 3;
        else if ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) && (Home_Mixer_Photo == 3) )
            Home_Mixer_Photo = 4;        
  }      
/* 
  // Jar Photoell NOT DARK during Mixing
  if ( (TintingAct.Check_Jar_presence == TRUE) && (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) ) {  
    Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;
  }
*/  
  // Door Open!
  if (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == DARK) {
    Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_DARK_ERROR_ST;
    return PROC_FAIL;            
  }
  
  // Door Microswitch Open!
  if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) {  
    Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
    return PROC_FAIL;
  }
      
  // Check for Motor Mixer Error
  if (Status_Board_Mixer.Bit.OCD == 0) {
    Mixer.errorCode = TINTING_MIXER_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Mixer.Bit.UVLO == 0) { //|| (Status_Board_Mixer.Bit.UVLO_ADC == 0) ) {
    Mixer.errorCode = TINTING_MIXER_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Mixer.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Mixer.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Mixer.errorCode = TINTING_MIXER_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }  
      
  if (isColorCmdStop()) {
    HardHiZ_Stepper(MOTOR_MIXER);        
    return PROC_OK;
  }
    
  //----------------------------------------------------------------------------
  switch(Mixer.step)
  {
// -----------------------------------------------------------------------------     
    case STEP_0:
          // Jar Photoell NOT DARK during Mixing
        if ( (TintingAct.Check_Jar_presence == TRUE) && (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) ) {  
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }

        if (TintingAct.Motor_Enable == FALSE) {
            return PROC_OK; 
        }
/*        
TintingAct.Mixer_N_cycle = 1;        
TintingAct.Mixer_Duration[0] = 0;
TintingAct.Mixer_Duration[1] = 0;
TintingAct.Mixer_Duration[2] = 0;
TintingAct.Mixer_Duration[3] = 0;
TintingAct.Mixer_Duration[4] = 0;
TintingAct.Mixer_Duration[5] = 0;
TintingAct.Mixer_Duration[6] = 0;
TintingAct.Mixer_Duration[7] = 0;
TintingAct.Mixer_Duration[8] = 0;
TintingAct.Mixer_Duration[9] = 5;
TintingAct.Mixer_Direction[9] = CCW;
*/     
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {  
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else {
            STEPPER_MIXER_OFF();
            SetStepperHomePosition(MOTOR_MIXER);  
            count = 0;
            Mixer.step++;
        }            
    break;
    
    case STEP_1:
        if ( (count < TintingAct.Mixer_N_cycle) || (TintingAct.Mixer_N_cycle == 0) ) {
            step_n = 0;    
            Check_Home_Mixer_Photo = FALSE;
            Home_Mixer_Photo = 0;            
            Mixer.step ++ ;
        }
        else {
            // Mixing completed
            Mixer.step = STEP_6; // Terminate
        }                
    break;
    
    // Mixer 'step_n'
    // -------------------------------------------------------------------------
    case STEP_2:
        if (TintingAct.Mixer_Duration[step_n] == 0) {
            STEPPER_MIXER_OFF();
            Mixer.step +=3;
        }				
        else if (TintingAct.Mixer_Speed[step_n] == 0) {
            Durata[T_WAIT_MIXER_TIME] = TintingAct.Mixer_Duration[step_n] * CONV_SEC_COUNT;
            StartTimer(T_WAIT_MIXER_TIME);
            Mixer.step +=2;
        }
        else {
            SetStepperHomePosition(MOTOR_MIXER);                
            Steps_Todo = (long)TintingAct.Mixer_Speed[step_n] * (long)TintingAct.Mixer_Duration[step_n] * (long)40*(long)CORRECTION_MIXER_STEP_RES / (long)6;
            if (Steps_Todo >= MIXER_MOTOR_ONE_ROUND)
                Check_Home_Mixer_Photo = TRUE;
                
            direction = TintingAct.Mixer_Direction[step_n];
            if (TintingAct.Mixer_Direction[step_n] == CW)
                // CW
                MoveStepper(MOTOR_MIXER, Steps_Todo, TintingAct.Mixer_Speed[step_n]);            
            else
                // CCW
                MoveStepper(MOTOR_MIXER, -Steps_Todo, TintingAct.Mixer_Speed[step_n]);            
            Mixer.step ++;				
        }			        
    break;
    
    case STEP_3:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0)            
            Mixer.step+=2;
    break;

    case STEP_4:
        if (StatusTimer(T_WAIT_MIXER_TIME)==T_ELAPSED) {
            StopTimer(T_WAIT_MIXER_TIME);
            Mixer.step ++;
        }	
    break;
        
    case STEP_5:
// 19.10.2020 - Controllo tolto xche non funziona a velocità elevate, ad es. 300rpm. Bisognerebbe mettere il controllo sotto Interrupt
/*
        // No HOME MIXER PHOTO DARKNESS during 1 cycle: Motor is Blocked! 
        if ( (Check_Home_Mixer_Photo == TRUE) && (Home_Mixer_Photo != 4) ) {
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;            
        }
*/        
        // Go to next 'step_n'
        if (step_n < (MIXER_N_PROFILE - 1) ) {
            Check_Home_Mixer_Photo = FALSE;
            Home_Mixer_Photo = 0;
            step_n++;
            Mixer.step = STEP_2;            
        }
        // Go to next 'cycle'
        else {
            if (TintingAct.Mixer_N_cycle != 0)
                count++;
            Mixer.step = STEP_1;
        }    
    break;
    // -------------------------------------------------------------------------
    // Go to Home position
    case STEP_6:
        SetStepperHomePosition(MOTOR_MIXER); 
        if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            // Move motor Mixer till Home Photocell transition LIGHT-DARK
            if (direction == CW)
                StartStepper(MOTOR_MIXER, TintingAct.Mixer_Homimg_Speed, CW, LIGHT_DARK, HOME_PHOTOCELL, 0);
            else
                StartStepper(MOTOR_MIXER, TintingAct.Mixer_Homimg_Speed, CCW, LIGHT_DARK, HOME_PHOTOCELL, 0);
                
            Photo_Mixer_Home = LIGHT;
            StartTimer(T_MOTOR_WAITING_TIME); 
            Mixer.step ++;
        }
        else {
            // Move motor Mixer till Home Photocell transition LIGHT-DARK
            if (direction == CW)
                StartStepper(MOTOR_MIXER, TintingAct.Mixer_Homimg_Speed, CW, DARK_LIGHT, HOME_PHOTOCELL, 0);
            else
                StartStepper(MOTOR_MIXER, TintingAct.Mixer_Homimg_Speed, CCW, DARK_LIGHT, HOME_PHOTOCELL, 0);
               
            Photo_Mixer_Home = DARK;
            StartTimer(T_MOTOR_WAITING_TIME); 
            Mixer.step ++;
        } 
    break;

    case STEP_7:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0) {                    
            if (direction == CW)
                Steps_Todo = (signed long)STEP_PHOTO_MIXER_OFFSET;
            else
                Steps_Todo = -(signed long)STEP_PHOTO_MIXER_OFFSET;
                
            MoveStepper(MOTOR_MIXER, Steps_Todo, (unsigned short)MIXER_LOW_HOMING_SPEED);
            Mixer.step++;                    
		}
        // HOME Photocell never DARK
        else if ( (direction == CW) && ((signed long)GetStepperPosition(MOTOR_MIXER) <= -(signed long)MAX_STEP_MIXER_MOTOR_HOME_PHOTOCELL) ) {
            StopTimer(T_MOTOR_WAITING_TIME);            
            if (Photo_Mixer_Home == LIGHT)
                Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            else
                Mixer.errorCode = TINTING_MIXER_PHOTO_READ_DARK_ERROR_ST;
                
            return PROC_FAIL;
        }
        else if ( (direction == CCW) && ((signed long)GetStepperPosition(MOTOR_MIXER) >= (signed long)MAX_STEP_MIXER_MOTOR_HOME_PHOTOCELL) ) {
            StopTimer(T_MOTOR_WAITING_TIME);            
            if (Photo_Mixer_Home == LIGHT)
                Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            else
                Mixer.errorCode = TINTING_MIXER_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }        
        else if (StatusTimer(T_MOTOR_WAITING_TIME)==T_ELAPSED) {         
            StopTimer(T_MOTOR_WAITING_TIME);
            Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                  
    break;

    case STEP_8:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0) {                    
            if ( (direction == CW) && (Photo_Mixer_Home == LIGHT) )
                Steps_Todo = (signed long)MIXER_HOME_PHOTO_STEPS - (signed long)STEP_PHOTO_MIXER_OFFSET;
            else if ( (direction == CW) && (Photo_Mixer_Home == DARK) )
                Steps_Todo = -(signed long)MIXER_HOME_PHOTO_STEPS - (signed long)STEP_PHOTO_MIXER_OFFSET; 
            else if ( (direction == CCW) && (Photo_Mixer_Home == LIGHT) )
                Steps_Todo = -(signed long)MIXER_HOME_PHOTO_STEPS + (signed long)STEP_PHOTO_MIXER_OFFSET;
            else if ( (direction == CCW) && (Photo_Mixer_Home == DARK) )
                Steps_Todo = (signed long)MIXER_HOME_PHOTO_STEPS + (signed long)STEP_PHOTO_MIXER_OFFSET;
                
                MoveStepper(MOTOR_MIXER, Steps_Todo, (unsigned short)MIXER_LOW_HOMING_SPEED);
            Mixer.step++;                    
		}                
    break;

    // Home position reached
    case STEP_9:
        if (Status_Board_Mixer.Bit.MOT_STATUS == 0) {                    
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
                Mixer.errorCode = TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;                           
            }
            else                
                ret = PROC_OK;
        }
    break; 

	default:
		Mixer.errorCode = TINTING_MIXER_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;    
  }
  return ret;      
}                

/*
*//*=====================================================================*//**
**      @brief Jar Motor Positioning process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char  MixerJarMotorSupply(void)
{
  unsigned char ret = PROC_RUN;
//  static unsigned short count, step_n;
  static signed long Steps_Todo;
//  static unsigned char Home_Mixer_Photo, Check_Home_Mixer_Photo;
  unsigned char currentReg;
  static char change_sts;
  static char door_closing_attempts;  
  //----------------------------------------------------------------------------
  Status_Board_Door.word = GetStatus(MOTOR_DOOR);

  // Check for Motor Door Error
  if (Status_Board_Door.Bit.OCD == 0) {
    Mixer.errorCode = TINTING_DOOR_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }      
  else if(Status_Board_Door.Bit.UVLO == 0) { //|| (Status_Board_Door.Bit.UVLO_ADC == 0) ) {
    Mixer.errorCode = TINTING_DOOR_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( (Status_Board_Door.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Door.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
    Mixer.errorCode = TINTING_DOOR_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }     
  if (isColorCmdStop()) {
    HardHiZ_Stepper(MOTOR_DOOR);        
    return PROC_OK; ;
  } 
  if ( (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) && (change_sts = 0) ) {
    // When Port is Opened MIXER Motor Current Phases is 0
    HardHiZ_Stepper(MOTOR_MIXER);
    change_sts = 1;
  }
  else if (change_sts == 1){
    // MIXER Motor with 0.2A
    currentReg = HOLDING_CURRENT_MIXER_MOVING  * 100 /156;
    cSPIN_RegsStruct.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct.TVAL_HOLD, MOTOR_MIXER);                    
    change_sts = 0;      
  }    
  //----------------------------------------------------------------------------
  switch(Mixer.step)
  {
// -----------------------------------------------------------------------------     
    // Start Opening 
    case STEP_0:
        door_closing_attempts = 0;
        change_sts = 1;
        if (TintingAct.Door_Enable == FALSE) {
            return PROC_OK;     
        }
        if ((PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) && (TintingAct.Check_Jar_presence == TRUE)) {  
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == DARK) {
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_DARK_ERROR_ST;
            return PROC_FAIL;            
        }
        else if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) {  
            Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
            return PROC_FAIL;
        }   
        else {
            STEPPER_MIXER_OFF();
            SetStepperHomePosition(MOTOR_DOOR); 
            StartTimer(T_WAIT_DOOR_OPENING); 
            // Rotate CCW DOOR motor till Door Open Photocell transition LIGHT_DARK
            StartStepper(MOTOR_DOOR,  TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
            Mixer.step ++ ;
        }            
    break;
    
    // Opening Door
    case STEP_1:
        if (Status_Board_Door.Bit.MOT_STATUS == 0) {          
            StopTimer(T_WAIT_DOOR_OPENING); 
            Mixer.step++; 
        }        
        // Door Open Photocell never DARK
        else if ((signed long)GetStepperPosition(MOTOR_DOOR) >= (signed long)MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL) {
            StopTimer(T_WAIT_DOOR_OPENING);            
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        // Timeout 8 sec elapsed without Door Open
        else if (StatusTimer(T_WAIT_DOOR_OPENING)==T_ELAPSED) {
            StopTimer(T_WAIT_DOOR_OPENING);
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }
/*        
        else if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) && (TintingAct.Check_Jar_presence == TRUE) ) {  
            StopTimer(T_WAIT_DOOR_OPENING); 
            Mixer.errorCode = TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
*/
    break;

    // Door Open, check Photocell
    case STEP_2:
        if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) {  
            Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_CLOSE_ERROR_ST;
            return PROC_FAIL;
        }
        else {
            StartTimer(T_WAIT_DOOR_OPEN);
            TintingAct.Jar_presence = TRUE;
            Mixer.step ++;    
        }
    break;
    
    // Wait Jar Removal
    case STEP_3:
        if (StatusTimer(T_WAIT_DOOR_OPEN)==T_ELAPSED) {
            // Home Photocell NOT covered --> Close the DOOR
            if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
                StopTimer(T_WAIT_DOOR_OPEN);
                // Start Closing Door
                SetStepperHomePosition(MOTOR_MIXER);   
                Steps_Todo = (signed long)STEP_DOOR_MOTOR_OPEN;
                // Move to reach Door Closed position
                indicator = LIGHT_PULSE_SLOW;  
                StartTimer(T_WAIT_DOOR_OPENING);
                MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);                            
                Mixer.step +=2 ;
            }                
        }
        if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) {
            StopTimer(T_WAIT_DOOR_OPEN);            
            Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_CLOSE_ERROR_ST;
            return PROC_FAIL;
        }        
        // Jar removed
        if ( (PhotocellStatus(JAR_PHOTOCELL, FILTER) == LIGHT) || (TintingAct.Check_Jar_presence == FALSE) ) {  
            TintingAct.Jar_presence = FALSE;
            StopTimer(T_WAIT_DOOR_OPEN);
            StopTimer(T_WAIT_BEFORE_CLOSING_DOOR);
            StartTimer(T_WAIT_BEFORE_CLOSING_DOOR);
            Mixer.step ++ ;    
        }
    break;

    // Wait before Start Closing Door    
    case STEP_4:
        if (StatusTimer(T_WAIT_BEFORE_CLOSING_DOOR)==T_ELAPSED) {
            StopTimer(T_WAIT_BEFORE_CLOSING_DOOR);
            // Start Closing Door
            SetStepperHomePosition(MOTOR_MIXER);   
            Steps_Todo = (signed long)STEP_DOOR_MOTOR_OPEN;
            // Move to reach Door Closed position
            indicator = LIGHT_PULSE_SLOW;  
            StartTimer(T_WAIT_DOOR_OPENING);
            MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);            
            Mixer.step ++ ;   
        }    
    break;

    // Closing Door
    case STEP_5:
        if ((Status_Board_Door.Bit.MOT_STATUS == 0) || (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) ) {  
            StopTimer(T_WAIT_DOOR_OPENING); 
            StopTimer(T_WAIT_MICRO);
            StartTimer(T_WAIT_MICRO);
            Mixer.step++;   
        }
        // Timeout elapsed without Door Closed
        else if (StatusTimer(T_WAIT_DOOR_OPENING)==T_ELAPSED) {
            door_closing_attempts++;
            if (door_closing_attempts < MAX_DOOR_CLOSING_ATTEMPTS) {
                // Try to Open the Door again
                StopStepper(MOTOR_DOOR);
                StopTimer(T_WAIT_DOOR_OPENING); 
                StartTimer(T_WAIT_DOOR_OPENING); 
                // Rotate CCW DOOR motor till Door Open Photocell transition LIGHT_DARK
                StartStepper(MOTOR_DOOR,  TintingAct.Mixer_Door_Homimg_Speed, CCW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0);
                Mixer.step +=3; 
            }
            else {
                Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
                return PROC_FAIL;                
            }    
        }        
    break;
    
    // Door Closed Check Photocell Status
    case STEP_6:
        if (StatusTimer(T_WAIT_MICRO)==T_ELAPSED) {
            StopTimer(T_WAIT_MICRO);
            StopStepper(MOTOR_DOOR);                
            if (PhotocellStatus(DOOR_OPEN_PHOTOCELL, FILTER) == DARK) {
                Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_DARK_ERROR_ST;
                return PROC_FAIL;            
            }
            else if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == OPEN) {
                Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST;
                return PROC_FAIL;
            }        
            else   
                Mixer.step ++ ;    
        }    
    break;

    // End Door process
    case STEP_7:
        StopStepper(MOTOR_DOOR);        
        ret = PROC_OK;    
    break;
    
    //--------------------------------------------------------------------------
    // Opening Door after problem during Closure
    case STEP_8:
        if (Status_Board_Door.Bit.MOT_STATUS == 0) {          
            StopTimer(T_WAIT_DOOR_OPENING); 
            Mixer.step++; 
        }        
        // Door Open Photocell never DARK
        else if ((signed long)GetStepperPosition(MOTOR_DOOR) >= (signed long)MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL) {
            StopTimer(T_WAIT_DOOR_OPENING);            
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        // Timeout 8 sec elapsed without Door Open
        else if (StatusTimer(T_WAIT_DOOR_OPENING)==T_ELAPSED) {
            StopTimer(T_WAIT_DOOR_OPENING);
            Mixer.errorCode = TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;                           
        }                 
    break;        
           
    // Door Open, check Photocell
    case STEP_9:
        if (PhotocellStatus(DOOR_MICROSWITCH, FILTER) == CLOSE) {  
            Mixer.errorCode = TINTING_MIXER_DOOR_MICROSWITCH_CLOSE_ERROR_ST;
            return PROC_FAIL;
        }
        else {
            // Start Closing Door
            SetStepperHomePosition(MOTOR_DOOR);   
            Steps_Todo = (signed long)STEP_DOOR_MOTOR_OPEN;
            // Move to reach Door Closed position
            indicator = LIGHT_PULSE_SLOW;  
            StartTimer(T_WAIT_DOOR_OPENING);
            MoveStepper(MOTOR_DOOR, Steps_Todo, MIXER_DOOR_CLOSING_SPEED);                        
            Mixer.step = STEP_5;  
        }    
    break;
    //--------------------------------------------------------------------------
    
	default:
		Mixer.errorCode = TINTING_MIXER_SOFTWARE_ERROR_ST;
        ret = PROC_FAIL;
    break;        
  }    
    return ret;        
}
