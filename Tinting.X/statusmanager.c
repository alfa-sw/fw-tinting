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
    // Motors configuration
    // PUMP Motor
    ConfigStepper(MOTOR_PUMP, RESOLUTION_PUMP, RAMP_PHASE_CURRENT_PUMP, PHASE_CURRENT_PUMP, HOLDING_CURRENT_PUMP, ACC_RATE_PUMP, DEC_RATE_PUMP, ALARMS_PUMP);
    // TABLE Motor
    ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);    
    // VALVE Motor
    ConfigStepper(MOTOR_VALVE, RESOLUTION_VALVE, RAMP_PHASE_CURRENT_VALVE, PHASE_CURRENT_VALVE, HOLDING_CURRENT_VALVE, ACC_RATE_VALVE, DEC_RATE_VALVE, ALARMS_VALVE);    
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
	// 'INTR' Command: from any state but NOT 'TINTING_INIT_ST' -> TINTING_READY_ST 
    if ( (isColorCmdIntr() ) && (Status.level != TINTING_INIT_ST) )
        Status.level = TINTING_READY_ST;
    // 'STOP' Command: from any state but NOT 'TINTING_PAR_RX' -> TINTING_STOP_ST
    else if ( (isColorCmdStop()) && (Status.level != TINTING_PAR_RX) )
        Status.level = TINTING_STOP_ST;
            
    switch (Status.level)
	{
        case TINTING_INIT_ST:
            Status.level++;
        break;

        case TINTING_READY_ST:
            // 'POS_HOMING' command Recived
            if (isColorCmdHome() ) {
                StartTimer(T_RESET);
                Status.level = TINTING_PUMP_SEARCH_HOMING_ST;
            }
            // 'SETUP_PARAMETRI_UMIDIFICATORE' or 'SETUP_PARAMETRI_POMPA' or 'SETUP_PARAMETRI_TAVOLA' command Received
            else if (isColorCmdSetupParam() ) {
                if (TintingAct.typeMessage == SETUP_PARAMETRI_UMIDIFICATORE)
                    Status.level = TINTING_WAIT_PARAMETERS_ST;                
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_POMPA)
                    Status.level = TINTING_WAIT_PUMP_PARAMETERS_ST;
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_TAVOLA)
                    Status.level = TINTING_WAIT_TABLE_PARAMETERS_ST;
            }
        break;
// HOMING ----------------------------------------------------------------------        
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
            Status.level++;            
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
            Status.level++;                        
        break;
        
        case TINTING_TABLE_SEARCH_HOMING_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_TABLE_GO_HOMING_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Table.errorCode;                                            
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_TABLE_RESET_ERROR_ST;
            }                        
        break;
                    
        case TINTING_TABLE_GO_HOMING_ST:
            Status.level++;                                    
        break;
// -----------------------------------------------------------------------------      
        case TINTING_HOMING_ST:
            if (isColorCmdSetupOutput() && 
               (PeripheralAct.Peripheral_Types.OpenValve_BigHole != ON) &&
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole != ON) )     
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;
            else if (isColorCmdSetupOutput() && 
               ( (PeripheralAct.Peripheral_Types.OpenValve_BigHole == ON) ||
               (PeripheralAct.Peripheral_Types.OpenValve_SmallHole == ON) ) )                     
                Status.level = TINTING_WAIT_SETUP_OUTPUT_VALVE_ST;
            else if (isColorCmdRecirc() ) {
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;  
                NextStatus.level = TINTING_STANDBY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }
            else if (isColorCmdSupply() ) {
                TintingAct.Refilling_Angle = 0;
                TintingAct.Direction = 0;  
                NextStatus.level = TINTING_SUPPLY_RUN_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;
            }                
            else if (TintingAct.typeMessage == POSIZIONAMENTO_TAVOLA_ROTANTE) {
                NextStatus.level = TINTING_HOMING_ST;
                Status.level = TINTING_TABLE_POSITIONING_ST;                       
            }
            else if (TintingAct.typeMessage == POSIZIONAMENTO_PASSI_TAVOLA_ROTANTE) {
                NextStatus.level = TINTING_HOMING_ST;
                Status.level = TINTING_TABLE_STEPS_POSITIONING_ST;                       
            }
            else if (TintingAct.typeMessage == AUTOAPPRENDIMENTO_TAVOLA_ROTANTE)
                Status.level = TINTING_TABLE_SELF_RECOGNITION_ST;
            else if (TintingAct.typeMessage == TEST_FUNZIONAMENTO_TAVOLA_ROTANTE)
                Status.level = TINTING_TABLE_TEST_ST;
            else if (TintingAct.typeMessage == ATTIVAZIONE_PULIZIA_TAVOLA_ROTANTE)
                Status.level = TINTING_TABLE_CLEANING_ST; 
        break;
// STOP ------------------------------------------------------------------------                        
        case TINTING_STOP_ST:
            StopStepper(MOTOR_TABLE);
            StopStepper(MOTOR_VALVE);
            StopStepper(MOTOR_PUMP);
            StopHumidifier();            
            StopTimer(T_RESET);
            Status.level = TINTING_READY_ST;
        break;
// SETUP PARAMETRS -------------------------------------------------------------                        
        // Humidifier
        case TINTING_WAIT_PARAMETERS_ST:
            if (Humidifier.level == HUMIDIFIER_PAR_RX)  {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_READY_ST;                
            }    
            else if (Humidifier.level == HUMIDIFIER_PAR_ERROR)
                Status.level = TINTING_BAD_PAR_HUMIDIFIER_ERROR;                    
        break;
        // Pump
    	case TINTING_WAIT_PUMP_PARAMETERS_ST:
            if (Pump.level == PUMP_PAR_RX)  {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_READY_ST;                
            }
            else if (Pump.level == PUMP_PAR_ERROR)
                Status.level = TINTING_BAD_PAR_PUMP_ERROR;                    
        break;
        // Table        
        case TINTING_WAIT_TABLE_PARAMETERS_ST:
            if (Table.level == TABLE_PAR_RX)  {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_READY_ST;                
            }
            else if (Table.level == TABLE_PAR_ERROR)
                Status.level = TINTING_BAD_PAR_TABLE_ERROR;                                
        break;
// SETUP OUTPUT NOT VALVE ------------------------------------------------------                        
        case TINTING_WAIT_SETUP_OUTPUT_ST:
            if (Humidifier.level == HUMIDIFIER_PAR_RX) {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_SETUP_OUTPUT_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_PAR_ERROR)
                Status.level = TINTING_BAD_PERIPHERAL_PARAM_ERROR;                                
        break;

        case TINTING_SETUP_OUTPUT_ST:
            if (Humidifier.level != HUMIDIFIER_SETUP_OUTPUT)
                Status.level = TINTING_HOMING_ST;
            else if (isColorCmdSetupOutput()) {
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;
            }                
        break;        
// SETUP OUTPUT VALVE ----------------------------------------------------------                        
        case TINTING_WAIT_SETUP_OUTPUT_VALVE_ST:
            if (Pump.level == PUMP_PAR_RX) {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_SETUP_OUTPUT_VALVE_ST;
            }
            else if (Pump.level == PUMP_ERROR)
                Status.level = TINTING_BAD_PERIPHERAL_PARAM_ERROR;                                
        break;

        case TINTING_SETUP_OUTPUT_VALVE_ST:
            if (Pump.level != PUMP_VALVE_OPEN_CLOSE)
                Status.level = TINTING_HOMING_ST;               
        break;        
// PARAMETERS CORRECTY RECEIVED ------------------------------------------------
        case TINTING_PAR_RX:
			if (isColorCmdStop())
                Status.level = NextStatus.level;
            // 'SETUP_PARAMETRI_UMIDIFICATORE' or 'SETUP_PARAMETRI_POMPA' or 'SETUP_PARAMETRI_TAVOLA' command Received
            else if (isColorCmdSetupParam() ) {
                if (TintingAct.typeMessage == SETUP_PARAMETRI_UMIDIFICATORE)
                    Status.level = TINTING_WAIT_PARAMETERS_ST;                
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_POMPA)
                    Status.level = TINTING_WAIT_PUMP_PARAMETERS_ST;
                else if (TintingAct.typeMessage == SETUP_PARAMETRI_TAVOLA)                    
                    Status.level = TINTING_WAIT_TABLE_PARAMETERS_ST;
            }            
        break;                        
// RICIRCULATION ---------------------------------------------------------------
        case TINTING_STANDBY_RUN_ST:
            if (Pump.level == PUMP_END)
                Status.level = TINTING_STANDBY_END_ST;
            else if (Pump.level == PUMP_ERROR)
                Status.level = Pump.errorCode;                
        break;

        case TINTING_STANDBY_END_ST:
            Status.level = TINTING_HOMING_ST;
        break;        
// TABLE POSITIONING -----------------------------------------------------------            
        case TINTING_TABLE_POSITIONING_ST:
            if (Table.level == TABLE_END)
                Status.level = NextStatus.level;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                            
        break;
// TABLE STEPS POSITIONING -----------------------------------------------------            
        case TINTING_TABLE_STEPS_POSITIONING_ST:
            if (Table.level == TABLE_END)
                Status.level = NextStatus.level;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                            
        break;        
// TABLE AUTO RECOGNITION ------------------------------------------------------
        case TINTING_TABLE_SELF_RECOGNITION_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_HOMING_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                    
        break;
// TABLE TEST ------------------------------------------------------------------
        case TINTING_TABLE_TEST_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_HOMING_ST;
            else if (Table.level == TABLE_ERROR)
                Status.level = Table.errorCode;                                                
        break;    
// CLEANING --------------------------------------------------------------------
        case TINTING_TABLE_CLEANING_ST:
            if (Table.level == TABLE_END)
                Status.level = TINTING_HOMING_ST;
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
            Status.level = TINTING_HOMING_ST;            
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
            if (Start_Jump_Boot == 1)
                jump_to_boot();
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
        case TINTING_BAD_PAR_HUMIDIFIER_ERROR:
        case TINTING_BAD_PAR_PUMP_ERROR:
        case TINTING_BAD_PAR_TABLE_ERROR:
        case TINTING_BAD_PERIPHERAL_PARAM_ERROR:  
        case TINTING_TABLE_SOFTWARE_ERROR_ST:
        case TINTING_TABLE_MOVE_ERROR_ST:
        case TINTING_PUMP_PHOTO_INGR_READ_LIGHT_ERROR_ST:
        case TINTING_SELF_LEARNING_PROCEDURE_ERROR_ST:            
        case TINTING_TABLE_MISMATCH_POSITION_ERROR_ST:
        case TINTING_VALVE_POS0_READ_LIGHT_ERROR_ST:  
        case TINTING_TABLE_TEST_ERROR_ST:
                    
        break;
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
#ifndef DEBUG_SLAVE
	/* kicking the dog ;-) */
	ClrWdt();
	DISABLE_WDT();
#endif
	/* jump to boot code, won't return */
//	__asm__ volatile ("goto "  __BOOT_GOTO_ADDR);
	__asm__ volatile ("goto "  "0x0400");
}
