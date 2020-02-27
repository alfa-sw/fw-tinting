/* 
 * File:   tablemanager.h
 * Author: michele.abelli
 * Description: Table Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "tableManager.h"
#include "tintingManager.h"
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
#include "humidifierManager.h"
#include "errorManager.h"

static cSPIN_RegsStruct_TypeDef cSPIN_RegsStruct2 = {0}; //to set

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

void initTableStatusManager(void) {
    Table.level = TABLE_IDLE;
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

void initTableParam(void) {
    unsigned char i;

    // Passi corrispondenti ad un giro completa di 360° della tavola
    TintingAct.Steps_Revolution = TintingTable.Steps_Revolution * (unsigned long) CORRECTION_TABLE_STEP_RES;
    // Tolleranza in passi corrispondente ad una rotazione completa di 360° della tavola
    TintingAct.Steps_Tolerance_Revolution = TintingTable.Steps_Tolerance_Revolution * (unsigned long) CORRECTION_TABLE_STEP_RES;
    // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
    TintingAct.Steps_Reference = TintingTable.Steps_Reference * (unsigned long) CORRECTION_TABLE_STEP_RES;
    // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
    TintingAct.Steps_Tolerance_Reference = TintingTable.Steps_Tolerance_Reference * (unsigned long) CORRECTION_TABLE_STEP_RES;
    // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito 
    TintingAct.Steps_Circuit = TintingTable.Steps_Circuit * (unsigned long) CORRECTION_TABLE_STEP_RES;
    // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito
    TintingAct.Steps_Tolerance_Circuit = TintingTable.Steps_Tolerance_Circuit * (unsigned long) CORRECTION_TABLE_STEP_RES;
    // Velocità massima di rotazione della tavola rotante
    TintingAct.High_Speed_Rotating_Table = TintingTable.High_Speed_Rotating_Table;
    // Velocità minima di rotazione della tavola rotante
    TintingAct.Low_Speed_Rotating_Table = TintingTable.Low_Speed_Rotating_Table;
    // N° di giri della Tavola per Agitare 
    TintingAct.Steps_Stirring = TintingTable.Steps_Stirring;
    // Maschera abilitazione coloranti Tavola
    for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
        if (i < 8)
            TintingAct.Table_Colorant_En[i] = (procGUI.slaves_en[1] & (1 << i)) >> i;
        else
            TintingAct.Table_Colorant_En[i] = (procGUI.slaves_en[2] & (1 << (i - 8))) >> (i - 8);
    }
    Table_circuits_pos = OFF;
    Total_circuit_n = 0;
    // Le posizioni inzialmente sono quelle teoriche. Nessun autoriconoscimento in qesta fase.
    /*  
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
     */
    // 15.1.2019 Nuovo Pignone
    TintingAct.Circuit_step_theorical_pos[0] = (signed long) 260 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[1] = (signed long) 781 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[2] = (signed long) 1301 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[3] = (signed long) 1821 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[4] = (signed long) 2342 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[5] = (signed long) 2862 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[6] = (signed long) 3382 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[7] = (signed long) 3903 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[8] = (signed long) 4422 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[9] = (signed long) 4942 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[10] = (signed long) 5463 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[11] = (signed long) 5983 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[12] = (signed long) 6504 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[13] = (signed long) 7024 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[14] = (signed long) 7544 * (signed long) CORRECTION_TABLE_STEP_RES;
    TintingAct.Circuit_step_theorical_pos[15] = (signed long) 8065 * (signed long) CORRECTION_TABLE_STEP_RES;

    TintingAct.Circuit_Engaged = 0;
    TintingAct.Table_Step_position = DYNAMIC;

    for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
        TintingAct.Circuit_step_pos[i] = 0;
    }

    //Stirring_Method = BEFORE_EVERY_RICIRCULATION;  
    Stirring_Method = AFTER_LAST_RICIRCULATING_CIRCUIT;
    TintingAct.Last_Cmd_Reset = OFF;

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
    Table_Motors = OFF;

    // All 8 - 23 possible Cleaning colorants are OFF 
    TintingAct.Cleaning_status = 0x000000;
}

/*
 *//*=====================================================================*//**
**      @brief Cleaning Initialization parameters
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/

void initCleanParam(void) {
    unsigned char i;

    // Cleaning Duration (sec)
    TintingAct.Cleaning_duration = TintingClean.Cleaning_duration;
    // Cleaning Pause (min)
    TintingAct.Cleaning_pause = TintingClean.Cleaning_pause;
    // Maschera abilitazione Pulizia Coloranti Tavola
    for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
        if (i < 8) {
            TintingAct.Clean_Mask[i] = (TintingClean.Cleaning_Colorant_Mask[1] & (1 << i)) >> i;
        } else {
            TintingAct.Clean_Mask[i] = (TintingClean.Cleaning_Colorant_Mask[2] & (1 << (i - 8))) >> (i - 8);
        }
    }
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

void TableManager(void) {
#ifndef NOLAB    
    unsigned char ret_proc;
#endif    
    switch (Table.level) {
        case TABLE_IDLE:
            break;

        case TABLE_PAR_RX:
            if (Status.level != TINTING_WAIT_SETUP_OUTPUT_TABLE_ST)
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
            // New Table Homing Command Received
            if ((Status.level == TINTING_TABLE_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) ||
                    (Status.level == TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_HOMING_ST) || (Status.level == TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_TABLE_HOMING_ST)) {
                Table.level = TABLE_HOMING;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// New Table Steps Positioning 
            else if ((Status.level == TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_NOT_ENGAGED_ST) || (Status.level == TINTING_PHOTO_LIGHT_VALVE_SEARCH_HOMING_ST)) {
                //                if (TintingAct.Circuit_Engaged != 0) {                   
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    TintingAct.Rotation_Type = INCREMENTAL;
                    TintingAct.Direction = CW;
                    TintingAct.Steps_N = STEPS_REFERENCE_CIRC_1;
                    Table_Steps_Positioning_Photocell_Ctrl = FALSE;
                    Table.level = TABLE_STEPS_POSITIONING;
                    Table.step = STEP_0;
                    Table_Motors = ON;
                } else
                    Table.level = TABLE_END;
            }// New Table Self Recognition Command Received
            else if (Status.level == TINTING_TABLE_SELF_RECOGNITION_ST) {
                //                Table.level = TABLE_HOMING;
                Table.level = TABLE_SELF_RECOGNITION;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// New Table Find Riference Command Received
            else if (Status.level == TINTING_TABLE_FIND_REFERENCE_ST) {
                Table.level = TABLE_FIND_REFERENCE;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// New Table Positioning Command Received
            else if (Status.level == TINTING_TABLE_POSITIONING_ST) {
                End_Table_Position = 0;
                Table.level = TABLE_POSITIONING;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// New Stirring Command Received
            else if (Status.level == TINTING_TABLE_STIRRING_ST) {
                Table.level = TABLE_STIRRING;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// New Table Cleaning Command Received
                // -----------------------------------------------------------------------------
            else if (Status.level == TINTING_TABLE_CLEANING_ST) {
                // Pulizia Puntuale
                if (Punctual_Clean_Act == ON) {
                    if (TintingAct.Table_Colorant_En[TintingAct.Id_Punctual_Cleaning - 1] == 0) {
                        Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                        Table.level = TABLE_ERROR;
                    } else {
                        End_Table_Position = 0;
                        Table.step = STEP_0;
                        Table_Motors = ON;
                        TintingAct.Color_Id = TintingAct.Id_Punctual_Cleaning;
                        Table.level = TABLE_CLEANING;
                    }
                } else {
                    // Pulizia Temporizzata
                    while (indx_Clean < MAX_COLORANT_NUMBER) {
                        if (TintingAct.Cleaning_Col_Mask[indx_Clean] == TRUE) {
                            TintingAct.Color_Id = indx_Clean + 1;
                            break;
                        } else
                            indx_Clean++;
                    }
                    // End process
                    if (indx_Clean == MAX_COLORANT_NUMBER) {
                        Clean_Activation = OFF;
                        Table.step = STEP_0;
                        NextTable.level = TABLE_END;
                        Table.level = TABLE_GO_REFERENCE;
                    } else {
                        if (TintingAct.Table_Colorant_En[TintingAct.Color_Id - 1] == 0) {
                            Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                            Clean_Activation = OFF;
                            Table.level = TABLE_ERROR;
                        } else {
                            indx_Clean++;
                            End_Table_Position = 0;
                            Table.step = STEP_0;
                            Table_Motors = ON;
                            Table.level = TABLE_CLEANING;
                        }
                    }
                }
            }// -----------------------------------------------------------------------------            
                // New Table Test Command Received
            else if (Status.level == TINTING_TABLE_TEST_ST) {
                Table.level = TABLE_TEST;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// New Table Steps Positioning Command Received
            else if (Status.level == TINTING_TABLE_STEPS_POSITIONING_ST) {
                Table_Steps_Positioning_Photocell_Ctrl = TRUE;
                Table.level = TABLE_STEPS_POSITIONING;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// Table has to go to Reference Position
            else if (Status.level == TINTING_TABLE_GO_REFERENCE_ST) {
                Table.level = TABLE_GO_REFERENCE;
                Table.step = STEP_0;
                Table_Motors = ON;
            }// New Table ON/OFF Command Received
            else if (Status.level == TINTING_WAIT_SETUP_OUTPUT_TABLE_ST) {
                Table.level = TABLE_SETUP_OUTPUT;
                Table.step = STEP_0;
                Table_Motors = ON;
            }
            break;

        case TABLE_SETUP:

            break;

        case TABLE_SETUP_OUTPUT:
            if (AnalyzeSetupOutputs() == TRUE) {
                Table.level = TABLE_PAR_RX;
                NextTable.level = TABLE_RUN;
            } else
                Table.level = PUMP_ERROR;
            break;

        case TABLE_RUN:
#ifndef NOLAB
            ret_proc = TableRun();
            if (ret_proc == PROC_OK)
                Table.level = TABLE_END;
            else if (ret_proc == PROC_FAIL) {
                Table.level = TABLE_ERROR;
            }
#else            
            Table.level = TABLE_END;
#endif                        
            break;

        case TABLE_HOMING:
#ifndef NOLAB           
            if ((Status.level == TINTING_TABLE_SEARCH_HOMING_ST) || (Status.level == TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) ||
                    (Status.level == TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_HOMING_ST) || (Status.level == TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_TABLE_HOMING_ST)) {
                ret_proc = TableHomingColorSupply();
                if (ret_proc == PROC_OK)
                    Table.level = TABLE_END;
                else if (ret_proc == PROC_FAIL)
                    Table.level = TABLE_ERROR;
            } else if (Status.level == TINTING_TABLE_SELF_RECOGNITION_ST) {
                ret_proc = TableHomingColorSupply();
                if (ret_proc == PROC_OK) {
                    Table.level = TABLE_SELF_RECOGNITION;
                    Table.step = STEP_0;
                } else if (ret_proc == PROC_FAIL)
                    Table.level = TABLE_ERROR;
            }
            //Table.level = TABLE_END;  
#else            
            Table.level = TABLE_END;
#endif            
            break;

        case TABLE_CLEANING:
#ifndef NOLAB
            //Table.level = TABLE_END;            
            ret_proc = TableCleaningColorSupply();
            if (ret_proc == PROC_OK) {
                Table.step = STEP_0;
                // Not Punctual Cleaning
                if ((indx_Clean != 0) && (!isColorCmdStopProcess()))
                    Table.level = TABLE_START;
                else
                    Table.level = TABLE_END;
            } else if (ret_proc == PROC_FAIL) {
                Clean_Activation = OFF;
                Table.level = TABLE_ERROR;
            }
#else            
            Table.level = TABLE_END;
#endif            
            break;

        case TABLE_POSITIONING:
#ifndef NOLAB            
            ret_proc = TablePositioningColorSupply();
            if (ret_proc == PROC_OK) {
                Num_Table_Error = 0;
                if (TintingAct.PanelTable_state == OPEN) {
                    End_Table_Position = 1;
                    Table.level = TABLE_END;
                } else
                    Table.level = TABLE_END;
            } else if (ret_proc == PROC_FAIL) {
                if ((Num_Table_Error < MAX_TABLE_ERROR) && (Table.errorCode == TINTING_TABLE_MOVE_ERROR_ST)) {
                    Num_Table_Error++;
                    NextTable.level = Table.level;
                    Table.step = STEP_0;
                    Table.level = TABLE_GO_REFERENCE;
                } else
                    Table.level = TABLE_ERROR;
            }
#else            
            Table.level = TABLE_END;
#endif            
            break;

        case TABLE_STIRRING:
#ifndef NOLAB            
            ret_proc = TableStirring();
            if (ret_proc == PROC_OK) {
                if (isColorCmdStopProcess())
                    Table.level = TABLE_END;
                else {
                    Table.step = STEP_0;
                    NextTable.level = Table.level;
                    Table.level = TABLE_GO_REFERENCE;
                }
            }
            else if (ret_proc == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_GO_REFERENCE;
#endif                        
            break;

        case TABLE_STOP_STIRRING:
#ifndef NOLAB            
            ret_proc = TableHomingColorSupply();
            if (ret_proc == PROC_OK)
                Table.level = TABLE_END;
            else if (ret_proc == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_END;
#endif            
            break;

        case TABLE_STEPS_POSITIONING:
#ifndef NOLAB                        
            ret_proc = TableStepsPositioningColorSupply();
            if (ret_proc == PROC_OK)
                Table.level = TABLE_END;
            else if (ret_proc == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_END;
#endif                        
            break;

        case TABLE_GO_REFERENCE:
#ifndef NOLAB                                    
            ret_proc = TableGoToReference();
            if (ret_proc == PROC_OK) {
                if (NextTable.level != TABLE_POSITIONING) {
                    End_Table_Position = 0;
                    Table.level = TABLE_END;
                } else {
                    Table.step = STEP_0;
                    Table.level = TABLE_POSITIONING;
                }
            } else if (ret_proc == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_END;
#endif                                    
            break;

        case TABLE_SELF_RECOGNITION:
#ifndef NOLAB            
            ret_proc = TableSelfRecognitionColorSupply();
            if (ret_proc == PROC_OK)
                Table.level = TABLE_END;
            else if (ret_proc == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_END;
#endif                                                
            break;

        case TABLE_FIND_REFERENCE:
#ifndef NOLAB            
            ret_proc = TableGoToReference();
            if (ret_proc == PROC_OK)
                Table.level = TABLE_END;
            else if (ret_proc == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_END;
#endif                                                        
            break;

        case TABLE_TEST:
#ifndef NOLAB            
            ret_proc = TableTestColorSupply();
            if (ret_proc == PROC_OK)
                Table.level = TABLE_END;
            else if (ret_proc == PROC_FAIL)
                Table.level = TABLE_ERROR;
#else            
            Table.level = TABLE_END;
#endif                                                            
            break;

        case TABLE_END:
            Table_Motors = OFF;
            if ((Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                    (Status.level != TINTING_TABLE_SEARCH_HOMING_ST) && (Status.level != TINTING_TABLE_POSITIONING_ST) &&
                    (Status.level != TINTING_TABLE_CLEANING_ST) && (Status.level != TINTING_TABLE_SELF_RECOGNITION_ST) &&
                    (Status.level != TINTING_TABLE_TEST_ST) && (Status.level != TINTING_TABLE_GO_REFERENCE_ST) &&
                    (Status.level != TINTING_TABLE_STEPS_POSITIONING_ST) && (Status.level != TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) &&
                    (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_HOMING_ST) && (Status.level != TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_TABLE_HOMING_ST) &&
                    (Status.level != TINTING_SETUP_OUTPUT_TABLE_ST) &&
                    (Status.level != TINTING_TABLE_FIND_REFERENCE_ST) &&
                    (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_NOT_ENGAGED_ST) && (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_HOMING_ST))
                Table.level = TABLE_START;
            break;

        case TABLE_ERROR:
            Table_Motors = OFF;
            if ((Status.level != TINTING_SUPPLY_RUN_ST) && (Status.level != TINTING_STANDBY_RUN_ST) &&
                    (Status.level != TINTING_TABLE_SEARCH_HOMING_ST) && (Status.level != TINTING_TABLE_POSITIONING_ST) &&
                    (Status.level != TINTING_TABLE_CLEANING_ST) && (Status.level != TINTING_TABLE_SELF_RECOGNITION_ST) &&
                    (Status.level != TINTING_TABLE_TEST_ST) && (Status.level != TINTING_TABLE_GO_REFERENCE_ST) &&
                    (Status.level != TINTING_TABLE_STEPS_POSITIONING_ST) && (Status.level != TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST) &&
                    (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_HOMING_ST) && (Status.level != TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_TABLE_HOMING_ST) &&
                    (Status.level != TINTING_SETUP_OUTPUT_TABLE_ST) &&
                    (Status.level != TINTING_TABLE_FIND_REFERENCE_ST) &&
                    (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_NOT_ENGAGED_ST) && (Status.level != TINTING_PHOTO_LIGHT_VALVE_SEARCH_HOMING_ST))
                Table.level = TABLE_START;
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

unsigned char AnalyzeTableParameters(void) {
    // Check Maximum Revolution Step
    if (TintingAct.Steps_Revolution > ((unsigned long) STEPS_REVOLUTION + (unsigned long) STEPS_TOLERANCE_REVOLUTION))
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
    if ((TintingAct.Steps_Revolution <= TintingAct.Steps_Tolerance_Revolution) ||
            (TintingAct.Steps_Reference <= TintingAct.Steps_Tolerance_Reference))
        //         (TintingAct.Steps_Circuit <= TintingAct.Steps_Tolerance_Circuit) )   
        return FALSE;

    return TRUE;
}

/*
 *//*=====================================================================*//**
**      @brief Analyze Clean parameter received
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/

unsigned char AnalyzeCleanParameters(void) {
    // NO Clean on Bases
    if (TintingClean.Cleaning_Colorant_Mask[0] > 1)
        return FALSE;
#ifndef CLEANING_AFTER_DISPENSING        
    else if ((TintingClean.Cleaning_Colorant_Mask[1] > 0) || (TintingClean.Cleaning_Colorant_Mask[2] > 0)) {
        // Clean Period has to be > 0 if at least 1 Colorant has to be cleaned
        if (TintingClean.Cleaning_pause == 0)
            return FALSE;
    }
#endif    
        // If a Duration is setted at least 1 circuit has to be present
    else if ((TintingAct.Cleaning_duration != 0) && (TintingClean.Cleaning_Colorant_Mask[1] == 0) && (TintingClean.Cleaning_Colorant_Mask[2] == 0))
        return FALSE;

    return TRUE;
}

/*
 *//*=====================================================================*//**
**      @brief Check if During Table Process there is some external error condition
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/

unsigned char CheckTableErrorCondition(void) {
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);
    // Check for Motor Table Error
    if (Status_Board_Table.Bit.OCD == 0) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
        return PROC_FAIL;
    } else if (Status_Board_Table.Bit.UVLO == 0) { //|| (Status_Board_Table.Bit.UVLO_ADC == 0) ) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
        return PROC_FAIL;
    } else if ((Status_Board_Table.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Table.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE)) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
        return PROC_FAIL;
    } else if ((PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    } else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
//    } else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
    } else if (CouplingPhotocell_sts == DARK) {        
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
        step_error = 5;                
        return PROC_FAIL;
    } else if ((Table.level != TABLE_HOMING) && (Table.level != TABLE_STOP_STIRRING) && (Table.level != TABLE_STIRRING) && (Table.level != TABLE_CLEANING) &&
            (TintingAct.Cleaning_duration != 0) && (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT)) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    } else
        return PROC_RUN;
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

unsigned char TableHomingColorSupply(void) {
    unsigned char ret = PROC_RUN;
    static unsigned char Find_Circuit, Tr_Dark_Light, Tr_Light_Dark, Old_Photocell_sts, New_Photocell_sts, Carriage_Open, Panel_Open;
    static signed long Position_Steps, Steps_Todo, Count_Steps, Max_Count_Steps, Reference_Position_Step;
    static unsigned char Find_circuit_n, Wait;
    unsigned char i, Total_circuit_enabled;
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    if (CheckTableErrorCondition() == PROC_FAIL) {
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        StopTimer(T_WAIT_BRUSH_ON);
        StopTimer(T_WAIT_GENERIC24V_TIME);
        TintingAct.Brush_state = OFF;
        SPAZZOLA_OFF();
        return PROC_FAIL;
    }
    // Table Panel OPEN
    if ((Table.level != TABLE_POSITIONING) && (TintingAct.PanelTable_state == OPEN)) {
        if (TintingAct.Brush_state == OFF) {
            StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            StopTimer(T_WAIT_BRUSH_ON);
            StopTimer(T_WAIT_GENERIC24V_TIME);
            StopTimer(T_TABLE_WAITING_TIME);
            SPAZZOLA_OFF();
            Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
            return PROC_FAIL;
        } else
            Panel_Open = TRUE;
    }
        // Bases Carriage Open      
    else if (TintingAct.BasesCarriageOpen == OPEN) {
        if (TintingAct.Brush_state == OFF) {
            SoftStopStepper(MOTOR_TABLE);
            StopTimer(T_TABLE_WAITING_TIME);
            StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            StopTimer(T_WAIT_BRUSH_ON);
            StopTimer(T_WAIT_GENERIC24V_TIME);
            SPAZZOLA_OFF();
            Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
            return PROC_FAIL;
        } else
            Carriage_Open = TRUE;
    }
    if (isColorCmdStopProcess()) {
        StopTimer(T_TABLE_WAITING_TIME);
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        TintingAct.Brush_state = OFF;
        SPAZZOLA_OFF();
        StopTimer(T_WAIT_BRUSH_ON);
        StopTimer(T_WAIT_GENERIC24V_TIME);
        Table.step = STEP_12;
    }
    //----------------------------------------------------------------------------
    switch (Table.step) {
            // -----------------------------------------------------------------------------     
            // Starts operations
        case STEP_0:
            SetStepperHomePosition(MOTOR_TABLE);
            TintingAct.Last_Cmd_Reset = OFF;
            Reference = REFERENCE_STEP_0;
            StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            Find_Circuit = OFF;
            Tr_Light_Dark = OFF;
            Tr_Dark_Light = OFF;
            Max_Count_Steps = 0;
            Status.errorCode = 0;
            Find_circuit_n = 0;
            Wait = FALSE;
            Panel_Open = FALSE;
            Carriage_Open = FALSE;
            //TintingAct.High_Speed_Rotating_Table = 160;        
            //TintingAct.Cleaning_duration = 0;        
            //SPAZZOLA_OFF();        
            // Cleaning Process is setted: it is necessary to Reset Brush position before to start moving Table
            if (TintingAct.Cleaning_duration != 0) {
                StopStepper(MOTOR_TABLE);
                SPAZZOLA_ON();
                StopTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = ON;
                Table.step = STEP_13;
                return ret;
            }
            else {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
                    Table.step++;
                else {
                    Steps_Todo = TintingAct.Steps_Revolution;
                    // Table CW rotation
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                    Table.step += 3;
                }
            }
            break;

            // Wait for DARK-LIGHT Table PhotoStatus_Board_Valvecell transition 
        case STEP_1:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                Old_Photocell_sts = LIGHT;
                SetStepperHomePosition(MOTOR_TABLE);
                Steps_Todo = STEP_PHOTO_TABLE_OFFSET;
                // Table CW rotation
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
                // Table Photocell never in Light status in one full Table Rotation
            else if ((signed long) GetStepperPosition(MOTOR_TABLE) <= (-(signed long) (TintingAct.Steps_Revolution))) {
                Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                return PROC_FAIL;
            }
            break;

            // Starts a full Table Rotation to find Reference circuit    
        case STEP_2:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                    return PROC_FAIL;
                } else {
                    SetStepperHomePosition(MOTOR_TABLE);
                    Steps_Todo = TintingAct.Steps_Revolution;
                    // Table CW rotation
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                    Table.step++;
                }
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
            if ((Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK)) {
                Position_Steps = (signed long) GetStepperPosition(MOTOR_TABLE);
                Count_Steps = (signed long) GetStepperPosition(MOTOR_TABLE);
                Tr_Light_Dark = ON;
            }// Transition DARK-LIGHT ?        
            else if ((Old_Photocell_sts == DARK) && (New_Photocell_sts == LIGHT)) {
                if (Tr_Light_Dark == ON)
                    Tr_Dark_Light = ON;
            }

            Old_Photocell_sts = New_Photocell_sts;

            if ((Tr_Light_Dark == ON) && (Tr_Dark_Light == ON)) {
                Tr_Light_Dark = OFF;
                Tr_Dark_Light = OFF;
                Find_Circuit = ON;
                Find_circuit_n++;
                Count_Steps = -(signed long) ((signed long) GetStepperPosition(MOTOR_TABLE) - (signed long) Count_Steps);
                if (Count_Steps > Max_Count_Steps) {
                    Max_Count_Steps = Count_Steps;
                    Reference_Position_Step = Position_Steps;
                }
            }
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (Find_Circuit == OFF) {
                    Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                    return PROC_FAIL;
                } else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                    return PROC_FAIL;
                } else {
                    StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                    // Reference Circuit Find
                    Table.step++;
                }
            }
            break;

            //  Check if Reference Circuit is correct
        case STEP_4:
            if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                // Reference Circuit found is correct
                if (((Max_Count_Steps >= TintingAct.Steps_Reference) && ((Max_Count_Steps - TintingAct.Steps_Reference) <= TintingAct.Steps_Tolerance_Reference)) ||
                        ((Max_Count_Steps < TintingAct.Steps_Reference) && ((TintingAct.Steps_Reference - Max_Count_Steps) <= TintingAct.Steps_Tolerance_Reference)))
                    Table.step++;
                    // Reference Circuit is NOT correct
                else {
                    Table.errorCode = TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST;
                    return PROC_FAIL;
                }
            }
            break;

            // Starts Go Homing = Reference position
        case STEP_5:
            if ((Reference_Position_Step - (signed long) GetStepperPosition(MOTOR_TABLE)) > TintingAct.Steps_Revolution / 2) {
                // Shortest way is a CW rotation
                Steps_Todo = -Reference_Position_Step - STEP_PHOTO_TABLE_OFFSET;
                Table.step++;
            } else {
                // Shortest way is a CCW rotation
                Steps_Todo = (signed long) GetStepperPosition(MOTOR_TABLE) - Reference_Position_Step + STEP_PHOTO_TABLE_OFFSET;
                Table.step += 4;
            }
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Wait = FALSE;
            break;
            // -----------------------------------------------------------------------------    
            // CW Rotation
            // Go Homing = Reference in CW rotation
        case STEP_6:
            if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == FALSE)) {
                Wait = TRUE;
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            } else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == TRUE)) {
                if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                    StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                    Wait = FALSE;
                    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                        Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                        return PROC_FAIL;
                    } else {
                        // Rotate CW motor Table till Photocell transition DARK-LIGHT
                        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step++;
                    }
                }
            }
            break;

        case STEP_7:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // TABLE Motor
                SetStepperHomePosition(MOTOR_TABLE);
                StopTimer(T_TABLE_WAITING_TIME);
                Steps_Todo = TintingAct.Steps_Reference / 2;
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            } else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

        case STEP_8:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // Check if Photocell is covered
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                    Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                    return PROC_FAIL;
                } else {
                    // Table positioned at the centre of Reference Circuit
                    SetStepperHomePosition(MOTOR_TABLE);
                    TintingAct.Last_Cmd_Reset = ON;
                    Set_Home_pos = 0;
                    Tr_Light_Dark_1 = OFF;
                    Old_Photocell_sts_1 = DARK;
                    Total_circuit_enabled = 0;
                    for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                        if (TintingAct.Table_Colorant_En[i] == TRUE)
                            Total_circuit_enabled++;
                    }
                    if (((Total_circuit_n != (Find_circuit_n - 1)) || (Total_circuit_enabled != (Find_circuit_n - 1))) && (Table_circuits_pos == ON)) {
                        SoftStopStepper(MOTOR_TABLE);
                        Status.errorCode = 0;
                        Table.errorCode = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        return PROC_FAIL;
                    } else {
                        Table.step += 4;
                    }
                }
            }
            break;
            // -----------------------------------------------------------------------------
            // CCW Rotation    
            // Go Homing = Reference in CCW rotation
        case STEP_9:
            if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == FALSE)) {
                Wait = TRUE;
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            } else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == TRUE)) {
                if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                    StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                    Wait = FALSE;
                    if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                        Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                        return PROC_FAIL;
                    } else {
                        // Rotate CCW motor Table till Photocell transition DARK-LIGHT
                        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step++;
                    }
                }
            }
            break;

        case STEP_10:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // TABLE Motor
                SetStepperHomePosition(MOTOR_TABLE);
                StopTimer(T_TABLE_WAITING_TIME);
                // Shortest way is a CCW rotation
                Steps_Todo = -(signed long) (TintingAct.Steps_Reference / 2);
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            } else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

        case STEP_11:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // Check if Photocell is covered
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                    Table.errorCode = TINTING_TABLE_HOMING_ERROR_ST;
                    return PROC_FAIL;
                } else {
                    // Table positioned at the centre of Reference Circuit
                    SetStepperHomePosition(MOTOR_TABLE);
                    TintingAct.Last_Cmd_Reset = ON;
                    Set_Home_pos = 0;
                    Tr_Light_Dark_1 = OFF;
                    Old_Photocell_sts_1 = DARK;
                    Total_circuit_enabled = 0;
                    for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                        if (TintingAct.Table_Colorant_En[i] == TRUE)
                            Total_circuit_enabled++;
                    }
                    if (((Total_circuit_n != (Find_circuit_n - 1)) || (Total_circuit_enabled != (Find_circuit_n - 1))) && (Table_circuits_pos == ON)) {
                        SoftStopStepper(MOTOR_TABLE);
                        Status.errorCode = 0;
                        Table.errorCode = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        return PROC_FAIL;
                    } else {
                        Table.step++;
                    }
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
            StopStepper(MOTOR_TABLE);
            TintingAct.RotatingTable_state = OFF;
            Num_Table_Error = 0;
            ret = PROC_OK;
            break;

            // -----------------------------------------------------------------------------
            // Brush Photocell control
        case STEP_13:
            if (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT) {
                StopTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_BRUSH_ON);
                Table.step++;
            } 
            else if (StatusTimer(T_WAIT_BRUSH_ON) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ON);
                StopTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = OFF;
                SPAZZOLA_OFF();
                Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
                ret = PROC_FAIL;
            }
            break;

        case STEP_14:
            if (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == DARK) {
                StopTimer(T_WAIT_BRUSH_ON);
                StopTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = OFF;
                SPAZZOLA_OFF();
                if (Panel_Open == TRUE) {
                    Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
                    return PROC_FAIL;
                } else if (Carriage_Open == TRUE) {
                    Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
                    return PROC_FAIL;
                }
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
                    Table.step = STEP_1;
                else {
                    Steps_Todo = TintingAct.Steps_Revolution;
                    // Table CW rotation
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                    Table.step = STEP_3;
                }
            } 
            else if (StatusTimer(T_WAIT_BRUSH_ON) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ON);
                StopTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = OFF;
                SPAZZOLA_OFF();
                Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;
            // -----------------------------------------------------------------------------    
        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
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

unsigned char TableSelfRecognitionColorSupply(void) {
    unsigned char ret = PROC_RUN;
    unsigned short i, j;
    static unsigned short circuit_id, circuit_id_ccw;
    static unsigned char Find_Circuit, Old_Photocell_sts, New_Photocell_sts;
    static signed long Steps_Todo;
    static signed long Circuit_step_temp[MAX_COLORANT_NUMBER];
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    if (CheckTableErrorCondition() == PROC_FAIL) {
        return PROC_FAIL;
    }
    // Bases Carriage Open    
    if (TintingAct.BasesCarriageOpen == OPEN) {
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        //        Table.step = STEP_18;
        Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
        return PROC_FAIL;
    }
    // Table Panel OPEN
    if ((Table.level != TABLE_POSITIONING) && (TintingAct.PanelTable_state == OPEN)) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
        return PROC_FAIL;
    }

    if (isColorCmdStopProcess()) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.step = STEP_18;
    }
    //----------------------------------------------------------------------------
    switch (Table.step) {
            // -----------------------------------------------------------------------------     
            // Starts operations
        case STEP_0:
            SetStepperHomePosition(MOTOR_TABLE);
            Reference = REFERENCE_STEP_0;
            Find_Circuit = OFF;
            circuit_id = 0;
            Status.errorCode = 0;
            // Clear alarm error code 
            resetAlarm();

            if ((signed long) GetStepperPosition(MOTOR_TABLE) != 0) {
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;
            }
            else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;
            }
            else if (TintingAct.Last_Cmd_Reset == OFF) {
                Total_circuit_n = 0;
                Table_circuits_pos = OFF;
                Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    CircStepPosAct.Circ_Pos[i] = 0;
                    TintingAct.Circuit_step_pos[i] = 0;
                    TintingAct.Circuit_step_pos_cw[i] = 0;
                    TintingAct.Circuit_step_pos_ccw[i] = 0;
                    Circuit_step_temp[i] = 0;
                }
                Table.step += 2;
                Old_Photocell_sts = DARK;
            }
            break;

            // Starts a full CW Table Rotation   
        case STEP_2:
            Steps_Todo = TintingAct.Steps_Revolution - STEP_PHOTO_TABLE_OFFSET;
            // Table CW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            //        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step++;
            break;

            // Self Recognition in CW direction
        case STEP_3:
            // Update Photocell Status
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
                New_Photocell_sts = DARK;
            else
                New_Photocell_sts = LIGHT;

            // Transition LIGHT-DARK ?
            if ((Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK)) {
                Find_Circuit = ON;
                Circuit_step_temp[circuit_id] = (signed long) GetStepperPosition(MOTOR_TABLE);
                if (Circuit_step_temp[circuit_id] < 0) {
                    Circuit_step_temp[circuit_id] = (-Circuit_step_temp[circuit_id]) % TintingAct.Steps_Revolution;
                    if (Circuit_step_temp[circuit_id] != 0)
                        Circuit_step_temp[circuit_id] = TintingAct.Steps_Revolution - (Circuit_step_temp[circuit_id] + (TintingAct.Steps_Circuit / 2));
                } else
                    Circuit_step_temp[circuit_id] = Circuit_step_temp[circuit_id] % TintingAct.Steps_Revolution;
                circuit_id++;
            }

            Old_Photocell_sts = New_Photocell_sts;
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (Find_Circuit == OFF) {
                    for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                        CircStepPosAct.Circ_Pos[j] = 0;
                        TintingAct.Circuit_step_pos[j] = 0;
                    }
                    // Setup EEPROM writing variables 
                    eeprom_byte = 0;
                    Total_circuit_n = 0;
                    Table_circuits_pos = OFF;
                    Table.step = STEP_17;
                }
                else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    Total_circuit_n = 0;
                    Table_circuits_pos = OFF;
                    Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                    return PROC_FAIL;
                }
                else if ((circuit_id) > MAX_COLORANT_NUMBER) {
                    Total_circuit_n = 0;
                    Table_circuits_pos = OFF;
                    Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                    return PROC_FAIL;
                }
                else {
                    for (i = 0; i < circuit_id; i++)
                        TintingAct.Circuit_step_pos_cw[i] = Circuit_step_temp[circuit_id - 1 - i];

                    Total_circuit_n = circuit_id;
                    Table_circuits_pos = ON;
                    // Self Learning Procedure End in CW direction
                    Table.step++;
                }
            }
            break;

            // Re-alignement in Reference position
        case STEP_4:
            // Rotate CW motor Table till Photocell transition DARK-LIGHT
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
            StartTimer(T_TABLE_WAITING_TIME);
            Table.step++;
            break;

            // Check for DARK-LIGHT transition
        case STEP_5:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                SetStepperHomePosition(MOTOR_TABLE);
                StopTimer(T_TABLE_WAITING_TIME);
                // Go to center position
                Steps_Todo = TintingAct.Steps_Reference / 2;
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
            else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

            // Table centered on Reference Circuit
        case STEP_6:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // Table positioned at the centre of Reference Circuit
                SetStepperHomePosition(MOTOR_TABLE);
                Table.step++;
            }
            break;

            // Starts a full CCW Table Rotation   
        case STEP_7:
            Old_Photocell_sts = DARK;
            circuit_id_ccw = 0;
            Steps_Todo = -TintingAct.Steps_Revolution + STEP_PHOTO_TABLE_OFFSET;
            // Table CCW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            //        MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step++;
            break;

            // Self Recognition in CCW direction
        case STEP_8:
            // Update Photocell Status
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
                New_Photocell_sts = DARK;
            else
                New_Photocell_sts = LIGHT;

            // Transition LIGHT-DARK ?
            if ((Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK)) {
                TintingAct.Circuit_step_pos_ccw[circuit_id_ccw] = (signed long) GetStepperPosition(MOTOR_TABLE) + (signed long) (TintingAct.Steps_Circuit / 2);
                circuit_id_ccw++;
                circuit_id--;
            }

            Old_Photocell_sts = New_Photocell_sts;

            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                    return PROC_FAIL;
                }
                else if (circuit_id > 0) {
                    Table.errorCode = TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST;
                    return PROC_FAIL;
                } else
                    // Self Learning Procedure End in CCW direction
                    Table.step++;
            }
            break;

            // Check if 'Circuit_step_pos[]' steps match between CW and CCW directions
        case STEP_9:
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                // Difference in Circuit position Steps between CW and CCW > tolerance --> Error
                if (((TintingAct.Circuit_step_pos_cw[i] > TintingAct.Circuit_step_pos_ccw[i]) && ((TintingAct.Circuit_step_pos_cw[i] - TintingAct.Circuit_step_pos_ccw[i]) >= TintingAct.Steps_Tolerance_Circuit)) ||
                        ((TintingAct.Circuit_step_pos_cw[i] <= TintingAct.Circuit_step_pos_ccw[i]) && ((TintingAct.Circuit_step_pos_ccw[i] - TintingAct.Circuit_step_pos_cw[i]) >= TintingAct.Steps_Tolerance_Circuit))) {
                    Total_circuit_n = 0;
                    Table_circuits_pos = OFF;
                    Status.errorCode = i;
                    SoftHiZ_Stepper(MOTOR_TABLE);
                    Table.errorCode = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                    return PROC_FAIL;
                }
            }
            for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                TintingAct.Circuit_step_pos[i] = (TintingAct.Circuit_step_pos_cw[i] + TintingAct.Circuit_step_pos_ccw[i]) / 2;
            Table.step++;
            break;

            // Check if 'Circuit_step_pos[i]' steps matches 'Circuit_step_theorical_pos[j]
        case STEP_10:
            for (i = 0; i < Total_circuit_n; i++) {
                Find_Circuit = OFF;
                for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                    if (((TintingAct.Circuit_step_theorical_pos[j] > TintingAct.Circuit_step_pos[i]) && ((TintingAct.Circuit_step_theorical_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit)) ||
                            ((TintingAct.Circuit_step_theorical_pos[j] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - TintingAct.Circuit_step_theorical_pos[j]) <= TintingAct.Steps_Tolerance_Circuit))) {
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
                SoftHiZ_Stepper(MOTOR_TABLE);
                Table.errorCode = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                return PROC_FAIL;
            }
            Table.step++;
            break;

            // Create the final Circuit position map
        case STEP_11:
            for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                Find_Circuit = FALSE;
                for (i = 0; i < Total_circuit_n; i++) {
                    if (((TintingAct.Circuit_step_theorical_pos[j] > TintingAct.Circuit_step_pos[i]) && ((TintingAct.Circuit_step_theorical_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit)) ||
                            ((TintingAct.Circuit_step_theorical_pos[j] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - TintingAct.Circuit_step_theorical_pos[j]) <= TintingAct.Steps_Tolerance_Circuit))) {
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
            if (TintingAct.Table_Step_position == DYNAMIC)
                Table.step++;
                // STATIC circuit position model: tabel with possible zeros
            else {
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    TintingAct.Circuit_step_pos[i] = Circuit_step_temp[i];
                }
                Table.step++;
            }
            break;

            // Check if there is a full match between positiong Table found and enabled circuit table 
        case STEP_12:
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if (TintingAct.Table_Colorant_En[i] == TRUE) {
                    if (Circuit_step_temp[i] == 0) {
                        for (j = 0; j < MAX_COLORANT_NUMBER; j++)
                            TintingAct.Circuit_step_pos[j] = 0;
                        SoftHiZ_Stepper(MOTOR_TABLE);
                        Table.errorCode = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        Status.errorCode = i;
                        return PROC_FAIL;
                    }
                } else if (TintingAct.Table_Colorant_En[i] == FALSE) {
                    if (Circuit_step_temp[i] != 0) {
                        for (j = 0; j < MAX_COLORANT_NUMBER; j++)
                            TintingAct.Circuit_step_pos[j] = 0;
                        SoftHiZ_Stepper(MOTOR_TABLE);
                        Table.errorCode = TINTING_TABLE_MISMATCH_POSITION_ERROR_ST;
                        Status.errorCode = i;
                        return PROC_FAIL;
                    }
                }
            }
            Table.step++;
            break;

            // Re-alignement in Reference position
        case STEP_13:
            // Rotate CCW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
            StartTimer(T_TABLE_WAITING_TIME);
            Table.step++;
            break;

            // Check for DARK-LIGHT transition
        case STEP_14:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                StopStepper(MOTOR_TABLE);
                StopTimer(T_TABLE_WAITING_TIME);
                SetStepperHomePosition(MOTOR_TABLE);
                // Go to center position
                Steps_Todo = -(signed long) (TintingAct.Steps_Reference / 2);
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
            else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

            // Table centered on Reference Circuit
        case STEP_15:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // Table positioned at the centre of Reference Circuit 
                SetStepperHomePosition(MOTOR_TABLE);
                Table.step++;
            }
            break;

            // -----------------------------------------------------------------------------
            // Write Circuits Steps postion CW and CCW on EEprom memory
        case STEP_16:
            // Setup EEPROM writing variables 
            eeprom_byte = 0;
            for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                CircStepPosAct.Circ_Pos[i] = TintingAct.Circuit_step_pos[i];

            Table.step++;
            //Table.step +=2 ;
            break;

        case STEP_17:
            StopStepper(MOTOR_TABLE);
            eeprom_write_result = updateEECirStepsPos();
            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                updateEEParamCirStepsPosCRC();
                read_eeprom = checkEEprom();
                if (InitFlags.CRCCircuitStepsPosFailed == FALSE)
                    EEprom_Crc_Error = 0;
                else
                    EEprom_Crc_Error = 1;
                Table.step++;
            } else if (eeprom_write_result == EEPROM_WRITE_FAILED) {
                Table.errorCode = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
                return PROC_FAIL;
            }
            break;
            // -----------------------------------------------------------------------------    

        case STEP_18:
            StopStepper(MOTOR_TABLE);
            TintingAct.Last_Cmd_Reset = OFF;
            ret = PROC_OK;
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
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

unsigned char TableGoToReference(void) {
    unsigned char ret = PROC_RUN;
    static unsigned short direction;
    static signed long Steps_Todo, Steps_Position;
    unsigned char currentReg, i;
    //  static signed long Steps_Pos;
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    if (CheckTableErrorCondition() == PROC_FAIL) {
        return PROC_FAIL;
    }
    // Bases Carriage Open    
    if (TintingAct.BasesCarriageOpen == OPEN) {
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        //        Table.step = STEP_6;
        Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
        return PROC_FAIL;
    }
    if ((Table.level != TABLE_POSITIONING) && (TintingAct.PanelTable_state == OPEN)) {
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
        return PROC_FAIL;
    }
    else if (ManageTableHomePosition() == FALSE) {
        // Loss of Steps  
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }
    //----------------------------------------------------------------------------
    switch (Table.step) {
            // Starts operations
        case STEP_0:
            Status.errorCode = 0;
            Reference = REFERENCE_STEP_0;
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            //        ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                          
            currentReg = HOLDING_CURRENT_TABLE * 100 / 156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);

            // Read current position
            Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);
            if (Steps_Position < 0) {
                Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
                if (Steps_Position != 0)
                    Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
            } else
                Steps_Position = Steps_Position % TintingAct.Steps_Revolution;

            StopTimer(T_DELAY_START_TABLE_MOTOR);
            StartTimer(T_DELAY_START_TABLE_MOTOR);
            Table.step++;
            break;

            // Start Table Rotation towards Reference
        case STEP_1:
            if (StatusTimer(T_DELAY_START_TABLE_MOTOR) == T_ELAPSED) {
                StopTimer(T_DELAY_START_TABLE_MOTOR);
                if ((Steps_Position) < (signed long) (TintingAct.Steps_Revolution / 2)) {
                    direction = CW;
                    Steps_Todo = Steps_Position - STEP_PHOTO_TABLE_OFFSET;
                    // 'Steps_Todo' has to be > 0            
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                    //                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                    Table.step++;
                }
                else if ((Steps_Position) >= (signed long) (TintingAct.Steps_Revolution / 2)) {
                    direction = CCW;
                    Steps_Todo = -(signed long) (TintingAct.Steps_Revolution - (Steps_Position) - STEP_PHOTO_TABLE_OFFSET);
                    // 'Steps_Todo' has to be < 0
                    MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                    //                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                    Table.step++;
                }
                else {
                    Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                    return PROC_FAIL;
                }
            }
            break;

        case STEP_2:
            if ((direction == CW) && (Status_Board_Table.Bit.MOT_STATUS == 0)) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                    return PROC_FAIL;
                }
                // Rotate CW motor Table till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                StartTimer(T_TABLE_WAITING_TIME);
                Table.step++;
            }
            else if ((direction == CCW) && (Status_Board_Table.Bit.MOT_STATUS == 0)) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                    return PROC_FAIL;
                }
                // Rotate CCW motor Table till Photocell transition LIGHT-DARK
                StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                StartTimer(T_TABLE_WAITING_TIME);
                Table.step++;
            }
            break;

            // Check for LIGHT-DARK transition
        case STEP_3:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                StopTimer(T_TABLE_WAITING_TIME);
                if (direction == CW)
                    Steps_Todo = TintingAct.Steps_Reference + STEP_PHOTO_TABLE_OFFSET;
                else
                    Steps_Todo = -(signed long) TintingAct.Steps_Reference - (signed long) STEP_PHOTO_TABLE_OFFSET;

                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
            else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

        case STEP_4:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (direction == CW)
                    Steps_Todo = -(signed long) (TintingAct.Steps_Reference / 2) -(signed long) STEP_PHOTO_TABLE_OFFSET;
                else
                    Steps_Todo = TintingAct.Steps_Reference / 2 + STEP_PHOTO_TABLE_OFFSET;

                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
            break;

            // Waiting to be placed at the center of 'Reference' position 
        case STEP_5:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // Check if Photocell is covered
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                    Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                    return PROC_FAIL;
                }
                else {
                    Set_Home_pos = 0;
                    Tr_Light_Dark_1 = OFF;
                    Old_Photocell_sts_1 = DARK;
                    Table.step++;
                }
            }
            break;

        case STEP_6:
            if (Table_circuits_pos == ON) {
                for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                    TintingAct.Circuit_step_pos[i] = CircStepPosAct.Circ_Pos[i];
            }
            TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0];
            StopStepper(MOTOR_TABLE);
            ret = PROC_OK;
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
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

unsigned char TableStirring(void) {
    unsigned char ret = PROC_RUN;
    unsigned char Dir_Neg;
    unsigned char currentReg;
    static signed long Steps_Todo, Steps_Position;
    static unsigned short contatore_stirring;
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    if (CheckTableErrorCondition() == PROC_FAIL) {
        StopTimer(T_TIMEOUT_STIRRING);
        StopTimer(T_WAIT_BRUSH_ON);
        StopTimer(T_WAIT_GENERIC24V_TIME);
        TintingAct.Brush_state = OFF;
        SPAZZOLA_OFF();
        Stirr_After_Last_Ricirc = FALSE;
        return PROC_FAIL;
    }
    // Bases Carriage Open    
    if (TintingAct.BasesCarriageOpen == OPEN) {
        if (TintingAct.Brush_state == OFF) {
            SoftStopStepper(MOTOR_TABLE);
            StopTimer(T_TABLE_WAITING_TIME);
            StopTimer(T_TIMEOUT_STIRRING);
            StopTimer(T_WAIT_BRUSH_ON);
            StopTimer(T_WAIT_GENERIC24V_TIME);
            SPAZZOLA_OFF();
            //Table.step = STEP_3;
            Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
            Stirr_After_Last_Ricirc = FALSE;
            return PROC_FAIL;
        }
    }
    // Table Panel OPEN
    if ((Table.level != TABLE_POSITIONING) && (TintingAct.PanelTable_state == OPEN)) {
        if (TintingAct.Brush_state == OFF) {
            SoftStopStepper(MOTOR_TABLE);
            StopTimer(T_TABLE_WAITING_TIME);
            StopTimer(T_TIMEOUT_STIRRING);
            StopTimer(T_WAIT_BRUSH_ON);
            StopTimer(T_WAIT_GENERIC24V_TIME);
            SPAZZOLA_OFF();
            Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
            Stirr_After_Last_Ricirc = FALSE;
            return PROC_FAIL;
        }
    }
    else if ((TintingAct.Cleaning_duration != 0) && (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT) && (TintingAct.Brush_state == OFF)) {
        StopTimer(T_TABLE_WAITING_TIME);
        StopTimer(T_TIMEOUT_STIRRING);
        StopTimer(T_WAIT_BRUSH_ON);
        StopTimer(T_WAIT_GENERIC24V_TIME);
        TintingAct.Brush_state = OFF;
        SPAZZOLA_OFF();
        Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
        Stirr_After_Last_Ricirc = FALSE;
        return PROC_FAIL;
    }
    else if (ManageTableHomePosition() == FALSE) {
        // Loss of Steps  
        StopTimer(T_TABLE_WAITING_TIME);
        StopTimer(T_TIMEOUT_STIRRING);
        StopTimer(T_WAIT_BRUSH_ON);
        StopTimer(T_WAIT_GENERIC24V_TIME);
        TintingAct.Brush_state = OFF;
        SPAZZOLA_OFF();
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        Stirr_After_Last_Ricirc = FALSE;
        return PROC_FAIL;
    }
    //--------------------------------------------------------------------------
    switch (Table.step) {
            // ---------------------------------------------------------------------    
            // Starts operations
        case STEP_0:
            Status.errorCode = 0;
            contatore_stirring = 0;
            Reference = REFERENCE_STEP_0;
            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            //        ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                          
            currentReg = HOLDING_CURRENT_TABLE * 100 / 156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
            // If STIRRING after Last Ricirculation AND Cleaning process Active --> Activate BRUSH 
            if ((Stirr_After_Last_Ricirc == TRUE) && (TintingAct.Cleaning_duration > 0)) {
                Stirr_After_Last_Ricirc = FALSE;
                StopTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_BRUSH_ON);
                StopTimer(T_WAIT_GENERIC24V_TIME);
                StartTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = ON;
                SPAZZOLA_ON();
                Table.step = STEP_5;
            } 
            else if (RicirculationCmd == 1)
                Table.step++;
            else {
                StopTimer(T_TIMEOUT_STIRRING);
                StartTimer(T_TIMEOUT_STIRRING);
                Table.step++;
            }
            break;

        case STEP_1:
            // Read current position
            Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);

            // Error Calculation
            if (Steps_Position < 0) {
                Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
                Dir_Neg = TRUE;
            } else {
                Steps_Position = Steps_Position % TintingAct.Steps_Revolution;
                Dir_Neg = FALSE;
            }
            if (Dir_Neg == FALSE)
                Steps_Todo = -TintingAct.Steps_Revolution;
            else
                Steps_Todo = TintingAct.Steps_Revolution;

            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step++;
            break;

        case STEP_2:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (RicirculationCmd == 1) {
                    contatore_stirring++;
                    if (contatore_stirring < TintingAct.Steps_Stirring)
                        Table.step--;
                    else {
                        RicirculationCmd = 0;
                        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                        StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                        Table.step++;
                    }
                } else
                    Table.step--;
            }                // Stop Stirring
            else if (isColorCmdStop() && (RicirculationCmd == 0)) {
                StopTimer(T_TIMEOUT_STIRRING);
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                //                StopStepper(MOTOR_TABLE);
                SoftStopStepper(MOTOR_TABLE);
                Table.step++;
            }
                // Stop Stirring
            else if (isColorCmdStopProcess()) {
                StopTimer(T_TIMEOUT_STIRRING);
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                //                StopStepper(MOTOR_TABLE);
                SoftStopStepper(MOTOR_TABLE);
                Table.step++;
            }
            else if (StatusTimer(T_TIMEOUT_STIRRING) == T_ELAPSED) {
                StopTimer(T_TIMEOUT_STIRRING);
                //StopStepper(MOTOR_TABLE);
                SoftStopStepper(MOTOR_TABLE);
                Table.step += 2;
            }
            break;

        case STEP_3:
            if ((StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) && (Status_Board_Table.Bit.MOT_STATUS == 0)) {
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                ret = PROC_OK;
            }
            break;

        case STEP_4:
            if ((isColorCmdStop() || isColorCmdStopProcess()) && (RicirculationCmd == 0)) {
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                Table.step -= 1;
            }
            break;

        case STEP_5:
            if (StatusTimer(T_WAIT_BRUSH_ON) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_BRUSH_ON);
                Table.step++;
            }
            break;

        case STEP_6:
            if (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT) {
                StopTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_BRUSH_ON);
                Table.step++;
            }
            else if (StatusTimer(T_WAIT_BRUSH_ON) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ON);
                StopTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = OFF;
                SPAZZOLA_OFF();
                Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
                ret = PROC_FAIL;
            }
            break;

        case STEP_7:
            if (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == DARK) {
                StopTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = OFF;
                SPAZZOLA_OFF();
                Table.step = STEP_1;
            }
            else if (StatusTimer(T_WAIT_BRUSH_ON) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ON);
                StopTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Brush_state = OFF;
                SPAZZOLA_OFF();
                Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
                ret = PROC_FAIL;
            }
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
    }
    return ret;
}

/*/*=====================================================================*//**
**      @brief Check and Set table Home Position
**
**      @param void
**
**      @retval TRUE: Good Home Position - FALSE: Bad Home Position
**
*//*=====================================================================*//**
*/

unsigned char ManageTableHomePosition(void) {
    signed long Steps_Pos;
    static signed long Steps_Pos0 = 0;
    //    static unsigned char Set_Home_pos = 0;
    //    static unsigned char Old_Photocell_st = DARK, New_Photocell_sts = DARK;
    static unsigned char New_Photocell_sts = DARK;
    //    static unsigned char Tr_Light_Dark = OFF;
    unsigned char i;

    // Update Photocell Status
    if (PhotocellStatus(TABLE_PHOTOCELL, NO_FILTER) == DARK)
        New_Photocell_sts = DARK;
    else
        New_Photocell_sts = LIGHT;
    // Transition LIGHT-DARK ?
    if ((Old_Photocell_sts_1 == LIGHT) && (New_Photocell_sts == DARK))
        Tr_Light_Dark_1 = ON;
    else if (Set_Home_pos == 0)
        Tr_Light_Dark_1 = OFF;

    // Transition DARK-LIGHT ?        
    if ((Old_Photocell_sts_1 == DARK) && (New_Photocell_sts == LIGHT))
        Set_Home_pos = 0;

    Old_Photocell_sts_1 = New_Photocell_sts;

    Steps_Pos = (signed long) GetStepperPosition(MOTOR_TABLE);
    if (Steps_Pos > 0) {
        //----------------------------------------------------------------------
        if ((Set_Home_pos == 0) &&
                ((Steps_Pos >= (signed long) (TintingAct.Steps_Revolution - STEP_PHOTO_TABLE_OFFSET)) ||
                (Steps_Pos <= (signed long) (STEP_PHOTO_TABLE_OFFSET)))) {
            if (Tr_Light_Dark_1 == ON) {
                Set_Home_pos = 1;
                Steps_Pos0 = Steps_Pos;
                if (Reference == REFERENCE_STEP_1)
                    Reference = REFERENCE_STEP_2;
            }
            Table_Error = 0;
            return TRUE;
        }
        else if ((Set_Home_pos == 0) && (Steps_Pos >= TintingAct.Steps_Revolution))
            return FALSE;

        else if ((Set_Home_pos == 1) &&
                (((Steps_Pos - Steps_Pos0) >= (signed long) (TintingAct.Steps_Reference / 2)) ||
                ((Steps_Pos - Steps_Pos0) <= -(signed long) (TintingAct.Steps_Reference / 2)))) {
            Set_Home_pos = 0;
            Tr_Light_Dark_1 = OFF;
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                return FALSE;
            } else {
                if (Steps_Pos <= (signed long) (STEP_PHOTO_TABLE_OFFSET))
                    Table_Error = Steps_Pos;
                else
                    Table_Error = TintingAct.Steps_Revolution - Steps_Pos;

                SetStepperHomePosition(MOTOR_TABLE);
                if (Reference == REFERENCE_STEP_2)
                    Reference = REFERENCE_STEP_ON;

                if (Table_circuits_pos == ON) {
                    for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                        TintingAct.Circuit_step_pos[i] = CircStepPosAct.Circ_Pos[i];
                }
                TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0];
                return TRUE;
            }
        }
        else {
            Table_Error = 0;
            return TRUE;
        }
    }
    else {
        //----------------------------------------------------------------------
        if ((Set_Home_pos == 0) &&
                ((Steps_Pos <= (-(signed long) (TintingAct.Steps_Revolution) + STEP_PHOTO_TABLE_OFFSET)) ||
                (Steps_Pos >= -(signed long) (STEP_PHOTO_TABLE_OFFSET)))) {
            if (Tr_Light_Dark_1 == ON) {
                Set_Home_pos = 1;
                Steps_Pos0 = Steps_Pos;
                if (Reference == REFERENCE_STEP_1)
                    Reference = REFERENCE_STEP_2;
            }
            Table_Error = 0;
            return TRUE;
        }
        else if ((Set_Home_pos == 0) && (Steps_Pos <= -(signed long) (TintingAct.Steps_Revolution)))
            return FALSE;

        else if ((Set_Home_pos == 1) &&
                (((Steps_Pos - Steps_Pos0) <= -(signed long) (TintingAct.Steps_Reference / 2)) ||
                ((Steps_Pos - Steps_Pos0) >= (signed long) (TintingAct.Steps_Reference / 2)))) {
            Set_Home_pos = 0;
            Tr_Light_Dark_1 = OFF;
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
                return FALSE;
            else {
                if (Steps_Pos >= -(signed long) (STEP_PHOTO_TABLE_OFFSET))
                    Table_Error = Steps_Pos;
                else
                    Table_Error = -(signed long) (TintingAct.Steps_Revolution + Steps_Pos);

                SetStepperHomePosition(MOTOR_TABLE);
                if (Reference == REFERENCE_STEP_2)
                    Reference = REFERENCE_STEP_ON;

                if (Table_circuits_pos == ON) {
                    for (i = 0; i < MAX_COLORANT_NUMBER; i++)
                        TintingAct.Circuit_step_pos[i] = CircStepPosAct.Circ_Pos[i];
                }
                return TRUE;
            }
        }
        else {
            Table_Error = 0;
            return TRUE;
        }
    }
    //--------------------------------------------------------------------------
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

unsigned char TablePositioningColorSupply(void) {
    unsigned char ret = PROC_RUN;
    unsigned short i;
    static unsigned short direction, count_circuits, theorical_circuits;
    static signed long Steps_Todo, Steps_Position;
    static unsigned char Old_Photocell_sts, New_Photocell_sts;
    static unsigned char Circ_Indx, Wait;
    unsigned char currentReg, Dir_Neg;
    signed long Error_position;
    static unsigned long Moving_Speed;
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    if (CheckTableErrorCondition() == PROC_FAIL) {
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        return PROC_FAIL;
    }
    // Table Panel OPEN
    if ((TintingAct.PanelTable_state == OPEN) && (PositioningCmd != 1)) {
        StopTimer(T_TABLE_WAITING_TIME);
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        //        Table.step = STEP_10;
        Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
        return PROC_FAIL;
    }        // Bases Carriage Open  
    else if (TintingAct.BasesCarriageOpen == OPEN) {
        StopTimer(T_TABLE_WAITING_TIME);
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        //        Table.step = STEP_10;
        Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
        return PROC_FAIL;
    } else if ((PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT)) {
        StopTimer(T_TABLE_WAITING_TIME);
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }
    else if (ManageTableHomePosition() == FALSE) {
        // Loss of Steps  
        StopTimer(T_TABLE_WAITING_TIME);
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }
    if (isColorCmdStopProcess()) {
        StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
        RicirculationCmd = 0;
        Table.step = STEP_10;
    }
    //----------------------------------------------------------------------------
    switch (Table.step) {
            // -----------------------------------------------------------------------------     
            // Starts operations
        case STEP_0:
            Status.errorCode = 0;
            count_circuits = 0;
            Wait = FALSE;
            if ((PositioningCmd == 1) && (TintingAct.PanelTable_state == OPEN))
                // Refill cmd arrived --> Table moves at Low Speed
                Moving_Speed = TintingAct.Low_Speed_Rotating_Table;
            else
                // No Refill cmd arrived --> Table moves at High Speed
                Moving_Speed = TintingAct.High_Speed_Rotating_Table;

            // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
            //        ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                          
            currentReg = HOLDING_CURRENT_TABLE * 100 / 156;
            cSPIN_RegsStruct2.TVAL_HOLD = currentReg;
            cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct2.TVAL_HOLD, MOTOR_TABLE);
            // EEprom CRC Error
            if (EEprom_Crc_Error == 1) {
                Table.errorCode = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
                return PROC_FAIL;
            }
            // Table is already positioned on the Circuit "Color_Id"
            if (((TintingAct.Circuit_Engaged == TintingAct.Color_Id) && (TintingAct.Refilling_Angle == 0) && (Stirring_Method != BEFORE_EVERY_RICIRCULATION))
                    ||
                    ((TintingAct.Circuit_Engaged == TintingAct.Color_Id) && (TintingAct.Refilling_Angle == 0) && (Stirring_Method == BEFORE_EVERY_RICIRCULATION) && (RicirculationCmd == 0))) {
                Table.step = STEP_9;
                return ret;
            }
            // Analyze Command parameters:
            if ((TintingAct.Color_Id > MAX_COLORANT_NUMBER) || (TintingAct.Color_Id == 0) || (TintingAct.Refilling_Angle > MAX_ROTATING_ANGLE) ||
                    ((TintingAct.Direction != CW) && (TintingAct.Direction != CCW)) ||
                    (TintingAct.Table_Colorant_En[TintingAct.Color_Id - 1]) == 0) {
                Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
                return PROC_FAIL;
            }

            // No Circuit Position steps available --> Process fail
            if (Table_circuits_pos == OFF) {
                Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                return PROC_FAIL;
            }

            Circ_Indx = 0;
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if ((i < (TintingAct.Color_Id - 1)) && (TintingAct.Table_Colorant_En[i] == 1))
                    Circ_Indx++;
            }
            if ((Stirring_Method == BEFORE_EVERY_RICIRCULATION) && (RicirculationCmd == 1)) {
                // Read current position
                Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);
                // Error Calculation
                if (Steps_Position < 0) {
                    Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
                    Dir_Neg = TRUE;
                }
                else {
                    Steps_Position = Steps_Position % TintingAct.Steps_Revolution;
                    Dir_Neg = FALSE;
                }
                if (Steps_Position >= (TintingAct.Steps_Revolution / 2))
                    Error_position = Steps_Position - TintingAct.Steps_Revolution;
                else
                    Error_position = Steps_Position;

                if (Dir_Neg == FALSE)
                    Steps_Todo = -(TintingAct.Steps_Revolution - Error_position);
                else
                    Steps_Todo = (TintingAct.Steps_Revolution - Error_position);

                MoveStepper(MOTOR_TABLE, Steps_Todo, Moving_Speed);
                Table.step++;
            } else

                Table.step += 4;
            break;

        case STEP_1:
            if (isColorCmdStop() || isColorCmdStopProcess()) {
                //                StopStepper(MOTOR_TABLE);
                SoftStopStepper(MOTOR_TABLE);
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                RicirculationCmd = 0;
                Table.step += 2;
            }
            else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == FALSE)) {
                Wait = TRUE;
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            }// Another full 360° CCW Table rotation
            else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == TRUE)) {
                if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                    StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                    Wait = FALSE;
                    // Read current position
                    Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);
                    // Error Calculation
                    if (Steps_Position < 0) {
                        Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
                        Dir_Neg = TRUE;
                    }
                    else {
                        Steps_Position = Steps_Position % TintingAct.Steps_Revolution;
                        Dir_Neg = FALSE;
                    }
                    if (Steps_Position >= (TintingAct.Steps_Revolution / 2))
                        Error_position = Steps_Position - TintingAct.Steps_Revolution;
                    else
                        Error_position = Steps_Position;

                    if (Dir_Neg == FALSE)
                        Steps_Todo = -(TintingAct.Steps_Revolution - Error_position);
                    else
                        Steps_Todo = (TintingAct.Steps_Revolution - Error_position);

                    MoveStepper(MOTOR_TABLE, Steps_Todo, Moving_Speed);
                    Table.step++;
                }
            }
            break;

        case STEP_2:
            if (isColorCmdStop() || isColorCmdStopProcess()) {
                //                StopStepper(MOTOR_TABLE);
                SoftStopStepper(MOTOR_TABLE);
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                RicirculationCmd = 0;
                Table.step++;
            }
            else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == FALSE)) {
                Wait = TRUE;
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            }
            else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == TRUE)) {
                if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                    StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                    Wait = FALSE;
                    RicirculationCmd = 0;
                    // Table is already positioned on the Circuit "Color_Id"
                    if (TintingAct.Circuit_Engaged == TintingAct.Color_Id)
                        Table.step = STEP_9;
                    else
                        Table.step += 2;
                }
            }
            break;

        case STEP_3:
            if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                Table.step++;
            }
            break;

            // Start Table Rotation towards circuit selected in the shortest way
        case STEP_4:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
                    Old_Photocell_sts = DARK;
                else
                    Old_Photocell_sts = LIGHT;
                Reference = REFERENCE_STEP_1;

                // Read current position
                Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);
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
                if (((signed long) (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) >= 0) &&
                        ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) < (TintingAct.Steps_Revolution / 2))) {
                    direction = CCW;
                    Steps_Todo = -(signed long) (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position - STEP_PHOTO_TABLE_OFFSET);
                    // 'Steps_Todo' has to be < 0
                    if (Steps_Todo >= 0) {
                        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
                            // Rotate CCW motor Table till Photocell transition LIGHT-DARK
                            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                        else
                            // Rotate CW motor Table till Photocell transition DARK-LIGHT
                            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, DARK_LIGHT, TABLE_PHOTOCELL, 0);
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step += 2;
                    }
                    else {
                        // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                        theorical_circuits = 0;
                        for (i = 0; i < Total_circuit_n; i++) {
                            if ((TintingAct.Circuit_step_pos[i] >= Steps_Position) && (TintingAct.Circuit_step_pos[i] <= TintingAct.Circuit_step_pos[Circ_Indx]))
                                theorical_circuits++;
                            else if ((TintingAct.Circuit_step_pos[i] < Steps_Position) && ((Steps_Position - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit))
                                theorical_circuits++;
                        }
                        if ((PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Reference)))
                            theorical_circuits--;

                        // Not troughout Reference                
                        if ((signed long) ((TintingAct.Steps_Threshold + Steps_Todo) <= 0))
                            MoveStepper(MOTOR_TABLE, Steps_Todo, Moving_Speed);
                        else
                            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                        Table.step++;
                    }
                }
                    // Case 2        
                else if (((signed long) (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) >= 0) &&
                        ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) >= (TintingAct.Steps_Revolution / 2))) {
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
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step += 2;
                    }
                    else {
                        // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                        theorical_circuits = 0;
                        for (i = 0; i < Total_circuit_n; i++) {
                            if ((TintingAct.Circuit_step_pos[i] >= TintingAct.Circuit_step_pos[Circ_Indx]) && (TintingAct.Circuit_step_pos[i] <= TintingAct.Steps_Revolution))
                                theorical_circuits++;
                            else if ((TintingAct.Circuit_step_pos[i] <= Steps_Position))
                                theorical_circuits++;
                            else if ((TintingAct.Circuit_step_pos[i] > Steps_Position) && ((TintingAct.Circuit_step_pos[i] - Steps_Position) <= TintingAct.Steps_Tolerance_Circuit))
                                theorical_circuits++;
                        }
                        if ((PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Reference)))
                            theorical_circuits--;

                        if (Steps_Todo > TintingAct.Steps_Threshold)
                            MoveStepper(MOTOR_TABLE, Steps_Todo, Moving_Speed);
                        else
                            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                        Table.step++;
                    }
                }
                    // Case 3        
                else if (((signed long) (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) < 0) &&
                        ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) > -(signed long) (TintingAct.Steps_Revolution / 2))) {
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
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step += 2;
                    }
                    else {
                        // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                        theorical_circuits = 0;
                        for (i = 0; i < Total_circuit_n; i++) {
                            if ((TintingAct.Circuit_step_pos[i] >= TintingAct.Circuit_step_pos[Circ_Indx]) && (TintingAct.Circuit_step_pos[i] <= Steps_Position))
                                theorical_circuits++;
                            else if ((TintingAct.Circuit_step_pos[i] > Steps_Position) && ((TintingAct.Circuit_step_pos[i] - Steps_Position) <= TintingAct.Steps_Tolerance_Circuit))
                                theorical_circuits++;
                        }
                        if ((PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Reference)))
                            theorical_circuits--;

                        // Not Troughout Reference
                        if (Steps_Todo > TintingAct.Steps_Threshold)
                            MoveStepper(MOTOR_TABLE, Steps_Todo, Moving_Speed);
                        else
                            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                        Table.step++;
                    }
                }
                    // Case 4        
                else if (((signed long) (TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) < 0) &&
                        ((TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position) <= -(signed long) (TintingAct.Steps_Revolution / 2))) {
                    direction = CCW;
                    Steps_Todo = -(signed long) (TintingAct.Steps_Revolution - (Steps_Position - TintingAct.Circuit_step_pos[Circ_Indx]) - STEP_PHOTO_TABLE_OFFSET);
                    // 'Steps_Todo' has to be < 0
                    if (Steps_Todo >= 0) {
                        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT)
                            // Rotate CCW motor Table till Photocell transition LIGHT-DARK
                            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                        else
                            // Rotate CW motor Table till Photocell transition DARK-LIGHT
                            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, DARK_LIGHT, TABLE_PHOTOCELL, 0);
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step += 2;
                    }
                    else {
                        // Number of circuits to be crossed in order to reach "TintingAct.Color_Id": icluded "TintingAct.Color_Id", "TintingAct.Circuit_Engaged", excluded Reference
                        theorical_circuits = 0;
                        for (i = 0; i < Total_circuit_n; i++) {
                            if ((TintingAct.Circuit_step_pos[i] <= TintingAct.Circuit_step_pos[Circ_Indx]))
                                theorical_circuits++;
                            else if ((TintingAct.Circuit_step_pos[i] >= Steps_Position) && (TintingAct.Circuit_step_pos[i] <= TintingAct.Steps_Revolution))
                                theorical_circuits++;
                            else if ((TintingAct.Circuit_step_pos[i] < Steps_Position) && ((Steps_Position - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit))
                                theorical_circuits++;
                        }
                        if ((PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) && (Steps_Position > TintingAct.Steps_Reference) && (Steps_Position < (TintingAct.Steps_Revolution - TintingAct.Steps_Reference)))
                            theorical_circuits--;

                        if ((signed long) ((TintingAct.Steps_Threshold + Steps_Todo) <= 0))
                            MoveStepper(MOTOR_TABLE, Steps_Todo, Moving_Speed);
                        else
                            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                        Table.step++;
                    }
                }
                else {
                    SoftStopStepper(MOTOR_TABLE);
                    Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                    return PROC_FAIL;
                }
            }
            break;

            // Waiting for be placed near 'TintingAct.Color_Id' position
        case STEP_5:
            // Update Photocell Status
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
                New_Photocell_sts = DARK;
            else
                New_Photocell_sts = LIGHT;

            // Transition LIGHT-DARK ?
            if ((Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK))
                count_circuits++;

            Old_Photocell_sts = New_Photocell_sts;

            if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == FALSE)) {
                Wait = TRUE;
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            }
            else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == TRUE)) {
                if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                    StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                    Wait = FALSE;
                    if ((direction == CW) && (Status_Board_Table.Bit.MOT_STATUS == 0)) {
                        // Check if Reference was found
                        if (Reference == REFERENCE_STEP_ON)
                            count_circuits--;

                        Reference = REFERENCE_STEP_0;
                        // Probably: loss of Table steps
                        if (count_circuits != (theorical_circuits - 1)) {
                            Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                            return PROC_FAIL;
                        }
                        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                            Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                            return PROC_FAIL;
                        }
                        // Rotate CW motor Table till Photocell transition LIGHT-DARK
                        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step++;
                    }
                    else if ((direction == CCW) && (Status_Board_Table.Bit.MOT_STATUS == 0)) {

                        // Check if Reference was found
                        if (Reference == REFERENCE_STEP_ON)
                            count_circuits--;

                        Reference = REFERENCE_STEP_0;
                        // Probably: loss of Table steps
                        if (count_circuits != (theorical_circuits - 1)) {
                            Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                            return PROC_FAIL;
                        }
                        if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                            Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                            return PROC_FAIL;
                        }
                        // Rotate CCW motor Table till Photocell transition LIGHT-DARK
                        StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                        StartTimer(T_TABLE_WAITING_TIME);
                        Table.step++;
                    }
                }
            }
            break;

            // Check for LIGHT-DARK transition
        case STEP_6:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                StopTimer(T_TABLE_WAITING_TIME);
                if (direction == CW)
                    // Steps_Todo = (TintingAct.Steps_Circuit/2);                        
                    Steps_Todo = TintingAct.Steps_Circuit + STEP_PHOTO_TABLE_OFFSET;
                else
                    // Steps_Todo = -(signed long)(TintingAct.Steps_Circuit/2);                                    
                    Steps_Todo = -(signed long) TintingAct.Steps_Circuit - (signed long) STEP_PHOTO_TABLE_OFFSET;

                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
            else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                return PROC_FAIL;
            }
            break;

        case STEP_7:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (direction == CW)
                    Steps_Todo = -(signed long) (TintingAct.Steps_Circuit / 2) -(signed long) STEP_PHOTO_TABLE_OFFSET;
                else
                    Steps_Todo = TintingAct.Steps_Circuit / 2 + STEP_PHOTO_TABLE_OFFSET;

                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
            break;

            // Waiting to be placed at the center of 'TintingAct.Color_Id' position 
        case STEP_8:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                // Check if Photocell is covered
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                    Table.errorCode = TINTING_TABLE_MOVE_ERROR_ST;
                    return PROC_FAIL;
                }
                if (TintingAct.Refilling_Angle > 0) {
                    // Find Steps corresponding to 'Refilling_Angle'
                    Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);
                    if (TintingAct.Direction == CW)
                        Steps_Todo = (long) ((float) TintingAct.Steps_Revolution * ((float) TintingAct.Refilling_Angle / (float) (2 * MAX_ROTATING_ANGLE)));
                    else
                        Steps_Todo = -(long) ((float) TintingAct.Steps_Revolution * ((float) TintingAct.Refilling_Angle / (float) (2 * MAX_ROTATING_ANGLE)));

                    MoveStepper(MOTOR_TABLE, Steps_Todo, Moving_Speed);
                    Table.step++;
                }
                else {
                    /*
                                    // Check if position reached is correct
                                    // Read current position               
                                    Steps_Position = (signed long)GetStepperPosition(MOTOR_TABLE);
                                    if (Steps_Position < 0) {
                                        Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;            
                                        if (Steps_Position != 0)
                                            Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
                                    }   
                                    else
                                        Steps_Position = Steps_Position % TintingAct.Steps_Revolution;                        
                
                                    // Update 'Circuit_step_pos' in order to take into account possible steps loss
                                    if (TintingAct.Circuit_step_pos[Circ_Indx] >= Steps_Position) {
                                        Steps_Todo = TintingAct.Circuit_step_pos[Circ_Indx] - Steps_Position;
                                        for (i = 0; i < Total_circuit_n; i++) 
                                            TintingAct.Circuit_step_pos[i] = TintingAct.Circuit_step_pos[i] - (Steps_Todo);
                                    }
                                    else  {
                                        Steps_Todo = Steps_Position - TintingAct.Circuit_step_pos[Circ_Indx];
                                        for (i = 0; i < Total_circuit_n; i++) 
                                            TintingAct.Circuit_step_pos[i] = TintingAct.Circuit_step_pos[i] + (Steps_Todo);                    
                                    }                     
                                    TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0];                                
                     */
                    Table.step += 2;
                }
            }
            break;

            // Waiting to reach the requested final position
        case STEP_9:
            if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == FALSE)) {
                Wait = TRUE;
                StartTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
            }
            else if ((Status_Board_Table.Bit.MOT_STATUS == 0) && (Wait == TRUE)) {
                if (StatusTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT) == T_ELAPSED) {
                    StopTimer(T_TABLE_WAIT_BEETWEN_MOVEMENT);
                    Wait = FALSE;
                    Table.step++;
                }
            }
            break;

        case STEP_10:
            if (Wait == FALSE) {
                Wait = TRUE;
                StartTimer(T_WAIT_END_TABLE_POSITIONING);
            }
            else if (StatusTimer(T_WAIT_END_TABLE_POSITIONING) == T_ELAPSED) {
                StopTimer(T_WAIT_END_TABLE_POSITIONING);
                Wait = FALSE;
                StopStepper(MOTOR_TABLE);
                ret = PROC_OK;
            }
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
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

unsigned char TableCleaningColorSupply(void) {
    unsigned char ret = PROC_RUN;
    static signed long Steps_Todo;
    static unsigned char Stop_exe = FALSE;
    static signed long Steps_Position;
    static signed long Brush1, Brush2, Circuit_Pos;
    //----------------------------------------------------------------------------
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    if (CheckTableErrorCondition() == PROC_FAIL) {
        StopTimer(T_WAIT_GENERIC24V_TIME);
        StopTimer(T_WAIT_BRUSH_ON);
        TintingAct.Brush_state = OFF;
        TintingAct.Cleaning_status = 0x0000;
        Punctual_Cleaning = OFF;
        SPAZZOLA_OFF();
        return PROC_FAIL;
    }
    // Table Panel OPEN
    if ((Table.level != TABLE_POSITIONING) && (TintingAct.PanelTable_state == OPEN)) {
        if (TintingAct.Brush_state == OFF) {
            StopTimer(T_WAIT_GENERIC24V_TIME);
            StopTimer(T_WAIT_BRUSH_ON);
            Punctual_Cleaning = OFF;
            SPAZZOLA_OFF();
            SoftStopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
            return PROC_FAIL;
        }
    }
        // Bases Carriage OPEN  
    else if (TintingAct.BasesCarriageOpen == OPEN) {
        if (TintingAct.Brush_state == OFF) {
            StopTimer(T_WAIT_GENERIC24V_TIME);
            StopTimer(T_WAIT_BRUSH_ON);
            Punctual_Cleaning = OFF;
            SPAZZOLA_OFF();
            SoftStopStepper(MOTOR_TABLE);
            Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
            return PROC_FAIL;
        }
    }
    else if (ManageTableHomePosition() == FALSE) {
        // Loss of Steps  
        StopTimer(T_WAIT_GENERIC24V_TIME);
        StopTimer(T_WAIT_BRUSH_ON);
        TintingAct.Brush_state = OFF;
        TintingAct.Cleaning_status = 0x0000;
        Punctual_Cleaning = OFF;
        SPAZZOLA_OFF();
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }
    if ((isColorCmdStop() || isColorCmdStopProcess()) && (Stop_exe == FALSE)) {
        Stop_exe = TRUE;
        StopTimer(T_WAIT_GENERIC24V_TIME);
        StopTimer(T_WAIT_BRUSH_ON);
        StartTimer(T_WAIT_BRUSH_ON);
        //        StopStepper(MOTOR_TABLE);
        SoftStopStepper(MOTOR_TABLE);
        Table.step = STEP_6;
    }
    else if ((Punctual_Clean_Act == ON) && (Punctual_Cleaning == OFF) && (Stop_exe == FALSE)) {
        Stop_exe = TRUE;
        StopTimer(T_WAIT_GENERIC24V_TIME);
        StopTimer(T_WAIT_BRUSH_ON);
        StartTimer(T_WAIT_BRUSH_ON);
        //        StopStepper(MOTOR_TABLE);
        SoftStopStepper(MOTOR_TABLE);
        Table.step = STEP_6;
    }

    // Check for GENERIC24V --> Spazzola
#ifndef SKIP_FAULT_GENERIC24V
    if (StatusTimer(T_WAIT_GENERIC24V_TIME) == T_ELAPSED) {
        if (isFault_Generic24V_Detection() && (TintingAct.Brush_state == ON)) {
            StopTimer(T_WAIT_BRUSH_ON);
            StopTimer(T_WAIT_GENERIC24V_TIME);
            StopTimer(T_WAIT_BRUSH_ACTIVATION);
            TintingAct.Brush_state = OFF;
            TintingAct.Cleaning_status = 0x0000;
//            SPAZZOLA_OFF();
            Table.errorCode = TINTING_GENERIC24V_OVERCURRENT_THERMAL_ERROR_ST;
            return PROC_FAIL;
        }
        else if (isFault_Generic24V_Detection() && (TintingAct.Brush_state == OFF)) {
            StopTimer(T_WAIT_BRUSH_ON);
            StopTimer(T_WAIT_GENERIC24V_TIME);
            StopTimer(T_WAIT_BRUSH_ACTIVATION);
            TintingAct.Brush_state = OFF;
            TintingAct.Cleaning_status = 0x0000;
            SPAZZOLA_OFF();
            Table.errorCode = TINTING_GENERIC24V_OPEN_LOAD_ERROR_ST;
            return PROC_FAIL;
        }
    }
#endif 
    //----------------------------------------------------------------------------
    switch (Table.step) {
            // -----------------------------------------------------------------------------     
            // Starts operations
        case STEP_0:
            Status.errorCode = 0;
            TintingAct.Brush_state = OFF;
            Stop_exe = FALSE;
            StopTimer(T_WAIT_GENERIC24V_TIME);
            StartTimer(T_WAIT_GENERIC24V_TIME);
            if (TintingAct.Cleaning_duration != 0)
                Durata[T_WAIT_BRUSH_ACTIVATION] = (unsigned short) (TintingAct.Cleaning_duration) * CONV_SEC_COUNT;

            // EEprom CRC Error
            if (EEprom_Crc_Error == 1) {
                StopTimer(T_WAIT_GENERIC24V_TIME);
                Table.errorCode = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
                return PROC_FAIL;
            }
            // No Circuit Position steps available --> Process fail
            if (Table_circuits_pos == OFF) {
                StopTimer(T_WAIT_GENERIC24V_TIME);
                Table.errorCode = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                return PROC_FAIL;
            }
            Table.step++;
            break;

        case STEP_1:
            // Read current position
            Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);
            if (Steps_Position < 0) {
                Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
                if (Steps_Position != 0)


                    Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
            } else
                Steps_Position = Steps_Position % TintingAct.Steps_Revolution;

            Table.step++;
            break;

            // Start Table Rotation 
        case STEP_2:
            Brush1 = (signed long) (STEPS_CLEANING);
            Brush2 = (signed long) (STEPS_CLEANING) + (signed long) ((signed long) STEPS_REVOLUTION / 2);
            // Circuit_Pos = 'TintingAct.Color_Id' position with respect to 0 position (not to Reference!!)
            if (Circuit_step_tmp[TintingAct.Color_Id - 1] > Steps_Position)
                Circuit_Pos = Circuit_step_tmp[TintingAct.Color_Id - 1] - Steps_Position;
            else
                Circuit_Pos = Circuit_step_tmp[TintingAct.Color_Id - 1] - Steps_Position + TintingAct.Steps_Revolution;

            // CCW Rotation (Steps_Todo < 0)
            if ((Circuit_Pos > Brush1) && (Circuit_Pos < Brush2)) {
                if (Brush1 < Circuit_Pos)
                    Steps_Todo = Brush1 - Circuit_Pos;
                else
                    Steps_Todo = Brush1 - Circuit_Pos - TintingAct.Steps_Revolution;
            }
                // CW Rotation (Steps_Todo > 0)
            else {
                if (Brush1 < Circuit_Pos)
                    Steps_Todo = Brush1 + TintingAct.Steps_Revolution - Circuit_Pos;
                else
                    Steps_Todo = Brush1 - Circuit_Pos;
            }
            // Rotate 'Color_Id' into Cleaning position
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step++;
            break;

            // Waiting 'TintingAct.Color_Id-1' position is on Cleaning position     
        case STEP_3:
            if (Status_Board_Table.Bit.MOT_STATUS == 0)
                Table.step++;
            break;

            // Brush activation
        case STEP_4:
            StartTimer(T_WAIT_BRUSH_ACTIVATION);
            StopTimer(T_WAIT_GENERIC24V_TIME);
            StartTimer(T_WAIT_GENERIC24V_TIME);
            TintingAct.Cleaning_status |= (1L << (TintingAct.Color_Id - 1 + TINTING_COLORANT_OFFSET));
            TintingAct.Brush_state = ON;
            SPAZZOLA_ON();
            // Temporized Cleaning
            if (Punctual_Clean_Act == OFF)
                Table.step++;
            // Punctual Cleaning
            else
                Table.step += 4;
            break;

        case STEP_5:
            if (StatusTimer(T_WAIT_BRUSH_ACTIVATION) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ACTIVATION);
                StartTimer(T_WAIT_BRUSH_ON);
                StopTimer(T_WAIT_GENERIC24V_TIME);
                Table.step++;
            }
            break;

        case STEP_6:
            // Wait till Brush Photocell is not covered
            if (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT) {
                StopTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_BRUSH_ON);
                Table.step++;
            }
            else if ((PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == DARK) && (TintingAct.Brush_state == OFF)) {
                StopTimer(T_WAIT_BRUSH_ON);
                StartTimer(T_WAIT_BRUSH_ON);
                Table.step++;
            }
            else if (StatusTimer(T_WAIT_BRUSH_ON) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ON);
                SPAZZOLA_OFF();
                Punctual_Clean_Act = OFF;
                TintingAct.Cleaning_status &= ~(1L << TintingAct.Color_Id);
                Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

        case STEP_7:
            // Stop BRUSH only when Photocell is covered
            if (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == DARK) {
                StopTimer(T_WAIT_BRUSH_ON);
                SPAZZOLA_OFF();
                Table.step++;
            }
            else if (StatusTimer(T_WAIT_BRUSH_ON) == T_ELAPSED) {
                StopTimer(T_WAIT_BRUSH_ON);
                SPAZZOLA_OFF();
                Punctual_Clean_Act = OFF;
                TintingAct.Cleaning_status = 0x0000;
                Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

        case STEP_8:
            StopTimer(T_WAIT_BRUSH_ON);
            if (Punctual_Clean_Act == OFF) {
                StopTimer(T_WAIT_GENERIC24V_TIME);
                TintingAct.Cleaning_status = 0x0000;
                TintingAct.Brush_state = OFF;
                if (Stop_exe == FALSE)
                    TintingAct.Cleaning_Col_Mask[TintingAct.Color_Id - 1] = 0;
            }                
            // Stop Punctual Cleaning
            else if ((Punctual_Clean_Act == ON) && (Punctual_Cleaning == OFF)) {
                Punctual_Clean_Act = OFF;
                TintingAct.Cleaning_status &= ~(1L << TintingAct.Color_Id);
            }
            // StopStepper(MOTOR_TABLE);  
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if ((TintingAct.BasesCarriageOpen == OPEN) || (TintingAct.PanelTable_state == OPEN))
                    ret = PROC_FAIL;
                else
                    ret = PROC_OK;
            }
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
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

unsigned char TableTestColorSupply(void) {
    unsigned char ret = PROC_RUN;
    unsigned short i, j;
    static unsigned short circuit_id, circuit_id_ccw, Total_circ;
    static unsigned char Find_Circuit, Old_Photocell_sts, New_Photocell_sts;
    static signed long Steps_Todo;
    static signed long Circuit_step_temp[MAX_COLORANT_NUMBER], Circuit_step_temp_1[MAX_COLORANT_NUMBER];
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    if (CheckTableErrorCondition() == PROC_FAIL) {
        return PROC_FAIL;
    }
    // Bases Carriage Open
    if (TintingAct.BasesCarriageOpen == OPEN) {
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        //Table.step = STEP_9;
        Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
        return PROC_FAIL;
    }
    // Table Panel OPEN
    if ((Table.level != TABLE_POSITIONING) && (TintingAct.PanelTable_state == OPEN)) {
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
        return PROC_FAIL;
    }

    if (isColorCmdStopProcess()) {
        //        StopStepper(MOTOR_TABLE);
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        Table.step = STEP_9;
    }
    //----------------------------------------------------------------------------
    switch (Table.step) {
            // -----------------------------------------------------------------------------     
            // Starts operations
        case STEP_0:
            Find_Circuit = OFF;
            circuit_id = 0;
            Status.errorCode = 0;
            Reference = REFERENCE_STEP_0;
            Total_circ = 0;

            if ((signed long) GetStepperPosition(MOTOR_TABLE) != 0) {
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return PROC_FAIL;
            }
            else if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                return PROC_FAIL;
            }
            else {
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    TintingAct.Circuit_step_pos_cw[i] = 0;
                    TintingAct.Circuit_step_pos_ccw[i] = 0;
                    Circuit_step_temp[i] = 0;
                }
                Table.step++;
                Old_Photocell_sts = DARK;
            }
            break;

            // Starts a full CW Table Rotation   
        case STEP_1:
            Steps_Todo = TintingAct.Steps_Revolution - STEP_PHOTO_TABLE_OFFSET;
            // Table CW rotation
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
            Table.step++;
            break;

            // Self Recognition in CW direction
        case STEP_2:
            // Update Photocell Status
            if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK)
                New_Photocell_sts = DARK;
            else
                New_Photocell_sts = LIGHT;

            // Transition LIGHT-DARK ?
            if ((Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK)) {
                Find_Circuit = ON;
                Circuit_step_temp[circuit_id] = (signed long) GetStepperPosition(MOTOR_TABLE);
                if (Circuit_step_temp[circuit_id] < 0) {
                    Circuit_step_temp[circuit_id] = (-Circuit_step_temp[circuit_id]) % TintingAct.Steps_Revolution;
                    if (Circuit_step_temp[circuit_id] != 0)
                        Circuit_step_temp[circuit_id] = TintingAct.Steps_Revolution - (Circuit_step_temp[circuit_id] + (TintingAct.Steps_Circuit / 2));
                }
                else
                    Circuit_step_temp[circuit_id] = Circuit_step_temp[circuit_id] % TintingAct.Steps_Revolution;
                circuit_id++;
            }

            Old_Photocell_sts = New_Photocell_sts;

            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (Find_Circuit == OFF) {
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;
                }
                else if (circuit_id > MAX_COLORANT_NUMBER) {
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;
                }
                else {
                    for (i = 0; i < circuit_id; i++)
                        TintingAct.Circuit_step_pos_cw[i] = Circuit_step_temp[circuit_id - 1 - i];

                    Total_circ = circuit_id;
                    // Rotate CW motor Table till Photocell transition DARK-LIGHT
                    StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
                    StartTimer(T_TABLE_WAITING_TIME);
                    Table.step++;
                }
            }
            break;

            // Check for DARK-LIGHT transition    
        case STEP_3:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                SetStepperHomePosition(MOTOR_TABLE);
                StopTimer(T_TABLE_WAITING_TIME);
                // Go to center position
                Steps_Todo = (signed long) (TintingAct.Steps_Circuit / 2);
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            } else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

            // Table centered on Reference Circuit
        case STEP_4:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                SetStepperHomePosition(MOTOR_TABLE);
                Old_Photocell_sts = DARK;
                circuit_id_ccw = 0;
                // Starts a full CCW Table Rotation   
                Steps_Todo = -TintingAct.Steps_Revolution + STEP_PHOTO_TABLE_OFFSET;
                // Table CCW rotation
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.High_Speed_Rotating_Table);
                Table.step++;
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
            if ((Old_Photocell_sts == LIGHT) && (New_Photocell_sts == DARK)) {
                TintingAct.Circuit_step_pos_ccw[circuit_id_ccw] = (signed long) GetStepperPosition(MOTOR_TABLE) + (signed long) (TintingAct.Steps_Circuit / 2);
                circuit_id_ccw++;
                circuit_id--;
            }

            Old_Photocell_sts = New_Photocell_sts;

            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == DARK) {
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;
                }
                else if (circuit_id > 0) {
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;
                }
                else
                    // Self Learning Procedure End in CCW direction
                    Table.step++;
            }
            break;

            // Check if 'Circuit_step_pos[]' steps match between CW and CCW directions
        case STEP_6:
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                // Difference in Circuit position Steps between CW and CCW > tolerance --> Error
                if (((TintingAct.Circuit_step_pos_cw[i] > TintingAct.Circuit_step_pos_ccw[i]) && ((TintingAct.Circuit_step_pos_cw[i] - TintingAct.Circuit_step_pos_ccw[i]) >= TintingAct.Steps_Tolerance_Circuit)) ||
                        ((TintingAct.Circuit_step_pos_cw[i] <= TintingAct.Circuit_step_pos_ccw[i])&& ((TintingAct.Circuit_step_pos_ccw[i] - TintingAct.Circuit_step_pos_cw[i]) >= TintingAct.Steps_Tolerance_Circuit))) {
                    Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                    return PROC_FAIL;
                }
                Circuit_step_temp[i] = (TintingAct.Circuit_step_pos_cw[i] + TintingAct.Circuit_step_pos_ccw[i]) / 2;
            }
            for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                Find_Circuit = FALSE;
                for (i = 0; i < Total_circ; i++) {
                    if (((TintingAct.Circuit_step_theorical_pos[j] > Circuit_step_temp[i]) && ((TintingAct.Circuit_step_theorical_pos[j] - Circuit_step_temp[i]) <= TintingAct.Steps_Tolerance_Circuit)) ||
                            ((TintingAct.Circuit_step_theorical_pos[j] <= Circuit_step_temp[i])&& ((Circuit_step_temp[i] - TintingAct.Circuit_step_theorical_pos[j]) <= TintingAct.Steps_Tolerance_Circuit))) {
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
            if (TintingAct.Table_Step_position == DYNAMIC) {
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    // Difference in Circuit position Steps between CW or CCW and values found in auto recognition procedure > tolerance --> Error            
                    if (((Circuit_step_temp[i] > TintingAct.Circuit_step_pos[i]) && ((Circuit_step_temp[i] - TintingAct.Circuit_step_pos[i]) >= TintingAct.Steps_Tolerance_Circuit)) ||
                            ((Circuit_step_temp[i] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - Circuit_step_temp[i]) >= TintingAct.Steps_Tolerance_Circuit))) {
                        Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                        return PROC_FAIL;
                    }
                }
                Table.step++;
            }// STATIC circuit position model: tabel with possible zeros
            else {
                for (i = 0; i < circuit_id; i++) {
                    Find_Circuit = FALSE;
                    for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                        if (((TintingAct.Circuit_step_pos[j] > Circuit_step_temp[i]) && ((TintingAct.Circuit_step_pos[j] - Circuit_step_temp[i]) <= TintingAct.Steps_Tolerance_Circuit)) ||
                                ((TintingAct.Circuit_step_pos[j] <= Circuit_step_temp[i])&& ((Circuit_step_temp[i] - TintingAct.Circuit_step_pos[j]) <= TintingAct.Steps_Tolerance_Circuit))) {
                            Find_Circuit = TRUE;
                            break;
                        }
                    }
                    // Circuit 'i' is not present on the Table
                    if (Find_Circuit == FALSE) {
                        Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                        return PROC_FAIL;
                    }
                }
                Table.step++;
            }
            break;

            // ---------------------------------------------------------------------
            // Check if there is a full match between positiong Table found and enabled circuit table 
        case STEP_7:
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if (TintingAct.Table_Colorant_En[i] == TRUE) {
                    if (Circuit_step_temp_1[i] == 0) {
                        Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                        return PROC_FAIL;
                    }
                }
                else if (TintingAct.Table_Colorant_En[i] == FALSE) {
                    if (Circuit_step_temp_1[i] != 0) {
                        Table.errorCode = TINTING_TABLE_TEST_ERROR_ST;
                        return PROC_FAIL;
                    }
                }
            }
            // Rotate CCW motor Table till Photocell transition LIGHT-DARK
            StartStepper(MOTOR_TABLE, TintingAct.Low_Speed_Rotating_Table, CCW, LIGHT_DARK, TABLE_PHOTOCELL, 0);
            StartTimer(T_TABLE_WAITING_TIME);
            Table.step++;
            break;

        case STEP_8:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                Steps_Todo = -(signed long) (TintingAct.Steps_Reference / 2);
                ;
                StopTimer(T_TABLE_WAITING_TIME);
                // Table CCW rotation
                MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
                Table.step++;
            }
            else if (StatusTimer(T_TABLE_WAITING_TIME) == T_ELAPSED) {
                StopTimer(T_TABLE_WAITING_TIME);
                Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                return PROC_FAIL;
            }
            break;

        case STEP_9:
            if (Status_Board_Table.Bit.MOT_STATUS == 0) {
                StopStepper(MOTOR_TABLE);
                ret = PROC_OK;
            }
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
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

unsigned char TableStepsPositioningColorSupply(void) {
    unsigned char ret = PROC_RUN;
    static signed long Steps_Todo, Steps_Position;
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    // Table Panel OPEN
    if (TintingAct.PanelTable_state == OPEN) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
        return PROC_FAIL;
    }        // Bases Carriage Open
    else if (TintingAct.BasesCarriageOpen == OPEN) {
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        //Table.step = STEP_3;
        Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
        return PROC_FAIL;
    }        // Check for Motor Table Error
    else if (Status_Board_Table.Bit.OCD == 0) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
        return PROC_FAIL;
    }
    else if (Status_Board_Table.Bit.UVLO == 0) { //|| (Status_Board_Table.Bit.UVLO_ADC == 0) ) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
        return PROC_FAIL;
    }
    else if ((Status_Board_Table.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Table.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE)) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
        return PROC_FAIL;
    }
    else if (Table_Steps_Positioning_Photocell_Ctrl == TRUE) {
        if ((PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)) {
            StopTimer(T_TABLE_WAITING_TIME);
            Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
        else if (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) {
            StopTimer(T_TABLE_WAITING_TIME);
            Table.errorCode = TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST;
            return PROC_FAIL;
        }
//        else if (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) {
        else if (CouplingPhotocell_sts == DARK) {
            StopTimer(T_TABLE_WAITING_TIME);
            step_error = 6;                            
            Table.errorCode = TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST;
            return PROC_FAIL;
        }
    }
    else if ((TintingAct.Cleaning_duration != 0) && (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT)) {
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }
    else if (ManageTableHomePosition() == FALSE) {
        // Loss of Steps  
        StopTimer(T_TABLE_WAITING_TIME);
        Table.errorCode = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }

    if (isColorCmdStopProcess()) {
        //        StopStepper(MOTOR_TABLE);
        SoftStopStepper(MOTOR_TABLE);
        StopTimer(T_TABLE_WAITING_TIME);
        Table.step = STEP_3;
    }
    //--------------------------------------------------------------------------
    switch (Table.step) {
            // ---------------------------------------------------------------------     
            // Starts operations
        case STEP_0:
            Status.errorCode = 0;
            Reference = REFERENCE_STEP_0;
            // Analyze Command parameters:
            if (((TintingAct.Rotation_Type != ABSOLUTE) && (TintingAct.Rotation_Type != INCREMENTAL)) ||
                    ((TintingAct.Direction != CW) && (TintingAct.Direction != CCW)) ||
                    (TintingAct.Steps_N > TintingAct.Steps_Revolution)) {
                Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
                return PROC_FAIL;
            }
            // Read current position
            Steps_Position = (signed long) GetStepperPosition(MOTOR_TABLE);
            if (Steps_Position < 0) {
                Steps_Position = (-Steps_Position) % TintingAct.Steps_Revolution;
                if (Steps_Position != 0)
                    Steps_Position = TintingAct.Steps_Revolution - Steps_Position;
            } else
                Steps_Position = Steps_Position % TintingAct.Steps_Revolution;

            Table.step++;
            break;

            // Start Table Rotation
        case STEP_1:
            // Absolute movement
            if (TintingAct.Rotation_Type == ABSOLUTE) {
                // Steps_Todo > 0  --> CW
                // Steps_Todo <= 0 --> CCW
                // Case 1
                if (((signed long) (TintingAct.Steps_N - Steps_Position) >= 0) && ((signed long) (TintingAct.Steps_N - Steps_Position) < (TintingAct.Steps_Revolution / 2)))
                    Steps_Todo = -(signed long) (TintingAct.Steps_N - Steps_Position);
                    // Case 2
                else if (((signed long) (TintingAct.Steps_N - Steps_Position) >= 0) && ((signed long) (TintingAct.Steps_N - Steps_Position) >= (TintingAct.Steps_Revolution / 2)))
                    Steps_Todo = TintingAct.Steps_Revolution - (TintingAct.Steps_N - Steps_Position);
                    // Case 3
                else if (((signed long) (TintingAct.Steps_N - Steps_Position) < 0) && ((signed long) (TintingAct.Steps_N - Steps_Position) > -(signed long) (TintingAct.Steps_Revolution / 2)))
                    Steps_Todo = Steps_Position - TintingAct.Steps_N;
                    // Case 4
                else if (((signed long) (TintingAct.Steps_N - Steps_Position) < 0) && ((signed long) (TintingAct.Steps_N - Steps_Position) <= -(signed long) (TintingAct.Steps_Revolution / 2)))
                    Steps_Todo = -(signed long) (TintingAct.Steps_Revolution - (Steps_Position - TintingAct.Steps_N));
            }// Incremental movement
            else {
                if (TintingAct.Direction == CW)
                    Steps_Todo = TintingAct.Steps_N;
                else
                    Steps_Todo = -(signed long) TintingAct.Steps_N;
            }
            MoveStepper(MOTOR_TABLE, Steps_Todo, TintingAct.Low_Speed_Rotating_Table);
            Table.step++;
            break;

            // Waiting to reach the desired position
        case STEP_2:
            if (Status_Board_Table.Bit.MOT_STATUS == 0)
                Table.step++;
            break;

        case STEP_3:
            StopStepper(MOTOR_TABLE);
            ret = PROC_OK;
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
    }
    return ret;
}

/*
 *//*=====================================================================*//**
**      @brief Table Running
**
**      @param void
**
**      @retval PROC_RUN/PROC_OK/PROC_FAIL
**
*//*=====================================================================*//**
*/

unsigned char TableRun(void) {
    unsigned char ret = PROC_RUN;
    //----------------------------------------------------------------------------
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);

    // Table Panel OPEN
    if (TintingAct.PanelTable_state == OPEN) {
        Table.errorCode = TINTING_PANEL_TABLE_ERROR_ST;
        return PROC_FAIL;
    }// Bases Carriage Open
    else if (TintingAct.BasesCarriageOpen == OPEN) {
        SoftStopStepper(MOTOR_TABLE);
        //return PROC_OK;
        Table.errorCode = TINTING_BASES_CARRIAGE_ERROR_ST;
        return PROC_FAIL;
    }// Check for Motor Table Error
    else if (Status_Board_Table.Bit.OCD == 0) {
        Table.errorCode = TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST;
        return PROC_FAIL;
    }
    else if (Status_Board_Table.Bit.UVLO == 0) { //|| (Status_Board_Table.Bit.UVLO_ADC == 0) ) {
        Table.errorCode = TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST;
        return PROC_FAIL;
    }
    else if ((Status_Board_Table.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Table.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE)) {
        Table.errorCode = TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
        return PROC_FAIL;
    }
    else if ((PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT)) {
        Table.errorCode = TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }
    else if ((TintingAct.Cleaning_duration != 0) && (PhotocellStatus(BRUSH_PHOTOCELL, FILTER) == LIGHT)) {
        Table.errorCode = TINTING_BRUSH_READ_LIGHT_ERROR_ST;
        return PROC_FAIL;
    }
    //----------------------------------------------------------------------------
    switch (Table.step) {
            // -----------------------------------------------------------------------------     
            // CHECK FOR TABLE RUN / STOP    
        case STEP_0:
            // Motor Table START / STOP    
            if (PeripheralAct.Peripheral_Types.RotatingTable == ON) {
                if (TintingAct.Output_Act == OUTPUT_ON) {
                    if (TintingAct.RotatingTable_state == OFF)
                        TintingAct.RotatingTable_state = ON;
                    else
                        return PROC_OK;
                }
                else {
                    if (TintingAct.RotatingTable_state == ON)
                        TintingAct.RotatingTable_state = OFF;
                    else
                        return PROC_OK;
                }
            }
            else
                return PROC_OK;

            Table.step++;
            break;
            // -----------------------------------------------------------------------------          
        case STEP_1:
            if (TintingAct.RotatingTable_state == ON)
                // Run Table in CW rotation at 'TintingAct.High_Speed_Rotating_Table' speed
                STEPPER_TABLE_ON();
            else
                STEPPER_TABLE_OFF();

            ret = PROC_OK;
            break;

        default:
            Table.errorCode = TINTING_TABLE_SOFTWARE_ERROR_ST;
            ret = PROC_FAIL;
            break;
    }
    return ret;
}
