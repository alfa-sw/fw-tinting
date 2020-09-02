/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "tintingmanager.h"
#include "humidifierManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "stepper.h"
#include <xc.h>
#include "typedef.h"
#include "eepromManager.h"
#include "mem.h"
#include "serialCom_GUI.h"
#include "eepromManager.h"
#include "eeprom.h"
#include "stepperParameters.h"
#include "stepper.h"
#include "colorAct.h"
#include "L6482H.h"

static cSPIN_RegsStruct_TypeDef  cSPIN_RegsStruct1 = {0};  //to set

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
void initStatusManager(void)
{
	Status.level = TINTING_INIT_ST; 
    MachineStatus.level = INIT_ST;
    Enable_Driver(MOTOR_PUMP);
    Enable_Driver(MOTOR_TABLE);
    Enable_Driver(MOTOR_VALVE);
    // Motors configuration
    // PUMP Motor    
    ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);
    // VALVE Motor    
    ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, ACC_RATE_VALVE, DEC_RATE_VALVE, ALARMS_VALVE);    
    // TABLE Motor
    ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);    
    
    SetStepperHomePosition(MOTOR_TABLE);
    SetStepperHomePosition(MOTOR_PUMP);
    SetStepperHomePosition(MOTOR_VALVE);
    Status_Board_Pump.word = GetStatus(MOTOR_PUMP);
    Status_Board_Valve.word = GetStatus(MOTOR_VALVE);
    Status_Board_Table.word = GetStatus(MOTOR_TABLE);          
    eeprom_retries = 0;
    StopTimer(T_RESET); 
    Photo_Ingr_Read_Dark_Counter_Error = 0;
    Photo_Ingr_Read_Light_Counter_Error = 0;
    Max_Retry_Photo_Ingr_Error = 0;
    Photo_Ingr_Direction = 0;
}

/*
*//*=====================================================================*//**
**      @brief Updates Tinting general status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void TintingManager(void)
{
    unsigned char i,j;
    unsigned short find_circ;    
    unsigned char currentReg;
    unsigned short calcCrc, readCrc;

    TintingAct.TintingFlags.allFlags = 0L;
  	TintingAct.TintingFlags_1.allFlags = 0L;
  	TintingAct.TintingFlags_2.allFlags = 0L;
    
    switch (Status.level)
	{        
        case TINTING_INIT_ST: 
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			set_slave_status(slave_id, 0);            
            Table_circuits_pos = OFF;
            Total_circuit_n = 0;
            EEprom_Crc_Error = 0;
            if (isTintingEnabled() ) {
                // Load Table Circuits Position 
                EEPROMReadArray(EE_CRC_VALUE_CIR_STEPS_POS, EE_CRC_SIZE,((unsigned char *) &readCrc));
                calcCrc = loadEEParamCirStepsPos();
                if (readCrc != calcCrc)
                    InitFlags.CRCCircuitStepsPosFailed = TRUE;
                else
                    InitFlags.CRCCircuitStepsPosFailed = FALSE;

                if (InitFlags.CRCCircuitStepsPosFailed == FALSE) {
                    for (i = 0; i < MAX_COLORANT_NUMBER; i++) { 
                        TintingAct.Circuit_step_pos[i] = CircStepPosAct.Circ_Pos[i];
                        if ( (TintingAct.Circuit_step_pos[i] > 0) && (TintingAct.Circuit_step_pos[i] != 0xFFFFFFFF) )  {
                            Total_circuit_n++;
                            Table_circuits_pos = ON;
                        }    
                    }
                    Last_Circ = 0xFF;
                    TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0]; 
                    // DYNAMIC circuit position model: Table without zeros
                    if(TintingAct.Table_Step_position == DYNAMIC) {
                        for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
                            find_circ = FALSE;
                            for (i = 0; i < Total_circuit_n; i++) {
                                if ( ((TintingAct.Circuit_step_theorical_pos[j] > TintingAct.Circuit_step_pos[i]) && ((TintingAct.Circuit_step_theorical_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit) ) ||
                                     ((TintingAct.Circuit_step_theorical_pos[j] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - TintingAct.Circuit_step_theorical_pos[j]) <= TintingAct.Steps_Tolerance_Circuit) ) ) {                      
                                    Circuit_step_tmp[j] = TintingAct.Circuit_step_pos[i];
                                    find_circ = TRUE;
                                    Last_Circ = j;
                                    break;                        
                                }
                            }
                            // Circuit 'j' is not present on the Table
                            if (find_circ == FALSE)
                                Circuit_step_tmp[j] = 0;
                        }            
                    }
                    // STATIC circuit position model: Table with zeros
                    else {
                        for (i = 0; i < MAX_COLORANT_NUMBER; i++)  {  
                            Circuit_step_tmp[i] = TintingAct.Circuit_step_pos[i];               
                            if ( (TintingAct.Circuit_step_pos[i] > 0) && (TintingAct.Circuit_step_pos[i] != 0xFFFFFFFF) ) 
                                Last_Circ = i;
                        }                           
                    }

                    for (i = 0; i < MAX_COLORANT_NUMBER; i++)  {                  
                        Circuit_step_original_pos[i] = 0;
                        if (Circuit_step_tmp[i] != 0)
                            Circuit_step_original_pos[i] = Circuit_step_tmp[i];
                        else
                            Circuit_step_original_pos[i] = TintingAct.Circuit_step_theorical_pos[i];
                    }                            
    /*                
                    if (Table_circuits_pos == OFF)
                        Status.level = TINTING_LACK_CIRCUITS_POSITION_ERROR_ST;
                    else
                        Status.level++;
                }            
                else 
                    Status.level = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
    */
                    Status.level++;                    
                }
                else {
                    eeprom_retries++;
                    if (eeprom_retries == MAX_EEPROM_RETRIES) {         
                        eeprom_retries = 0;
                        EEprom_Crc_Error = 1;
                        Status.level++;                   
                    }                    
                }
            }    
//Status.level++;        
        break;

        case TINTING_READY_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			TintingAct.TintingFlags.tinting_ready = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			set_slave_status(slave_id, 0);
			setAttuatoreAttivo(slave_id,0);	

            if ( (PositioningCmd == 1) && (End_Table_Position == 1) ) {
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                    // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
//                    ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                                              
                    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                    cSPIN_RegsStruct1.TVAL_HOLD = currentReg;          
                    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct1.TVAL_HOLD, MOTOR_TABLE);
                    
                    End_Table_Position = 0;
                    Status.level = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                }
                else if (TintingAct.PanelTable_state == CLOSE) {
                    End_Table_Position = 0;
                    Status.level = TINTING_TABLE_GO_REFERENCE_ST;
                }    
            }
/*            
            if (TintingAct.typeMessage == CONTROLLO_PRESENZA) {
                if (isColorCmdStop())
                    Status.level = TINTING_STOP_ST;
            }
*/            
            if ( isColorCmdStopProcess() && (TintingAct.RotatingTable_state == ON) ) {
                TintingAct.RotatingTable_state = OFF;
                STEPPER_TABLE_OFF();
            }
            else if ( (isColorCmdStopProcess() || isColorCmdStop() ) && (Punctual_Clean_Act == ON) ) { 
                Punctual_Clean_Act = OFF;
                TintingAct.Cleaning_status = 0x00;            
                SPAZZOLA_OFF();            
            }                                
            // 'POS_HOMING' command Recived
            else if (isColorCmdHome() ) {
                StartTimer(T_RESET);                
                if ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) )
                    Valve_Position = UNDETERMINED;
                else
                    Valve_Position = DETERMINED;
/*                
                if ( ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) )
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) )                    
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) )                    
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) )                    
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) || (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == LIGHT) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) ) )
*/
                if ( ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (CouplingPhotocell_sts == DARK) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) )
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (CouplingPhotocell_sts == DARK) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) )                    
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (CouplingPhotocell_sts == DARK) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) )                    
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) || (CouplingPhotocell_sts == LIGHT) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) )                    
                                                                        ||
                    ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) || (CouplingPhotocell_sts == LIGHT) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) ) )                
//                    Status.level = TINTING_PUMP_SEARCH_HOMING_ST;
                      Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST;                    
                
                else  if ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) && (CouplingPhotocell_sts == LIGHT) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == DARK) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == DARK) ) )                
                    Status.level = TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST;
                
                else  if ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == DARK) && (CouplingPhotocell_sts == LIGHT) ) &&
                       ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) )                
//                    Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_HOMING_ST;
//                      Status.level = TINTING_PUMP_SEARCH_HOMING_ST;
                      Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST;                    
                
//                if ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(COUPLING_PHOTOCELL, FILTER) == DARK) ) &&
                if ( ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (CouplingPhotocell_sts == DARK) ) &&
                    ( (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) && (PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER) == LIGHT) ) )                
//                    Status.level = TINTING_LIGHT_VALVE_PUMP_SEARCH_HOMING_ST; 
//                      Status.level = TINTING_PUMP_SEARCH_HOMING_ST;                   
                      Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST;                                        
            }
            
            else if (isColorCmdSetupOutput() && 
               ( (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) ||
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) ) ) {
                TintingNoCmd();
                Status.level = TINTING_WAIT_SETUP_OUTPUT_VALVE_ST;
            }
            else if (isColorCmdSetupOutput() && 
               (PeripheralAct.Peripheral_Types.Rotating_Valve == ON) ) {
                TintingNoCmd();
                Status.level = TINTING_WAIT_SETUP_OUTPUT_VALVE_ST;
            }            
            else if (isColorCmdSetupOutput() && (PeripheralAct.Peripheral_Types.RotatingTable == ON) ) {
                TintingNoCmd();
                Status.level = TINTING_WAIT_SETUP_OUTPUT_TABLE_ST;
            }
            
            else if (isColorCmdRecirc() ) {
                TintingAct.Last_Cmd_Reset = OFF;
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;
                if (TintingAct.N_cycles == 9999)
                    // Before to Start Ricirculation it is necessary to do Stirring: 1 full 360° table rotation in CW and CCW
                    RicirculationCmd = 1;                        
                else
                    // Before to Start Pre Dispensation Ricirculation no STIRRING
                    RicirculationCmd = 0;                        
                    
                NextStatus.level = TINTING_STANDBY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }
            else if (TintingAct.typeMessage == AGITAZIONE_COLORE) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;                  
                TintingAct.Color_Id = Last_Circ + 1;
                RicirculationCmd = 0;
                Stirr_After_Last_Ricirc = FALSE;
                NextStatus.level = TINTING_TABLE_STIRRING_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;  
            }
            else if ( (TintingAct.typeMessage == DISPENSAZIONE_COLORE) ||
                      (TintingAct.typeMessage == DISPENSAZIONE_COLORE_CONT) ) {              
//            else if (isColorCmdSupply() ) {
                setTintingActMessage(NO_MESSAGE);  
                TintingAct.Last_Cmd_Reset = OFF;
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;  
                NextStatus.level = TINTING_SUPPLY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;                               
            }                
            else if (TintingAct.typeMessage == POSIZIONAMENTO_TAVOLA_ROTANTE) {
//                setTintingActMessage(NO_MESSAGE);  
                TintingAct.Last_Cmd_Reset = OFF;
                NextStatus.level = TINTING_TABLE_POSITIONING_ST;                
                Status.level = TINTING_TABLE_POSITIONING_ST;                       
            }
            else if (TintingAct.typeMessage == POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE) {
                if (isAllCircuitsHome() ) {
                    setTintingActMessage(NO_MESSAGE);  
                    TintingAct.Last_Cmd_Reset = OFF;
                    Status.level = TINTING_TABLE_STEPS_POSITIONING_ST;  
                }     
            }         
            else if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
                if (isAllCircuitsHome() ) {
                    setTintingActMessage(NO_MESSAGE);  
                    Status.level = TINTING_TABLE_SELF_RECOGNITION_ST;
                }     
            }  
            else if (TintingAct.typeMessage == RICERCA_RIFERIMENTO_TAVOLA_ROTANTE) {
                if (isAllCircuitsHome() ) {
                    setTintingActMessage(NO_MESSAGE);  
                    TintingAct.Last_Cmd_Reset = OFF;
                    Status.level = TINTING_TABLE_FIND_REFERENCE_ST;
                }     
            }              
            else if (TintingAct.typeMessage == TEST_FUNZIONAMENTO_TAVOLA_ROTANTE) {
                if (isAllCircuitsHome() ) {
                    setTintingActMessage(NO_MESSAGE);  
                    TintingAct.Last_Cmd_Reset = OFF;
                    Status.level = TINTING_TABLE_TEST_ST;                
                }                                
            }                
            else if (TintingAct.typeMessage == ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE) {
                //if (isAllCircuitsHome() ) {
                if (isTintingReady() ) {
                    setTintingActMessage(NO_MESSAGE);  
                    TintingAct.Last_Cmd_Reset = OFF;
                    indx_Clean = 0;                    
                    Status.level = TINTING_TABLE_CLEANING_ST;  
                    if (Punctual_Clean_Act == OFF)
                        NEW_Calculates_Cleaning_Tinting_Colorants_Order();
                }                                                    
            }                                        
        break;
// HOMING ----------------------------------------------------------------------
// (HOME_PHOTOCELL = LIGHT OR COUPLING_PHOTOCELL = DARK) AND (VALVE_PHOTOCELL = DARK) AND ((VALVE_OPEN_PHOTOCELL = DARK)
//                                          OR
// (HOME_PHOTOCELL = LIGHT OR COUPLING_PHOTOCELL = DARK) AND (VALVE_PHOTOCELL = LIGHT) AND ((VALVE_OPEN_PHOTOCELL = DARK)        
//                                          OR
// (HOME_PHOTOCELL = LIGHT OR COUPLING_PHOTOCELL = DARK) AND (VALVE_PHOTOCELL = DARK) AND ((VALVE_OPEN_PHOTOCELL = LIGHT)        
//                                          OR
// (HOME_PHOTOCELL = DARK AND COUPLING_PHOTOCELL = LIGHT) AND (VALVE_PHOTOCELL = DARK) AND ((VALVE_OPEN_PHOTOCELL = DARK)
//                                          OR
// (HOME_PHOTOCELL = DARK AND COUPLING_PHOTOCELL = LIGHT) AND (VALVE_PHOTOCELL = LIGHT) AND ((VALVE_OPEN_PHOTOCELL = DARK)        
//                                          OR
// (HOME_PHOTOCELL = DARK AND COUPLING_PHOTOCELL = LIGHT) AND (VALVE_PHOTOCELL = DARK) AND ((VALVE_OPEN_PHOTOCELL = LIGHT)
//1. Pump Homing 
//2. Valve Homing
//3. Table Homing        
        case TINTING_PUMP_SEARCH_HOMING_ST:
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);
            
            if (Pump.level == PUMP_END)
                Status.level = TINTING_PUMP_GO_HOMING_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_PUMP_RESET_ERROR_ST;
            }
            break;    

        case TINTING_PUMP_GO_HOMING_ST:
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);            
            Status.level = TINTING_VALVE_SEARCH_HOMING_ST;            
            break;    

        case TINTING_VALVE_SEARCH_HOMING_ST:
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);            
            if (Pump.level == PUMP_END)
                Status.level = TINTING_VALVE_GO_HOMING_ST;  
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_VALVE_RESET_ERROR_ST;
            }            
        break;

        case TINTING_VALVE_GO_HOMING_ST:
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);            
            Status.level = TINTING_TABLE_SEARCH_HOMING_ST;                        
        break;
            
        case TINTING_TABLE_SEARCH_HOMING_ST:
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);            
            if (Table.level == TABLE_END)
                Status.level = TINTING_TABLE_GO_HOMING_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_TABLE_RESET_ERROR_ST;
            }                        
        break;
        
        case TINTING_TABLE_GO_HOMING_ST:
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);            
            Status.level = TINTING_HOMING_ST;                                
        break;
                                                    
// (HOME_PHOTOCELL = DARK AND COUPLING_PHOTOCELL = LIGHT) AND (VALVE_PHOTOCELL = DARK) AND ((VALVE_OPEN_PHOTOCELL = DARK)
//1. Table Homing 
//2. Pump Homing
//3. Valve Homing         
        case TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_PHOTO_DARK_TABLE_GO_HOMING_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_TABLE_RESET_ERROR_ST;
            }                        
        break;

        case TINTING_PHOTO_DARK_TABLE_GO_HOMING_ST:
            Status.level = TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST;                                    
        break;
        
        case TINTING_PHOTO_DARK_PUMP_SEARCH_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_PHOTO_DARK_PUMP_GO_HOMING_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_PUMP_RESET_ERROR_ST;
            }
            break;    

        case TINTING_PHOTO_DARK_PUMP_GO_HOMING_ST:
            Status.level = TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST;            
        break;  
        
        case TINTING_PHOTO_DARK_VALVE_SEARCH_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_PHOTO_DARK_VALVE_GO_HOMING_ST;  
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_VALVE_RESET_ERROR_ST;
            }            
        break;
        
        case TINTING_PHOTO_DARK_VALVE_GO_HOMING_ST:
            Status.level = TINTING_HOMING_ST;                        
        break;  
        
// (HOME_PHOTOCELL = DARK AND COUPLING_PHOTOCELL = LIGHT) AND ( (VALVE_PHOTOCELL = LIGHT) AND (VALVE_OPEN_PHOTOCELL = LIGHT) )
//1. Table Steps (Engaged)
//2. Valve Homing                 
//3. Pump Homing
//4. Table Homing
//5. New Valve Homing         
        //1. Table Steps (Not Engaged)  
/*        
        case TINTING_PHOTO_LIGHT_VALVE_SEARCH_HOMING_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_GO_TABLE_NOT_ENGAGED_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_TABLE_RESET_ERROR_ST;
            }                                    
        break;              
        
        case TINTING_PHOTO_LIGHT_VALVE_GO_TABLE_NOT_ENGAGED_ST:
            Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST;                                                
        break;              
*/
        //2. Valve Homing                 
        case TINTING_PHOTO_LIGHT_VALVE_SEARCH_VALVE_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_GO_VALVE_HOMING_ST;  
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_VALVE_RESET_ERROR_ST;
            }                        
        break;      

        case TINTING_PHOTO_LIGHT_VALVE_GO_VALVE_HOMING_ST:
            Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_PUMP_HOMING_ST;                                                            
        break;      

        //3. Pump Homing                
        case TINTING_PHOTO_LIGHT_VALVE_SEARCH_PUMP_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_GO_PUMP_HOMING_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_PUMP_RESET_ERROR_ST;
            }            
        break;    
        
        case TINTING_PHOTO_LIGHT_VALVE_GO_PUMP_HOMING_ST:
            Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_HOMING_ST;                                                                        
        break;    

        //4. Table Homing         
        case TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_HOMING_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_GO_TABLE_HOMING_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_TABLE_RESET_ERROR_ST;
            }                                    
        break;    
            
        case TINTING_PHOTO_LIGHT_VALVE_GO_TABLE_HOMING_ST:
Valve_Position = DETERMINED;
            if (Valve_Position == UNDETERMINED)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_NEW_SEARCH_VALVE_HOMING_ST;                        
            else
                Status.level = TINTING_HOMING_ST;                                        
        break;    

        //5. New Valve Homing (only if at the beginning Valve position is not determined)        
        case TINTING_PHOTO_LIGHT_VALVE_NEW_SEARCH_VALVE_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_NEW_GO_VALVE_HOMING_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_VALVE_RESET_ERROR_ST;
            }                                    
        break;    
            
        case TINTING_PHOTO_LIGHT_VALVE_NEW_GO_VALVE_HOMING_ST:
            Status.level = TINTING_HOMING_ST;                        
        break;    
        
// (HOME_PHOTOCELL = LIGHT OR COUPLING_PHOTOCELL = DARK) AND ( (VALVE_PHOTOCELL = LIGHT) AND (VALVE_OPEN_PHOTOCELL = LIGHT) )
//1. Pump Homing
//2. Table Steps (Engaged)
//3. Valve Homing                 
//4. Table Homing 
        //1. Pump Homing        
        case TINTING_LIGHT_VALVE_PUMP_SEARCH_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_LIGHT_VALVE_PUMP_GO_HOMING_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_PUMP_RESET_ERROR_ST;
            }                        
        break;              
        
        case TINTING_LIGHT_VALVE_PUMP_GO_HOMING_ST:
            Status.level = TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_NOT_ENGAGED_ST;                                                                        
        break;    

        //2. Table Steps (Engaged)        
        case TINTING_PHOTO_LIGHT_VALVE_SEARCH_TABLE_NOT_ENGAGED_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_TABLE_NOT_ENGAGED_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_TABLE_RESET_ERROR_ST;
            }                                                
        break;    
        
        case TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_TABLE_NOT_ENGAGED_ST:
            Status.level = TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_VALVE_HOMING_ST;                                                        
        break;
        
        //2. Valve Homing                  
        case TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_VALVE_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_VALVE_HOMING_ST;  
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_VALVE_RESET_ERROR_ST;
            }                                    
        break;
        
        case TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_VALVE_HOMING_ST:
            Status.level = TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_TABLE_HOMING_ST;                                                                                    
        break;    

        //4. Table Homing         
        case TINTING_PHOTO_LIGHT_VALVE_PUMP_SEARCH_TABLE_HOMING_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_TABLE_HOMING_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_TABLE_RESET_ERROR_ST;
            }                                    
        break;    
            
        case TINTING_PHOTO_LIGHT_VALVE_PUMP_GO_TABLE_HOMING_ST:
            Status.level = TINTING_HOMING_ST;                        
        break;    
// -----------------------------------------------------------------------------      
        case TINTING_HOMING_ST:
			TintingAct.TintingFlags.tinting_homing = TRUE;
			set_slave_status(slave_id, 0);
			setAttuatoreAttivo(slave_id,0);	
            
            if (TintingAct.typeMessage == CONTROLLO_PRESENZA) {
                if (isColorCmdIntr())
                    Status.level = TINTING_INIT_ST;
                else if (isColorCmdStop())
                    Status.level = TINTING_STOP_ST;
                if (isColorCmdStopProcess())
                    Status.level = TINTING_INIT_ST;                
            }
            
            else if (isColorCmdSetupOutput() && 
               ( (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) ||
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) ) )                     
                Status.level = TINTING_WAIT_SETUP_OUTPUT_VALVE_ST;
            else if (isColorCmdSetupOutput() && (PeripheralAct.Peripheral_Types.RotatingTable == ON) )
                Status.level = TINTING_WAIT_SETUP_OUTPUT_TABLE_ST;                         
            else if (isColorCmdSetupOutput() && (PeripheralAct.Peripheral_Types.Rotating_Valve == ON) )
                Status.level = TINTING_WAIT_SETUP_OUTPUT_VALVE_ST;
            
            else if (TintingAct.typeMessage == RICIRCOLO_COLORE) { 
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;                
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;
                if (TintingAct.N_cycles == 9999)
                    // Before to Start Ricirculation it is necessary to do Stirring: 1 full 360° table rotation in CW and CCW
                    RicirculationCmd = 1;                        
                else
                    // Before to Start Pre Dispensation Ricirculation no STIRRING
                    RicirculationCmd = 0;                        
                
                NextStatus.level = TINTING_STANDBY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }
            else if (TintingAct.typeMessage == AGITAZIONE_COLORE) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;                
//                Status.level = TINTING_TABLE_STIRRING_ST;
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;                  
                TintingAct.Color_Id = Last_Circ + 1;
                Stirr_After_Last_Ricirc = FALSE;
                NextStatus.level = TINTING_TABLE_STIRRING_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;                
            }            
            else if ( (TintingAct.typeMessage == DISPENSAZIONE_COLORE) ||
                      (TintingAct.typeMessage == DISPENSAZIONE_COLORE_CONT) ) {  
//            else if (isColorCmdSupply() ) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;                
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;  
                NextStatus.level = TINTING_SUPPLY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }                
            else if (TintingAct.typeMessage == POSIZIONAMENTO_TAVOLA_ROTANTE) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;                
                Status.level = TINTING_TABLE_POSITIONING_ST;                       
            }
            else if (TintingAct.typeMessage == POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;                
                Status.level = TINTING_TABLE_STEPS_POSITIONING_ST;                       
            }
            else if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
                setTintingActMessage(NO_MESSAGE);                  
                Status.level = TINTING_TABLE_SELF_RECOGNITION_ST;
            }
            else if (TintingAct.typeMessage == RICERCA_RIFERIMENTO_TAVOLA_ROTANTE) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;                
                Status.level = TINTING_TABLE_FIND_REFERENCE_ST;
            }                          
            else if (TintingAct.typeMessage == TEST_FUNZIONAMENTO_TAVOLA_ROTANTE) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;                
                Status.level = TINTING_TABLE_TEST_ST;                
            }                            
            else if (TintingAct.typeMessage == ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE) {
                setTintingActMessage(NO_MESSAGE);                  
                TintingAct.Last_Cmd_Reset = OFF;
                indx_Clean = 0;
                if (Punctual_Clean_Act == OFF)
                    NEW_Calculates_Cleaning_Tinting_Colorants_Order();
                Status.level = TINTING_TABLE_CLEANING_ST;                                
            }                                        
        break;
// STOP ------------------------------------------------------------------------                        
        case TINTING_STOP_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			set_slave_status(slave_id, 0);            
            SoftHiZ_Stepper(MOTOR_TABLE);
            HardHiZ_Stepper(MOTOR_VALVE);
            HardHiZ_Stepper(MOTOR_PUMP);
            StopHumidifier();            
            StopTimer(T_RESET);
            if (isColorCmdIntr()) {
                Status.level = TINTING_INIT_ST;
                Table.level  = TABLE_IDLE;    
                Pump.level   = PUMP_IDLE;
            }    
        break;
// SETUP OUTPUT VALVE ----------------------------------------------------------                        
        case TINTING_WAIT_SETUP_OUTPUT_VALVE_ST:
            if (Pump.level == PUMP_PAR_RX)
                Status.level = TINTING_SETUP_OUTPUT_VALVE_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST;                                
        break;

        case TINTING_SETUP_OUTPUT_VALVE_ST:
			TintingAct.TintingFlags_1.tinting_table_setup_output_valve = TRUE;
			set_slave_status(slave_id, 1);            
            if (Pump.level == PUMP_END) 
                Status.level = TINTING_READY_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;              
        break;                
// SETUP OUTPUT TABLE ----------------------------------------------------------                        
        case TINTING_WAIT_SETUP_OUTPUT_TABLE_ST:
            if (Table.level == TABLE_PAR_RX)
                Status.level = TINTING_SETUP_OUTPUT_TABLE_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST;                                
        break;
        
        case TINTING_SETUP_OUTPUT_TABLE_ST:
            if (Table.level == TABLE_END) 
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;              
        break;                
// RICIRCULATION ---------------------------------------------------------------
        case TINTING_STANDBY_RUN_ST:
			TintingAct.TintingFlags.tinting_recirc_run = TRUE;
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);            
//            if (isColorCmdStop())
//                Status.level = TINTING_STANDBY_END_ST;
//            if (Pump.level == PUMP_END)
//            if ( (Pump.level == PUMP_END) && (isColorCmdRecirc()) )
            if (Pump.level == PUMP_END) {
                // Stirring at the End of last Circuit Configured Ricirculation 
                if ( (Stirring_Method == AFTER_LAST_RICIRCULATING_CIRCUIT) && (RicirculationCmd == 1) &&  
                     (TintingAct.Steps_Stirring > 0) && ((TintingAct.Color_Id - 1) == Last_Circ) ) {
                    if (New_Erogation == TRUE)  {
                        Status.level = TINTING_STANDBY_END_ST;
                    }                        
                    else
                        Status.level = TINTING_TABLE_STIRRING_ST;                                    
                }
                else
                    Status.level = TINTING_STANDBY_END_ST;                
            }     
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;              
        break;

        case TINTING_STANDBY_END_ST:
			TintingAct.TintingFlags.tinting_recirc_end = TRUE;
			set_slave_status(slave_id, 0);
			setAttuatoreAttivo(slave_id,0);            
            if (isColorCmdStop() || isColorCmdStopProcess()) {
//                StopStepper(MOTOR_TABLE);
//                StopStepper(MOTOR_VALVE);
//                StopStepper(MOTOR_PUMP);
// Qui forse andrebbe impostato l'Homing della pompa e della valvola
                Status.level = TINTING_READY_ST;
            }     
        break;        
// TABLE POSITIONING -----------------------------------------------------------            
        case TINTING_TABLE_POSITIONING_ST:  
			TintingAct.TintingFlags_1.tinting_table_positioning = TRUE;
			set_slave_status(slave_id, 1);            
            if (Table.level == TABLE_END) {
                if (NextStatus.level == TINTING_TABLE_POSITIONING_ST)
                    Status.level = TINTING_READY_ST;
                else
                    Status.level = NextStatus.level;                    
            } 
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                            
        break;        
// STIRRING --------------------------------------------------------------------                    
        case TINTING_TABLE_STIRRING_ST:
			TintingAct.TintingFlags_2.tinting_stirring_run = TRUE;
			// All 8 - 23 possible Stirring colorants are ON	
			for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if (TintingAct.Table_Colorant_En[i] == TRUE)
                    procGUI.stirring_status |= (1L << (i + TINTING_COLORANT_OFFSET));
            } 
//            procGUI.stirring_status |= (0xFFFF00);
			
            set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);
            
            if (Table.level == TABLE_END) 
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                            
        break;            
// TABLE STEPS POSITIONING -----------------------------------------------------            
        case TINTING_TABLE_STEPS_POSITIONING_ST:
			TintingAct.TintingFlags_1.tinting_table_steps_positioning = TRUE;
			set_slave_status(slave_id, 1);            
            if (Table.level == TABLE_END)
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                            
        break;        
// TABLE AUTO RECOGNITION ------------------------------------------------------
        case TINTING_TABLE_SELF_RECOGNITION_ST:
			TintingAct.TintingFlags_1.tinting_table_self_recognition = TRUE;
			set_slave_status(slave_id, 1);            
            if (Table.level == TABLE_END)
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                    
        break;   
// TABLE FIND REFERENCE --------------------------------------------------------
        case TINTING_TABLE_FIND_REFERENCE_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                    
        break;                   
// TINTING TABLE GO TO REFERENCE POSITION --------------------------------------
        case TINTING_TABLE_GO_REFERENCE_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                                
        break;    
// TABLE TEST ------------------------------------------------------------------
        case TINTING_TABLE_TEST_ST:
			TintingAct.TintingFlags_1.tinting_table_test = TRUE;
			set_slave_status(slave_id, 1);            
            if (Table.level == TABLE_END)
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                                
        break;    
// CLEANING --------------------------------------------------------------------
        case TINTING_TABLE_CLEANING_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                                        
        break;
// DISPENSING  -----------------------------------------------------------------
        case TINTING_SUPPLY_RUN_ST:
			TintingAct.TintingFlags.tinting_supply_run = TRUE;
			set_slave_status(slave_id, 1);
			setAttuatoreAttivo(slave_id,1);
            
            if (Pump.level == PUMP_END)
                Status.level = TINTING_SUPPLY_END_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                        
        break;
                                
        case TINTING_SUPPLY_END_ST:
			TintingAct.TintingFlags.tinting_supply_end = TRUE;
			set_slave_status(slave_id, 0);
			setAttuatoreAttivo(slave_id,0);            
			if (isColorCmdStop())  {
                Status.level = TINTING_READY_ST; 
            } 
        break;
// CLEANING --------------------------------------------------------------------                                
        // Implementato quando decideremo di inserire la pulizia
        case TINTING_CLEANING_ST:
			TintingAct.TintingFlags.tinting_cleaning = TRUE;
			set_slave_status(slave_id, 1);            
        break;                        
// JUMP TO BOOT
// ------------------------------------------------------------------------------------------------------------        
        case TINTING_JUMP_TO_BOOT:
			TintingAct.TintingFlags.tinting_jump_to_boot = TRUE;
			set_slave_status(slave_id, 0);
            
            SoftStopStepper(MOTOR_TABLE);
            StopStepper(MOTOR_VALVE);
            StopStepper(MOTOR_PUMP);
            StopHumidifier();
            // Se è arrivato il comando di "JUMP_TO_BOOT", ed è terminata la risposta al comando viene effettuato il salto al Boot
 //           if (Start_Jump_Boot == 1)
 //               jump_to_boot();
        break;
   
        case TINTING_PUMP_RESET_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;	
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_pump_reset_error = TRUE;
			set_slave_status(slave_id, 1);        
        break;    
        case TINTING_VALVE_RESET_ERROR_ST: 
			TintingAct.TintingFlags.tinting_stopped = TRUE;	
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_valve_reset_error = TRUE;
			set_slave_status(slave_id, 1);        
        break;    
        case TINTING_TABLE_RESET_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;		
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_table_reset_error = TRUE;
			set_slave_status(slave_id, 1);            
        break;    
		// OVERCURRENT ERROR				
		case TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_pump_motor_overcurrent_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		//  POS0 READ LIGHT Error: Home Photocell is not covered (=LIGHT) after "GO to HOMING position" movement        
        case TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_pos0_read_light_error = TRUE;
			set_slave_status(slave_id, 1);        
        break;    
		// PUMP PHOTO HOME READ DARK Error
		case TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_pump_photo_home_read_dark_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// PUMP SOFTWARE Error
		case TINTING_PUMP_SOFTWARE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_pump_software_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		case TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_valve_motor_overcurrent_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
	    case TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_valve_motor_home_back_error = TRUE;
			set_slave_status(slave_id, 1);
		break;		
		case TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_table_motor_overcurrent_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
        case TINTING_TABLE_HOMING_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;	
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_table_homing_pos_error = TRUE;
			set_slave_status(slave_id, 1);            
        break;    
		// TABLE SEARCH POSITION REFERENCE Error
		case TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_table_search_position_reference_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
        case TINTING_BAD_PAR_PUMP_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;            
			TintingAct.TintingFlags_1.tinting_bad_pump_param_error = TRUE;
			set_slave_status(slave_id, 0);        
        break;    
        case TINTING_BAD_PAR_TABLE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;            
			TintingAct.TintingFlags_1.tinting_bad_table_param_error = TRUE;
			set_slave_status(slave_id, 0);        
        break;    
        case TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;	
			TintingAct.TintingFlags_1.tinting_bad_peripheral_param_error = TRUE;
			set_slave_status(slave_id, 0);            
        break;    
        case TINTING_TABLE_SOFTWARE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;	
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags.tinting_table_software_error = TRUE;
			set_slave_status(slave_id, 1);
        break;
		// TABLE MOVE Error
		case TINTING_TABLE_MOVE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_table_move_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// PUMP PHOTO INGR READ LIGHT Error
		case TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_pump_photo_ingr_read_light_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// SELF LEARNING PROCEDURE Error
		case TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_self_learning_procedure_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// LACK CIRCUITS POSITION Error
		case TINTING_LACK_CIRCUITS_POSITION_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_lack_circuits_position_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// TABLE MISMATCH POSITION Error
		case TINTING_TABLE_MISMATCH_POSITION_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_table_mismatch_position_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// PUMP VALVE POS0 READ LIGHT Error
		case TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_valve_pos0_read_light_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		//  TABLE TEST Error
		case TINTING_TABLE_TEST_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_1.tinting_table_test_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		//  GENERIC 24V OPEN LOAD error
		case TINTING_GENERIC24V_OPEN_LOAD_ERROR_ST: 
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			TintingAct.TintingFlags_1.tinting_generic24v_open_load_error = TRUE;
			set_slave_status(slave_id, 1);		
		break;
		//  GENERIC 24V OVERCURRENT THERMAL error
		case TINTING_GENERIC24V_OVERCURRENT_THERMAL_ERROR_ST: 
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			TintingAct.TintingFlags_1.tinting_generic24v_overcurrent_thermal_error = TRUE;
			set_slave_status(slave_id, 1);		
		break;
		// THERMAL SHUTDOWN ERROR				
		case TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_pump_motor_thermal_shutdown_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		case TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_valve_motor_thermal_shutdown_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		case TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_table_motor_thermal_shutdown_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// UNDER VOLTAGE ERROR				
		case TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_pump_motor_under_voltage_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		case TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_valve_motor_under_voltage_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		case TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_table_motor_under_voltage_error = TRUE;
			set_slave_status(slave_id, 1);
		break;				
		case TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_eeprom_colorants_steps_position_crc_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// TABLE PHOTOCELL READ LIGHT AFTER POSITIONING
		case TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_table_photo_read_light_error = TRUE;
			set_slave_status(slave_id, 1);		
		break;
		// VALVE ALREADY OPEN
		case TINTING_VALVE_OPEN_READ_DARK_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_valve_open_read_dark_error = TRUE;
			set_slave_status(slave_id, 1);		
		break;
		// VALVE NOT CORRECTLY OPENED
		case TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_valve_open_read_light_error = TRUE;
			set_slave_status(slave_id, 1);
		break;
		// TINTING PANEL OPEN
		case TINTING_PANEL_TABLE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			TintingAct.TintingFlags_2.tinting_panel_table_error = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);			
			set_slave_status(slave_id, 1);
		break;            
        case TINTING_VALVE_HOMING_ERROR_ST: 
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);			
			TintingAct.TintingFlags.tinting_stopped = TRUE;			
			TintingAct.TintingFlags.tinting_valve_homing_pos_error = TRUE;
			set_slave_status(slave_id, 1);        
        break;    
        case TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_pump_photo_ingr_read_dark_error = TRUE;
			set_slave_status(slave_id, 1);                        
        break;
        case TINTING_BRUSH_READ_LIGHT_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_brush_read_light_error = TRUE;
			set_slave_status(slave_id, 1);                        
        break;            
/*
        case TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);						
			TintingAct.TintingFlags_2.tinting_eeprom_colorants_steps_position_crc_error = TRUE;
			set_slave_status(slave_id, 1);
            if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
                Status.level = TINTING_TABLE_SELF_RECOGNITION_ST;
            }                            
        break;
*/            
        case TINTING_BAD_PAR_CLEAN_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;            
			TintingAct.TintingFlags_2.tinting_bad_clean_param_error = TRUE;
			set_slave_status(slave_id, 0);                    
        break;

		case TINTING_BASES_CARRIAGE_ERROR_ST:
			TintingAct.TintingFlags.tinting_stopped = TRUE;
			TintingAct.TintingFlags_2.tinting_carriage_bases_error = TRUE;
			procGUI.stirring_status &= ~(0xFFFF00);
			procGUI.recirc_status &= ~(0xFFFF00);			
			set_slave_status(slave_id, 1);
		break;            
        
    	default:
        break;    
    }
    if ( (Status.level == TINTING_PUMP_RESET_ERROR_ST) || (Status.level == TINTING_VALVE_RESET_ERROR_ST) || (Status.level == TINTING_TABLE_RESET_ERROR_ST) || 
              (Status.level == TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST) || (Status.level == TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST) || (Status.level == TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST) ||
              (Status.level == TINTING_PUMP_SOFTWARE_ERROR_ST) || (Status.level == TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST) || (Status.level == TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST) ||
              (Status.level == TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST) || (Status.level == TINTING_TABLE_HOMING_ERROR_ST) || (Status.level == TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST) ||
              (Status.level == TINTING_BAD_PAR_PUMP_ERROR_ST) || (Status.level == TINTING_BAD_PAR_TABLE_ERROR_ST) || (Status.level == TINTING_BAD_PAR_CLEAN_ERROR_ST) ||
              (Status.level == TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST) || (Status.level == TINTING_TABLE_SOFTWARE_ERROR_ST) || (Status.level == TINTING_TABLE_MOVE_ERROR_ST) ||
              (Status.level == TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST) || (Status.level == TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST) || (Status.level == TINTING_LACK_CIRCUITS_POSITION_ERROR_ST) || 
              (Status.level == TINTING_TABLE_MISMATCH_POSITION_ERROR_ST) || (Status.level == TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST) || (Status.level == TINTING_TABLE_TEST_ERROR_ST) || 
              (Status.level == TINTING_GENERIC24V_OPEN_LOAD_ERROR_ST) || 
              (Status.level == TINTING_GENERIC24V_OVERCURRENT_THERMAL_ERROR_ST) || (Status.level == TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST) || (Status.level == TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST) || 
              (Status.level == TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST) || (Status.level == TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST) || (Status.level == TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST) || 
              (Status.level == TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST) || (Status.level == TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST) || (Status.level == TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST) || 
              (Status.level == TINTING_VALVE_OPEN_READ_DARK_ERROR_ST) || (Status.level == TINTING_VALVE_OPEN_READ_LIGHT_ERROR_ST) || 
              (Status.level == TINTING_PANEL_TABLE_ERROR_ST) || (Status.level == TINTING_VALVE_HOMING_ERROR_ST) || (Status.level == TINTING_PUMP_PHOTO_INGR_READ_DARK_ERROR_ST) || (Status.level == TINTING_BRUSH_READ_LIGHT_ERROR_ST) ||
              (Status.level == TINTING_BASES_CARRIAGE_ERROR_ST))  {        
        if (Status.level != TINTING_TABLE_MISMATCH_POSITION_ERROR_ST) {
            HardHiZ_Stepper(MOTOR_VALVE);                        
            SoftHiZ_Stepper(MOTOR_TABLE);
            HardHiZ_Stepper(MOTOR_PUMP);
        }
        if (isColorCmdIntr() )
//                Status.level = TINTING_INIT_ST;		                
            Status.level = TINTING_READY_ST;
//        else if (isColorCmdStop())
//            Status.level = TINTING_STOP_ST;
        else if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
            setTintingActMessage(NO_MESSAGE);              
            Status.level = TINTING_TABLE_SELF_RECOGNITION_ST;
        }                
    }
}    

unsigned char isTintingEnabled(void)
{
	return procGUI.slaves_en[(TINTING-1) / 8] & 1 << (TINTING-1) % 8;
}

unsigned char isColorTintingModule(unsigned char color_id)
{
	//  Tintintg module Color_ID >= 8 AND <= 31
	if ((color_id >= 8) && (color_id <= 31))
		return TRUE;
	else
		return FALSE;	
}

unsigned char isColorReadyTintingModule(unsigned char color_id)
{	
	//  Tintintg module Color_ID >= 8 AND <= 31
	if ((color_id >= 8) && (color_id <= 31) && isTintingReady() && isColorantActEnabled(color_id))
		return TRUE;
	else
		return FALSE;
}

unsigned char isColorSupllyEndTintingModule(void)
{
	if (!isTintingSupplyEnd() )
		// NOT again terminated
		return FALSE;
	else
		// Terminated
		return TRUE;	
}


void setTintingActMessage(unsigned char packet_type)
/**/
/*==========================================================================*/
/**
**   @brief Set the type of serial message to be send to uC Syringe
**
**   @param packet_type type of packet
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  TintingAct.typeMessage = packet_type;
}

void resetStandbyProcessesTinting(void)
{
	tinting_ricirc_active = OFF;
	stirring_counter_tinting = 0;
	stirring_act_fsm_tinting = PROC_IDLE;
}

void resetStandbyProcessesSingle(unsigned char id)
/**/
/*==========================================================================*/
/**
**   @brief  Reset recirc and stirring FSMs for all used colors
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    recirc_act_fsm[id] = PROC_IDLE;
    recirc_counter[id] = 0;

    stirring_act_fsm[id] = PROC_IDLE;
    stirring_counter[id] = 0;
  
	if (isColorTintingModule(id) ) {
		tinting_ricirc_active = OFF;
//		cleaning_act_fsm[id] = PROC_IDLE;
//		cleaning_counter[id] = 0;
		stirring_counter_tinting = 0;
		stirring_act_fsm_tinting = PROC_IDLE;
	}	
}

void NEW_Calculates_Tinting_Colorants_Order(void)
/**/
/*==========================================================================*/
/**
**   @brief  Set Colorant Orders to minimize Erogation sequence
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    signed long dist, minimum_dist, passi_tavola;
	unsigned char current_circ, num_bases, i_circuit, num_circ, new_circuit;
	short i,j,k;
	short RetArray[N_SLAVES_COLOR_ACT];		
	signed long Positions[16];

	for (i = 0; i < 16; i++)    
        Positions[i] = Circuit_step_tmp[i]/(unsigned long)CORRECTION_TABLE_STEP_RES;
    
	for (i = 0; i < N_SLAVES_COLOR_ACT + 1; i++)
		RetArray[i] = 0xFF;		

    passi_tavola = TintingAct.Steps_Revolution / (unsigned long)CORRECTION_TABLE_STEP_RES;
	// Se Nessun circuito è ingaggiato la formula è ordinata in ordine di circuito crescente e NON facciamo nulla	
	// Se la Formula NON contiene COLORANTI ma solo BASI NON facciamo nulla		
	if (TintingAct.Circuit_Engaged != 0) {
		// Trovo il circuito della Formula piu vicino a quello ingaggiato 'TintingAct.Circuit_Engaged'
		minimum_dist = passi_tavola;
		RetArray[0] = 0xFF;
		num_bases = 0;
		for (i = 0;i < procGUI.dispensation_colors_number; i++) {
			if (procGUI.dispensation_colors_id[i] <= B8_BASE_IDX) {
				num_bases++;
				continue;
			}	
			i_circuit = procGUI.dispensation_colors_id[i] - C1_COLOR_IDX;
			if (Positions[i_circuit] >= Positions[TintingAct.Circuit_Engaged-1]) {
				if ((signed long)((signed long)Positions[i_circuit] - (signed long)Positions[TintingAct.Circuit_Engaged-1]) <= (signed long)(passi_tavola / 2))
					dist = (signed long)((signed long)Positions[i_circuit] - (signed long)Positions[TintingAct.Circuit_Engaged-1]);
				else
					dist = (signed long)((signed long)passi_tavola - (signed long)Positions[i_circuit] + (signed long)Positions[TintingAct.Circuit_Engaged-1]);
			}
			else {
				if ((signed long)((signed long)Positions[TintingAct.Circuit_Engaged-1] - (signed long)Positions[i_circuit]) <= (signed long)(passi_tavola / 2))
					dist = (signed long)((signed long)Positions[TintingAct.Circuit_Engaged-1] - (signed long)Positions[i_circuit]);
				else
					dist = (signed long)((signed long)passi_tavola - (signed long)Positions[TintingAct.Circuit_Engaged-1] + (signed long)Positions[i_circuit]);
			}
			if (dist < minimum_dist) {
				minimum_dist = dist;
				RetArray[0] = i_circuit;
			}
		}	
		current_circ = RetArray[0];
		num_circ = 1; 		
		for (k = 0; k < procGUI.dispensation_colors_number - num_bases - 1; ++ k) {
			minimum_dist = passi_tavola;	
			for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
				if (procGUI.dispensation_colors_id[i] <= B8_BASE_IDX)
					continue;
					
				i_circuit = procGUI.dispensation_colors_id[i] - C1_COLOR_IDX;
				new_circuit = TRUE;
				for (j = 0; j < num_circ; ++ j) {	
					if (i_circuit == RetArray[j])
						new_circuit = FALSE;
				}
				// Il circuito non appartiene a quelli già classificati
				if (new_circuit == TRUE) {	
					// Calcolo la distanza tra il circuito 'i_circuit' e quello attualmente ingaggiato 'RetArray[num_circ-1]'
					if (Positions[i_circuit] >= Positions[RetArray[num_circ-1]]) {
						if ((signed long)((signed long)Positions[i_circuit] - (signed long)Positions[RetArray[num_circ-1]]) <= (signed long)(passi_tavola / 2))
							dist = (signed long)((signed long)Positions[i_circuit] - (signed long)Positions[RetArray[num_circ-1]]);
						else
							dist = (signed long)((signed long)passi_tavola - (signed long)Positions[i_circuit] + (signed long)Positions[RetArray[num_circ-1]]);
					}
					else {
						if ((signed long)((signed long)Positions[RetArray[num_circ-1]] - (signed long)Positions[i_circuit]) <= (signed long)(passi_tavola / 2))
							dist = (signed long)((signed long)Positions[RetArray[num_circ-1]] - (signed long)Positions[i_circuit]);
						else
							dist = (signed long)((signed long)passi_tavola - (signed long)Positions[RetArray[num_circ-1]] + (signed long)Positions[i_circuit]);
					}
					if (dist < minimum_dist) {
						minimum_dist = dist;
						RetArray[num_circ] = i_circuit;
					}
				}
			}
			num_circ++;	
		}
		j = 0;
		for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
			if (procGUI.dispensation_colors_id[i] <= B8_BASE_IDX)
				continue;
			
			procGUI.dispensation_colors_id[i] = RetArray[j] + C1_COLOR_IDX;
			j++;			
		}
	}
}

void NEW_Calculates_Cleaning_Tinting_Colorants_Order(void)
/**/
/*==========================================================================*/
/**
**   @brief  Set Colorant Orders to minimize Erogation sequence
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    
}

void Cleaning_Manager(void)
/*
*//*=====================================================================*//**
**      @brief tinting colorants cleaning manager
**
**      @param 
**
**      @retval void
**					 
**
*//*=====================================================================*//**
*/
{
    unsigned char i;
//    static unsigned char cleaning_status = CLEAN_INIT_ST;
#ifndef CLEANING_AFTER_DISPENSING    
    static long count_brush_pause;

    switch(cleaning_status) {
        case CLEAN_INIT_ST:
cleaning_status = CLEAN_DISABLE_ST;	
/*
TintingAct.Cleaning_pause = 1;     
TintingClean.Cleaning_duration = 5;
            Clean_Activation = OFF;
            count_brush_pause = 0;
            if (TintingClean.Cleaning_duration > 0) {
                // Maschera abilitazione Pulizia Coloranti Tavola
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                    if (TintingAct.Cleaning_Col_Mask[i] == TRUE) {
                        StopTimer(T_WAIT_BRUSH_PAUSE);			
                        StartTimer(T_WAIT_BRUSH_PAUSE);							
                        cleaning_status = CLEAN_PAUSE_ST;
                        break;
                    }  
                }
            }
            else 
                cleaning_status = CLEAN_DISABLE_ST;
                
            if (cleaning_status == CLEAN_INIT_ST)
                cleaning_status = CLEAN_DISABLE_ST; 
*/
        break;

        case CLEAN_DISABLE_ST:
            if (StopCleaningManage == TRUE) {
                StopCleaningManage = FALSE;
                cleaning_status = CLEAN_INIT_ST;
            }                                                        
        break;
        
        case CLEAN_PAUSE_ST:
            if (StatusTimer(T_WAIT_BRUSH_PAUSE) == T_ELAPSED) {    
				count_brush_pause++;
				StopTimer(T_WAIT_BRUSH_PAUSE);			
				if (count_brush_pause >= TintingAct.Cleaning_pause) {
					count_brush_pause = 0;
                    Clean_Activation = ON;
                    cleaning_status = CLEAN_START_ST;                
                }
                else                   
                    StartTimer(T_WAIT_BRUSH_PAUSE);							                    
            }
            if (StopCleaningManage == TRUE) {
                count_brush_pause = 0;
                StopCleaningManage = FALSE;
				StopTimer(T_WAIT_BRUSH_PAUSE);			
                StartTimer(T_WAIT_BRUSH_PAUSE);							                
            }                                        
        break;

        case CLEAN_START_ST:
            if (isAllCircuitsHome() ) {
                cleaning_status = CLEAN_ACTIVE_ST;
                TintingPuliziaTavola();
            }
            if (StopCleaningManage == TRUE) {
                StopCleaningManage = FALSE;
                cleaning_status = CLEAN_PAUSE_ST;
            }                                                    
        break;

        case CLEAN_ACTIVE_ST:
            if (Clean_Activation == OFF) {
                StartTimer(T_WAIT_BRUSH_PAUSE);							                                    
                cleaning_status = CLEAN_PAUSE_ST;
            }
            if (StopCleaningManage == TRUE) {
                StopCleaningManage = FALSE;
                Clean_Activation = ON;                
//                cleaning_status = CLEAN_START_ST;                                
                cleaning_status = CLEAN_PAUSE_ST;                
            }                            
        break;
        
        case CLEAN_STOP_ST:
        break;
                
        default:
        break;    
    }
#else
    switch(cleaning_status) {
        case CLEAN_INIT_ST:
            Clean_Activation = OFF;
            if (TintingClean.Cleaning_duration > 0) {
                StopTimer(T_WAIT_BRUSH_PAUSE);			
                cleaning_status = CLEAN_PAUSE_ST;
            }
            else 
                cleaning_status = CLEAN_DISABLE_ST;
        break;

        case CLEAN_DISABLE_ST:
            if (StopCleaningManage == TRUE) {
                StopCleaningManage = FALSE;
                cleaning_status = CLEAN_INIT_ST;
            }                                                        
        break;
        
        case CLEAN_PAUSE_ST:
            if (Enable_Cleaning == TRUE) {
                Enable_Cleaning = FALSE;
                for (i=0 ; i< MAX_COLORANT_NUM; i++) {
                    if (TintingAct.Cleaning_Col_Mask[i] != 0) {                                
//                        StartTimer(T_WAIT_BRUSH_PAUSE);
						Clean_Activation = ON;
						cleaning_status = CLEAN_START_ST;                                        
                        break;                        
                    }    
                }                            
            }    
/*            
            if (StatusTimer(T_WAIT_BRUSH_PAUSE) == T_ELAPSED) {    
                StopTimer(T_WAIT_BRUSH_PAUSE);			
                Clean_Activation = ON;
                cleaning_status = CLEAN_START_ST;                
            }            
*/
            if (StopCleaningManage == TRUE) {
                StopCleaningManage = FALSE;
				StopTimer(T_WAIT_BRUSH_PAUSE);			
            }                                        
        break;

        case CLEAN_START_ST:
            if (isAllCircuitsSupplyHome() ) {
                cleaning_status = CLEAN_ACTIVE_ST;
                TintingPuliziaTavola();
            }
            if (StopCleaningManage == TRUE) {
                StopCleaningManage = FALSE;
                cleaning_status = CLEAN_PAUSE_ST;
            }                                                    
        break;

        case CLEAN_ACTIVE_ST:
            if (Clean_Activation == OFF)
                cleaning_status = CLEAN_PAUSE_ST;
            if (StopCleaningManage == TRUE) {
                StopCleaningManage = FALSE;
                Clean_Activation = ON;                
                cleaning_status = CLEAN_PAUSE_ST;                
            }                            
        break;
        
        case CLEAN_STOP_ST:
        break;
                
        default:
        break;    
    }

#endif    
}
/*
*//*=====================================================================*//**
**      @brief jump_to_boot
**
**      @param 
**
**      @retval void
**					 
**
*//*=====================================================================*//**
*/
void jump_to_boot()
{
#ifndef WATCH_DOG_DISABLE
	/* kicking the dog ;-) */
	ClrWdt();
	DISABLE_WDT();
#endif
	/* jump to boot code, won't return */
	__asm__ volatile ("goto "  __BOOT_GOTO_ADDR);
}
