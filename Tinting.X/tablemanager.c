/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Table Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "tableManager.h"
#include "statusManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "stepper.h"
#include <xc.h>
#include <math.h>
#include "typedef.h"
#include "eepromManager.h"
#include "stepperParameters.h"
#include "spi.h"
#include "math.h"
#include "L6482H.h"


#ifndef SKIP_FAULT_1
static unsigned char getFault_1Error(void);
#endif

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
void initTableStatusManager(void)
{
	Status.level = TINTING_INIT_ST;    
    // BRUSH Driver DRV8842 Reset 
    Init_DRIVER_RESET();
    #ifndef SKIP_FAULT_1
    resetFault_1();
    #endif    
}

/*
*//*=====================================================================*//**
**      @brief Table Initialization parameters
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void initTableParam(void)
{
  unsigned char i;  
  // Passi corrispondenti ad un giro completa di 360° della tavola
  TintingAct.Steps_Revolution = (unsigned long)STEPS_REVOLUTION;
  // Tolleranza in passi corrispondente ad una rotazione completa di 360° della tavola
  TintingAct.Steps_Tolerance_Revolution = STEPS_TOLERANCE_REVOLUTION;
  // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
  TintingAct.Steps_Reference = STEPS_REFERENCE;
  // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
  TintingAct.Steps_Tolerance_Reference = STEPS_TOLERANCE_REFERENCE;
  // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito 
  TintingAct.Steps_Circuit = STEPS_CIRCUIT;
  // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito
  TintingAct.Steps_Tolerance_Circuit = STEPS_TOLERANCE_CIRCUIT;
  // Velocità massima di rotazione della tavola rotante
  TintingAct.High_Speed_Rotating_Table = HIGH_SPEED_ROTATING_TABLE;
  // Velocità minima di rotazione della tavola rotante
  TintingAct.Low_Speed_Rotating_Table = LOW_SPEED_ROTATING_TABLE;
  // Distanza in passi tra il circuito di riferimento e la spazzola
  TintingAct.Steps_Cleaning = STEPS_CLEANING;
  
  // Cleaning Duration (sec))
  TintingAct.Cleaning_duration = CLEANING_DURATION;
  // Cleaning Pause (min))
  TintingAct.Cleaning_pause = CLEANING_PAUSE;

  Table_circuits_pos = OFF;
  Total_circuit_n = 0;
  // Le posizioni inzialmente sono quelle teoriche. Nessun autoriconoscimento in qesta fase.
  TintingAct.Circuit_step_theorical_pos[0] = (signed long)198 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[1] = (signed long)595 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[2] = (signed long)991 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[3] = (signed long)1387 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[4] = (signed long)1784 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[5] = (signed long)2180 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[6] = (signed long)2576 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[7] = (signed long)2973 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[8] = (signed long)3369 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[9] = (signed long)3765 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[10] = (signed long)4162 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[11] = (signed long)4558 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[12] = (signed long)4955 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[13] = (signed long)5351 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[14] = (signed long)5747 * (signed long)CORRECTION_TABLE_STEP_RES;
  TintingAct.Circuit_step_theorical_pos[15] = (signed long)6144 * (signed long)CORRECTION_TABLE_STEP_RES;
  
  TintingAct.Circuit_Engaged = 0;
  TintingAct.Table_Step_position = DYNAMIC;

  for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
      TintingAct.Circuit_step_pos[i] = 0;
  }
  
/*  
  CircStepPosAct.Circ_Pos[0] = 770;
  CircStepPosAct.Circ_Pos[1] = 2357;
  CircStepPosAct.Circ_Pos[2] = 3936;
  CircStepPosAct.Circ_Pos[3] = 5523;
  CircStepPosAct.Circ_Pos[4] = 7112;
  CircStepPosAct.Circ_Pos[5] = 8691;
  CircStepPosAct.Circ_Pos[6] = 10281;
  CircStepPosAct.Circ_Pos[7] = 11869;
  CircStepPosAct.Circ_Pos[8] = 13457;
  CircStepPosAct.Circ_Pos[9] = 15047;
  CircStepPosAct.Circ_Pos[10] = 16636;
  CircStepPosAct.Circ_Pos[11] = 21393;
  CircStepPosAct.Circ_Pos[12] = 0;
  CircStepPosAct.Circ_Pos[13] = 0;
  CircStepPosAct.Circ_Pos[14] = 0;
  CircStepPosAct.Circ_Pos[15] = 0;  
*/ 
//  Total_circuit_n = 12;  
}    

/*
*//*=====================================================================*//**
**      @brief Updates Table status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void TableManager(void)
{
    switch(Table.level)
    {
        case TABLE_IDLE:
            if (Status.level == TINTING_WAIT_TABLE_PARAMETERS_ST) {
                if (AnalyzeTableParameters() == TRUE) {
                    Table.level = TABLE_PAR_RX;
                    NextTable.level = TABLE_START;
                }
                else
                    Table.level = TABLE_PAR_ERROR;
            } 
        break;

		case TABLE_PAR_RX:
			if (Status.level != TINTING_WAIT_TABLE_PARAMETERS_ST)
                Table.level = NextTable.level;
			// STOP PROCESS command received
            else if (Status.level == TINTING_STOP_ST) { 
                Table.level = TABLE_IDLE;
			}                        
        break;
        
        case TABLE_START:            
// Ma servono davvero ???
/*
            // New Erogation Command Request
            if (Status.level == TINTING_SUPPLY_RUN_ST)
                Table.level = TABLE_SETUP; 
            // New Ricirculation Command Received
            else if (Status.level == TINTING_STANDBY_RUN_ST)
                Table.level = TABLE_SETUP; 
*/
            if (Status.level == TINTING_WAIT_TABLE_PARAMETERS_ST) {
                if (AnalyzeTableParameters() == TRUE) {
                    Table.level = TABLE_PAR_RX;
                    NextTable.level = TABLE_START;
                }
                else
                    Table.level = TABLE_PAR_ERROR;
            }             
            // New Table Homing Command Received
            else if ( (Status.level == TINTING_TABLE_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) ) {
                Table.level = TABLE_HOMING;
                Table.step = STEP_0;
            }
            // New Table Self Recognition Command Received
            else if (Status.level == TINTING_TABLE_SELF_RECOGNITION_ST) {
//                Table.level = TABLE_HOMING;
                Table.level = TABLE_SELF_RECOGNITION;
                Table.step = STEP_0;    
            }
            // New Table Positioning Command Received
            else if (Status.level == TINTING_TABLE_POSITIONING_ST) {
                End_Table_Position = 0;
                Table.level = TABLE_POSITIONING;
                Table.step = STEP_0;    
            }
            // New Stirring Command Received
            else if (Status.level == TINTING_TABLE_STIRRING_ST) {
                Table.level = TABLE_STIRRING;
                Table.step = STEP_0;    
            }                        
            // New Table Cleaning Command Received
            else if (Status.level == TINTING_TABLE_CLEANING_ST) {
                Table.level = TABLE_CLEANING;
                Table.step = STEP_0;    
            }
            // New Table Test Command Received
            else if (Status.level == TINTING_TABLE_TEST_ST) {
                Table.level = TABLE_TEST;
                Table.step = STEP_0;    
            } 
            // New Table Steps Positioning Command Received
            else if (Status.level == TINTING_TABLE_STEPS_POSITIONING_ST) {
                Table.level = TABLE_STEPS_POSITIONING;
                Table.step = STEP_0;    
            }
            // Table has to go to Reference Position
            else if (Status.level == TINTING_TABLE_GO_REFERENCE_ST) {
                Table.level = TABLE_GO_REFERENCE;
                Table.step = STEP_0;    
            }            
        break;
            
        case TABLE_SETUP:

        break;

        case TABLE_HOMING:
#ifndef NOLAB	            
            if ( (Status.level == TINTING_TABLE_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) ) {
                if (TableHomingColorSupply() == PROC_OK)
                    Table.level = TABLE_END;
                else if (TableHomingColorSupply() == PROC_FAIL)
                   Table.level = TABLE_ERROR;
            }
            else if (Status.level == TINTING_TABLE_SELF_RECOGNITION_ST) {
                if (TableHomingColorSupply() == PROC_OK) {
                    Table.level = TABLE_SELF_RECOGNITION;
                    Table.step = STEP_0;    
                }
                else if (TableHomingColorSupply() == PROC_FAIL)
                   Table.level = TABLE_ERROR;                
            }
#else            
            Table.level = TABLE_END;  
#endif            
        break;

        case TABLE_CLEANING:
Table.level = TABLE_END;            
/*
            if (TableCleaningColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TableCleaningColorSupply() == PROC_FAIL)
                Table.level = TABLE_ERROR;
*/
        break;
                
        case TABLE_POSITIONING:
#ifndef NOLAB            
            if (TablePositioningColorSupply() == PROC_OK) {
                if (TintingAct.PanelTable_state == OPEN)
                    End_Table_Position = 1;
                Table.level = TABLE_END;
            }
            else if (TablePositioningColorSupply() == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_END;
#endif            
        break;

        case TABLE_STIRRING:
            if (TableStirring() == PROC_OK) {
                
                Table.step  = STEP_0;
//                Table.level = TABLE_STOP_STIRRING;
                Table.level = TABLE_GO_REFERENCE;
            }
            else if (TableStirring() == PROC_FAIL)
                Table.level = TABLE_ERROR;       
        break;
        
        case TABLE_STOP_STIRRING:
            if (TableHomingColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TableHomingColorSupply() == PROC_FAIL)
                Table.level = TABLE_ERROR;
        break;
            
        case TABLE_STEPS_POSITIONING:
            if (TableStepsPositioningColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TableStepsPositioningColorSupply() == PROC_FAIL)
               Table.level = TABLE_ERROR;
        break;
        
        case TABLE_GO_REFERENCE:
            if (TableGoToReference() == PROC_OK) {
                End_Table_Position = 0;
                Table.level = TABLE_END;
            }
            else if (TableGoToReference() == PROC_FAIL)
               Table.level = TABLE_ERROR;            
        break;
            
        case TABLE_SELF_RECOGNITION:
            if (TableSelfRecognitionColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TableSelfRecognitionColorSupply() == PROC_FAIL)
                Table.level = TABLE_ERROR;
        break;

        case TABLE_TEST:        
            if (TableTestColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TableTestColorSupply() == PROC_FAIL)
                Table.level = TABLE_ERROR;        
        break;
        
        case TABLE_END:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_TABLE_SEARCH_HOMING_ST) && (Status.level != TINTING_TABLE_POSITIONING_ST) && 
                 (Status.level != TINTING_TABLE_CLEANING_ST) && (Status.level != TINTING_TABLE_SELF_RECOGNITION_ST) &&
                 (Status.level != TINTING_TABLE_TEST_ST) && (Status.level != TINTING_TABLE_GO_REFERENCE_ST) &&
                 (Status.level != TINTING_TABLE_STEPS_POSITIONING_ST) && (Status.level != TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) )
                Table.level = TABLE_START; 
        break;

        case TABLE_ERROR:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_TABLE_SEARCH_HOMING_ST) && (Status.level != TINTING_TABLE_POSITIONING_ST) &&
                 (Status.level != TINTING_TABLE_CLEANING_ST) && (Status.level != TINTING_TABLE_SELF_RECOGNITION_ST) && 
                 (Status.level != TINTING_TABLE_TEST_ST) && (Status.level != TINTING_TABLE_GO_REFERENCE_ST) &&
                 (Status.level != TINTING_TABLE_STEPS_POSITIONING_ST) && (Status.level != TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) )
                Table.level = TABLE_START; 
        break;
        
        case TABLE_PAR_ERROR:
            if (Status.level == TINTING_WAIT_TABLE_PARAMETERS_ST) {
                if ( AnalyzeTableParameters() == TRUE) {
                    Table.level = TABLE_PAR_RX;
                    NextTable.level = TABLE_START;
                }
                else
                    Table.level = TABLE_PAR_ERROR;
            } 
        break;
            
        default:
            Table.level = TABLE_IDLE;             
        break;            
    }        
}
/*
*//*=====================================================================*//**
**      @brief Analyze Table parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
unsigned char AnalyzeTableParameters(void)
{
    unsigned char i;
    // Check Maximum Revolution Step
    if (TintingAct.Steps_Revolution > ((unsigned long)STEPS_REVOLUTION + (unsigned long)STEPS_TOLERANCE_REVOLUTION))
        return FALSE;
    // Maximum Revolution Step has to be > Steps Cleaning
    if (TintingAct.Steps_Cleaning > TintingAct.Steps_Revolution) 
        return FALSE;
    // Low Speed >= High Speed
    if (TintingAct.Low_Speed_Rotating_Table > TintingAct.High_Speed_Rotating_Table)
        return FALSE;
    // High Speed > Maximum Table Speed
    if (TintingAct.High_Speed_Rotating_Table > MAX_TABLE_SPEED)
        return FALSE;
    // Low Speed < Minimum Table Speed
    if (TintingAct.Low_Speed_Rotating_Table < MIN_TABLE_SPEED)
        return FALSE;
    // Tolerances have to be smaller than Steps
    if ( (TintingAct.Steps_Revolution <= TintingAct.Steps_Tolerance_Revolution) ||  
         (TintingAct.Steps_Reference <= TintingAct.Steps_Tolerance_Reference) ) 
//         (TintingAct.Steps_Circuit <= TintingAct.Steps_Tolerance_Circuit) )   
        return FALSE;

    for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
        if (i < 8)
            TintingAct.Table_Colorant_En[i] = (TintingAct.Colorant_1 & (1 << i) ) >> i;
        else
            TintingAct.Table_Colorant_En[i] = (TintingAct.Colorant_2 & (1 << (i - 8) ) ) >> (i - 8);            
    }
    return TRUE;
}

/*
*//*=====================================================================*//**
**      @brief Recirculation processe
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char TableHomingColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static unsigned char Find_Circuit, Tr_Dark_Light, Tr_Light_Dark, Old_Photocell_sts, New_Photocell_sts;
  static signed long Position_Steps, Steps_Todo, Count_Steps, Max_Count_Steps, Reference_Position_Step; 
  static unsigned char count_tr, check_motor, Find_circuit_n;   
  unsigned char i;
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if (((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE))  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  
  else if (((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0)) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/  
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_TABLE);                        
    Table.step = STEP_12;
  }
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);    
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
	case STEP_0:
		SetStepperHomePosition(MOTOR_TABLE);
        Find_Circuit  = OFF;
        Tr_Light_Dark = OFF;
        Tr_Dark_Light = OFF;
        Max_Count_Steps = 0;
        Status.errorCode = 0;
        Find_circuit_n = 0;
count_tr = 0;
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
            Table.step ++;
            check_motor = 0;
        }
        else {
            Steps_Todo = TintingAct.Steps_Revolution; 
            // Table CW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);            
            Table.step +=3;           
        }            
	break;

    // Wait for DARK-LIGHT Table PhotoStatus_Board_Valvecell transition 
    case STEP_1:
        if (Status_Board_Table.Bit.MOT_STATUS == 0) {                
            Old_Photocell_sts = LIGHT;
            SetStepperHomePosition(MOTOR_TABLE);
            Steps_Todo = STEP_PHOTO_TABLE_OFFSET; 
            // Table CW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);            
            Table.step ++;
        }                        
        // Table Photocell never in Light status in one full Table Rotation
        else if ((signed long)GetStepperPosition(MOTOR_TABLE) <= (-(signed long)(TintingAct.Steps_Revolution))) {
            HardHiZ_Stepper(MOTOR_TABLE);                                    
            Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
            return PROC_FAIL;                
        }
    break;
    
	// Starts a full Table Rotation to find Reference circuit    
    case STEP_2:
        if (Status_Board_Table.Bit.MOT_STATUS == 0) {        
            SetStepperHomePosition(MOTOR_TABLE);
            Steps_Todo = TintingAct.Steps_Revolution; 
            // Table CW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step ++;     
        }            
    break;
    
	//  Check if position required is reached
	case STEP_3:
        // Update Photocell Status
        if (PhotocellStatus(TABLE_PHOTOCELL, NO_FILTER) == DARK)
            New_Photocell_sts = DARK;
        else
            New_Photocell_sts = LIGHT;
        
        // Transition LIGHT-DARK ?
        if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) ) {
            Position_Steps = (signed long)GetStepperPosition(MOTOR_TABLE);            
            Count_Steps = (signed long)GetStepperPosition(MOTOR_TABLE);
            Tr_Light_Dark = ON;
        }
        // Transition DARK-LIGHT ?        
        else if ( (Old_Photocell_sts == DARK) && (New_Photocell_sts == LIGHT) ) {
            if (Tr_Light_Dark == ON)
                Tr_Dark_Light = ON;
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if ( (Tr_Light_Dark == ON) && (Tr_Dark_Light == ON) ) {
count_tr++;
            Tr_Light_Dark = OFF;
            Tr_Dark_Light = OFF;
            Find_Circuit  = ON;
            Find_circuit_n++;
            Count_Steps = -(signed long)((signed long)GetStepperPosition(MOTOR_TABLE) - (signed long)Count_Steps);
            if (Count_Steps > Max_Count_Steps) {
                Max_Count_Steps = Count_Steps;
                Reference_Position_Step = Position_Steps;
            }
        }
pippo = (signed long)GetStepperPosition(MOTOR_TABLE);        
        if ( ((signed long)GetStepperPosition(MOTOR_TABLE) <= -(signed long)Steps_Todo) && (Status_Board_Table.Bit.MOT_STATUS == 0) ) {
            if (Find_Circuit == OFF) {
                HardHiZ_Stepper(MOTOR_TABLE);                                        
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return PROC_FAIL;                
            }             
            else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);                                        
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return PROC_FAIL;                                
            }
            else
                // Reference Circuit Find
                Table.step ++;        
        }    
	break; 

	//  Check if Reference Circuit is correct
	case STEP_4:
        // Reference Circuit found is correct
        if ( ((Max_Count_Steps >= TintingAct.Steps_Reference) && ((Max_Count_Steps - TintingAct.Steps_Reference) <= TintingAct.Steps_Tolerance_Reference)) ||
             ((Max_Count_Steps < TintingAct.Steps_Reference)  && ((TintingAct.Steps_Reference - Max_Count_Steps) <= TintingAct.Steps_Tolerance_Reference)) )
            Table.step ++;        
        // Reference Circuit is NOT correct
        else {
            HardHiZ_Stepper(MOTOR_TABLE);                                        
            Table.errorCode = TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST;
            return PROC_FAIL;                
        }          
    break;

	// Starts Go Homing = Reference position
	case STEP_5:
        if ( (Reference_Position_Step -(signed long)GetStepperPosition(MOTOR_TABLE)) > TintingAct.Steps_Revolution/2) {
            // Shortest way is a CW rotation
            Steps_Todo = -Reference_Position_Step - STEP_PHOTO_TABLE_OFFSET;
    		Table.step ++ ;            
        }             
        else {
            // Shortest way is a CCW rotation
            Steps_Todo = (signed long)GetStepperPosition(MOTOR_TABLE) - Reference_Position_Step + STEP_PHOTO_TABLE_OFFSET;
    		Table.step +=4;
        }                             
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
	break;
// -----------------------------------------------------------------------------    
// CW Rotation
	// Go Homing = Reference in CW rotation
	case STEP_6:
//        if ( ((signed long)GetStepperPosition(MOTOR_TABLE) <= (-(signed long)(Steps_Todo +  TintingAct.Steps_Revolution))) && (Status_Board_Table.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Table.Bit.MOT_STATUS == 0) {
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);                                        
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return PROC_FAIL;                                
            }
            else  {
                // Rotate CW motor Table till Photocell transition DARK-LIGHT
                StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
                Table.step ++;
            }
        }            
	break;

    case STEP_7:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){  
            // TABLE Motor
            SetStepperHomePosition(MOTOR_TABLE);
            Steps_Todo = STEP_PHOTO_TABLE_REFERENCE_CW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;
        } 
	break;

    case STEP_8:
        if (Status_Board_Table.Bit.MOT_STATUS == 0) { 
            // Check if Photocell is covered
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return PROC_FAIL;            
            }            
            else { 
                // Table positioned at the centre of Reference Circuit
                SetStepperHomePosition(MOTOR_TABLE);
                if (Total_circuit_n != (Find_circuit_n-1)) {
                    HardHiZ_Stepper(MOTOR_TABLE);                                    
                    Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                    return PROC_FAIL;                                
                }    
                else
                    Table.step +=4;
            }                            
        }             
    break;
// -----------------------------------------------------------------------------
// CCW Rotation    
	// Go Homing = Reference in CCW rotation
	case STEP_9:
//        if ( (-(signed long)GetStepperPosition(MOTOR_TABLE) <= (signed long)(Steps_Todo + TintingAct.Steps_Revolution)) && (Status_Board_Table.Bit.MOT_STATUS == 0) ) {
        if (Status_Board_Table.Bit.MOT_STATUS == 0) { 
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);                        
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return PROC_FAIL;                                
            }
            else  {
                // Rotate CCW motor Table till Photocell transition DARK-LIGHT
                StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
                Table.step ++;
            }
        }             
	break;
    
    case STEP_10:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){         
            // TABLE Motor
            SetStepperHomePosition(MOTOR_TABLE);
            // Shortest way is a CCW rotation
            Steps_Todo = -STEP_PHOTO_TABLE_REFERENCE_CCW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;
        } 
	break;

    case STEP_11:
        if (Status_Board_Table.Bit.MOT_STATUS == 0) {   
            // Check if Photocell is covered
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return PROC_FAIL;            
            }                        
            else {
                // Table positioned at the centre of Reference Circuit
                SetStepperHomePosition(MOTOR_TABLE);
                if (Total_circuit_n != (Find_circuit_n-1)) {
                    HardHiZ_Stepper(MOTOR_TABLE);                                    
                    Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                    return PROC_FAIL;                                
                }    
                else
                    Table.step ++;
            }    
        }             
    break;    
// -----------------------------------------------------------------------------
    case STEP_12:
        if (Table_circuits_pos == ON) {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) 
                TintingAct.Circuit_step_pos[i] = CircStepPosAct.Circ_Pos[i];
        }
        TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0];                        
 //        HardHiZ_Stepper(MOTOR_TABLE);              
        StopStepper(MOTOR_TABLE);          
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Self Recognition process circuit assignement
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char TableSelfRecognitionColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm, i, j;
  static unsigned short circuit_id, circuit_id_ccw;
  static unsigned char Find_Circuit, Old_Photocell_sts, New_Photocell_sts;
  static signed long Steps_Todo;
  static signed long Circuit_step_temp[MAX_COLORANT_NUMBER];
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/   
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.step = STEP_18;
  }
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }  
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  } 
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);      
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
	case STEP_0:
		SetStepperHomePosition(MOTOR_TABLE);
        Find_Circuit  = OFF;
        circuit_id = 0;
        Status.errorCode = 0;

        if (GetStepperPosition(MOTOR_TABLE) != 0){
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
            return PROC_FAIL;
        }            
        else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
            Total_circuit_n = 0;
            Table_circuits_pos = OFF;
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
            return PROC_FAIL;
        }
        else {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                TintingAct.Circuit_step_pos[i] = 0; 
                TintingAct.Circuit_step_pos_cw[i] = 0;
                TintingAct.Circuit_step_pos_ccw[i] = 0;
                Circuit_step_temp[i] = 0;
            }    
            Table.step +=2;
            Old_Photocell_sts = DARK;
        }            
	break;

	// Starts a full CW Table Rotation   
    case STEP_2:
        Steps_Todo = TintingAct.Steps_Revolution - STEP_PHOTO_TABLE_OFFSET; 
        // Table CW rotation
//        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
        Table.step ++;          
    break;
    
    // Self Recognition in CW direction
	case STEP_3:
        // Update Photocell Status
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
            New_Photocell_sts = DARK;
        else
            New_Photocell_sts = LIGHT;
        
        // Transition LIGHT-DARK ?
        if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) ) {
            Find_Circuit = ON;
            Circuit_step_temp[circuit_id] = GetStepperPosition(MOTOR_TABLE);
            if (Circuit_step_temp[circuit_id] < 0) {
                Circuit_step_temp[circuit_id] = (-Circuit_step_temp[circuit_id]) % TintingAct.Steps_Revolution; 
                if (Circuit_step_temp[circuit_id] != 0)
                    Circuit_step_temp[circuit_id] = TintingAct.Steps_Revolution - (Circuit_step_temp[circuit_id] + STEP_PHOTO_TABLE_CIRCUIT_CW);
            }   
            else
                Circuit_step_temp[circuit_id] = Circuit_step_temp[circuit_id] % TintingAct.Steps_Revolution;                        
            circuit_id++;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            if (Find_Circuit == OFF) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;                
            }
            else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;
            }
            else if ((circuit_id) > MAX_COLORANT_NUMBER) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;
            }   
            else { 
                for (i = 0; i < circuit_id; i++)
                    TintingAct.Circuit_step_pos_cw[i] = Circuit_step_temp[circuit_id - 1 - i]; 
                
                Total_circuit_n = circuit_id;
                Table_circuits_pos = ON;
                // Self Learning Procedure End in CW direction
                Table.step ++;
            }                   
        }            
 	break;

	// Re-alignement in Reference position
    case STEP_4:
        // Rotate CW motor Table till Photocell transition DARK-LIGHT
        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
        Table.step ++;        
 	break;

	// Check for DARK-LIGHT transition
    case STEP_5:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                            
            SetStepperHomePosition(MOTOR_TABLE);
            // Go to center position
            Steps_Todo = STEP_PHOTO_TABLE_CIRCUIT_CW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;            
        }    
 	break;
    
    // Table centered on Reference Circuit
    case STEP_6:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
    		// Table positioned at the centre of Reference Circuit
            SetStepperHomePosition(MOTOR_TABLE);
            Table.step ++;
        }             
    break;  
    
	// Starts a full CCW Table Rotation   
    case STEP_7:
        Old_Photocell_sts = DARK;
        circuit_id_ccw = 0;
        Steps_Todo = -TintingAct.Steps_Revolution + STEP_PHOTO_TABLE_OFFSET; 
        // Table CCW rotation
//        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
        Table.step ++;          
    break;

    // Self Recognition in CCW direction
	case STEP_8:
        // Update Photocell Status
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
            New_Photocell_sts = DARK;
        else
            New_Photocell_sts = LIGHT;
        
        // Transition LIGHT-DARK ?
        if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) ) {
            TintingAct.Circuit_step_pos_ccw[circuit_id_ccw] = GetStepperPosition(MOTOR_TABLE) + STEP_PHOTO_TABLE_CIRCUIT_CCW;
            circuit_id_ccw++;
            circuit_id--;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;
            }
            else if (circuit_id > 0) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;
            }   
            else  
                // Self Learning Procedure End in CCW direction
                Table.step ++;        
        }            
 	break;

    // Check if 'Circuit_step_pos[]' steps match between CW and CCW directions
	case STEP_9:
        for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
            // Difference in Circuit position Steps between CW and CCW > tolerance --> Error
            if ( ((TintingAct.Circuit_step_pos_cw[i] > TintingAct.Circuit_step_pos_ccw[i]) && ((TintingAct.Circuit_step_pos_cw[i] - TintingAct.Circuit_step_pos_ccw[i]) >= TintingAct.Steps_Tolerance_Circuit ) ) ||
                 ((TintingAct.Circuit_step_pos_cw[i] <= TintingAct.Circuit_step_pos_ccw[i]) && ((TintingAct.Circuit_step_pos_ccw[i] - TintingAct.Circuit_step_pos_cw[i]) >= TintingAct.Steps_Tolerance_Circuit ) ) ) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                Status.errorCode = i; 
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                return PROC_FAIL;                    
            }
        }
        for (i = 0; i < MAX_COLORANT_NUMBER; i++)
            TintingAct.Circuit_step_pos[i] = (TintingAct.Circuit_step_pos_cw[i] + TintingAct.Circuit_step_pos_ccw[i]) / 2; 
        Table.step ++;                
    break;

	// Check if 'Circuit_step_pos[i]' steps matches 'Circuit_step_theorical_pos[j]
    case STEP_10:
        for (i = 0; i < Total_circuit_n; i++) {
            Find_Circuit = OFF;
            for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                if ( ((TintingAct.Circuit_step_theorical_pos[j] > TintingAct.Circuit_step_pos[i]) && ((TintingAct.Circuit_step_theorical_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit) ) ||
                     ((TintingAct.Circuit_step_theorical_pos[j] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - TintingAct.Circuit_step_theorical_pos[j]) <= TintingAct.Steps_Tolerance_Circuit) ) ) {                      
                    Find_Circuit = TRUE;
                    break;
                }
            }
            // Circuit 'i' position TintingAct.Circuit_step_pos[i] is NOT correct
            if (Find_Circuit == FALSE)
                break;
        }
        if (Find_Circuit == FALSE) {
            Total_circuit_n = 0;
            Table_circuits_pos = OFF;
            Status.errorCode = i; 
            for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                TintingAct.Circuit_step_pos[i] = 0; 
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
            return PROC_FAIL;                    
        }    
        Table.step ++;        
 	break;
    
    // Create the final Circuit position map
    case STEP_11:
        for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
            Find_Circuit = FALSE;
            for (i = 0; i < Total_circuit_n; i++) {
                if ( ((TintingAct.Circuit_step_theorical_pos[j] > TintingAct.Circuit_step_pos[i]) && ((TintingAct.Circuit_step_theorical_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit) ) ||
                     ((TintingAct.Circuit_step_theorical_pos[j] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - TintingAct.Circuit_step_theorical_pos[j]) <= TintingAct.Steps_Tolerance_Circuit) ) ) {                      
                    Circuit_step_temp[j] = TintingAct.Circuit_step_pos[i];
                    Find_Circuit = TRUE;
                    break;                        
                }
            }
            // Circuit 'j' is not present on the Table
            if (Find_Circuit == FALSE)
                Circuit_step_temp[j] = 0;
        }
        // DYNAMIC circuit position model: tabel without zeros
        if(TintingAct.Table_Step_position == DYNAMIC)
            Table.step ++;        
        // STATIC circuit position model: tabel with possible zeros
        else {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                TintingAct.Circuit_step_pos[i] = Circuit_step_temp[i];
            }
            Table.step ++;        
        }                
 	break;

    // Check if there is a full match between positiong Table found and enabled circuit table 
    case STEP_12:
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if (TintingAct.Table_Colorant_En[i] == TRUE) {
                    if (Circuit_step_temp[i] == 0) {
                        for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                            TintingAct.Circuit_step_pos[i] = 0;    
                        HardHiZ_Stepper(MOTOR_TABLE);        
                        Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        Status.errorCode = i; 
                        return PROC_FAIL;                                            
                    }
                }
                else if (TintingAct.Table_Colorant_En[i] == FALSE) {
                    if (Circuit_step_temp[i] != 0) {
                        for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                            TintingAct.Circuit_step_pos[i] = 0;    
                        HardHiZ_Stepper(MOTOR_TABLE);        
                        Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        Status.errorCode = i; 
                        return PROC_FAIL;                                            
                    }
                }
            }
            Table.step ++;        
 	break;
        
	// Re-alignement in Reference position
    case STEP_13:
        // Rotate CCW motor Table till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
        Table.step ++;        
 	break;

	// Check for DARK-LIGHT transition
    case STEP_14:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            HardHiZ_Stepper(MOTOR_TABLE);        
            SetStepperHomePosition(MOTOR_TABLE);
            // Go to center position
            Steps_Todo = -STEP_PHOTO_TABLE_REFERENCE_CCW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;            
        }    
 	break;
    
    // Table centered on Reference Circuit
    case STEP_15:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
    		// Table positioned at the centre of Reference Circuit 
            SetStepperHomePosition(MOTOR_TABLE);
            Table.step ++;
        }             
    break;      
// -----------------------------------------------------------------------------
// Write Circuits Steps postion CW and CCW on EEprom memory
    case STEP_16:
        // Setup EEPROM writing variables 
        eeprom_byte = 0;
        for (i = 0; i < MAX_COLORANT_NUMBER; i++)                              
            CircStepPosAct.Circ_Pos[i] = TintingAct.Circuit_step_pos[i]; 
        
        Table.step ++;
//Table.step +=2 ;
    break;

    case STEP_17:
        HardHiZ_Stepper(MOTOR_TABLE);            
        eeprom_write_result = updateEECirStepsPos();
        if (eeprom_write_result == EEPROM_WRITE_DONE) {
            updateEEParamCirStepsPosCRC();
            if (checkEEprom() ) 
                EEprom_Crc_Error = 0;
            else
                EEprom_Crc_Error = 1;                
            Table.step ++;  
        }
        else if (eeprom_write_result == EEPROM_WRITE_FAILED) {
            Table.errorCode = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
            return PROC_FAIL;
        }    
    break;
// -----------------------------------------------------------------------------    
    case STEP_18:
        HardHiZ_Stepper(MOTOR_TABLE);            
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Table Positioning process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char TableGoToReference(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static unsigned short direction;
  static signed long Steps_Todo, Steps_Position;
  unsigned char currentReg, i;
  static signed long Steps_Pos;
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/  
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.step = STEP_5;
  }
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  } 
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Steps_Pos = GetStepperPosition(MOTOR_TABLE);
  if (Steps_Pos >= (signed long)TintingAct.Steps_Revolution) {
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }
    else
        SetStepperHomePosition(MOTOR_TABLE);
  }      
  else if (Steps_Pos <= -(signed long)TintingAct.Steps_Revolution) {   
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }   
    else 
        SetStepperHomePosition(MOTOR_TABLE);
  }
  
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);         
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
	// Starts operations
    case STEP_0:
        Status.errorCode = 0;
        // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
//        ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                          
        currentReg = HOLDING_CURRENT_TABLE * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        // Read current position
        Steps_Position = GetStepperPosition(MOTOR_TABLE);
        if (Steps_Position < 0) {
            Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
            if (Steps_Position != 0)
                Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
        }   
        else
            Steps_Position = Steps_Position % TintingAct.Steps_Revolution;
        
        Table.step ++;
	break;

    // Start Table Rotation towards Reference
    case STEP_1:
        if((Steps_Position) < (signed long)(TintingAct.Steps_Revolution / 2)) {
            direction = CW;
            Steps_Todo = Steps_Position - STEP_PHOTO_TABLE_OFFSET; 
            // 'Steps_Todo' has to be > 0            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step ++;                                         
        }        
        else if ((Steps_Position) >= (signed long)(TintingAct.Steps_Revolution / 2)) {
            direction = CCW;
            Steps_Todo = -(signed long)(TintingAct.Steps_Revolution - (Steps_Position) - STEP_PHOTO_TABLE_OFFSET); 
            // 'Steps_Todo' has to be < 0
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step ++;                                         
        }        
        else {
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
            return PROC_FAIL;
        }
	break;

    case STEP_2:
        if ( (direction == CW) && (Status_Board_Table.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;
            }
            // Rotate CW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
            Table.step ++;                                              
        }
        else if ( (direction == CCW) && (Status_Board_Table.Bit.MOT_STATUS == 0) ) {
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;
            }
            // Rotate CCW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
            Table.step ++;                                  
        }
	break;

	// Check for LIGHT-DARK transition
    case STEP_3:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_TABLE);            
            if (direction == CW)
                // Go to center 'Reference' position CW
                Steps_Todo = STEP_PHOTO_TABLE_REFERENCE_CW;                        
            else
                // Go to center 'Reference' position CCW
                Steps_Todo = -STEP_PHOTO_TABLE_REFERENCE_CCW;                                    

            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++; 
        }    
    break;        

	// Waiting to be placed at the center of 'Reference' position 
    case STEP_4:
        if (Status_Board_Table.Bit.MOT_STATUS == 0) {
            // Check if Photocell is covered
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;            
            }
            else {
                SetStepperHomePosition(MOTOR_TABLE);                
                Table.step ++;                 
            } 
        }                   
    break;        
        
    case STEP_5:
        if (Table_circuits_pos == ON) {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) 
                TintingAct.Circuit_step_pos[i] = CircStepPosAct.Circ_Pos[i];
        }
        TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0];                                
//        HardHiZ_Stepper(MOTOR_TABLE);              
        StopStepper(MOTOR_TABLE);          
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Stirring process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char TableStirring(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  unsigned char currentReg;
  signed long Steps_Pos;
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/  
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.step = STEP_3;
  }
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  } 
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }

  Steps_Pos = GetStepperPosition(MOTOR_TABLE);
  if (Steps_Pos >= (signed long)TintingAct.Steps_Revolution) {
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }
    else
        SetStepperHomePosition(MOTOR_TABLE);
  }      
  else if (Steps_Pos <= -(signed long)TintingAct.Steps_Revolution) {   
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }   
    else 
        SetStepperHomePosition(MOTOR_TABLE);
  }
  
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);         
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
    case STEP_0:
        Status.errorCode = 0;
        // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
//        ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                          
        currentReg = HOLDING_CURRENT_TABLE * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        Run_Stepper(MOTOR_TABLE, TintingAct.High_Speed_Rotating_Table, CW);
        Table.step ++;
    break;
    
    case STEP_1:
        // Stop Stirring
        if (isColorCmdStop() ) {
//            MoveStepperToHome(MOTOR_TABLE, TintingAct.High_Speed_Rotating_Table);
//            Table.step ++;       
            Table.step = STEP_3;                   
        }            
    break;      

    case STEP_2:
        if (Status_Board_Table.Bit.MOT_STATUS == 0)   
            Table.step ++;          
    break;      
    
    case STEP_3:
        HardHiZ_Stepper(MOTOR_TABLE);            
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Check and Set table Home Position
**
**      @param void
**
**      @retval TRUE: Good Home Position - FALSE: Bad Home Position
**
*//*=====================================================================*//**
*/
unsigned char ManageTableHomePosition(void)
{
    signed long Steps_Pos;
    static unsigned char Set_Home_pos = 0;
    static unsigned char Old_Photocell_sts = DARK, New_Photocell_sts = DARK;
    static unsigned char Tr_Light_Dark = OFF;
    
    // Update Photocell Status
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
        New_Photocell_sts = DARK;
    else
        New_Photocell_sts = LIGHT;    
    // Transition LIGHT-DARK ?
    if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) )
        Tr_Light_Dark = ON;
    // Transition DARK-LIGHT ?        
    else if ( (Old_Photocell_sts == DARK) && (New_Photocell_sts == LIGHT) )
        Tr_Light_Dark = OFF;
    Old_Photocell_sts = New_Photocell_sts;

    Steps_Pos = GetStepperPosition(MOTOR_TABLE);
    //--------------------------------------------------------------------------
    if ( (Set_Home_pos == 0) && (Steps_Pos >= (signed long)(TintingAct.Steps_Revolution - STEP_PHOTO_TABLE_OFFSET)) ) {
        Set_Home_pos = 0;
        if (Tr_Light_Dark == ON)  {
            Set_Home_pos = 1;
            SetStepperHomePosition(MOTOR_TABLE);
        }
        return TRUE;                
    }
    else if ( (Set_Home_pos == 0) && (Steps_Pos >= (signed long)(TintingAct.Steps_Revolution) ) ) 
        return FALSE; 
    
    else if ( (Set_Home_pos == 1) && (Steps_Pos >= STEP_PHOTO_TABLE_REFERENCE_CW) ) {
        Set_Home_pos = 0;
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
            return FALSE;
        else {
            SetStepperHomePosition(MOTOR_TABLE);            
            return TRUE; 
        }        
    }
    //--------------------------------------------------------------------------
    else if ( (Set_Home_pos == 0) && (Steps_Pos <= (-(signed long)(TintingAct.Steps_Revolution) + STEP_PHOTO_TABLE_OFFSET)) ) {
        Set_Home_pos = 0;
        if (Tr_Light_Dark == ON)  {
            Set_Home_pos = 1;
            SetStepperHomePosition(MOTOR_TABLE);
        }
        return TRUE;                
    }
    else if ( (Set_Home_pos == 0) && (Steps_Pos <= -(signed long)(TintingAct.Steps_Revolution) ) ) 
        return FALSE; 
    
    else if ( (Set_Home_pos == 1) && (Steps_Pos <= -(signed long)(STEP_PHOTO_TABLE_REFERENCE_CW)) ) {
        Set_Home_pos = 0;
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
            return FALSE;
        else {
            SetStepperHomePosition(MOTOR_TABLE);            
            return TRUE;
         }     
    }
    //--------------------------------------------------------------------------
    else
        return FALSE;
}

/*
*//*=====================================================================*//**
**      @brief Table Positioning process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char TablePositioningColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm, i;
  static unsigned short direction, count_circuits, theorical_circuits;
  static signed long Steps_Todo, Steps_Position, Count_Steps, Max_Count_Steps;
  static unsigned char Old_Photocell_sts, New_Photocell_sts;
  static unsigned char Tr_Dark_Light, Tr_Light_Dark;
  static unsigned char Circ_Indx;
  unsigned char currentReg; 
  signed long Steps_Pos;
   
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/  
  // Table Panel OPEN
  if ( (PositioningCmd != 1) && (TintingAct.PanelTable_state == OPEN) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.step = STEP_8;
  }
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  } 
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }

/*    
  if (ManageTableHomePosition() == FALSE) {
    // Loss of Steps  
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                        
  }      
*/  
  Steps_Pos = GetStepperPosition(MOTOR_TABLE);
  if (Steps_Pos >= (signed long)TintingAct.Steps_Revolution) {
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }
    else
        SetStepperHomePosition(MOTOR_TABLE);
  }      
  else if (Steps_Pos <= -(signed long)TintingAct.Steps_Revolution) {   
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }   
    else 
        SetStepperHomePosition(MOTOR_TABLE);
  }
    
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);         
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
    case STEP_0:
        Status.errorCode = 0;
        count_circuits = 0;
        // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
//        ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                          
        currentReg = HOLDING_CURRENT_TABLE * 100 /156;
        cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
        cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        // EEprom CRC Error
        if (EEprom_Crc_Error == 1) {
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
            return PROC_FAIL;                    
        }
        // Table is already positioned on the Circuit "Color_Id"
        if ( (TintingAct.Circuit_Engaged == TintingAct.Color_Id) && (TintingAct.Refilling_Angle == 0) )
    		ret = PROC_OK;
            
        // Analyze Command parameters:
        if ( (TintingAct.Color_Id > MAX_COLORANT_NUMBER) || (TintingAct.Color_Id == 0) || (TintingAct.Refilling_Angle > MAX_ROTATING_ANGLE) ||
             ( (TintingAct.Direction != CW) && (TintingAct.Direction != CCW) ) ||
               (TintingAct.Table_Colorant_En[TintingAct.Color_Id - 1]) == 0) {    
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            return PROC_FAIL;            
        }   

        // No Circuit Position steps available --> Process fail
        if (Table_circuits_pos == OFF) {
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
            return PROC_FAIL;            
        }
        
        Circ_Indx = 0;        
        for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
            if ( (i < (TintingAct.Color_Id - 1)) && (TintingAct.Table_Colorant_En[i] == 1) )
                Circ_Indx++;
        }   

        // Read current position
        Steps_Position = GetStepperPosition(MOTOR_TABLE);
        if (Steps_Position < 0) {
            Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
            if (Steps_Position != 0)
                Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
        }   
        else
            Steps_Position = Steps_Position % TintingAct.Steps_Revolution;
        
        // If a Ricirculation Command has arrived it is necessary before to make Stirring: a full 360° CW Table rotation
        if (RicirculationCmd == 1) {
            Steps_Todo = TintingAct.Steps_Revolution; 
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);            
            Table.step ++;                    
        }    
        else    
            Table.step +=3;
	break;

      case STEP_1:
        if (isColorCmdStop() ) {
//            Table.step = STEP_8;
            HardHiZ_Stepper(MOTOR_TABLE); 
            RicirculationCmd = 0; 
            Table.step = STEP_3;          
        }
        // Another full 360° CCW Table rotation
        else if (Status_Board_Table.Bit.MOT_STATUS == 0) {   
            Steps_Todo = -TintingAct.Steps_Revolution; 
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);            
            Table.step ++;                                
        }                
    break; 
    
    case STEP_2:
        if (isColorCmdStop() ) {
//            Table.step = STEP_8;
            HardHiZ_Stepper(MOTOR_TABLE);
            RicirculationCmd = 0; 
            Table.step = STEP_3;                      
        }
        else if (Status_Board_Table.Bit.MOT_STATUS == 0) {   
            RicirculationCmd = 0;            
            Table.step ++; 
        }              
	break;
    
    // Start Table Rotation towards circuit selected in the shortest way
    case STEP_3:
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
            Old_Photocell_sts = DARK;
        else
            Old_Photocell_sts = LIGHT;
        Tr_Light_Dark = OFF;
        Tr_Dark_Light = OFF;
        Max_Count_Steps = 0;
    
        // Read current position
        Steps_Position = GetStepperPosition(MOTOR_TABLE);
        if (Steps_Position < 0) {
            Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;            
            if (Steps_Position != 0)
                Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
        }   
        else
            Steps_Position = Steps_Position % TintingAct.Steps_Revolution;        
        // Steps_position: Current Position
        // TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1]: Position where to go
        // Steps_Todo > 0  --> CW
        // Steps_Todo <= 0 --> CCW
        // Case 1
        if ( ((signed long)(TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) >= 0) && 
             ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) < (TintingAct.Steps_Revolution / 2)) ) {
            direction = CCW;
            Steps_Todo = -(signed long)(TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position - STEP_PHOTO_TABLE_OFFSET); 
            // 'Steps_Todo' has to be < 0
            if (Steps_Todo >= 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) 
                    // Rotate CCW motor Table till Photocell transition LIGHT-DARK
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
                else
                    // Rotate CW motor Table till Photocell transition DARK-LIGHT
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, DARK_LIGHT, TABLE_PHOTOCELL, 0);                                         
                Table.step += 2;                                         
            }                        
            else {
                // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                theorical_circuits = 0;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    if ( (TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] >= Steps_Position) && (TintingAct.Circuit_step_pos[i] <= TintingAct.Circuit_step_pos[Circ_Indx]) )
                        theorical_circuits++;
                    else if ( (TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] < Steps_Position) && ((Steps_Position - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit) )
                        theorical_circuits++;
                }
                if ( (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Tolerance_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Tolerance_Reference)))
                    theorical_circuits--;

                // Not troughout Reference                
                if ( (signed long)((TintingAct.Steps_Threshold + Steps_Todo) <= 0) )
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                else
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);                
                Table.step ++;                                         
            }                                        
        }        
        // Case 2        
        else if( ((signed long)(TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) >= 0) && 
                 ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) >= (TintingAct.Steps_Revolution / 2)) )  {
            direction = CW;
            Steps_Todo = TintingAct.Steps_Revolution - (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) - STEP_PHOTO_TABLE_OFFSET; 
            // 'Steps_Todo' has to be > 0            
            if (Steps_Todo <= 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
                    // Rotate CW motor Table till Photocell transition LIGHT-DARK
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                                        
                else 
                    // Rotate CCW motor Table till Photocell transition DARK-LIGHT
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, DARK_LIGHT, TABLE_PHOTOCELL, 0);                                                             
                Table.step += 2;                                                         
            }
            else {
                // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                theorical_circuits = 0;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    if ((TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] >= TintingAct.Circuit_step_pos[Circ_Indx]) && (TintingAct.Circuit_step_pos[i] <= TintingAct.Steps_Revolution) )
                        theorical_circuits++;
                    else if ( (TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] <= Steps_Position) )
                        theorical_circuits++;
                    else if ( (TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] > Steps_Position) && ((TintingAct.Circuit_step_pos[i] - Steps_Position)<= TintingAct.Steps_Tolerance_Circuit) )
                        theorical_circuits++;                    
                }
                if ( (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Tolerance_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Tolerance_Reference)))
                    theorical_circuits--;
                
                if (Steps_Todo > TintingAct.Steps_Threshold)
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                else
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step ++;                                         
            }                                                    
        }        
        // Case 3        
        else if( ((signed long)(TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) < 0) && 
                 ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) > -(signed long)(TintingAct.Steps_Revolution / 2)) )  {
            direction = CW;
            Steps_Todo = Steps_Position - TintingAct.Circuit_step_pos[Circ_Indx] - STEP_PHOTO_TABLE_OFFSET; 
            // 'Steps_Todo' has to be > 0            
            if (Steps_Todo <= 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
                    // Rotate CW motor Table till Photocell transition LIGHT-DARK
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                                        
                else 
                    // Rotate CCW motor Table till Photocell transition DARK-LIGHT
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, DARK_LIGHT, TABLE_PHOTOCELL, 0);                                                             
                Table.step += 2;                                                         
            }
            else {
                // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                theorical_circuits = 0;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    if ((TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] >= TintingAct.Circuit_step_pos[Circ_Indx]) && (TintingAct.Circuit_step_pos[i] <= Steps_Position))
                        theorical_circuits++;
                    else if ( (TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] > Steps_Position) && ((TintingAct.Circuit_step_pos[i] - Steps_Position)<= TintingAct.Steps_Tolerance_Circuit) )
                        theorical_circuits++;                    
                }
                if ( (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Tolerance_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Tolerance_Reference)))
                    theorical_circuits--;
                
                // Not Troughout Reference
                if (Steps_Todo > TintingAct.Steps_Threshold)
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                else
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);                
                Table.step ++;                                         
            }                                                    
        }        
        // Case 4        
        else if( ((signed long)(TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) < 0) && 
                 ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) <= -(signed long)(TintingAct.Steps_Revolution / 2)) )  {
            direction = CCW;
            Steps_Todo = -(signed long)(TintingAct.Steps_Revolution - (Steps_Position - TintingAct.Circuit_step_pos[Circ_Indx]) - STEP_PHOTO_TABLE_OFFSET); 
            // 'Steps_Todo' has to be < 0
            if (Steps_Todo >= 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) 
                    // Rotate CCW motor Table till Photocell transition LIGHT-DARK
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
                else
                    // Rotate CW motor Table till Photocell transition DARK-LIGHT
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, DARK_LIGHT, TABLE_PHOTOCELL, 0); 
                Table.step += 2;                                         
            }                
            else {
                // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                theorical_circuits = 0;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    if ((TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] <= TintingAct.Circuit_step_pos[Circ_Indx]) )
                        theorical_circuits++;
                    else if ( (TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] >= Steps_Position) && (TintingAct.Circuit_step_pos[i] <= TintingAct.Steps_Revolution) )
                        theorical_circuits++;
                    else if ( (TintingAct.Circuit_step_pos[i] != 0) && (TintingAct.Circuit_step_pos[i] < Steps_Position) && ((Steps_Position - TintingAct.Circuit_step_pos[i])<= TintingAct.Steps_Tolerance_Circuit) )
                        theorical_circuits++;                    
                }
                if ( (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Tolerance_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Tolerance_Reference)))
                    theorical_circuits--;
                
                if ( (signed long)((TintingAct.Steps_Threshold + Steps_Todo) <= 0) )
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                else
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step ++;                                         
            }                                        
        }        
        else {
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
            return PROC_FAIL;
        }
    break;        

	// Waiting for be placed near 'TintingAct.Color_Id' position
    case STEP_4:
        // Update Photocell Status
        if (PhotocellStatus(TABLE_PHOTOCELL, NO_FILTER) == DARK)
            New_Photocell_sts = DARK;
        else
            New_Photocell_sts = LIGHT;
        
        // Transition LIGHT-DARK ?
        if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) ) {
            Count_Steps = (signed long)GetStepperPosition(MOTOR_TABLE);
            Tr_Light_Dark = ON;
            count_circuits++;
        }
        // Transition DARK-LIGHT ?        
        else if ( (Old_Photocell_sts == DARK) && (New_Photocell_sts == LIGHT) ) {
            if (Tr_Light_Dark == ON)
                Tr_Dark_Light = ON;
        }        
        Old_Photocell_sts = New_Photocell_sts;
        if ( (Tr_Light_Dark == ON) && (Tr_Dark_Light == ON) ) {
            Tr_Light_Dark = OFF;
            Tr_Dark_Light = OFF;
            if (direction == CCW) {
                if ((signed long)GetStepperPosition(MOTOR_TABLE) >= (signed long)Count_Steps)
                   Count_Steps = (signed long)((signed long)GetStepperPosition(MOTOR_TABLE) - (signed long)Count_Steps);
                else
                   Count_Steps = (signed long)((signed long)GetStepperPosition(MOTOR_TABLE) + (signed long)TintingAct.Steps_Revolution -(signed long)Count_Steps);                    
            }
            else {
                if ((signed long)GetStepperPosition(MOTOR_TABLE) <= (signed long)Count_Steps)
                    Count_Steps = -(signed long)((signed long)GetStepperPosition(MOTOR_TABLE) - (signed long)Count_Steps);
                else 
                   Count_Steps = (signed long)((signed long)TintingAct.Steps_Revolution +(signed long)Count_Steps - (signed long)GetStepperPosition(MOTOR_TABLE));                                        
            }    
            if (Count_Steps > Max_Count_Steps) {
                Max_Count_Steps = Count_Steps;
            }
        }
        
        if ( (direction == CW) && (Status_Board_Table.Bit.MOT_STATUS == 0) ) {
            // Check if Reference was found
            if ( ((Max_Count_Steps >= TintingAct.Steps_Reference) && ((Max_Count_Steps - TintingAct.Steps_Reference) <= TintingAct.Steps_Tolerance_Reference)) ||
                 ((Max_Count_Steps < TintingAct.Steps_Reference)  && ((TintingAct.Steps_Reference - Max_Count_Steps) <= TintingAct.Steps_Tolerance_Reference)) )
                count_circuits--;
            
            // Probably: loss of Table steps
            if (count_circuits != (theorical_circuits-1)) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;            
            }
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;
            }
            // Rotate CW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
            Table.step ++;                                              
        }
        else if ( (direction == CCW) && (Status_Board_Table.Bit.MOT_STATUS == 0) ) {
            // Check if Reference was found
            if ( ((Max_Count_Steps >= TintingAct.Steps_Reference) && ((Max_Count_Steps - TintingAct.Steps_Reference) <= TintingAct.Steps_Tolerance_Reference)) ||
                 ((Max_Count_Steps < TintingAct.Steps_Reference)  && ((TintingAct.Steps_Reference - Max_Count_Steps) <= TintingAct.Steps_Tolerance_Reference)) )
                count_circuits--;

            // Probably: loss of Table steps
            if (count_circuits != (theorical_circuits-1)) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;            
            }
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;
            }           
            // Rotate CCW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
            Table.step ++;                                  
        }
    break;        

	// Check for LIGHT-DARK transition
    case STEP_5:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            if (direction == CW)
                // Go to center 'TintingAct.Color_Id' position CW
                Steps_Todo = STEP_PHOTO_TABLE_CIRCUIT_CW;                        
            else
                // Go to center 'TintingAct.Color_Id' position CCW
                Steps_Todo = -STEP_PHOTO_TABLE_CIRCUIT_CCW;                                    
            Steps_Position = GetStepperPosition(MOTOR_TABLE);
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);  
            Table.step ++; 
        }    
    break;        

	// Waiting to be placed at the center of 'TintingAct.Color_Id' position 
    case STEP_6:
        if (Status_Board_Table.Bit.MOT_STATUS == 0) {
            // Check if Photocell is covered
            if (PhotocellStatus(TABLE_PHOTOCELL, NO_FILTER) == LIGHT) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;            
            }    
            if (TintingAct.Refilling_Angle > 0)  {
                // Find Steps corresponding to 'Refilling_Angle'
                Steps_Position = GetStepperPosition(MOTOR_TABLE);
                if (TintingAct.Direction == CW)
                    Steps_Todo = (long)((float)TintingAct.Steps_Revolution * ((float)TintingAct.Refilling_Angle / (float)(2 * MAX_ROTATING_ANGLE)));
                else
                    Steps_Todo = -(long)((float)TintingAct.Steps_Revolution * ((float)TintingAct.Refilling_Angle / (float)(2 * MAX_ROTATING_ANGLE)));
                
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                 Table.step ++; 
            }
            else  {
                // Check if position reached is correct
                // Read current position
/*                
                Steps_Position = GetStepperPosition(MOTOR_TABLE);
                if (Steps_Position < 0)
                    Steps_Position = TintingAct.Steps_Revolution + Steps_Position;
                if (Steps_Position >= TintingAct.Circuit_step_pos[Circ_Indx]) {
                    if ( (Steps_Position - TintingAct.Circuit_step_pos[Circ_Indx]) > TintingAct.Steps_Tolerance_Circuit) {
                        HardHiZ_Stepper(MOTOR_TABLE);        
                        Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                        return PROC_FAIL;            
                    }    
                }                
                else {
                    if ( (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) > TintingAct.Steps_Tolerance_Circuit) {
                        HardHiZ_Stepper(MOTOR_TABLE);        
                        Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                        return PROC_FAIL;            
                    }                    
                }
*/
                // Update 'Circuit_step_pos' in order to take into account possible steps loss
                if (TintingAct.Circuit_step_pos[Circ_Indx] >= Steps_Position) {
                    for (i = 0; i < MAX_COLORANT_NUMBER; i++) 
                        TintingAct.Circuit_step_pos[i] = TintingAct.Circuit_step_pos[i] - (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position);
                }
                else  {
                    for (i = 0; i < MAX_COLORANT_NUMBER; i++) 
                        TintingAct.Circuit_step_pos[i] = TintingAct.Circuit_step_pos[i] + (Steps_Position - TintingAct.Circuit_step_pos[Circ_Indx]);                    
                }                     
                TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0];                                
                
                Table.step +=2;                
            }                
        }
    break;

	// Waiting to reach the requested final position
    case STEP_7:
        if (Status_Board_Table.Bit.MOT_STATUS == 0)
            Table.step ++;       
    break;
        
    case STEP_8:
        // If PANEl is OPEN Config TABLE Motor with the Maximum Retention Torque (= Maximum Holding Current)
        if (TintingAct.PanelTable_state == OPEN) {
//            ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE_FINAL, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                
            currentReg = HOLDING_CURRENT_TABLE_FINAL * 100 /156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;          
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
        }
        StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
        StartTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
    
//        HardHiZ_Stepper(MOTOR_TABLE);  
        StopStepper(MOTOR_TABLE);
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Table Cleaning process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char TableCleaningColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static unsigned short direction;
  static signed long Steps_Todo, Steps_done;

  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  } 
*/  
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.step = STEP_4;
  }
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  } 
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);    
  // Check for BRUSH ERRORS
#ifndef SKIP_FAULT_1
  if (getFault_1Error() == FAULT_1_ERROR) {
       HardHiZ_Stepper(MOTOR_TABLE);        
       Table.errorCode = TINTING_BRUSH_OPEN_LOAD_ERROR_ST;
       return PROC_FAIL;
  }
  else if ( (!IS_IN1_BRUSH_OFF() || !IS_IN2_BRUSH_OFF()) && (isFault_1_Detection()) ) {
       HardHiZ_Stepper(MOTOR_TABLE);        
       Table.errorCode = TINTING_BRUSH_OVERCURRENT_THERMAL_ERROR_ST;
       return PROC_FAIL;  
  }  
#endif
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
    case STEP_0:
        Status.errorCode = 0;
        if (Clean_Activation == ON) {
            // Analyze Command parameters:
            if (TintingAct.Color_Id > MAX_COLORANT_NUMBER) {    
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
                return PROC_FAIL;            
            }   
            if ( (TintingAct.Table_Colorant_En[TintingAct.Color_Id] - 1) == 0) {    
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
                return PROC_FAIL;            
            }
            // EEprom CRC Error
            if (EEprom_Crc_Error == 1) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
                return PROC_FAIL;                    
            }
            // No Circuit Position steps available --> Process fail
            if (Table_circuits_pos == OFF) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                return PROC_FAIL;            
            }
            else if (TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1] == 0) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                return PROC_FAIL;            
            }               
            Table.step ++;
        }   
        else 
            Table.step +=3;
	break;

    // Start Table Rotation 
    case STEP_1:
        if (TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1] > TintingAct.Steps_Cleaning)
            direction = CCW;
        else
            direction = CW;
        
        // Rotate 'Color_Id' into Cleaning position
        Steps_Todo = TintingAct.Steps_Cleaning - TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1];                        
            
        Steps_done = GetStepperPosition(MOTOR_TABLE);
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
        Table.step ++;                 
    break; 

	// Waiting 'TintingAct.Color_Id' position is on Cleaning position     
    case STEP_2:
        if ( (direction == CW) && (GetStepperPosition(MOTOR_TABLE) >= (Steps_Todo + Steps_done) ) )
            Table.step ++;                  
        else if ( (direction == CCW) && (GetStepperPosition(MOTOR_TABLE) <= (Steps_Todo + Steps_done) ) )
            Table.step ++;                  
    break; 

    // Brush activation
    case STEP_3:
        if (Clean_Activation == ON)
            BRUSH_ON();
        else
            BRUSH_OFF();
        
        Table.step ++;                  
    break; 
      
    case STEP_4:
        HardHiZ_Stepper(MOTOR_TABLE);        
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Table Test process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/
unsigned char TableTestColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm, i, j;
  static unsigned short circuit_id, circuit_id_ccw;
  static unsigned char Find_Circuit, Old_Photocell_sts, New_Photocell_sts;
  static signed long Steps_Todo;
  static signed long Circuit_step_temp[MAX_COLORANT_NUMBER], Circuit_step_temp_1[MAX_COLORANT_NUMBER];
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }
*/ 
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.step = STEP_9;
  }
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  } 
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }  
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);    
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
	case STEP_0:
        Find_Circuit  = OFF;
        circuit_id = 0;
        Status.errorCode = 0;
        Total_circuit_n = 0;

        if (GetStepperPosition(MOTOR_TABLE) != 0){
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
            return PROC_FAIL;
        }            
        else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
            return PROC_FAIL;
        }
        else {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                TintingAct.Circuit_step_pos_cw[i] = 0;
                TintingAct.Circuit_step_pos_ccw[i] = 0;
                Circuit_step_temp[i] = 0;
            }
            Table.step ++;
            Old_Photocell_sts = DARK;            
        } 
	break;

	// Starts a full CW Table Rotation   
    case STEP_1:
        Steps_Todo = TintingAct.Steps_Revolution - STEP_PHOTO_TABLE_OFFSET; 
        // Table CW rotation
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
        Table.step ++;          
    break;
    
    // Self Recognition in CW direction
	case STEP_2:
        // Update Photocell Status
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
            New_Photocell_sts = DARK;
        else
            New_Photocell_sts = LIGHT;
        
        // Transition LIGHT-DARK ?
        if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) ) {
            Find_Circuit = ON;
            Circuit_step_temp[circuit_id] = GetStepperPosition(MOTOR_TABLE);
            if (Circuit_step_temp[circuit_id] < 0) {
                Circuit_step_temp[circuit_id] = (-Circuit_step_temp[circuit_id]) % TintingAct.Steps_Revolution; 
                if (Circuit_step_temp[circuit_id] != 0)
                    Circuit_step_temp[circuit_id] = TintingAct.Steps_Revolution - (Circuit_step_temp[circuit_id] + STEP_PHOTO_TABLE_CIRCUIT_CW);
            }   
            else
                Circuit_step_temp[circuit_id] = Circuit_step_temp[circuit_id] % TintingAct.Steps_Revolution;                        
            circuit_id++;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            if (Find_Circuit == OFF) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return PROC_FAIL;                
            }
            else if (circuit_id > MAX_COLORANT_NUMBER) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                for (i = 0; i < circuit_id; i++)
                    TintingAct.Circuit_step_pos_cw[i] = Circuit_step_temp[circuit_id - 1 - i];
                
                Total_circuit_n = circuit_id;                
                // Rotate CW motor Table till Photocell transition DARK-LIGHT
                StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
                Table.step ++;
            }
        }            
 	break;

	// Check for DARK-LIGHT transition    
    case STEP_3:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                            
            SetStepperHomePosition(MOTOR_TABLE);
            // Go to center position
            Steps_Todo = STEP_PHOTO_TABLE_CIRCUIT_CW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;            
        }            
 	break;

    // Table centered on Reference Circuit
    case STEP_4:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            SetStepperHomePosition(MOTOR_TABLE);
            Old_Photocell_sts = DARK;
            circuit_id_ccw = 0;            
        	// Starts a full CCW Table Rotation   
            Steps_Todo = -TintingAct.Steps_Revolution  + STEP_PHOTO_TABLE_OFFSET; 
            // Table CCW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);            
            Table.step ++;
        }             
    break;
        
    // Self Recognition in CCW direction
	case STEP_5:
        // Update Photocell Status
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
            New_Photocell_sts = DARK;
        else
            New_Photocell_sts = LIGHT;
        
        // Transition LIGHT-DARK ?
        if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) ) {
            TintingAct.Circuit_step_pos_ccw[circuit_id_ccw] = GetStepperPosition(MOTOR_TABLE) + STEP_PHOTO_TABLE_CIRCUIT_CCW;
            circuit_id_ccw++;
            circuit_id--;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return PROC_FAIL;
            }
            else if (circuit_id > 0) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return PROC_FAIL;
            }   
            else  
                // Self Learning Procedure End in CCW direction
                Table.step ++;        
        }            
 	break;

    // Check if 'Circuit_step_pos[]' steps match between CW and CCW directions
	case STEP_6:
        for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
            // Difference in Circuit position Steps between CW and CCW > tolerance --> Error
            if ( ((TintingAct.Circuit_step_pos_cw[i] > TintingAct.Circuit_step_pos_ccw[i]) && ((TintingAct.Circuit_step_pos_cw[i] - TintingAct.Circuit_step_pos_ccw[i]) >= TintingAct.Steps_Tolerance_Circuit ) ) ||
                 ((TintingAct.Circuit_step_pos_cw[i] <= TintingAct.Circuit_step_pos_ccw[i])&& ((TintingAct.Circuit_step_pos_ccw[i] - TintingAct.Circuit_step_pos_cw[i]) >= TintingAct.Steps_Tolerance_Circuit ) ) ) {
                HardHiZ_Stepper(MOTOR_TABLE);        
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return PROC_FAIL;                    
            }            
            Circuit_step_temp[i] = (TintingAct.Circuit_step_pos_cw[i] + TintingAct.Circuit_step_pos_ccw[i]) / 2; 
        }
        for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
            Find_Circuit = FALSE;
            for (i = 0; i < Total_circuit_n; i++) {
                if ( ((TintingAct.Circuit_step_theorical_pos[j] > Circuit_step_temp[i]) && ((TintingAct.Circuit_step_theorical_pos[j] - Circuit_step_temp[i]) <= TintingAct.Steps_Tolerance_Circuit) ) ||
                     ((TintingAct.Circuit_step_theorical_pos[j] <= Circuit_step_temp[i])&& ((Circuit_step_temp[i] - TintingAct.Circuit_step_theorical_pos[j]) <= TintingAct.Steps_Tolerance_Circuit) ) ) {                      
                    Circuit_step_temp_1[j] = Circuit_step_temp[i];
                    Find_Circuit = TRUE;
                    break;                        
                }
            }
            // Circuit 'j' is not present on the Table
            if (Find_Circuit == FALSE)
                Circuit_step_temp_1[j] = 0;
        }
        
        // DYNAMIC circuit position model: tabel without zeros
        if(TintingAct.Table_Step_position == DYNAMIC) {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                // Difference in Circuit position Steps between CW or CCW and values found in auto recognition procedure > tolerance --> Error            
                if ( ((Circuit_step_temp[i] > TintingAct.Circuit_step_pos[i]) && ((Circuit_step_temp[i] - TintingAct.Circuit_step_pos[i]) >= TintingAct.Steps_Tolerance_Circuit) ) ||
                     ((Circuit_step_temp[i] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - Circuit_step_temp[i]) >= TintingAct.Steps_Tolerance_Circuit) ) ) {
                    HardHiZ_Stepper(MOTOR_TABLE);        
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;                    
                }
            }
            Table.step ++;        
        }    
        // STATIC circuit position model: tabel with possible zeros
        else {
            for (i = 0; i < circuit_id; i++) {
                Find_Circuit = FALSE;
                for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                    if ( ((TintingAct.Circuit_step_pos[j] > Circuit_step_temp[i]) && ((TintingAct.Circuit_step_pos[j] - Circuit_step_temp[i]) <= TintingAct.Steps_Tolerance_Circuit)) ||
                         ((TintingAct.Circuit_step_pos[j] <= Circuit_step_temp[i])&& ((Circuit_step_temp[i] - TintingAct.Circuit_step_pos[j]) <= TintingAct.Steps_Tolerance_Circuit)) ) {                          
                        Find_Circuit = TRUE;
                        break;                        
                    }
                }
                // Circuit 'i' is not present on the Table
                if (Find_Circuit == FALSE) {
                    HardHiZ_Stepper(MOTOR_TABLE);        
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;                    
                }                
            }
            Table.step ++;        
        }                
    break;
// -----------------------------------------------------------------------------
    // Check if there is a full match between positiong Table found and enabled circuit table 
    case STEP_7:
        for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
            if (TintingAct.Table_Colorant_En[i] == TRUE) {
                if (Circuit_step_temp_1[i] == 0) {
                    HardHiZ_Stepper(MOTOR_TABLE);        
                    Table.errorCode  = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;                                            
                }
            }
            else if (TintingAct.Table_Colorant_En[i] == FALSE) {
                if (Circuit_step_temp_1[i] != 0) {
                    HardHiZ_Stepper(MOTOR_TABLE);        
                    Table.errorCode  = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;                                            
                }
            }
        }
        // Rotate CCW motor Table till Photocell transition LIGHT-DARK
        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                                
        Table.step ++;        
 	break;

    case STEP_8:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            Steps_Todo = -STEP_PHOTO_TABLE_REFERENCE_CCW; 
            // Table CCW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);            
            Table.step ++;            
        }
    break; 
    
    case STEP_9:
        if (Status_Board_Table.Bit.MOT_STATUS == 0){                
            HardHiZ_Stepper(MOTOR_TABLE);            
            ret = PROC_OK;
        }                     
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

/*
*//*=====================================================================*//**
**      @brief Table Steps Positioning process
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/

unsigned char TableStepsPositioningColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static signed long Steps_Todo, Steps_Position;
  signed long Steps_Pos;
  
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
/*  
  if ((Motor_alarm & OVER_CURRENT_DETECTION) == 0) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_BRIDGE) || ((Motor_alarm & THERMAL_ERROR) == THERMAL_SHUTDOWN_DEVICE) )  {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
    return PROC_FAIL;
  }
  else if ( ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT) == 0) || ((Motor_alarm & UNDER_VOLTAGE_LOCK_OUT_ADC) == 0) ) {
    HardHiZ_Stepper(MOTOR_TABLE);        
    Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
    return PROC_FAIL;
  }  
*/
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
       HardHiZ_Stepper(MOTOR_TABLE);        
       Table.step = STEP_3;
  }
  if (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }
  else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  } 
  else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    HardHiZ_Stepper(MOTOR_TABLE);                                    
    Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST;
    return PROC_FAIL;                
  }

  Steps_Pos = GetStepperPosition(MOTOR_TABLE);
  if (Steps_Pos >= (signed long)TintingAct.Steps_Revolution) {
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }
    else
        SetStepperHomePosition(MOTOR_TABLE);
  }      
  else if (Steps_Pos <= -(signed long)TintingAct.Steps_Revolution) {   
    // Errore di Sgranamento
    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
        HardHiZ_Stepper(MOTOR_TABLE);                                    
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;                        
    }   
    else 
        SetStepperHomePosition(MOTOR_TABLE);
  }
  
  Status_Board_Table.word = GetStatus(MOTOR_TABLE);  
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
    case STEP_0:
        Status.errorCode = 0;
        // Analyze Command parameters:
        if ( ( (TintingAct.Rotation_Type != ABSOLUTE) && (TintingAct.Rotation_Type != INCREMENTAL) ) ||
             ( (TintingAct.Direction != CW) && (TintingAct.Direction != CCW) ) ||
               (TintingAct.Steps_N > TintingAct.Steps_Revolution) ) {    
            HardHiZ_Stepper(MOTOR_TABLE);        
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            return PROC_FAIL;            
        }   
        // Read current position
        Steps_Position = GetStepperPosition(MOTOR_TABLE);
        if (Steps_Position < 0) {
            Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution; 
            if (Steps_Position != 0)
                Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
        }   
        else
            Steps_Position = Steps_Position % TintingAct.Steps_Revolution;
        
        Table.step ++;
	break;
    
    // Start Table Rotation
    case STEP_1:
        // Absolute movement
        if (TintingAct.Rotation_Type == ABSOLUTE) { 
            // Steps_Todo > 0  --> CW
            // Steps_Todo <= 0 --> CCW
            // Case 1
            if ( ((signed long)(TintingAct.Steps_N - Steps_Position) >= 0) && ((signed long)(TintingAct.Steps_N - Steps_Position) < (TintingAct.Steps_Revolution / 2)) ) 
                Steps_Todo = -(signed long)(TintingAct.Steps_N - Steps_Position); 
            // Case 2
            else if ( ((signed long)(TintingAct.Steps_N - Steps_Position) >= 0) && ((signed long)(TintingAct.Steps_N - Steps_Position) >= (TintingAct.Steps_Revolution / 2)) ) 
                Steps_Todo = TintingAct.Steps_Revolution - (TintingAct.Steps_N - Steps_Position); 
            // Case 3
            else if ( ((signed long)(TintingAct.Steps_N - Steps_Position) < 0) && ((signed long)(TintingAct.Steps_N - Steps_Position) > -(signed long)(TintingAct.Steps_Revolution / 2)) ) 
                Steps_Todo = Steps_Position - TintingAct.Steps_N; 
            // Case 4
            else if ( ((signed long)(TintingAct.Steps_N - Steps_Position) < 0) && ((signed long)(TintingAct.Steps_N - Steps_Position) <= -(signed long)(TintingAct.Steps_Revolution / 2)) ) 
                Steps_Todo = -(signed long)(TintingAct.Steps_Revolution - (Steps_Position - TintingAct.Steps_N));             
        } 
        // Incremental movement
        else {
            if (TintingAct.Direction == CW) 
                Steps_Todo = TintingAct.Steps_N;
            else 
                Steps_Todo = -(signed long)TintingAct.Steps_N;                
        }
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
        Table.step ++;                                 
    break;        

	// Waiting to reach the desired position
    case STEP_2:
        if (Status_Board_Table.Bit.MOT_STATUS == 0)               
            Table.step ++;                                 
    break;        
       
      case STEP_3:
//        HardHiZ_Stepper(MOTOR_TABLE);              
        StopStepper(MOTOR_TABLE);          
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

#ifndef SKIP_FAULT_1
static unsigned char getFault_1Error(void)
/**/
/*===========================================================================*/
/**
**   @brief Return Fault_1 detection state
**
**   @param void
**
**   @return unsigned char
**/
/*===========================================================================*/
{
    switch(fault_1_state) {

    case FAULT_1_IDLE:
        StopTimer(T_DELAY_FAULT_1_ENABLING);
        StopTimer(T_DELAY_FAULT_1_ACTIVATION);
        if (isFault_1_Conditions()) {
          StartTimer(T_DELAY_FAULT_1_ENABLING);
          fault_1_state = FAULT_1_WAIT_ENABLING;
        }
    break;

    case FAULT_1_WAIT_ENABLING:
        if (!isFault_1_Conditions()) {
          fault_1_state = FAULT_1_IDLE;
        }
        else if (StatusTimer(T_DELAY_FAULT_1_ENABLING) == T_ELAPSED) {
          fault_1_state = FAULT_1_WAIT_ACTIVATION;
          StartTimer(T_DELAY_FAULT_1_DETECTION);
          DRIVER_RESET = OFF;
        }
    break;

    case FAULT_1_WAIT_ACTIVATION:
        if (StatusTimer(T_DELAY_FAULT_1_DETECTION) == T_ELAPSED &&
            StatusTimer(T_DELAY_FAULT_1_ACTIVATION) != T_RUNNING)
          DRIVER_RESET = ON;

        if (!isFault_1_Conditions()) {
          fault_1_state = FAULT_1_IDLE;
          DRIVER_RESET = ON;
        }
        else if (isFault_1_Detection()) {
          if (StatusTimer(T_DELAY_FAULT_1_ACTIVATION) == T_HALTED) {
            StartTimer(T_DELAY_FAULT_1_ACTIVATION);
          }

          else if (StatusTimer(T_DELAY_FAULT_1_ACTIVATION) == T_ELAPSED) {
            fault_1_state = FAULT_1_ERROR;
            DRIVER_RESET = ON;
          }
        }
        else
          StopTimer(T_DELAY_FAULT_1_ACTIVATION);

    break;

    case FAULT_1_ERROR:
    break;
  } /* switch() */
  return fault_1_state;
}
#endif 
