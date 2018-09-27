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
  unsigned short i;
  // Passi corrispondenti ad un giro completa di 360° della tavola
  TintingAct.Steps_Revolution = STEPS_REVOLUTION;
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
  for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
    TintingAct.Circuit_step_pos_cw[i] = 0; 
    TintingAct.Circuit_step_pos_ccw[i] = 0;     
    TintingAct.Circuit_step_theorical_pos[i] = STEPS_REFERENCE_CIRC_1 + i*STEPS_CIRCUITS;
  }    
  TintingAct.Circuit_Engaged = 0;  
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
            // New Erogation Command Request
            if (Status.level == TINTING_SUPPLY_RUN_ST)
                Table.level = TABLE_SETUP; 
            // New Ricirculation Command Received
            else if (Status.level == TINTING_STANDBY_RUN_ST)
                Table.level = TABLE_SETUP; 
            // New Table Homing Command Received
            else if (Status.level == TINTING_TABLE_SEARCH_HOMING_ST) {
                Table.level = TABLE_HOMING;
                Table.step = STEP_0;
            }
            // New Table Self Recognition Command Received
            else if (Status.level == TINTING_TABLE_SELF_RECOGNITION_ST) {
                Table.level = TABLE_HOMING;
                Table.step = STEP_0;    
            }
            // New Table Positioning Command Received
            else if (Status.level == TINTING_TABLE_POSITIONING_ST) {
                Table.level = TABLE_POSITIONING;
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
        break;
            
        case TABLE_SETUP:

        break;

        case TABLE_HOMING:
            if (Status.level == TINTING_TABLE_SEARCH_HOMING_ST) {
                if (TableHomingColorSupply() == PROC_OK)
                    Table.level = TABLE_END;
                else if (TableHomingColorSupply() == PROC_FAIL)
                   Table.level = TABLE_ERROR;
            }
            else if (Status.level == TINTING_TABLE_SELF_RECOGNITION_ST) {
                if (TableHomingColorSupply() == PROC_OK)
                    Table.level = TABLE_SELF_RECOGNITION;
                else if (TableHomingColorSupply() == PROC_FAIL)
                   Table.level = TABLE_ERROR;                
            }
        break;

        case TABLE_CLEANING:
            if (TableCleaningColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TableCleaningColorSupply() == PROC_FAIL)
                Table.level = TABLE_ERROR;
        break;
                
        case TABLE_POSITIONING:
            if (TablePositioningColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TablePositioningColorSupply() == PROC_FAIL)
               Table.level = TABLE_ERROR;
        break;

        case TABLE_STEPS_POSITIONING:
            if (TableStepsPositioningColorSupply() == PROC_OK)
                Table.level = TABLE_END;
            else if (TableStepsPositioningColorSupply() == PROC_FAIL)
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
                 (Status.level != TINTING_TABLE_TEST_ST) )
                Table.level = TABLE_IDLE; 
        break;

        case TABLE_ERROR:
            if ( (Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                 (Status.level != TINTING_TABLE_SEARCH_HOMING_ST) && (Status.level != TINTING_TABLE_POSITIONING_ST) &&
                 (Status.level != TINTING_TABLE_CLEANING_ST) && (Status.level != TINTING_TABLE_SELF_RECOGNITION_ST) && 
                 (Status.level != TINTING_TABLE_TEST_ST) )
                Table.level = TABLE_IDLE; 
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
    if (TintingAct.Steps_Revolution > (STEPS_REVOLUTION + STEPS_TOLERANCE_REVOLUTION))
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
         (TintingAct.Steps_Reference <= TintingAct.Steps_Tolerance_Reference)   ||
         (TintingAct.Steps_Circuit <= TintingAct.Steps_Tolerance_Circuit) )   
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
  static unsigned char Count_Steps, Position_Steps, Max_Count_Steps, Reference_Position_Step;
  static long Steps_Todo;

  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_TABLE);
       Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
       StopStepper(MOTOR_TABLE);
       Table.step = STEP_12;
  }       
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

        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
            // Rotate CW motor Table till Photocell transition DARK-LIGHT
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, DARK_LIGHT, TABLE_PHOTOCELL, 0);                        
            Table.step ++;
        }
        else
            Table.step +=2;            
	break;

    // Wait for DARK-LIGHT Table Photocell transition 
    case STEP_1:
        // Table Photocell Light --> proceed to find Reference
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
            StopStepper(MOTOR_TABLE);
            Old_Photocell_sts = LIGHT;
            SetStepperHomePosition(MOTOR_TABLE);
            Table.step ++;
        }                        
        // Table Photocell never in Light status in one full Table Rotation
        else if (GetStepperPosition(MOTOR_TABLE) >= TintingAct.Steps_Revolution) {
            StopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
            return FALSE;                
        }            
    break;
    
	// Starts a full Table Rotation to find Reference circuit    
    case STEP_2:
        Steps_Todo = TintingAct.Steps_Revolution; 
        // Table CW rotation
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
        Table.step ++;          
    break;
    
	//  Check if position required is reached
	case STEP_3:
        // Update Photocell Status
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
            New_Photocell_sts = DARK;
        else
            New_Photocell_sts = LIGHT;
        
        // Transition LIGHT-DARK ?
        if ( (Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK) ) {
            Position_Steps = GetStepperPosition(MOTOR_TABLE);            
            Count_Steps = GetStepperPosition(MOTOR_TABLE);
            Tr_Light_Dark = ON;
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
            Find_Circuit  = ON;
            // Motor steps with photocell DARK
            Count_Steps = ABS(GetStepperPosition(MOTOR_TABLE) - Count_Steps);
            if (Count_Steps > Max_Count_Steps) {
                Max_Count_Steps = Count_Steps;
                Reference_Position_Step = Position_Steps;
            }
        }
        
        if (GetStepperPosition(MOTOR_TABLE) >= Steps_Todo) {
            if (Find_Circuit == OFF) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return FALSE;                
            }
            else
                // Reference Circuit Find
                Table.step ++;        
        }    
	break; 

	//  Check if Reference Circuit is correct
	case STEP_4:
        if (ABS(Max_Count_Steps - TintingAct.Steps_Reference) <= TintingAct.Steps_Tolerance_Reference)
        // Reference Circuit found is correct
            Table.step ++;        
        // Reference Circuit is NOT correct
        else {
            StopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST;
            return FALSE;                
        }          
    break;

	// Starts Go to Homing = Reference position
	case STEP_5:
        if ( (TintingAct.Steps_Revolution - Position_Steps) < TintingAct.Steps_Revolution/2) {
            // Shortest way is a CW rotation
            Steps_Todo = Position_Steps - STEP_PHOTO_TABLE_OFFSET;
    		Table.step ++ ;
        }             
        else {
            // Shortest way is a CCW rotation
            Steps_Todo = -(TintingAct.Steps_Revolution - STEP_PHOTO_TABLE_OFFSET - Position_Steps);
    		Table.step +=4;            
        }                             
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
	break;
// -----------------------------------------------------------------------------    
// CW Rotation
	// Go Homing = Reference in CW rotation
	case STEP_6:
        if (GetStepperPosition(MOTOR_TABLE) >= Steps_Todo)  {
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return FALSE;                                
            }
            else  {
                // Rotate CW motor Table till Photocell transition DARK-LIGHT
                StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
                Table.step ++;
            }
        }             
	break;

    case STEP_7:
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
            StopStepper(MOTOR_TABLE);
            SetStepperHomePosition(MOTOR_TABLE);
            // Shortest way is a CCW rotation
            Steps_Todo = STEP_PHOTO_TABLE_REFERENCE_CW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;
        } 
	break;

    case STEP_8:
        if (GetStepperPosition(MOTOR_TABLE) >= Steps_Todo)  {
    		// Table positioned at the centre of Reference Circuit
            SetStepperHomePosition(MOTOR_TABLE);
            Table.step +=4;
        }             
    break;
// -----------------------------------------------------------------------------
// CCW Rotation    
	// Go Homing = Reference in CCW rotation
	case STEP_9:
        if (GetStepperPosition(MOTOR_TABLE) <= Steps_Todo)  {
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return FALSE;                                
            }
            else  {
                // Rotate CCW motor Table till Photocell transition DARK-LIGHT
                StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);                        
                Table.step ++;
            }
        }             
	break;
    
    case STEP_10:
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
            StopStepper(MOTOR_TABLE);
            SetStepperHomePosition(MOTOR_TABLE);
            // Shortest way is a CCW rotation
            Steps_Todo = -STEP_PHOTO_TABLE_REFERENCE_CCW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;
        } 
	break;

    case STEP_11:
        if (GetStepperPosition(MOTOR_TABLE) <= Steps_Todo)  {
    		// Table positioned at the centre of Reference Circuit
            SetStepperHomePosition(MOTOR_TABLE);
            Table.step ++;
        }             
    break;    
// -----------------------------------------------------------------------------
    case STEP_12:
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
  static unsigned short circuit_id;
  static unsigned char Find_Circuit, Old_Photocell_sts, New_Photocell_sts;
  static long Steps_Todo;
  long Circuit_step_temp[MAX_COLORANT_NUMBER];
  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_TABLE);
       Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
       StopStepper(MOTOR_TABLE);
       Table.step = STEP_13;
  }       
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

        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
            Total_circuit_n = 0;
            Table_circuits_pos = OFF;
            for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                TintingAct.Circuit_step_pos[i] = 0; 
            StopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
            return FALSE;
        }
        else {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                TintingAct.Circuit_step_pos_cw[i] = 0;
                TintingAct.Circuit_step_pos_ccw[i] = 0;
            }    
            Table.step ++;
            Old_Photocell_sts = LIGHT;
        }            
	break;

	// Starts a full CW Table Rotation   
    case STEP_2:
        Steps_Todo = TintingAct.Steps_Revolution; 
        // Table CW rotation
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
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
            TintingAct.Circuit_step_pos_cw[circuit_id] = GetStepperPosition(MOTOR_TABLE) + STEP_PHOTO_TABLE_CIRCUIT_CW;
            circuit_id++;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (GetStepperPosition(MOTOR_TABLE) >= Steps_Todo) {
            if (Find_Circuit == OFF) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                    TintingAct.Circuit_step_pos[i] = 0; 
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return FALSE;                
            }
            else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                    TintingAct.Circuit_step_pos[i] = 0; 
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return FALSE;
            }
            else if (circuit_id > MAX_COLORANT_NUMBER) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                    TintingAct.Circuit_step_pos[i] = 0; 
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return FALSE;
            }   
            else { 
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
        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, DARK-LIGHT, TABLE_PHOTOCELL, 0);                        
        Table.step ++;        
 	break;

	// Check for DARK-LIGHT transition
    case STEP_5:
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
            StopStepper(MOTOR_TABLE);
            SetStepperHomePosition(MOTOR_TABLE);
            // Go to center position
            Steps_Todo = -STEP_PHOTO_TABLE_REFERENCE_CCW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;            
        }    
 	break;
    
    // Table centered on Reference Circuit
    case STEP_6:
        if (GetStepperPosition(MOTOR_TABLE) <= Steps_Todo)  {
    		// Table positioned at the centre of Reference Circuit
            SetStepperHomePosition(MOTOR_TABLE);
            Table.step ++;
        }             
    break;  
    
	// Starts a full CCW Table Rotation   
    case STEP_7:
        Steps_Todo = -TintingAct.Steps_Revolution; 
        // Table CCW rotation
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
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
            TintingAct.Circuit_step_pos_ccw[circuit_id] = TintingAct.Steps_Revolution + GetStepperPosition(MOTOR_TABLE) + STEP_PHOTO_TABLE_CIRCUIT_CCW;
            circuit_id--;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (GetStepperPosition(MOTOR_TABLE) <= Steps_Todo) {
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return FALSE;
            }
            else if (circuit_id > 0) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return FALSE;
            }   
            else  
                // Self Learning Procedure End in CCW direction
                Table.step ++;        
        }            
 	break;

    // Check if 'Circuit_step_pos[]' steps match between CW and CCW directions
	case STEP_9:
        for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
            // Difference in Circuit position Steps between Cw and CCW > tolerance --> Error
            if (ABS(TintingAct.Circuit_step_pos_cw[i] - TintingAct.Circuit_step_pos_ccw[i]) >= TintingAct.Steps_Tolerance_Circuit )  {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                    TintingAct.Circuit_step_pos[i] = 0; 
                StopStepper(MOTOR_TABLE);
                Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                Status.errorCode = i; 
                return FALSE;                    
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
                if (ABS(TintingAct.Circuit_step_theorical_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit )  {
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
            for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                TintingAct.Circuit_step_pos[i] = 0; 
            StopStepper(MOTOR_TABLE);
            Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
            Status.errorCode = i; 
            return FALSE;                    
        }    
        Table.step ++;        
 	break;
    
    // Create the final Circuit position map
    case STEP_11:
        // DYNAMIC circuit position model: tabel without zeros
        if(TintingAct.Table_Step_position == DYNAMIC)
            Table.step ++;        
        // STATIC circuit position model: tabel with possible zeros
        else {
            for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                Find_Circuit = FALSE;
                for (i = 0; i < Total_circuit_n; i++) {
                    if (ABS(TintingAct.Circuit_step_theorical_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit )  {
                        Circuit_step_temp[j] = TintingAct.Circuit_step_pos[i];
                        Find_Circuit = TRUE;
                        break;                        
                    }
                }
                // Circuit 'j' is not present on the Table
                if (Find_Circuit == FALSE)
                    Circuit_step_temp[j] = 0;
            }
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
                    if (TintingAct.Circuit_step_pos[i] == 0) {
                        StopStepper(MOTOR_TABLE);
                        Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        Status.errorCode = i; 
                        return FALSE;                                            
                    }
                }
                else if (TintingAct.Table_Colorant_En[i] == FALSE) {
                    if (TintingAct.Circuit_step_pos[i] != 0) {
                        StopStepper(MOTOR_TABLE);
                        Table.errorCode  = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        Status.errorCode = i; 
                        return FALSE;                                            
                    }
                }
            }
            Table.step ++;        
 	break;
        
	// Re-alignement in Reference position
    case STEP_13:
        // Rotate CCW motor Table till Photocell transition DARK-LIGHT
        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, DARK-LIGHT, TABLE_PHOTOCELL, 0);                        
        Table.step ++;        
 	break;

	// Check for DARK-LIGHT transition
    case STEP_14:
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
            StopStepper(MOTOR_TABLE);
            SetStepperHomePosition(MOTOR_TABLE);
            // Go to center position
            Steps_Todo = STEP_PHOTO_TABLE_REFERENCE_CW;            
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++;            
        }    
 	break;
    
    // Table centered on Reference Circuit
    case STEP_15:
        if (GetStepperPosition(MOTOR_TABLE) >= Steps_Todo)  {
    		// Table positioned at the centre of Reference Circuit 
            SetStepperHomePosition(MOTOR_TABLE);
            Table.step ++;
        }             
    break;      
// -----------------------------------------------------------------------------
    case STEP_16:
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

unsigned char TablePositioningColorSupply(void)
{
  unsigned char ret = PROC_RUN;
  unsigned short Motor_alarm;
  static unsigned short direction;
  static long Steps_Todo, Steps_done;

  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_TABLE);
       Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
       StopStepper(MOTOR_TABLE);
       Table.step = STEP_6;
  }       
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
    case STEP_0:
        Status.errorCode = 0;
        // Analyze Command parameters:
        if ( (TintingAct.Color_Id > MAX_COLORANT_NUMBER) || (TintingAct.Refilling_Angle > MAX_ROTATING_ANGLE) ||
             ( (TintingAct.Direction != CW) && (TintingAct.Direction != CCW) ) ||
               (TintingAct.Table_Colorant_En[TintingAct.Color_Id] - 1) == 0) {    
            StopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            return FALSE;            
        }   

        // No Circuit Position steps available --> Process fail
        if (Table_circuits_pos == OFF) {
            StopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
            return FALSE;            
        }
        else if (TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1] == 0) {
            StopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
            return FALSE;            
        }   
        // Read current position
        Steps_done = GetStepperPosition(MOTOR_TABLE);        
        Table.step ++;
	break;
    
    // Start Table Rotation towards circuit selected in the shortest way
    case STEP_1:
        if ( ((TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1]) - Steps_done)  < TintingAct.Steps_Revolution / 2)
            direction = CW;
        else
            direction = CCW;
        
        if (direction == CW)
            // Go to near 'TintingAct.Color_Id'
            Steps_Todo = TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1] -2 * STEP_PHOTO_TABLE_CIRCUIT_CW;                        
        else
            // Go to near 'TintingAct.Color_Id'
            Steps_Todo = -TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1] +2 * STEP_PHOTO_TABLE_CIRCUIT_CCW;                                    
            
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
        Table.step ++;                                 
    break;        

	// Waiting for be placed near 'TintingAct.Color_Id' position
    case STEP_2:
        if ( (direction == CW) && (GetStepperPosition(MOTOR_TABLE) >= (Steps_Todo + Steps_done)  ) ){
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return FALSE;
            }
            // Rotate CW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.High_Speed_Rotating_Table, CW, LIGHT-DARK, TABLE_PHOTOCELL, 0);                        
        }
        else if ( (direction == CCW) && (GetStepperPosition(MOTOR_TABLE) <= (Steps_Todo + Steps_done) ) ) {
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return FALSE;
            }
            // Rotate CCW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.High_Speed_Rotating_Table, CCW, LIGHT-DARK, TABLE_PHOTOCELL, 0);                        
        }
        Table.step ++;          
    break;        

	// Check for LIGHT-DARK transition
    case STEP_3:
        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
            StopStepper(MOTOR_TABLE);
            if (direction == CW)
                // Go to center 'TintingAct.Color_Id' position CW
                Steps_Todo = STEP_PHOTO_TABLE_CIRCUIT_CW;                        
            else
                // Go to center 'TintingAct.Color_Id' position CCW
                Steps_Todo = -STEP_PHOTO_TABLE_CIRCUIT_CCW;                                    

            Steps_done = GetStepperPosition(MOTOR_TABLE);
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step ++; 
        }    
    break;        

	// Waiting to be placed at the center of 'TintingAct.Color_Id' position 
    case STEP_4:
        if ( ( (direction == CW)  && (GetStepperPosition(MOTOR_TABLE) >= (Steps_Todo + Steps_done) ) ) ||
             ( (direction == CCW) && (GetStepperPosition(MOTOR_TABLE) <= (Steps_Todo + Steps_done) ) ) ){
            
            // Check if Photocell is covered
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return FALSE;            
            }    
            if (TintingAct.Refilling_Angle > 0)  {
                // Find Steps corresponding to 'Refilling_Angle'
                Steps_done = GetStepperPosition(MOTOR_TABLE);
                if (TintingAct.Direction == CW)
                    Steps_Todo = (long)(TintingAct.Steps_Revolution * (float)(TintingAct.Refilling_Angle / MAX_ROTATING_ANGLE));
                else
                    Steps_Todo = -(long)(TintingAct.Steps_Revolution * (float)(TintingAct.Refilling_Angle / MAX_ROTATING_ANGLE));
                
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            }
            else
                Table.step +=2;       
        }
    break;

	// Waiting to reach the requested final position
    case STEP_5:
        if ( ( (direction == CW)  && (GetStepperPosition(MOTOR_TABLE) >= (Steps_Todo + Steps_done) ) )||
             ( (direction == CCW) && (GetStepperPosition(MOTOR_TABLE) <= (Steps_Todo + Steps_done) ) ) ){
            TintingAct.Circuit_Engaged = 
            Table.step ++;       
        }    
    break;
        
    case STEP_6:
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
  static long Steps_Todo, Steps_done;

  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_TABLE);
       Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
       StopStepper(MOTOR_TABLE);
       Table.step = STEP_4;
  }       
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
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
                return FALSE;            
            }   
            if ( (TintingAct.Table_Colorant_En[TintingAct.Color_Id] - 1) == 0) {    
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
                return FALSE;            
            }
            // No Circuit Position steps available --> Process fail
            if (Table_circuits_pos == OFF) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                return FALSE;            
            }
            else if (TintingAct.Circuit_step_pos[TintingAct.Color_Id - 1] == 0) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                return FALSE;            
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
  static unsigned short circuit_id;
  static unsigned char Find_Circuit, Old_Photocell_sts, New_Photocell_sts;
  static long Steps_Todo, Steps_pos;
  long Circuit_step_temp[MAX_COLORANT_NUMBER];

  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_TABLE);
       Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
       StopStepper(MOTOR_TABLE);
       Table.step = STEP_7;
  }       
  //----------------------------------------------------------------------------
  switch(Table.step)
  {
// -----------------------------------------------------------------------------     
	// Starts operations
	case STEP_0:
        Find_Circuit  = OFF;
        circuit_id = 0;
        Status.errorCode = 0;

        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
            Old_Photocell_sts = LIGHT;
        else
            Old_Photocell_sts = DARK;
                
        for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
            TintingAct.Circuit_step_pos_cw[i] = 0;
            TintingAct.Circuit_step_pos_ccw[i] = 0;
        }
        Steps_pos = GetStepperPosition(MOTOR_TABLE);
        Table.step ++;
	break;

	// Starts a full CW Table Rotation   
    case STEP_2:
        Steps_Todo = TintingAct.Steps_Revolution; 
        // Table CW rotation
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
            TintingAct.Circuit_step_pos_cw[circuit_id] = GetStepperPosition(MOTOR_TABLE) + STEP_PHOTO_TABLE_CIRCUIT_CW;
            circuit_id++;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (GetStepperPosition(MOTOR_TABLE) >= (Steps_Todo + Steps_pos) ) {
            if (Find_Circuit == OFF) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return FALSE;                
            }
            else if (circuit_id > MAX_COLORANT_NUMBER) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return FALSE;
            }   
            // Table Test End in CW direction
            Steps_pos = GetStepperPosition(MOTOR_TABLE);
            Table.step ++;                   
        }            
 	break;
    
	// Starts a full CCW Table Rotation   
    case STEP_4:
        Steps_Todo = -TintingAct.Steps_Revolution; 
        // Table CCW rotation
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
        Table.step ++;          
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
            TintingAct.Circuit_step_pos_ccw[circuit_id] = TintingAct.Steps_Revolution + GetStepperPosition(MOTOR_TABLE) + STEP_PHOTO_TABLE_CIRCUIT_CCW;
            circuit_id--;            
        }
                
        Old_Photocell_sts = New_Photocell_sts;
        
        if (GetStepperPosition(MOTOR_TABLE) <= (Steps_Todo + Steps_pos) ){
            if (circuit_id > 0) {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return FALSE;
            }   
            else  
                // Self Learning Procedure End in CCW direction
                Table.step ++;        
        }            
 	break;

    // Check if 'Circuit_step_pos[]' steps match between CW and CCW directions
	case STEP_6:
        for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
            // Difference in Circuit position Steps between Cw and CCW > tolerance --> Error
            if (ABS(TintingAct.Circuit_step_pos_cw[i] - TintingAct.Circuit_step_pos_ccw[i]) >= TintingAct.Steps_Tolerance_Circuit )  {
                StopStepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return FALSE;                    
            }            
            Circuit_step_temp[i] = (TintingAct.Circuit_step_pos_cw[i] + TintingAct.Circuit_step_pos_ccw[i]) / 2; 
        }    
        // DYNAMIC circuit position model: tabel without zeros
        if(TintingAct.Table_Step_position == DYNAMIC) {
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                // Difference in Circuit position Steps between CW or CCW and values found in auto recognition procedure > tolerance --> Error            
                if ( (ABS(Circuit_step_temp[i] - TintingAct.Circuit_step_pos[i]) >= TintingAct.Steps_Tolerance_Circuit) ) {
                    StopStepper(MOTOR_TABLE);
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return FALSE;                    
                }
            }
            Table.step ++;        
        }    
        // STATIC circuit position model: tabel with possible zeros
        else {
            for (i = 0; i < circuit_id; i++) {
                Find_Circuit = FALSE;
                for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                    if (ABS(TintingAct.Circuit_step_pos[j] - Circuit_step_temp[i]) <= TintingAct.Steps_Tolerance_Circuit )  {
                        Find_Circuit = TRUE;
                        break;                        
                    }
                }
                // Circuit 'i' is not present on the Table
                if (Find_Circuit == FALSE) {
                    StopStepper(MOTOR_TABLE);
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return FALSE;                    
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
                if (TintingAct.Circuit_step_pos[i] == 0) {
                    StopStepper(MOTOR_TABLE);
                    Table.errorCode  = TINTING_TABLE_TEST_ERROR_ST;
                    return FALSE;                                            
                }
            }
            else if (TintingAct.Table_Colorant_En[i] == FALSE) {
                if (TintingAct.Circuit_step_pos[i] != 0) {
                    StopStepper(MOTOR_TABLE);
                    Table.errorCode  = TINTING_TABLE_TEST_ERROR_ST;
                    return FALSE;                                            
                }
            }
        }
        Table.step ++;        
 	break;

    case STEP_8:
		ret = PROC_OK;
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
  static unsigned short direction;
  static long Steps_Todo, Steps_done;

  //----------------------------------------------------------------------------
  // Check for Motor Table Error
  ReadStepperError(MOTOR_TABLE,&Motor_alarm);
  if ( (Motor_alarm == OVER_CURRENT_DETECTION) || (Motor_alarm == THERMAL_SHUTDOWN) ||
       (Motor_alarm == UNDER_VOLTAGE_LOCK_OUT) || (Motor_alarm == STALL_DETECTION) ) {
       StopStepper(MOTOR_TABLE);
       Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
       return FALSE;
  }    
  // Table Panel OPEN
  if (TintingAct.PanelTable_state == OPEN) {
       StopStepper(MOTOR_TABLE);
       Table.step = STEP_3;
  }       
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
            StopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            return FALSE;            
        }   
        // Read current position
        Steps_done = GetStepperPosition(MOTOR_TABLE);        
        Table.step ++;
	break;
    
    // Start Table Rotation
    case STEP_1:
        // Absolute movement
        if (TintingAct.Rotation_Type == ABSOLUTE) { 
            if (TintingAct.Steps_N > Steps_done)
                direction = CW;
            else
                direction = CCW;
                
            Steps_Todo = TintingAct.Steps_N - Steps_done;
        }
        // Incremental movement
        else {
            if (direction == CW) 
                Steps_Todo = TintingAct.Steps_N;
            else 
                Steps_Todo = -TintingAct.Steps_N;                
        }
        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
        Table.step ++;                                 
    break;        

	// Waiting to reach the desired position
    case STEP_2:
        if ( (TintingAct.Rotation_Type == ABSOLUTE) && (direction == CW) && (GetStepperPosition(MOTOR_TABLE) >= TintingAct.Steps_N) ) 
            Table.step ++;                                 
        else if ( (TintingAct.Rotation_Type == ABSOLUTE) && (direction == CCW) && (GetStepperPosition(MOTOR_TABLE) <= TintingAct.Steps_N) )
            Table.step ++;                                 
        else if ( (TintingAct.Rotation_Type == INCREMENTAL) && (direction == CW) && (GetStepperPosition(MOTOR_TABLE) >= (TintingAct.Steps_N + Steps_done) ) ) 
            Table.step ++;                                 
        else if ( (TintingAct.Rotation_Type == INCREMENTAL) && (direction == CCW) && (GetStepperPosition(MOTOR_TABLE) <= (TintingAct.Steps_N + Steps_done) ) ) 
            Table.step ++;                                 
    break;        
       
      case STEP_3:
		ret = PROC_OK;
    break; 

	default:
		Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
  }
  return ret;      
}

