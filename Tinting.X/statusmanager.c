/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "statusmanager.h"
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
#include "stepperParameters.h"
#include "stepper.h"
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
    Enable_Driver(MOTOR_PUMP);
    Enable_Driver(MOTOR_TABLE);
    Enable_Driver(MOTOR_VALVE);
    // Motors configuration
    // PUMP Motor    
    ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);
    // TABLE Motor
    ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);    
    // VALVE Motor    
    ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, ACC_RATE_VALVE, DEC_RATE_VALVE, ALARMS_VALVE);    

    eeprom_retries = 0;    
}


/*
*//*=====================================================================*//**
**      @brief Updates general status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void StatusManager(void)
{
    unsigned char i;
    unsigned char currentReg;
    
/* Forse NON serve    
	// 'INTR' Command: from any state but NOT 'TINTING_INIT_ST' -> TINTING_READY_ST 
    if ( (isColorCmdIntr() ) && (Status.level != TINTING_INIT_ST) )
        Status.level = TINTING_READY_ST;
    // 'STOP' Command: from any state but NOT 'TINTING_PAR_RX' -> TINTING_STOP_ST
    else if ( (isColorCmdStop()) && (Status.level != TINTING_PAR_RX) )
        Status.level = TINTING_STOP_ST;
*/          
    switch (Status.level)
	{        
        case TINTING_INIT_ST:            
            Table_circuits_pos = OFF;
            Total_circuit_n = 0;
            EEprom_Crc_Error = 0;
            if (checkEEprom() ) {
                for (i = 0; i < MAX_COLORANT_NUMBER; i++) { 
                    TintingAct.Circuit_step_pos[i] = CircStepPosAct.Circ_Pos[i];
                    if ( (TintingAct.Circuit_step_pos[i] > 0) && (TintingAct.Circuit_step_pos[i] != 0xFFFFFFFF) )  {
                        Total_circuit_n++;
                        Table_circuits_pos = ON;
                    }    
                }
                TintingAct.Steps_Threshold = TintingAct.Circuit_step_pos[1] - TintingAct.Circuit_step_pos[0];                
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

//Status.level++;        
        break;

        case TINTING_READY_ST:
            if ( (PositioningCmd == 1) && (End_Table_Position == 1) ) {
                if (StatusTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL)==T_ELAPSED) {
                    // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
//                    ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                                              
                    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                    cSPIN_RegsStruct1.TVAL_HOLD = currentReg;          
                    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct1.TVAL_HOLD, MOTOR_TABLE);
                    
                    StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                }
                if (PhotocellStatus(TABLE_PHOTOCELL, FILTER) == LIGHT) {
                    // TABLE Motor with the Minimum Retention Torque (= Minimum Holding Current)
//                    ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);                                              
                    currentReg = HOLDING_CURRENT_TABLE * 100 /156;
                    cSPIN_RegsStruct1.TVAL_HOLD = currentReg;          
                    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct1.TVAL_HOLD, MOTOR_TABLE);
                    
                    End_Table_Position = 0;
                    StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);                    
                    Status.level = TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST;
                }
                else if (TintingAct.PanelTable_state == CLOSE) {
                    End_Table_Position = 0;
                    StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);                    
                    Status.level = TINTING_TABLE_GO_REFERENCE_ST;
                }    
            }
/*            
eeprom_write_result = updateEECirStepsPos();
if (eeprom_write_result == EEPROM_WRITE_DONE) {
    for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
      CircStepPosAct.Circ_Pos[i] = 0;
    }    
    updateEEParamCirStepsPosCRC();
    Status.level ++;                
}
else if (eeprom_write_result == EEPROM_WRITE_FAILED) {
    Table.errorCode = TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST;
    Status.level ++; 
}
*/
            // 'POS_HOMING' command Recived
            if (isColorCmdHome() ) { 
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);                
                StartTimer(T_RESET);
                if ( (PhotocellStatus(HOME_PHOTOCELL, FILTER) == LIGHT) || (PhotocellStatus(COUPLING_PHOTOCELL, NO_FILTER) == DARK) ||
                     (PhotocellStatus(VALVE_PHOTOCELL, FILTER) == LIGHT) )                
                    Status.level = TINTING_PUMP_SEARCH_HOMING_ST;
                else
                    Status.level = TINTING_PHOTO_DARK_TABLE_SEARCH_HOMING_ST;
            }
            // 'SETUP_PARAMETRI_UMIDIFICATORE' or 'SETUP_PARAMETRI_POMPA' or 'SETUP_PARAMETRI_TAVOLA' command Received
            else if (isColorCmdSetupParam() ) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);                
                if (TintingAct.typeMessage == SETUP_PARAMETRI_UMIDIFICATORE)
                    Status.level = TINTING_WAIT_PARAMETERS_ST;                
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_POMPA)
                    Status.level = TINTING_WAIT_PUMP_PARAMETERS_ST;
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_TAVOLA)
                    Status.level = TINTING_WAIT_TABLE_PARAMETERS_ST;
            }
            else if (isColorCmdSetupOutput() && 
               (PeripheralAct.Peripheral_Types.OpenValve_BigHole != ON) &&
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole != ON) ) { 
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);                                
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;
            }
            else if (isColorCmdSetupOutput() && 
               ( (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) ||
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) ) ) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);                                
                Status.level = TINTING_WAIT_SETUP_OUTPUT_VALVE_ST;
            }
            else if (isColorCmdRecirc() ) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
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
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_TABLE_STIRRING_ST;
            }
            else if ( (TintingAct.typeMessage == DISPENSAZIONE_COLORE) ||
                      (TintingAct.typeMessage == DISPENSAZIONE_COLORE_CONTINUOUS) ) {              
//            else if (isColorCmdSupply() ) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;  
                NextStatus.level = TINTING_SUPPLY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }                
            else if (TintingAct.typeMessage == POSIZIONAMENTO_TAVOLA_ROTANTE) {
                Status.level = TINTING_PAR_RX;                       
                NextStatus.level = TINTING_TABLE_POSITIONING_ST;
            }
            else if (TintingAct.typeMessage == POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_PAR_RX;                
                NextStatus.level = TINTING_TABLE_STEPS_POSITIONING_ST;
            }
            else if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_TABLE_SELF_RECOGNITION_ST;
            }                
            else if (TintingAct.typeMessage == TEST_FUNZIONAMENTO_TAVOLA_ROTANTE) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_PAR_RX;                
                NextStatus.level = TINTING_TABLE_TEST_ST;
            }                
            else if (TintingAct.typeMessage == ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_PAR_RX;                                
                NextStatus.level = TINTING_TABLE_CLEANING_ST;    
            }                                        
            else if (Humidifier.level == HUMIDIFIER_RH_ERROR) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_RH_ERROR_ST;
            }                                                    
            else if (Humidifier.level == TINTING_TEMPERATURE_ERROR_ST)
                Status.level = TINTING_TEMPERATURE_ERROR_ST;
            else if (Humidifier.level == HUMIDIFIER_NEBULIZER_OVERCURRENT_THERMAL_ERROR)
                Status.level = TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST;
            else if (Humidifier.level == HUMIDIFIER_NEBULIZER_OPEN_LOAD_ERROR)
                Status.level = TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST;
            else if (Humidifier.level == HUMIDIFIER_PUMP_OVERCURRENT_THERMAL_ERROR)
                Status.level = TINTING_PUMP_OVERCURRENT_THERMAL_ERROR_ST;
            else if (Humidifier.level == HUMIDIFIER_PUMP_OPEN_LOAD_ERROR)
                Status.level = TINTING_PUMP_OPEN_LOAD_ERROR_ST;
            else if (Humidifier.level == HUMIDIFIER_RELE_OVERCURRENT_THERMAL_ERROR)
                Status.level = TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST;
            else if (Humidifier.level == HUMIDIFIER_RELE_OPEN_LOAD_ERROR)
                Status.level = TINTING_RELE_OPEN_LOAD_ERROR_ST;                                           
        break;
// HOMING ----------------------------------------------------------------------        
// HOME_PHOTOCELL = LIGHT:
//1. Pump and Valve Homing 
//2. Table Homing
        case TINTING_PUMP_SEARCH_HOMING_ST:
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
            Status.level = TINTING_VALVE_SEARCH_HOMING_ST;            
        break;    

        case TINTING_VALVE_SEARCH_HOMING_ST:
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
            Status.level = TINTING_TABLE_SEARCH_HOMING_ST;                        
        break;
        
        case TINTING_TABLE_SEARCH_HOMING_ST:
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
            Status.level = TINTING_HOMING_ST;                                    
        break;
        
// HOME_PHOTOCELL = DARK:
//1. Table Homing 
//2. Pump and Valve Homing
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
// -----------------------------------------------------------------------------      
        case TINTING_HOMING_ST:
            if (TintingAct.typeMessage == CONTROLLO_PRESENZA) {
                if (isColorCmdIntr())
                    Status.level = TINTING_INIT_ST;
                else if (isColorCmdStop())
                    Status.level = TINTING_STOP_ST;		                
            }    
            else if (isColorCmdSetupOutput() && 
               (PeripheralAct.Peripheral_Types.OpenValve_BigHole != ON) &&
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole != ON) )     
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;
            else if (isColorCmdSetupOutput() && 
               ( (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) ||
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) ) )                     
                Status.level = TINTING_WAIT_SETUP_OUTPUT_VALVE_ST;
//            else if (isColorCmdRecirc() ) {
            else if (TintingAct.typeMessage == RICIRCOLO_COLORE) { 
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;
                // Before to Start Ricirculation it is necessary to do Stirring: 1 full 360° table rotation
                RicirculationCmd = 1;                
                NextStatus.level = TINTING_STANDBY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }
            else if (isColorCmdStirring() ) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_TABLE_STIRRING_ST;
            }            
            else if ( (TintingAct.typeMessage == DISPENSAZIONE_COLORE) ||
                      (TintingAct.typeMessage == DISPENSAZIONE_COLORE_CONTINUOUS) ) {  
//            else if (isColorCmdSupply() ) {
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;  
                NextStatus.level = TINTING_SUPPLY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }                
            else if (TintingAct.typeMessage == POSIZIONAMENTO_TAVOLA_ROTANTE) {
                Status.level = TINTING_PAR_RX;                       
                NextStatus.level = TINTING_TABLE_POSITIONING_ST;
            }
            else if (TintingAct.typeMessage == POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE) {
                Status.level = TINTING_PAR_RX;                       
                NextStatus.level = TINTING_TABLE_STEPS_POSITIONING_ST;
            }
            else if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_TABLE_SELF_RECOGNITION_ST;
            }                
            else if (TintingAct.typeMessage == TEST_FUNZIONAMENTO_TAVOLA_ROTANTE) {
                Status.level = TINTING_PAR_RX;                
                NextStatus.level = TINTING_TABLE_TEST_ST;
            }                            
            else if (TintingAct.typeMessage == ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE) {
                Status.level = TINTING_PAR_RX;                                
                NextStatus.level = TINTING_TABLE_CLEANING_ST;    
            }                                        
        break;
// STOP ------------------------------------------------------------------------                        
        case TINTING_STOP_ST:
            HardHiZ_Stepper(MOTOR_TABLE);
            HardHiZ_Stepper(MOTOR_VALVE);
            HardHiZ_Stepper(MOTOR_PUMP);
            StopHumidifier();            
            StopTimer(T_RESET);
            if (TintingAct.typeMessage == CONTROLLO_PRESENZA) 
                if (isColorCmdIntr())
                    Status.level = TINTING_INIT_ST;                                
        break;
// SETUP PARAMETRS -------------------------------------------------------------                        
        // Humidifier
        case TINTING_WAIT_PARAMETERS_ST:
            if (Humidifier.level == HUMIDIFIER_PAR_RX)  {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_READY_ST;                
            }    
            else if (Humidifier.level == HUMIDIFIER_PAR_ERROR)
                Status.level = TINTING_BAD_PAR_HUMIDIFIER_ERROR_ST;                    
        break;
        // Pump
    	case TINTING_WAIT_PUMP_PARAMETERS_ST:
            if (Pump.level == PUMP_PAR_RX)  {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_READY_ST;                
            }
            else if (Pump.level == PUMP_PAR_ERROR)
                Status.level = TINTING_BAD_PAR_PUMP_ERROR_ST;                    
        break;
        // Table        
        case TINTING_WAIT_TABLE_PARAMETERS_ST:
            if (Table.level == TABLE_PAR_RX)  {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_READY_ST;                
            }
            else if (Table.level == TABLE_PAR_ERROR)
                Status.level = TINTING_BAD_PAR_TABLE_ERROR_ST;                                
        break;
// SETUP OUTPUT NOT VALVE ------------------------------------------------------                        
        case TINTING_WAIT_SETUP_OUTPUT_ST:
            if (Humidifier.level == HUMIDIFIER_PAR_RX) {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_SETUP_OUTPUT_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_PAR_ERROR)
                Status.level = TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST;                                
        break;

        case TINTING_SETUP_OUTPUT_ST:
            if (Humidifier.level == HUMIDIFIER_START)
                Status.level = TINTING_READY_ST;
            else if (isColorCmdSetupOutput())
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;
        break;        
// SETUP OUTPUT VALVE ----------------------------------------------------------                        
        case TINTING_WAIT_SETUP_OUTPUT_VALVE_ST:
            if (Pump.level == PUMP_PAR_RX) {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_SETUP_OUTPUT_VALVE_ST;
            }
            else if (Pump.level == PUMP_ERROR)
                Status.level = TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST;                                
        break;

        case TINTING_SETUP_OUTPUT_VALVE_ST:
            if (Pump.level == PUMP_END) 
                Status.level = TINTING_READY_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;              
/*            
            else if (Pump.level != PUMP_VALVE_OPEN_CLOSE)
                Status.level = TINTING_READY_ST;             
*/ 
        break;        
// PARAMETERS CORRECTY RECEIVED ------------------------------------------------
        case TINTING_PAR_RX:
			if (isColorCmdStop())
                Status.level = NextStatus.level;
            // 'SETUP_PARAMETRI_UMIDIFICATORE' or 'SETUP_PARAMETRI_POMPA' or 'SETUP_PARAMETRI_TAVOLA' command Received
/*            
            else if (isColorCmdSetupParam() ) {
                if (TintingAct.typeMessage == SETUP_PARAMETRI_UMIDIFICATORE)
                    Status.level = TINTING_WAIT_PARAMETERS_ST;                
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_POMPA)
                    Status.level = TINTING_WAIT_PUMP_PARAMETERS_ST;
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_TAVOLA)                    
                    Status.level = TINTING_WAIT_TABLE_PARAMETERS_ST;
            }    
*/        
        break;                        
// RICIRCULATION ---------------------------------------------------------------
        case TINTING_STANDBY_RUN_ST:
//            if (isColorCmdStop())
//                Status.level = TINTING_STANDBY_END_ST;
//            if (Pump.level == PUMP_END)
//            if ( (Pump.level == PUMP_END) && (isColorCmdRecirc()) )
            if (Pump.level == PUMP_END) 
                Status.level = TINTING_STANDBY_END_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;              
        break;

        case TINTING_STANDBY_END_ST:
            if (isColorCmdStop()) {
//                StopStepper(MOTOR_TABLE);
//                StopStepper(MOTOR_VALVE);
//                StopStepper(MOTOR_PUMP);
// Qui forse andrebbe impostato l'Homing della pompa e della valvola
                Status.level = TINTING_READY_ST;

            }     
        break;        
// TABLE POSITIONING -----------------------------------------------------------            
        case TINTING_TABLE_POSITIONING_ST:
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
            if (Table.level == TABLE_END) 
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                            
        break;            
// TABLE STEPS POSITIONING -----------------------------------------------------            
        case TINTING_TABLE_STEPS_POSITIONING_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_READY_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                            
        break;        
// TABLE AUTO RECOGNITION ------------------------------------------------------
        case TINTING_TABLE_SELF_RECOGNITION_ST:
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
            if (Pump.level == PUMP_END)
                Status.level = TINTING_SUPPLY_END_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                        
        break;
                                
        case TINTING_SUPPLY_END_ST:
			if (isColorCmdStop())
                Status.level = TINTING_READY_ST;                        
        break;
// CLEANING --------------------------------------------------------------------                                
        // Implementato quando decideremo di inserire la pulizia
        case TINTING_CLEANING_ST:
            
        break;                        
// JUMP TO BOOT
// ------------------------------------------------------------------------------------------------------------        
        case TINTING_JUMP_TO_BOOT:
            StopStepper(MOTOR_TABLE);
            StopStepper(MOTOR_VALVE);
            StopStepper(MOTOR_PUMP);
            StopHumidifier();
            // Se è arrivato il comando di "JUMP_TO_BOOT", ed è terminata la risposta al comando viene effettuato il salto al Boot
 //           if (Start_Jump_Boot == 1)
 //               jump_to_boot();
        break;
   
        case TINTING_PUMP_RESET_ERROR_ST:
        case TINTING_VALVE_RESET_ERROR_ST: 
        case TINTING_TABLE_RESET_ERROR_ST:
        case TINTING_PUMP_MOTOR_OVERCURRENT_ERROR_ST:
        case TINTING_PUMP_POS0_READ_LIGHT_ERROR_ST:
        case TINTING_PUMP_PHOTO_HOME_READ_DARK_ERROR_ST:
        case TINTING_PUMP_SOFTWARE_ERROR_ST:
        case TINTING_VALVE_MOTOR_OVERCURRENT_ERROR_ST:
        case TINTING_VALVE_PHOTO_READ_DARK_ERROR_ST:
        case TINTING_TABLE_MOTOR_OVERCURRENT_ERROR_ST:
        case TINTING_TABLE_HOMING_ERROR_ST:
        case TINTING_TABLE_SEARCH_POSITION_REFERENCE_ERROR_ST:
        case TINTING_BAD_PAR_HUMIDIFIER_ERROR_ST:
        case TINTING_BAD_PAR_PUMP_ERROR_ST:
        case TINTING_BAD_PAR_TABLE_ERROR_ST:
        case TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST:  
        case TINTING_TABLE_SOFTWARE_ERROR_ST:
        case TINTING_TABLE_MOVE_ERROR_ST:
        case TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST:
        case TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST:
        case TINTING_LACK_CIRCUITS_POSITION_ERROR_ST:
        case TINTING_TABLE_MISMATCH_POSITION_ERROR_ST:
        case TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST:  
        case TINTING_TABLE_TEST_ERROR_ST:
        case TINTING_BRUSH_OPEN_LOAD_ERROR_ST:
        case TINTING_BRUSH_OVERCURRENT_THERMAL_ERROR_ST:
        case TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST:
        case TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST:
        case TINTING_PUMP_OPEN_LOAD_ERROR_ST:
        case TINTING_PUMP_OVERCURRENT_THERMAL_ERROR_ST:
        case TINTING_RELE_OPEN_LOAD_ERROR_ST:
        case TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST: 
        case TINTING_GENERIC24V_OPEN_LOAD_ERROR_ST:
        case TINTING_GENERIC24V_OVERCURRENT_THERMAL_ERROR_ST:                            
        case TINTING_PUMP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
        case TINTING_VALVE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
        case TINTING_TABLE_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
        case TINTING_PUMP_MOTOR_UNDER_VOLTAGE_ERROR_ST:
        case TINTING_VALVE_MOTOR_UNDER_VOLTAGE_ERROR_ST:
        case TINTING_TABLE_MOTOR_UNDER_VOLTAGE_ERROR_ST:
        case TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST:
        case TINTING_TABLE_PHOTO_READ_LIGHT_ERROR_ST:            
            if (isColorCmdIntr() )
//                Status.level = TINTING_INIT_ST;		                
                Status.level = TINTING_READY_ST;
            else if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_TABLE_SELF_RECOGNITION_ST;
            }                
        break;
/*        
        case TINTING_EEPROM_COLORANTS_STEPS_POSITION_CRC_ERROR_ST:
            if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE) {
                StopTimer(T_WAIT_HOLDING_CURRENT_TABLE_FINAL);
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_TABLE_SELF_RECOGNITION_ST;
            }                            
        break;
*/            
    	default:
        break;    
    }    
}

/*
*//*=====================================================================*//**
**      @brief jump_to_boott
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
