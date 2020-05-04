/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Processes management
 * Created on 13 marzo 2020, 17.59
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

/*
*//*=====================================================================*//**
**      @brief Initialization status
**
**      @param void
**
** 
**      @retval void
**
*//*=====================================================================*//**
*/
void initStatusManager(void)
{
	Status.level = TINTING_INIT_ST; 
    Status.errorCode = 0;
    // Motors configuration
    // MIXER Motor    
    Enable_Driver(MOTOR_MIXER);
    ConfigStepper(MOTOR_MIXER, RESOLUTION_MIXER, RAMP_PHASE_CURRENT_MIXER, PHASE_CURRENT_MIXER, HOLDING_CURRENT_MIXER_MOVING, ACC_RATE_MIXER, DEC_RATE_MIXER, ALARMS_MIXER);
    SetStepperHomePosition(MOTOR_MIXER);
    Status_Board_Mixer.word = GetStatus(MOTOR_MIXER);
    // DOOR Motor    
    Enable_Driver(MOTOR_DOOR);
    ConfigStepper(MOTOR_DOOR, RESOLUTION_DOOR, RAMP_PHASE_CURRENT_DOOR, PHASE_CURRENT_DOOR, HOLDING_CURRENT_DOOR_MOVING, ACC_RATE_DOOR, DEC_RATE_DOOR, ALARMS_DOOR);
    SetStepperHomePosition(MOTOR_DOOR);   
    Status_Board_Door.word = GetStatus(MOTOR_DOOR);
    // AUTOCAP Motor    
    Enable_Driver(MOTOR_AUTOCAP);
    ConfigStepper(MOTOR_AUTOCAP, RESOLUTION_AUTOCAP, RAMP_PHASE_CURRENT_AUTOCAP, PHASE_CURRENT_AUTOCAP, HOLDING_CURRENT_AUTOCAP_MOVING, ACC_RATE_AUTOCAP, DEC_RATE_AUTOCAP, ALARMS_AUTOCAP);
    SetStepperHomePosition(MOTOR_AUTOCAP);   
    Status_Board_Autocap.word = GetStatus(MOTOR_AUTOCAP);
    
    eeprom_retries = 0;
    TintingAct.Jar_presence = FALSE; 
    Autocap_Enabled = TRUE;
    DRV8842_RESET();   
    StopTimer(T_RESET);
}

/*
*//*=====================================================================*//**
**      @brief LED Management
**
**      @param void
**
** 
**      @retval void
**
*//*=====================================================================*//**
*/
void visualIndicator()
{
    static unsigned char bit;
    static unsigned char limit;
    static unsigned char counter;

    if (StatusTimer(T_HEARTBEAT) == T_ELAPSED) {
        if (LIGHT_OFF == indicator) {
            LED_ON_OFF = OFF;;
            counter = limit = 0;
        }
        else if (LIGHT_STEADY == indicator) {
            LED_ON_OFF = ON;
            counter = limit = 0;
        }
        else if (LIGHT_PULSE_SLOW == indicator) {
            limit = 10;
        }      
        else if (LIGHT_PULSE_FAST == indicator) {
            limit = 3;
        }
        if ( limit && ++ counter >= limit ) {
            counter = 0;
            bit = ! bit ;
            LED_ON_OFF = bit;
        }
        StartTimer(T_HEARTBEAT);
    } // HEARTBEAT 
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
    switch (Status.level)
	{        
        case TINTING_INIT_ST:   
            indicator = LIGHT_OFF;            
            Status.level++;
        break;

        case TINTING_READY_ST:
            indicator = LIGHT_STEADY;                    
            if ( (TintingAct.Jar_presence == TRUE) && (TintingAct.Jar_photocell == LIGHT) ) {
                TintingAct.Jar_presence = FALSE;
                // Closing Door
                Status.level = TINTING_MIXER_SEARCH_HOMING_ST;
            }        
            
            // 'POS_HOMING' Mixer and Door command Received
            if (isColorCmdHome() && (TintingAct.Homing_type == 0) ) 
                Status.level = TINTING_MIXER_SEARCH_HOMING_ST;
            // 'POS_HOMING' Autocap command Received
            else if (isColorCmdHome() && (TintingAct.Homing_type > 0) && (TintingAct.Autocap_Enable == TRUE) ) {
                Autocap.step = AUTOCAP_READY_ST;
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_SEARCH_HOMING_ST;   
            }    
            // 'SETUP_PARAMETRI_UMIDIFICATORE' 
            else if (TintingAct.typeMessage == SETUP_PARAMETRI_UMIDIFICATORE)
                Status.level = TINTING_WAIT_PARAMETERS_ST;  
            // 'SETUP_PARAMETRI_MIXER' 
            else if (TintingAct.typeMessage == SETUP_PARAMETRI_MIXER)
                Status.level = TINTING_WAIT_MIXER_PARAMETERS_ST;
            else if (TintingAct.typeMessage == IMPOSTA_USCITE_MIXER) 
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;          
            else if (TintingAct.typeMessage == TEST_FUNZIONAMENTO_MIXER) {
                Status.level = TINTING_PAR_RX;                
                NextStatus.level = TINTING_MIXER_TEST_ST;
            }
            else if (TintingAct.typeMessage == MISCELAZIONE_PRODOTTO) {              
                Status.level = TINTING_SUPPLY_RUN_ST;
            }
            else if (TintingAct.typeMessage == USCITA_BARATTOLO) {              
                Status.level = TINTING_JAR_MOTOR_RUN_ST;
            }
            else if (TintingAct.typeMessage == IMPOSTA_CORRENTE_MANTENIMENTO_MOTORE_MIXER)  {   
                Status.level = TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_RUN_ST;                
            } 
            // AUTOCAP ---------------------------------------------------------
            else if (isColorCmdPacking() && ((Autocap.step == AUTOCAP_READY_ST) || (Autocap.step == AUTOCAP_CLOSE_ST) || (Autocap.step == AUTOCAP_OPEN_ST) || 
                                             (Autocap.step == AUTOCAP_EXTEND_ST)) && (TintingAct.Autocap_Enable == TRUE) ) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_PACKING_ST;                                
            }           
            else if (isColorCmdOpen() && (Autocap.step == AUTOCAP_CLOSE_ST) && (TintingAct.Autocap_Enable == TRUE) ) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_OPEN_RUN_ST;                                                
            }         
            // -----------------------------------------------------------------            
            else if (Humidifier.level == HUMIDIFIER_RH_ERROR) {
                Status.level = TINTING_RH_ERROR_ST;
            }                                                    
            else if (Humidifier.level == HUMIDIFIER_TEMPERATURE_ERROR) {
                Status.level = TINTING_TEMPERATURE_ERROR_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_NEBULIZER_OVERCURRENT_THERMAL_ERROR) {
                Status.level = TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_NEBULIZER_OPEN_LOAD_ERROR) {
                Status.level = TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_RELE_OVERCURRENT_THERMAL_ERROR) {
                Status.level = TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_RELE_OPEN_LOAD_ERROR) {
                Status.level = TINTING_RELE_OPEN_LOAD_ERROR_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_AIR_PUMP_OVERCURRENT_THERMAL_ERROR) {
                Status.level = TINTING_AIR_PUMP_OVERCURRENT_THERMAL_ERROR_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_AIR_PUMP_OPEN_LOAD_ERROR) {
                Status.level = TINTING_AIR_PUMP_OPEN_LOAD_ERROR_ST;
            }            
        break;
// HOMING -----------------------------------------------------------------------------      
        case TINTING_MIXER_SEARCH_HOMING_ST:
            if (Mixer.level == MIXER_END)
                Status.level = TINTING_HOMING_ST;
            else if (Mixer.level == MIXER_ERROR)
                Status.level = Mixer.errorCode;                
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);
                Status.level = TINTING_MIXER_RESET_ERROR_ST;
            }
            break;  
            
        case TINTING_AUTOCAP_SEARCH_HOMING_ST:
            if (Autocap.step == AUTOCAP_CLOSE_ST) {
                Autocap_Enabled = FALSE;
                Status.level = TINTING_AUTOCAP_CLOSE_ST;
            }    
            else if (Autocap.step == AUTOCAP_ERROR_ST) {
                Autocap_Enabled = FALSE;
                Status.level = Autocap.errorCode;
            }    
            break;
            
        case TINTING_HOMING_ST:
            if (TintingAct.typeMessage == CONTROLLO_PRESENZA) {
                if (isColorCmdIntr())
                    Status.level = TINTING_INIT_ST;
                else if (isColorCmdStop())
                    Status.level = TINTING_STOP_ST;
            }
            if (TintingAct.typeMessage == SETUP_PARAMETRI_UMIDIFICATORE)
                Status.level = TINTING_WAIT_PARAMETERS_ST;  
            else if (TintingAct.typeMessage == SETUP_PARAMETRI_MIXER)
                Status.level = TINTING_WAIT_MIXER_PARAMETERS_ST;
            else if (TintingAct.typeMessage == IMPOSTA_USCITE_MIXER) 
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;          
            else if (TintingAct.typeMessage == TEST_FUNZIONAMENTO_MIXER) {
                Status.level = TINTING_PAR_RX;                
                NextStatus.level = TINTING_MIXER_TEST_ST;
            }
            else if (TintingAct.typeMessage == MISCELAZIONE_PRODOTTO) {              
                Status.level = TINTING_SUPPLY_RUN_ST;
            }    
            else if (TintingAct.typeMessage == USCITA_BARATTOLO) {              
                Status.level = TINTING_JAR_MOTOR_RUN_ST;
            }
            else if (TintingAct.typeMessage == IMPOSTA_CORRENTE_MANTENIMENTO_MOTORE_MIXER)  {   
                Status.level = TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_RUN_ST;                
            }                        
        break;

// AUTOCAP ---------------------------------------------------------------------                                
        // CmdPacking
        case TINTING_AUTOCAP_PACKING_ST:
            if ( (Autocap.step == AUTOCAP_PACKED_ST) || (Autocap.step == AUTOCAP_PACKED_CLOSED_ST) ) { 
                Autocap_Enabled = FALSE;
                Status.level = TINTING_AUTOCAP_PACKING_END_ST;
            }    
            else if (Autocap.step == AUTOCAP_ERROR_ST) { 
                Autocap_Enabled = FALSE;
                Status.level = Autocap.errorCode; 
            }    
        break;           

        case TINTING_AUTOCAP_PACKING_END_ST:
            if (isColorCmdStop() && (Autocap.step == AUTOCAP_PACKED_ST) ) {            
                Autocap_Enabled = TRUE;
                Autocap.step = AUTOCAP_READY_ST;
                Status.level = TINTING_READY_ST;            
            }
            else if (isColorCmdStop() && (Autocap.step == AUTOCAP_PACKED_CLOSED_ST) ) {            
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_CLOSE_RUN_ST;            
            }                    
            break;
        
        // CmdOpen
        case TINTING_AUTOCAP_OPEN_RUN_ST:
            if ( (Autocap.step == AUTOCAP_OPEN_ST)  ) { 
                Autocap_Enabled = FALSE;
                Status.level = TINTING_AUTOCAP_OPEN_ST;
            } 
            else if (Autocap.step == AUTOCAP_ERROR_ST) { 
                Autocap_Enabled = FALSE;
                Status.level = Autocap.errorCode; 
            }                
        break;
        
        case TINTING_AUTOCAP_OPEN_ST:
            if (isColorCmdIntr()) {        
                Status.level = TINTING_READY_ST;            
            }
           
            if (isColorCmdClose() ) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_CLOSE_RUN_ST;                                                
            }
            else if (isColorCmdExtend() ) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_EXTEND_RUN_ST; 
            }                
            else if (isColorCmdHome() && (TintingAct.Homing_type > 0) ) {
                Autocap.step = AUTOCAP_READY_ST;
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_SEARCH_HOMING_ST;   
            }                                
        break;
        
        // CmdClose
        case TINTING_AUTOCAP_CLOSE_RUN_ST:
            if ( (Autocap.step == AUTOCAP_CLOSE_ST)  ) { 
                Autocap_Enabled = FALSE;
                Status.level = TINTING_AUTOCAP_CLOSE_ST;
            } 
            else if (Autocap.step == AUTOCAP_ERROR_ST) { 
                Autocap_Enabled = FALSE;
                Status.level = Autocap.errorCode; 
            }                
        break;    

        case TINTING_AUTOCAP_CLOSE_ST:           
            if (isColorCmdIntr()) {           
                Status.level = TINTING_READY_ST;
            }
            
            if (isColorCmdPacking()) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_PACKING_ST;                                
            }            
            else if (isColorCmdOpen() ) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_OPEN_RUN_ST;                                                
            }
            else if (isColorCmdHome() && (TintingAct.Homing_type > 0) ) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_SEARCH_HOMING_ST;   
            }                
        break;    
        
        // CmdExtend
        case TINTING_AUTOCAP_EXTEND_RUN_ST:
            if ( (Autocap.step == AUTOCAP_EXTEND_ST)  ) { 
                Autocap_Enabled = FALSE;
                Status.level = TINTING_AUTOCAP_EXTEND_ST;
            } 
            else if (Autocap.step == AUTOCAP_ERROR_ST) { 
                Autocap_Enabled = FALSE;
                Status.level = Autocap.errorCode; 
            }                            
        break;  

        case TINTING_AUTOCAP_EXTEND_ST:
            if (isColorCmdIntr()) {           
                Autocap.step = AUTOCAP_READY_ST;
                Status.level = TINTING_READY_ST;
            }
            else if (isColorCmdRetract()) {
                Autocap_Enabled = TRUE;
                Status.level = TINTING_AUTOCAP_RETRACT_ST;                                                
            }  
        break;    
            
        // CmdRetract
        case TINTING_AUTOCAP_RETRACT_ST:
            if ( (Autocap.step == AUTOCAP_OPEN_ST)  ) { 
                Autocap_Enabled = FALSE;
                Status.level = TINTING_AUTOCAP_OPEN_ST;
            } 
            else if (Autocap.step == AUTOCAP_ERROR_ST) { 
                Autocap_Enabled = FALSE;
                Status.level = Autocap.errorCode; 
            }                            
        break;                  
// STOP ------------------------------------------------------------------------                        
        case TINTING_STOP_ST:
            HardHiZ_Stepper(MOTOR_MIXER);
            HardHiZ_Stepper(MOTOR_DOOR);   
            HardHiZ_Stepper(MOTOR_AUTOCAP);
            StopHumidifier();            
            StopTimer(T_RESET);
            if (isColorCmdIntr()) {
                Status.level  = TINTING_INIT_ST;
//                Mixer.level   = MIXER_IDLE;
            }    
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
        // Mixer
    	case TINTING_WAIT_MIXER_PARAMETERS_ST:
            if (Mixer.level == MIXER_PAR_RX)  {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_READY_ST;                
            }
            else if (Mixer.level == MIXER_PAR_ERROR)
                Status.level = TINTING_BAD_PAR_MIXER_ERROR_ST;                    
        break;
        // Setup Output 
        case TINTING_WAIT_SETUP_OUTPUT_ST:
            if (Humidifier.level == HUMIDIFIER_PAR_RX) {
                Status.level = TINTING_PAR_RX;
                NextStatus.level = TINTING_SETUP_OUTPUT_ST;
            }
            else if (Humidifier.level == HUMIDIFIER_PAR_ERROR)
                Status.level = TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST;                                
        break;
        case TINTING_SETUP_OUTPUT_ST:
            if ( (Humidifier.level == HUMIDIFIER_START) || (Humidifier.level == HUMIDIFIER_RUNNING) )
                Status.level = TINTING_READY_ST;
            else if (TintingAct.typeMessage == IMPOSTA_USCITE_MIXER)
                Status.level = TINTING_WAIT_SETUP_OUTPUT_ST;
        break;                
// PARAMETERS CORRECTY RECEIVED ------------------------------------------------
        case TINTING_PAR_RX:
			if (isColorCmdStop())
                Status.level = NextStatus.level;
            // 'SETUP_PARAMETRI_UMIDIFICATORE' o 'SETUP_PARAMETRI_MIXER'
            else if (isColorCmdIntr() )
                Status.level = TINTING_READY_ST;
// MIXER TEST ------------------------------------------------------------------
        case TINTING_MIXER_TEST_ST:
            if (Mixer.level == MIXER_END)
                Status.level = TINTING_READY_ST;
            else if (Mixer.level == MIXER_ERROR)
                Status.level = Mixer.errorCode;                                                
        break;    
// MIXER RUN  ------------------------------------------------------------------        
        case TINTING_SUPPLY_RUN_ST:
            if (Mixer.level == MIXER_END)
                Status.level = TINTING_SUPPLY_END_ST;
            else if (Mixer.level == MIXER_ERROR)
                Status.level = Mixer.errorCode;                        
        break;           
        case TINTING_SUPPLY_END_ST:
			if (isColorCmdStop())  {
                Status.level = TINTING_READY_ST; 
            } 
        break;
// JAR MOTOR RUN  --------------------------------------------------------------        
        case TINTING_JAR_MOTOR_RUN_ST:
            if (Mixer.level == MIXER_END)
                Status.level = TINTING_JAR_MOTOR_END_ST;
            else if (Mixer.level == MIXER_ERROR)
                Status.level = Mixer.errorCode;                        
        break;           
        case TINTING_JAR_MOTOR_END_ST:
			if (isColorCmdStop())  {
                Status.level = TINTING_READY_ST; 
            } 
        break;
// JAR MOTOR RUN  --------------------------------------------------------------        
        case TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_RUN_ST:
            if (Mixer.level == MIXER_END)
                Status.level = TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_END_ST;
            else if (Mixer.level == MIXER_ERROR)
                Status.level = Mixer.errorCode;                        
        break;           
        case TINTING_SET_HIGH_CURRENT_MIXER_MOTOR_END_ST:
			if (isColorCmdStop())  {
                Status.level = TINTING_READY_ST; 
            } 
        break;        
//  JUMP TO BOOT ---------------------------------------------------------------       
        case TINTING_JUMP_TO_BOOT:
            StopStepper(MOTOR_MIXER);
            StopStepper(MOTOR_DOOR);   
            StopStepper(MOTOR_AUTOCAP); 
            StopHumidifier();
            // Se è arrivato il comando di "JUMP_TO_BOOT", ed è terminata la risposta al comando viene effettuato il salto al Boot
 //           if (Start_Jump_Boot == 1)
 //               jump_to_boot();
        break;
// -----------------------------------------------------------------------------      
        case TINTING_TIMEOUT_ERROR_ST: 
        case TINTING_MIXER_JAR_PHOTO_READ_LIGHT_ERROR_ST:
        case TINTING_MIXER_JAR_PHOTO_READ_DARK_ERROR_ST:
        case TINTING_MIXER_PHOTO_READ_LIGHT_ERROR_ST:
        case TINTING_MIXER_PHOTO_READ_DARK_ERROR_ST:
        case TINTING_MIXER_RESET_ERROR_ST:
        case TINTING_MIXER_TEST_ERROR_ST:    
        case TINTING_MIXER_SOFTWARE_ERROR_ST:
        case TINTING_BAD_PAR_HUMIDIFIER_ERROR_ST:
        case TINTING_BAD_PAR_MIXER_ERROR_ST:
        case TINTING_BAD_PERIPHERAL_PARAM_ERROR_ST: 
        case TINTING_MIXER_HOMING_ERROR_ST:
        case TINTING_MIXER_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
        case TINTING_MIXER_MOTOR_UNDER_VOLTAGE_ERROR_ST:
        case TINTING_MIXER_MOTOR_OVERCURRENT_ERROR_ST:
        case TINTING_MIXER_MOTOR_BLOCKED_ERROR_ST:
        case TINTING_DOOR_MOTOR_BLOCKED_ERROR_ST:
        case TINTING_DOOR_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
        case TINTING_DOOR_MOTOR_UNDER_VOLTAGE_ERROR_ST:
        case TINTING_DOOR_MOTOR_OVERCURRENT_ERROR_ST: 
        case TINTING_MIXER_DOOR_MICROSWITCH_OPEN_ERROR_ST:
        case TINTING_MIXER_DOOR_MICROSWITCH_CLOSE_ERROR_ST:
        case TINTING_MIXER_DOOR_OPEN_PHOTO_READ_LIGHT_ERROR_ST:
        case TINTING_MIXER_DOOR_OPEN_PHOTO_READ_DARK_ERROR_ST: 
        case TINTING_TIMEOUT_JAR_PRESENCE_ERROR_ST: 
        case TINTING_MIXER_MOTOR_HIGH_CURRENT_NOT_SETTED_ST:
        case TINTING_AUTOCAP_HOMING_ERROR_ST: 
        case TINTING_AUTOCAP_MOTOR_OVERCURRENT_ERROR_ST:
        case TINTING_AUTOCAP_MOTOR_UNDER_VOLTAGE_ERROR_ST:
        case TINTING_AUTOCAP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST:
        case TINTING_AUTOCAP_PACKING_ERROR_ST:
        case TINTING_AUTOCAP_SOFTWARE_ERROR_ST:
        case TINTING_AUTOCAP_LIFTER_ERROR_ST:
        case TINTING_AUTOCAP_HOME_POS_ERROR_ST:
        case TINTING_AUTOCAP_DRV_OVER_CURR_TEMP_ERROR_ST:
            HardHiZ_Stepper(MOTOR_MIXER);
            HardHiZ_Stepper(MOTOR_DOOR);   
            HardHiZ_Stepper(MOTOR_AUTOCAP);
            if (isColorCmdIntr() )
                Status.level = TINTING_READY_ST;
        break;

        // Humidifier Errors
        case TINTING_RELE_OPEN_LOAD_ERROR_ST:
        case TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST:         
        case TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST:        
        case TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST:        
        case TINTING_AIR_PUMP_OPEN_LOAD_ERROR_ST:        
        case TINTING_AIR_PUMP_OVERCURRENT_THERMAL_ERROR_ST:        
        case TINTING_RH_ERROR_ST:
        case TINTING_TEMPERATURE_ERROR_ST:     
            if (isColorCmdIntr() )
                Status.level = TINTING_READY_ST;
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
#ifndef WATCH_DOG_DISABLE
	/* kicking the dog ;-) */
	ClrWdt();
	DISABLE_WDT();
#endif
	/* jump to boot code, won't return */
	__asm__ volatile ("goto "  __BOOT_GOTO_ADDR);
}
