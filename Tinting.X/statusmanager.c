
/*******************************************************************************
 **
 **      Filename     : statusManager.c
 **
 **      Description  : Main FSM module
 **
 **      Project      : Alfa Dispensers
 **
 **   ==========================================================================
 **
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
#include "typedef.h"
#include "eepromManager.h"
#include "eeprom.h"
#include "mem.h"
#include "ErrorManager.h"
#include "stepperParameters.h"
#include "stepper.h"
#include "L6482H.h"
#include "serialCom.h"
#include "serialCom_GUI.h"
#include "colorAct.h"
#include "autocapAct.h"
#include "statusManager.h"
#include "pumpManager.h"
#include "tableManager.h"
#include "spi.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <xc.h>

static enum {
  LIGHT_OFF,
  LIGHT_STEADY,
  LIGHT_PULSE_SLOW,
  LIGHT_PULSE_FAST
} indicator;

# define LEAVE_DIAGNOSTIC_COUNT (60 * 30)
static unsigned short diag_idle_counter;

/**
 * Module data
 */
static unsigned char autoRecoveryFromAlarm;
static unsigned char indiceReset;

static unsigned char fasiCancellazione;
static unsigned char statoAutoCap  = AUTOCAP_CLOSED;	
static unsigned char turnToStandBy = 0;
static unsigned char primoReset = 0;
static unsigned char turnToState;

/**
 * Prototypes
 */
static void stopAllActuators(void);
static void diagResetIdleCounter(void);
static void diagEvalIdleCounter(void);

/**
 * Definitions
 */
static void run()
//static void __attribute__ ((optimize("-O1"))) run()
/**/
/*==========================================================================*/
/**
**   @brief Management of state transitions, in compliance with
**          machine state diagram
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char rc;
    unsigned char i, indx;
    static unsigned long Count_Timer_Out_Supply, Timer_Out_Supply_Duration;
    static unsigned long slaves_boot_ver[N_SLAVES-1];
    unsigned char jump_boot;
    static unsigned char bases_open;
    static unsigned short Autotest_indx, Autotest_dosing_amount, Autotest_Color_done, Stop_Autotest;
    unsigned short crc_slave;
    static unsigned char New_Tinting_Cmd;
    rc = 0; // suppress warning
    
    switch(MachineStatus.level) {
    /************************************************************************ */
    /*                              INIT_ST                                   */
    /************************************************************************ */
        case INIT_ST:            
            indicator = LIGHT_OFF;
            for (i = 0; i < (N_SLAVES-1); i++) 
        		attuatoreAttivo[i] = 0;
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    // Disable auto COLD RESET upon ALARMs 
                    autoRecoveryFromAlarm = FALSE;
                    initColorStandbyProcesses();
                    StartTimer(T_POWER_ON_TIMER);
                    // RESET on POWER ON is a cold one 
                    procGUI.reset_mode = 0;
                    resetAlarm();
                    resetManualAlarm();
                break;

                case RUN_PH:
                    nextStatus = IDLE_ST;
                break;

                case EXIT_PH:
                break;    
            }
        break; // INIT_ST 

    /************************************************************************ */
    /*                              IDLE_ST                                   */
    /************************************************************************ */        
        case IDLE_ST:            
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                break;

                case RUN_PH:
                    if ((StatusTimer(T_POWER_ON_TIMER) == T_ELAPSED)) {
                        if (1)
                            nextStatus = RESET_ST;
                        else
                            forceAlarm(DOOR_OPEN);
                        
                        primoReset = 0;
                
                        //Timer_Out_Supply_High = 0;
                        //Timer_Out_Supply_Low = 0;
                        //Diag_Setup_Timer_Received = 0;
                		
                        // Timeout 10 minutes
                        Timer_Out_Supply_High = 1;
                        Timer_Out_Supply_Low = 3000000;
                        Diag_Setup_Timer_Received = 1;
                        StopCleaningManage = FALSE;
                        Start_Tinting = FALSE;                        
                    }
                break;

                case EXIT_PH:
                break;
            }
        break; // IDLE_ST 

    /************************************************************************ */
    /*                              IDLE_ST                                   */
    /************************************************************************ */        
        case RESET_ST:            
            indicator = LIGHT_PULSE_SLOW;
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    // Force COLD RESET when an alarm is signalled 
                    force_cold_reset = (NO_ALARM != alarm())
                    ? TRUE
                    : FALSE
                    ;
                    fastIndex = 0;
                    slowIndex = 0;
                    // Clear alarm error code 
                    resetAlarm();
                    // Stop all actuators 
                    stopAllActuators();
                    // Reset comm timeouts counters 
                    resetSlaveRetries();
                    // Overall timeout, as a safety net to prevent getting stuck while performing a RESET. 
                    StartTimer(T_RESET_TIMEOUT);
                    // Reset start delay, this is needed to make sure processMGBrd sees the transition into the RESET state 
                    StartTimer(T_WAIT_START_RESET);
                    // Stop Humidifier and Heater activities
                    StopHumidifier();
//                    spi_init(SPI_3); //SPI Sensore temperatura                      
                    Humidifier.level = HUMIDIFIER_IDLE;
                    New_Reset_Done = TRUE;
                    Temp_Process_Stop = FALSE;
                    cleaning_status = CLEAN_INIT_ST;
                    DoubleGoup_Stirring_st = 0;
                    StartTimer(T_WAIT_AIR_PUMP_TIME);
                    Dosing_Half_Speed = FALSE;
                    Punctual_Cleaning = OFF;
                    Punctual_Clean_Act = OFF;
#ifdef CLEANING_AFTER_DISPENSING
                    Enable_Cleaning = FALSE;
#endif
                    // EEprom Test setup 1
                    EEpromTestWrite.EE_Test[0] = 0xAAAA;
                    EEpromTestWrite.EE_Test[1] = 0x5555;
                    EEpromTestWrite.EE_Test[2] = 0xAAAA;
                    EEpromTestWrite.EE_Test[3] = 0x5555;
                    EEpromTestWrite.EE_Test[4] = 0xAAAA;
                    EEpromTestWrite.EE_Test[5] = 0x5555;
                    EEpromTestWrite.EE_Test[6] = 0xAAAA;
                    EEpromTestWrite.EE_Test[7] = 0x5555;
                    EEpromTestWrite.EE_Test[8] = 0xAAAA;                    
                    EEpromTestWrite.EE_Test[9] = 0x5555;
                    // Setup EEPROM writing variables 
                    eeprom_byte = 0;
                    eeprom_crc = 0;
                    eeprom_i = 0;                   
                    break;
                    
                break; // ENTRY_PH 

                case RUN_PH:
                    // Wait for initial delay, before even considering anhything else 
                    if (StatusTimer(T_WAIT_START_RESET) != T_ELAPSED)
                        break;                    
                    if (isDeviceGlobalError()) {
                        // Perform *at most* one software reset (this check is necessary in order to avoid reset loop) 
                        if (! inhibitReset)
                            __asm__ volatile ("reset");  
                        // A device global error can only be fixed by a MCU software reset. If we came to this point we already tried that once.
                        // EEPROM parameters CRC check                        
                        if (InitFlags.CRCParamColorCircuitFailed) {
                            forceAlarm(EEPROM_COLOR_CIRC_PARAM_CRC_FAULT);
                        }   
                        else if (InitFlags.CRCParamCalibCurvesFailed) {
                            forceAlarm(EEPROM_CALIB_CURVES_PARAM_CRC_FAULT);
                        }
                        else if (InitFlags.CRCParamSlavesEnFailed) {
                            forceAlarm(EEPROM_SLAVES_EN_PARAM_CRC_FAULT);
                        }
                        else if (InitFlags.CRCParamHumidifier_paramFailed) {
                            forceAlarm(EEPROM_HUM_20_PARAM_CRC_FAULT);
                        }				
                        else if (InitFlags.CRCParamTinting_Pump_paramFailed) {
                            forceAlarm(EEPROM_PUMP_PARAM_CRC_FAULT);
                        }
                        else if (InitFlags.CRCParamTinting_Table_paramFailed) {  
                            forceAlarm(EEPROM_TABLE_PARAM_CRC_FAULT);
                        }
                        else if (InitFlags.CRCParamCircuitPumpTypesFailed) {
                            forceAlarm(EEPROM_CIRCUIT_PUMP_TYPES_CRC_FAULT);
                        }
                        else if (InitFlags.CRCParamTinting_Clean_paramFailed) {
                            forceAlarm(EEPROM_CLEAN_PARAM_CRC_FAULT);
                        }                        
                        break;
                    } // isDeviceGlobalError()
                    // This allows to interrupt a RESET with ENTER_DIAGNOSTIC 
                    if (isGUIDiagnosticCmd()) {
                        nextStatus = DIAGNOSTIC_ST;
                        resetNewProcessingMsg();
                        break;
                    }
                    else if (StatusTimer(T_RESET_TIMEOUT) == T_ELAPSED) {
                        forceAlarm(RESET_TIMEOUT);
                        break;
                    }
/*                    
                    // Tinting Panel Open!	  
                    if (New_Panel_table_status == PANEL_OPEN) {
                        forceAlarm(TINTING_PANEL_TABLE_ERROR);
                        break;
                    }		  
                    // Bases Carriage Open
                    else if (Bases_Carriage_transition == LOW_HIGH) {
                        forceAlarm(TINTING_BASES_CARRIAGE_ERROR);
                        break;
                    } 		  
*/                    
                    if (force_cold_reset)
                        procGUI.reset_mode = 0;
                    
                    switch (MachineStatus.step) {
                        case STEP_0:
                            eeprom_write_result = updateEETest();
                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                eeprom_read_result = updateEETestCRC();
                                if (eeprom_read_result == EEPROM_READ_DONE) {
                                    for (i = 0; i < 10; i++) {   
                                        if (EEpromTestWrite.EE_Test[i] != EEpromTest.EE_Test[i]) {
                                            forceAlarm(EEPROM_TEST_ERROR);
                                            break;
                                        }
                                    }                                   
                                    // EEprom Test setup 2
                                    EEpromTestWrite.EE_Test[0] = 0x5555;
                                    EEpromTestWrite.EE_Test[1] = 0xAAAA;
                                    EEpromTestWrite.EE_Test[2] = 0x5555;
                                    EEpromTestWrite.EE_Test[3] = 0xAAAA;
                                    EEpromTestWrite.EE_Test[4] = 0x5555;
                                    EEpromTestWrite.EE_Test[5] = 0xAAAA;
                                    EEpromTestWrite.EE_Test[6] = 0x5555;
                                    EEpromTestWrite.EE_Test[7] = 0xAAAA;                    
                                    EEpromTestWrite.EE_Test[8] = 0x5555;
                                    EEpromTestWrite.EE_Test[9] = 0xAAAA;
                                    // Setup EEPROM writing variables 
                                    eeprom_byte = 0;
                                    eeprom_crc = 0;
                                    eeprom_i = 0;                                    
                                    MachineStatus.step++;
                                }                                
                            }
                            else if (eeprom_write_result == EEPROM_WRITE_FAILED) {
                                forceAlarm(EEPROM_TEST_ERROR);
                                break;
                            }		  
                        break;    
                            
                        case STEP_1:
                            eeprom_write_result = updateEETest();
                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                eeprom_read_result = updateEETestCRC();
                                if (eeprom_read_result == EEPROM_READ_DONE) {
                                    for (i = 0; i < 10; i++) {   
                                        if (EEpromTestWrite.EE_Test[i] != EEpromTest.EE_Test[i]) {
                                            forceAlarm(EEPROM_TEST_ERROR);
                                            break;
                                        }
                                    }
                                    // EEprom Test setup 3
                                    EEpromTestWrite.EE_Test[0] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[1] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[2] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[3] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[4] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[5] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[6] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[7] = 0xFFFF;                    
                                    EEpromTestWrite.EE_Test[8] = 0xFFFF;
                                    EEpromTestWrite.EE_Test[9] = 0xFFFF;
                                    // Setup EEPROM writing variables 
                                    eeprom_byte = 0;
                                    eeprom_crc = 0;
                                    eeprom_i = 0;                                                                        
                                    MachineStatus.step++;
                                }                                
                            }
                            else if (eeprom_write_result == EEPROM_WRITE_FAILED) {
                                forceAlarm(EEPROM_TEST_ERROR);
                                break;
                            }		  
                        break;
                       
                        case STEP_2:
                            eeprom_write_result = updateEETest();
                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                eeprom_read_result = updateEETestCRC();
                                if (eeprom_read_result == EEPROM_READ_DONE) {
                                    for (i = 0; i < 10; i++) {   
                                        if (EEpromTestWrite.EE_Test[i] != EEpromTest.EE_Test[i]) {
                                            forceAlarm(EEPROM_TEST_ERROR);
                                            break;
                                        }
                                    }
                                    MachineStatus.step++;
                                }                                
                            }
                            else if (eeprom_write_result == EEPROM_WRITE_FAILED) {
                                forceAlarm(EEPROM_TEST_ERROR);
                                break;
                            }		                              
                        break;    
                        
                        case STEP_3:
#ifdef AUTOCAP_MMT
                            // Load Slaves enable mask 
                            EEPROMReadArray(EE_CRC_VALUE_SLAVES_EN, EE_CRC_SIZE,((unsigned char *) &crc_slave));
                            crc_slave = loadEEParamSlavesEn();                        
                            init_Autocap();
                            if (isSlaveCircuitEn(AUTOCAP_ID-1) == TRUE) {
                                procGUI.slaves_en[5] = procGUI.slaves_en[5] & 0xFE;
                                autocap_enabled = TRUE;
                            }
#endif                                                                                                	  
                            Stop_Process = FALSE;
                            // Symbolic value 64
                            Double_Group_0 = 64;
                            Double_Group_1 = 64;                            
                            if (isAllCircuitsStopped()) {
                                Punctual_Clean_Act = OFF;                    
                                //  Find Double Groups 0 and 1 if present (THOR machine, maximum 2 doubles groups)
                                countDoubleGroupAct();                                
                                MachineStatus.step ++;
                            }    
                        break;

                        case STEP_4:
                            //--------------------------------------------------
                            // HUMIDIFIER
                            // Analyze Humidifier Parameters
                            if (AnalyzeHumidifierParam() == FALSE)
                                setAlarm(HUMIDIFIER_20_PARAM_ERROR);
                            
                            MachineStatus.step ++;                            
                        break;
                                                         
                        //
                        // BASE PUMPS (B1 - B8)
                        // ***********************
                        case STEP_5:
                            // INTR + HOMING B1 - B8 
                            if (isCircuitsReady(B1_BASE_IDX, B8_BASE_IDX)) {
                              resetCircuitAct(B1_BASE_IDX, B8_BASE_IDX);
                              MachineStatus.step ++;
                            }
                            else {
                              intrCircuitAct(B1_BASE_IDX, B8_BASE_IDX);
                            }
                        break;

                        case STEP_6:
                            // Wait for acts B1 - B8 to start homing 
                            if (isCircuitsStartedHoming(B1_BASE_IDX, B8_BASE_IDX)) {
                                MachineStatus.step ++;
                            }
                        break;

                        case STEP_7:
                            // Wait for acts B1 - B8 to complete homing
                            if (isCircuitsHoming(B1_BASE_IDX, B8_BASE_IDX)) {
                                recircResetCircuitAct(B1_BASE_IDX, B8_BASE_IDX);
                                MachineStatus.step ++;
                            }
                        break;

                        case STEP_8:
                            // Wait for acts B1 - B8 to complete first recirculation 
                            if (isCircuitsRecircEnd(B1_BASE_IDX, B8_BASE_IDX)) {
                                stopColorActs(B1_BASE_IDX, B8_BASE_IDX);
                                MachineStatus.step ++;
                            }
                        break;
                        
                        case STEP_9:
#ifdef AUTOCAP_MMT
                            if (isAutocapActEnabled() )
                                DRV8842_STOP_RESET();
#endif                                                                                                  
                    		if (!isTintingEnabled() ) {
                                MachineStatus.step += 6; 
                            }    
                            else  {
                                //----------------------------------------------
                                // PUMP
                                initPumpParam();
                                // Analyze Pump Parameters
                                if (AnalyzePumpParameters() == FALSE) {
                                    Status.level = TINTING_BAD_PAR_PUMP_ERROR_ST;
                                    setAlarm(TINTING_BAD_PUMP_PARAM_ERROR);
                                }
                                //----------------------------------------------
                                // TABLE
                                initTableParam();
                                // Analyze Table Parameters
                                if (AnalyzeTableParameters() == FALSE) {
                                    Status.level = TINTING_BAD_PAR_TABLE_ERROR_ST;
                                    setAlarm(TINTING_BAD_TABLE_PARAM_ERROR);
                                }
                                //----------------------------------------------
                                // CLEAN
                                initCleanParam();
                                // Analyze Clean Parameters
                                if (AnalyzeCleanParameters() == FALSE) {
                                    Status.level = TINTING_BAD_PAR_CLEAN_ERROR_ST;
                                    setAlarm(TINTING_BAD_PARAM_CLEAN_ERROR);
                                }
                                //----------------------------------------------                                
                                Status.level = TINTING_INIT_ST;    
                                MachineStatus.step ++;  
                            }    
                        break;

                        //
                        // COLORANT PUMPS (C1 - C8)
                        // ***********************
                        case STEP_10:
                            if(isTintingReady() || isTintingActError() ) {
                                posHomingTintingAct();				
                                MachineStatus.step ++;
                            }
                            else
                                intrTintingAct();							
                        break;

                        case STEP_11:
                            Pump.level  = PUMP_START;
                            Table.level = TABLE_START;                            
                            MachineStatus.step ++;  
                        break;
                        
                        case STEP_12:
                            if (isTintingHoming() ) {
                                intrTintingAct();	
                                MachineStatus.step++;
                            }       
                        break;

                        case STEP_13:
                            if (isTintingReady() ) {
                                idleTintingAct();
                                MachineStatus.step++;
                            }                                       
                        break;		

                        case STEP_14:
                            // Fast forward 
                            MachineStatus.step ++;
                        break;

                        //
                        // AUTOCAP
                        // ***********************
                        case STEP_15:
#ifndef AUTOCAP_MMT
                            // Autocap HOMING if act is enabled and not closed or we're doing a COLD RESET
                            if (isAutocapActEnabled() && (! isAutocapActClose() || ! procGUI.reset_mode)) {
                                intrAutocapAct();
#else
                            // Autocap HOMING if is enabled and not RUNNING
                            if (isAutocapActEnabled() && !isAutocapActRunning() ) {
                                posHomingAutocapMMT();
#endif                                                           
                                MachineStatus.step ++ ;
                            }
                            else
                                MachineStatus.step += 3; // skip 
                        break;

                        case STEP_16:
#ifndef AUTOCAP_MMT                                                        
                            // Initialize autocap HOMING 
                            if (isAutocapActReady()) {
                                initAutocapActHoming();
                                MachineStatus.step ++ ;
                            }
#else                                                        
                            if (isAutocapActClose() )
                                MachineStatus.step +=2 ;                                
#endif                            
                        break;

                        case STEP_17:
#ifndef AUTOCAP_MMT                                                        
                            // Wait for homing to complete 
                            if (isAutocapActHomingCompleted() || isAutocapActError()) {
                                MachineStatus.step ++;
                                statoAutoCap = AUTOCAP_CLOSED;
                            }
#endif                                                        
                        break;

                        case STEP_18:
                            if (TintingHumidifier.Temp_Enable == TEMP_ENABLE)  {
                                Test_rele = ON;
                                StartTimer(T_TEST_RELE);
                                RISCALDATORE_ON();
                                MachineStatus.step ++ ;
                            }
                            else 
                                MachineStatus.step +=2 ;
                        break;

                        case STEP_19:
                            if (StatusTimer(T_TEST_RELE) == T_ELAPSED) {
                                Test_rele = OFF;
                                RISCALDATORE_OFF();
                                StopTimer(T_TEST_RELE);
                                MachineStatus.step ++ ;
                            }                                
                        break;
                        
                        case STEP_20: 
                            Check_Neb_Timer = TRUE;
                            Humidifier.level = HUMIDIFIER_START;                            
                            StopTimer(T_WAIT_AIR_PUMP_TIME);                            
                            // RESET cycle completed 
                            nextStatus = COLOR_RECIRC_ST;
//SPAZZOLA_ON();                                
                        break;

                        default:
                            HALT(); // Unexpected
                        break;    
                    } // switch(MachineStatus.step) 
                break; // RUN_PH 

                case EXIT_PH:
                    // RESET is done. 
                    StopTimer(T_RESET_TIMEOUT);
                break; // EXIT_PH                 
            } // Switch (MachineStatus.phase)
        break; // RESET_ST 
    /************************************************************************ */
    /*                              COLOR_RECIRC_ST                           */
    /************************************************************************ */        
        case COLOR_RECIRC_ST:            
            indicator = LIGHT_STEADY;        
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    // Enable auto COLD RESET upon ALARMs 
                    autoRecoveryFromAlarm = TRUE;
                    setColorRecirc();
                    StartTimer(T_STANDBY_TIMEBASE);
                    New_Tinting_Cmd = FALSE;
                break;

                case RUN_PH:
                    // Tinting Panel Open!	  
                    if (Panel_table_transition == LOW_HIGH) {
                        forceAlarm(TINTING_PANEL_TABLE_ERROR);
                        break;
                    }			
                    // Bases Carriage Open
                    else if (Bases_Carriage_transition == LOW_HIGH) {
                        forceAlarm(TINTING_BASES_CARRIAGE_ERROR);
                        break;
                    }
                    // -----------------------------------------------------
                    // Dosing Temperature: setting critical Temperature field
                    if ( (TintingHumidifier.Temp_Enable == TEMP_ENABLE) && (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && ((TintingAct.Dosing_Temperature/10) > TintingHumidifier.Temp_T_LOW) && 
                         ((TintingAct.Dosing_Temperature/10) <= TintingHumidifier.Temp_T_HIGH))	
                        TintingAct.CriticalTemperature_state = ON;  
                    else 				
                        TintingAct.CriticalTemperature_state = OFF;  
                      // -------------------------------------------------------	
                    if (isTintingEnabled() ) {
                        Cleaning_Manager();

                        if (Clean_Activation == OFF) {
                            // Manage periodic processes
                            standbyProcesses();
                            Temp_Process_Stop = TRUE;
                        }    
                        else if ( (Clean_Activation == ON) && (Temp_Process_Stop == TRUE) ) {
                            Temp_Process_Stop = FALSE;
                            stopAllActuators();
                        }
                    }
                    else
                        // Manage periodic processes
                        standbyProcesses();
                        
                    // Can Locator Manager Active
                    Can_Locator_Manager(ON);		
                    if (isNewProcessingMsg()) {		  
                        switch(procGUI.typeMessage) {
                            case RESET_MACCHINA:
                                Can_Locator_Manager(OFF);
                                StopCleaningManage = TRUE;
                                indx_Clean = MAX_COLORANT_NUMBER;
                                StopTimer(T_WAIT_BRUSH_PAUSE);			                                
                                nextStatus = RESET_ST;
                            break;
                            case DISPENSAZIONE_COLORE_MACCHINA:
                                // Temperature TOO LOW --> Can't Erogate
                                if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && ((TintingAct.Dosing_Temperature/10) <= TintingHumidifier.Temp_T_LOW) ){
                                    setAlarm(TEMPERATURE_TOO_LOW);
                                    break;
                                }
                                indx_Clean = MAX_COLORANT_NUMBER;                                
                                Can_Locator_Manager(OFF);
                                nextStatus = COLOR_SUPPLY_ST;
                            break;
                            case DIAGNOSTICA_MACCHINA:
                                Can_Locator_Manager(OFF);
                                indx_Clean = MAX_COLORANT_NUMBER;                                
                                nextStatus = DIAGNOSTIC_ST;
                                turnToStandBy = 0;
                            break;
                            case DIAG_ROTATING_TABLE_POSITIONING: 
                                Can_Locator_Manager(OFF);
                                indx_Clean = MAX_COLORANT_NUMBER;                                
                                nextStatus = ROTATING_ST;
                                turnToState = DIAGNOSTIC_ST;				
                            break;
                            case DIAG_AUTOTEST_SETTINGS: 
                                Can_Locator_Manager(OFF);
                                indx_Clean = MAX_COLORANT_NUMBER;                                                                
                                nextStatus = AUTOTEST_ST;
                                turnToState = DIAGNOSTIC_ST;				
                            break;			
                        } // switch() 
                        if ( (procGUI.typeMessage >= DIAG_POS_STAZIONE_PRELIEVO)      && (procGUI.typeMessage <= DIAG_ROTATING_TABLE_STEPS_POSITIONING) && 
                             (procGUI.typeMessage != DIAG_ROTATING_TABLE_POSITIONING) && (procGUI.typeMessage != DIAG_AUTOTEST_SETTINGS) && 
                             (procGUI.typeMessage != DIAG_ATTIVA_RICIRCOLO_CIRCUITI)  && (procGUI.typeMessage != DIAG_ATTIVA_AGITAZIONE_CIRCUITI) ) {
                            if (procGUI.typeMessage == DIAG_MOVIMENTAZIONE_AUTOCAP) {
                                turnToStandBy = 1;
                            }
                            indx_Clean = MAX_COLORANT_NUMBER;                                                            
                            nextStatus = DIAGNOSTIC_ST;
                            Can_Locator_Manager(OFF);		  			
                		}
              		    else if ( (procGUI.typeMessage == DIAG_ATTIVA_RICIRCOLO_CIRCUITI) || (procGUI.typeMessage == DIAG_ATTIVA_AGITAZIONE_CIRCUITI) )	 {
                            indx_Clean = MAX_COLORANT_NUMBER;                                
                            if (New_Tinting_Cmd == FALSE) {
                                stopAllActuators();
                                New_Tinting_Cmd = TRUE;
                            }					
                            else if  ( (New_Tinting_Cmd == TRUE) && (!isTintingEnabled()) ) {
                                New_Tinting_Cmd = FALSE;
                                nextStatus = DIAGNOSTIC_ST;
                                Can_Locator_Manager(OFF);		  
                            }                            
                            else if  ( (New_Tinting_Cmd == TRUE) && isTintingEnabled() && isTintingReady() ) {
                                New_Tinting_Cmd = FALSE;
                                nextStatus = DIAGNOSTIC_ST;
                                Can_Locator_Manager(OFF);		  
                            }
                   	    }	
                        else if (((procGUI.typeMessage>=PAR_CURVA_CALIBRAZIONE_MACCHINA) && (procGUI.typeMessage<=DIAG_JUMP_TO_BOOT)) && 
                                  (procGUI.typeMessage!=CAN_LIFTER_MOVEMENT) ) {
                            indx_Clean = MAX_COLORANT_NUMBER;                                                            
                            nextStatus = DIAGNOSTIC_ST;
                            Can_Locator_Manager(OFF);		  
                		}
                        else
                          	resetNewProcessingMsg();
                    }
                break;                    

                case EXIT_PH:
                break;
            } 
        break; // COLOR_RECIRC_ST 

    /************************************************************************ */
    /*                              ROTATING_ST                               */
    /************************************************************************ */                
        case ROTATING_ST:            
            indicator = LIGHT_STEADY;
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    // Disable auto COLD RESET upon ALARMs 
                    autoRecoveryFromAlarm = FALSE;
                    // Reset recirculation and stirring FSMs for all used colors. 
                    resetStandbyProcesses();
                    StartTimer(T_WAIT_TABLE_POSITIONING);
                    stopAllActuators();
                    StopCleaningManage = TRUE; 
                    StopTimer(T_WAIT_BRUSH_PAUSE);			
                    MachineStatus.step = STEP_1;
                break;

                case RUN_PH:

/*                        
            // Bases Carriage Open
            if (Bases_Carriage_transition == LOW_HIGH) 
            {
                StopTimer(T_SEND_PARAMETERS);
                forceAlarm(TINTING_BASES_CARRIAGE_ERROR);
            } 
*/				
                    switch (MachineStatus.step){	
                        case STEP_0:
                            nextStatus = turnToState;												
                        break;

                        case STEP_1:
                            if (StatusTimer(T_WAIT_TABLE_POSITIONING) == T_ELAPSED) {	
                                StopTimer(T_WAIT_TABLE_POSITIONING);		
                                nextStatus = turnToState;					
                            }
                            // Send Rotating Table Positioning command
                            else if (isAllCircuitsHome() ) {
                                TintingPosizionamentoTavola();
                                MachineStatus.step++;	  							
                            }
                            else if (isTintingActError() )	
                                nextStatus = turnToState;					
                        break;

                        case STEP_2:
                            TintingStop();
                            StartTimer(T_WAIT_TABLE_POSITIONING);							
                            MachineStatus.step++;	  														                                
                        break;

                        case STEP_3:
                            if (StatusTimer(T_WAIT_TABLE_POSITIONING) == T_ELAPSED) 
                            {
                                StopTimer(T_WAIT_TABLE_POSITIONING);
                                setAlarm(TINTING_TIMEOUT_TABLE_MOVE_ERROR);
                            }
                            else if (isTintingReady() ) {	
                                if ( ( (TintingAct.Circuit_Engaged != 0) && (TintingAct.Refilling_Angle == 0) ) || (TintingAct.Refilling_Angle != 0) )
								{
									StopTimer(T_WAIT_TABLE_POSITIONING);
									StartTimer(T_WAIT_TABLE_POSITIONING);								
									MachineStatus.step+=2;
								}		
                            }							
                        break;

                        case STEP_4:
                        case STEP_5:
                            if (StatusTimer(T_WAIT_TABLE_POSITIONING) == T_ELAPSED) 
                            {
                                StopTimer(T_WAIT_TABLE_POSITIONING);
                                setAlarm(TINTING_TIMEOUT_TABLE_MOVE_ERROR);
                            }
                            else if (isGUIStatusCmd()) 
                            {
                                StopTimer(T_WAIT_TABLE_POSITIONING);
                                resetNewProcessingMsg();
                                MachineStatus.step = STEP_0;	
                            }
                            else if (isGUIAbortCmd())
                            {
                                StopTimer(T_WAIT_TABLE_POSITIONING);							
                                setAlarm(USER_INTERRUPT);			
                            }
                            else if (isNewProcessingMsg() && (procGUI.typeMessage == RESET_MACCHINA) )
                            {
                                resetNewProcessingMsg();
                                StopTimer(T_WAIT_TABLE_POSITIONING);							
                                nextStatus = RESET_ST;
                            }						                           
                        break;

                        default:
                        break;
                    }
                    break;

                case EXIT_PH:
                break;
            }
        break; // ROTATING_ST 

    /************************************************************************ */
    /*                              AUTOTEST_ST                               */
    /************************************************************************ */                        
        case AUTOTEST_ST:            
            indicator = LIGHT_STEADY;
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    // Disable auto COLD RESET upon ALARMs 
                    autoRecoveryFromAlarm = FALSE;
                    // Reset recirculation and stirring FSMs for all used colors. 
                    resetStandbyProcesses();
                    stopAllActuators();
                    StopCleaningManage = TRUE;
                    StopTimer(T_WAIT_BRUSH_PAUSE);			                    
                    procGUI.Autotest_Cycles_Number = 0;	
                    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) 
                            TintingAct.Autotest_Start[i] = 0;
                    Autotest_indx = 0xFF;
                    Stop_Autotest = OFF;
                    MachineStatus.step = STEP_1;
                break;
		
                case RUN_PH:
                    if (nextStatus == ALARM_ST)
                        turnToState= ALARM_ST;				

                    // Tinting Panel Open!	  
                    else if (isTintingEnabled() && (New_Panel_table_status == PANEL_OPEN) ) {
                        forceAlarm(TINTING_PANEL_TABLE_ERROR);
                        break;
                    } 
                    // Bases Carriage Open
                    else if (Bases_Carriage_transition == LOW_HIGH) {
                        forceAlarm(TINTING_BASES_CARRIAGE_ERROR);
                        break;
                    } 		  
                    if ( (TintingAct.Autotest_Status == OFF) && (Stop_Autotest == OFF) ) {
                        Stop_Autotest = ON;
                        stopAllActuators();
                        MachineStatus.step = STEP_1;	
                    } 		  
                    switch (MachineStatus.step)
                    {	
                        case STEP_0:
                            if (StatusTimer(T_SEND_PARAMETERS) == T_ELAPSED) {
                                StopTimer(T_SEND_PARAMETERS);
                                if (TintingAct.Autotest_Status == OFF)
                                    nextStatus = turnToState;						
                                else {
                                    if (TintingHumidifier.Temp_Enable == TRUE) {                                    
                                        // Disable Heater Process
                                        Dos_Temperature_Enable = FALSE;
                                        StopTimer(T_WAIT_RELE_TIME);
                                        StartTimer(T_WAIT_RELE_TIME);   
                                        TintingAct.HeaterResistance_state = OFF;
                                        RISCALDATORE_OFF();  
                                    }    
                                    MachineStatus.step+= 3;
                                }    
                            }							
                        break;

                        case STEP_1:
                            if (isAllCircuitsHome() )
                                MachineStatus.step++;	  							
                            else if (isTintingActError() )
                                nextStatus = turnToState;
                        break;

                        case STEP_2:
                            if (isGUIStatusCmd()) {
                                StartTimer(T_SEND_PARAMETERS);
                                resetNewProcessingMsg();
                                MachineStatus.step = STEP_0;	
                            }							
                        break;

                        // Start Ricirculation
                        case STEP_3:
                            if (TintingAct.Autotest_Ricirculation_Time != 0) {
                                // Find next circuit to Ricirculate
                                procGUI.diag_color_en = 0;
                                Autotest_indx = 0XFF;							
                                for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
                                    if (isSlaveCircuitEn(i) ){
                                        // 'i' is a Colorant of a Tinting enabled, or a Base of a Single Group, or a Base of a Double Group Master
                                        if ( ( (isColorTintingModule(i) && !isTintingActError() )							||
                                                (isBaseCircuit(i) && (procGUI.circuit_pump_types[i] != PUMP_DOUBLE) )		||
                                                (isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 == 0) ) ) && (TintingAct.Autotest_Start[i] == 0) ){
                                            StopTimer(T_AUTOTEST_RICIRCULATION_TIME);
                                            StartTimer(T_AUTOTEST_RICIRCULATION_TIME);
                                            Autotest_indx = i;
                                            TintingAct.Autotest_Start[i] = 1;
                                            procGUI.diag_color_en = 1L << i;
                                            // Start Ricirculation
                                            procGUI.command = ON;
                                            setColorRecirc();
                                            DiagColorRecirc();
                                            MachineStatus.step ++;	
                                            break;
                                        }	
                                    }
                                }
                                // Ricirculation completed on all circuits enabled
                                if (Autotest_indx == 0xFF)
                                    MachineStatus.step +=3;			  															
                            }
                            // No Ricirculation
                            else 
                                MachineStatus.step +=3;			  							
                        break;

                        case STEP_4:
                            // Ricirculation Time elapsed: Stop
                            if (StatusTimer(T_AUTOTEST_RICIRCULATION_TIME) == T_ELAPSED) {	
                                StopTimer(T_AUTOTEST_RICIRCULATION_TIME);
                                // Stop Ricirculation
                                procGUI.command = OFF;
                                setColorRecirc();
                                DiagColorRecirc();
                                MachineStatus.step ++;								
                            }					
                        break;

                        // Waiting for Ricirculation END					
                        case STEP_5:
                            // Colorant Ricirculation Ended
                            if (isColorTintingModule(Autotest_indx) && isTintingReady() )
                                MachineStatus.step -=2;
                            // Base Ricirculation Ended
                            else if (isBaseCircuit(Autotest_indx) && (isColorActRecircEnd(Autotest_indx) || isColorActHoming(Autotest_indx) ) )
                                MachineStatus.step -=2;
                        break;

                        case STEP_6:
                            for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) 
                                TintingAct.Autotest_Start[i] = 0;
                            Autotest_indx = 0xFF;
                            MachineStatus.step++;
                        break;

                        // Start Dispensing Small Amounts					
                        case STEP_7:												
                            if ( (TintingAct.Autotest_Small_Volume != 0) || (TintingAct.Autotest_Medium_Volume != 0) || (TintingAct.Autotest_Big_Volume != 0) ){
                                Autotest_dosing_amount = 0;
                                if (isAutocapActEnabled() ) {	
                                    if (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK){
                                        openAutocapAct();
                                        MachineStatus.step++;																
                                    }								
                                }								
                                else if (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK)
                                        MachineStatus.step+=3;								
                            }
                            else
                                MachineStatus.step += 10;	
                        break;

                        // Wait for ACK 					
                        case STEP_8:
                            if (isAutocapActOpen())
                                MachineStatus.step ++ ;
                        break;

                        // Wait for completion
                        case STEP_9:
                            if (! isAutocapActRunning()) 
                                MachineStatus.step ++ ;
                        break;

                        // Find Dosing Amounts
                        case STEP_10:
                            if ( (Autotest_dosing_amount == 0) && (TintingAct.Autotest_Small_Volume != 0) )
                                Autotest_dosing_amount = AUTOTEST_SMALL_VOLUME;
                            else if ( (Autotest_dosing_amount == 0) && (TintingAct.Autotest_Medium_Volume != 0) )
                                Autotest_dosing_amount = AUTOTEST_MEDIUM_VOLUME;
                            else if ( (Autotest_dosing_amount == 0) && (TintingAct.Autotest_Big_Volume != 0) )
                                Autotest_dosing_amount = AUTOTEST_BIG_VOLUME;

                            else if ( (Autotest_dosing_amount == AUTOTEST_SMALL_VOLUME) && (TintingAct.Autotest_Medium_Volume != 0) )
                                Autotest_dosing_amount = AUTOTEST_MEDIUM_VOLUME;
                            else if ( (Autotest_dosing_amount == AUTOTEST_SMALL_VOLUME) && (TintingAct.Autotest_Big_Volume != 0) )
                                Autotest_dosing_amount = AUTOTEST_BIG_VOLUME;

                            else if ( (Autotest_dosing_amount == AUTOTEST_MEDIUM_VOLUME) && (TintingAct.Autotest_Big_Volume != 0) )
                                Autotest_dosing_amount = AUTOTEST_BIG_VOLUME;

                            else if (Autotest_dosing_amount == AUTOTEST_BIG_VOLUME)
                                Autotest_dosing_amount = 0;

                            else
                                Autotest_dosing_amount = 0;

                            // Dispensing END
                            if (Autotest_dosing_amount == 0)
                                MachineStatus.step +=5 ;						
                            else				
                                MachineStatus.step ++ ;						
                        break;

                        // Find Dosing Circuit
                        case STEP_11:
                            Autotest_indx = 0xFF;						
                            for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
                                if (isSlaveCircuitEn(i) ) {
                                    // 'i' is a Colorant of a Tinting enabled, or a Base of a Single Group, or a Base of a Double Group Master, or a Base of a Double Froup Slave
                                    if ( ( (isColorTintingModule(i) && !isTintingActError() )							||
                                            (isBaseCircuit(i) && (procGUI.circuit_pump_types[i] != PUMP_DOUBLE) )		||
                                            (isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) ) ) && (TintingAct.Autotest_Start[i] == 0) ) {
                                        Autotest_indx = i;
                                        TintingAct.Autotest_Start[i] = 1;									
                                        break;	
                                    }
                                }
                            }	
                            // Dosing completed on all circuits enabled
                            if (Autotest_indx == 0xFF) {	
                                for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) 
                                    TintingAct.Autotest_Start[i] = 0;							
                                MachineStatus.step --;	
                            }	
                            else {	
                                if (Autotest_dosing_amount == AUTOTEST_SMALL_VOLUME)
                                    colorAct[Autotest_indx].vol_t = TintingAct.Autotest_Small_Volume;
                                else if (Autotest_dosing_amount == AUTOTEST_MEDIUM_VOLUME)
                                    colorAct[Autotest_indx].vol_t = TintingAct.Autotest_Medium_Volume;
                                else if (Autotest_dosing_amount == AUTOTEST_BIG_VOLUME)
                                    colorAct[Autotest_indx].vol_t = TintingAct.Autotest_Big_Volume;

                                MachineStatus.step ++;
                            }	
                        break;

                        case STEP_12:
                            if (TintingAct.Dosing_Temperature == DOSING_TEMP_PROCESS_DISABLED)
                                Dosing_Half_Speed = FALSE;
                            else if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && 
                                 ((TintingAct.Dosing_Temperature/10) <= (TintingHumidifier.Temp_T_HIGH - TintingHumidifier.Heater_Hysteresis)) )
                                Dosing_Half_Speed = TRUE;
                            else if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && 
                                 ((TintingAct.Dosing_Temperature/10) >= (TintingHumidifier.Temp_T_HIGH + TintingHumidifier.Heater_Hysteresis)) )
                                Dosing_Half_Speed = FALSE;
                            
                            calcSupplyPar(colorAct[Autotest_indx].vol_t, color_supply_par[Autotest_indx].vol_mu,color_supply_par[Autotest_indx].vol_mc,Autotest_indx);

                            // Find a Single Group Tinting or Bases
                            if ( (isColorTintingModule(Autotest_indx) && isColorReadyTintingModule(Autotest_indx) ) ||
                                  (isBaseCircuit(Autotest_indx) && (procGUI.circuit_pump_types[Autotest_indx] != PUMP_DOUBLE) && isColorActHoming(Autotest_indx) ) ) {	
                                if ((colorAct[Autotest_indx].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[Autotest_indx].algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) {
                                    colorAct[Autotest_indx].command.cmd = CMD_SUPPLY;	
                                    // Colorant Circuit
                                    if (isColorTintingModule(Autotest_indx) ) {
                                        colorAct[Autotest_indx].algorithm = COLOR_ACT_STROKE_OPERATING_MODE; 
                                        startSupplyContinuousTinting(Autotest_indx);
                                        #if defined NOLAB        
                                            TintingAct.Color_Id = 1;       
                                        #endif                                                                                                                                                    
                                    }
                                    // Base Circuit
                                    else if (isBaseCircuit(Autotest_indx) ) 
                                        setColorActMessage(DISPENSAZIONE_COLORE_CONT, Autotest_indx);
                                }
                                else {
                                    // Colorant Circuit
                                    if (isColorTintingModule(Autotest_indx) ) {
                                        startSupplyTinting(Autotest_indx);
                                        #if defined NOLAB        
                                            TintingAct.Color_Id = 1;       
                                        #endif                                                                                    
                                    }    
                                    // Base Circuit
                                    else if (isBaseCircuit(Autotest_indx) ) 	
                                        startSupplyColor(Autotest_indx);						
                                }
                            }
                            // Find a Double Group with even index (0,2,4,6,): MASTER
                            else if ( (procGUI.circuit_pump_types[Autotest_indx] == PUMP_DOUBLE) && (Autotest_indx % 2 == 0) &&  (isColorActHoming(Autotest_indx)) ) { 
                                if ((colorAct[Autotest_indx].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[Autotest_indx].algorithm == ALG_ASYMMETRIC_CONTINUOUS)) 
                                    startSupplyColorContinuousDoubleGroupMaster(Autotest_indx);
                                else
                                    startSupplyColorDoubleGroupMaster(Autotest_indx);							
                            }						
                            // Find a Double Group with odd index (1,3,5,7,): SLAVE
                            else if ( (procGUI.circuit_pump_types[Autotest_indx] == PUMP_DOUBLE) && (Autotest_indx % 2 != 0) && (isColorActHoming(Autotest_indx-1)) ) {
                                if ((colorAct[Autotest_indx].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[Autotest_indx].algorithm == ALG_ASYMMETRIC_CONTINUOUS))
                                    startSupplyColorContinuousDoubleGroupSlave(Autotest_indx);
                                else						
                                    startSupplyColorDoubleGroupSlave(Autotest_indx);
                            }
                            MachineStatus.step ++;	
                        break;

                        // Waiting for Dispensing END		
                        case STEP_13:
                            // Colorant Circuit
                            if (isTintingSupplyEnd() ) {	
                                TintingStop();	
                                MachineStatus.step ++;
                            }	
                            // Base Circuit
                            else if (isBaseCircuit(Autotest_indx) )  {								
                                // DOUBLE GROUP SLAVE circuit
                                if ( (procGUI.circuit_pump_types[Autotest_indx] == PUMP_DOUBLE) && (Autotest_indx % 2 != 0) && isColorActSupplyEnd(Autotest_indx - 1) ) {
                                    setColorActMessage(CONTROLLO_PRESENZA, Autotest_indx - 1);        
                                    colorAct[Autotest_indx - 1].command.cmd = CMD_STOP;              								
                                    MachineStatus.step ++;
                                }							
                                // SINGLE GROUP or DOUBLE GROUP MASTER circuit								
                                else if isColorActSupplyEnd(Autotest_indx) {
                                    setColorActMessage(CONTROLLO_PRESENZA, Autotest_indx);        
                                    colorAct[Autotest_indx].command.cmd = CMD_STOP;              																
                                    MachineStatus.step ++;
                                }	
                            }
                        break;

                        case STEP_14:
                            if (isAllCircuitsHome() )
                                MachineStatus.step -=3;
                        break;

                        // Closing Autocap
                        case STEP_15:
                            if (isAutocapActEnabled() && (statoAutoCap == AUTOCAP_CLOSED) ) {
                                closeAutocapAct();
                                MachineStatus.step ++;
                            }
                            else
                                MachineStatus.step +=3;							
                        break;

                        case STEP_16:
                            // Wait for ACK 
                            if (isAutocapActClose()) 
                                MachineStatus.step ++ ;
                            break;

                        case STEP_17:
                            // Wait for completion 
                            if (! isAutocapActRunning())
                                MachineStatus.step ++ ;
                        break;

                        case STEP_18:
                            for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) 
                                TintingAct.Autotest_Start[i] = 0;
                            Autotest_indx = 0xFF;
                            Autotest_Color_done = FALSE;
                            MachineStatus.step++;
                        break;

                        // Start Stirring	
                        case STEP_19:
                            if (TintingAct.Autotest_Stirring_Time != 0) {
                                // Find next circuit to Stirr
                                procGUI.diag_color_en = 0;
                                Autotest_indx = 0xFF;							
                                for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) 
                                {
                                    if (isSlaveCircuitEn(i) )
                                    {
                                        // 'i' is a Colorant of a Tinting enabled, or a Base of a Single Group, or a Base of a Double Group Master
                                        if ( ( (isColorTintingModule(i) && !isTintingActError() && (Autotest_Color_done == FALSE) )	||
                                               (isBaseCircuit(i) && (procGUI.circuit_pump_types[i] != PUMP_DOUBLE) )		||
                                               (isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 == 0) ) ) && (TintingAct.Autotest_Start[i] == 0) )
                                        {
                                            if (isColorTintingModule(i) )
                                                Autotest_Color_done = TRUE;

                                            StopTimer(T_AUTOTEST_STIRRING_TIME);
                                            StartTimer(T_AUTOTEST_STIRRING_TIME);
                                            Autotest_indx = i;
                                            TintingAct.Autotest_Start[i] = 1;
                                            diagResetIdleCounter();
                                            // Start Stirring
                                            procGUI.command = ON;
                                            procGUI.diag_color_en = 1L << i;
                                            DiagColorReshuffle();
                                            MachineStatus.step ++;	
                                            break;
                                        }	
                                    }
                                }
                                // Stirring completed on all circuits enabled
                                if (Autotest_indx == 0xFF)
                                    MachineStatus.step +=3;			  															
                            }
                            // No Stirring
                            else 
                                MachineStatus.step +=3;			  												
                        break;

                        case STEP_20:
                            // Stirring Time elapsed: Stop
                            if (StatusTimer(T_AUTOTEST_STIRRING_TIME) == T_ELAPSED) {	
                                StopTimer(T_AUTOTEST_STIRRING_TIME);
                                // Stop Stirring
                                procGUI.command = OFF;
                                DiagColorReshuffle();
                                MachineStatus.step ++;								
                            }					
                        break;

                        // Waiting for Stirring END					
                        case STEP_21:
                            // Colorant Stirring Ended
                            if (isColorTintingModule(Autotest_indx) && isTintingReady() )
                                MachineStatus.step -=2;
                            // Base Stirring Ended
                            else if (isBaseCircuit(Autotest_indx) && isColorActHoming(Autotest_indx) )
                                MachineStatus.step -=2;
                        break;

                        // Cleaning Process
                        case STEP_22:
                            if (TintingClean.Cleaning_duration != 0)
                                TintingAct.Autotest_Cleaning_Status = ON;                            
                            if (TintingAct.Autotest_Cleaning_Status == ON) {
                                // Cleaning Duration (sec)
                                TintingAct.Cleaning_duration = TintingClean.Cleaning_duration;
                                // Cleaning Pause (min)
                                TintingAct.Cleaning_pause = TintingClean.Cleaning_pause;                                  
                                // Start Cleaning Process
                                unsigned short clean_auto;
                                clean_auto = OFF;
                                for (i = 0; i < MAX_COLORANT_NUM; i ++) {
                                    if (TintingAct.Table_Colorant_En[i] == TRUE) { 
                                        TintingAct.Cleaning_Col_Mask[i] = 1;
                                        clean_auto = ON;    
                                    }
                                    else
                                        TintingAct.Cleaning_Col_Mask[i] = 0;                                            
                                }
                                if (clean_auto == ON) {                                
                                  Clean_Activation = ON;
                                  TintingPuliziaTavola();
                                  MachineStatus.step++;  
                                }
                                else
                                  MachineStatus.step +=2;              
                            }
                            else
                                MachineStatus.step +=2;
                        break;

                        // Wait Cleaning end
                        case STEP_23:
                            if ( (Clean_Activation == OFF) && (Table.level == TABLE_END) )
                                MachineStatus.step++;
                        break;
                        
                        // Heater Process
                        case STEP_24:
                            if (TintingHumidifier.Temp_Enable == TRUE)
                                TintingAct.Autotest_Heater_Status = ON;                            
                            if (TintingAct.Autotest_Heater_Status == ON) {
                                if (TintingHumidifier.Temp_Enable == TRUE) {
                                    // Enable Heater Process
                                    Dos_Temperature_Enable = TRUE;                                    
                                    // Duration: 5 min
                                    StartTimer(T_WAIT_AUTOTEST_HEATER);
                                    // Temperature Type MICROCHIP TC72
                                    TintingHumidifier.Temp_Type = TEMPERATURE_TYPE_1;
                                    // Temperature controlled Dosing process Period: 10sec 
                                    TintingHumidifier.Temp_Period = TEMP_PERIOD;
                                    // LOW Temperature threshold value 
                                    TintingHumidifier.Temp_T_LOW = TEMP_T_LOW;
                                    // HIGH Temperature threshold value 
                                    TintingHumidifier.Temp_T_HIGH = TEMP_T_HIGH;
                                    // Heater Activation: > 40°C 
                                    TintingHumidifier.Heater_Temp = HEATER_TEMP;
                                    // Heater Hysteresis 
                                    TintingHumidifier.Heater_Hysteresis = HEATER_HYSTERESIS;
                                    StopTimer(T_WAIT_RELE_TIME);                                
                                    StartTimer(T_WAIT_RELE_TIME);                                                                
                                    TintingAct.HeaterResistance_state = ON;          
                                    RISCALDATORE_ON();
                                    MachineStatus.step++;
                                }
                                else
                                    MachineStatus.step +=2;                                
                            }
                            else
                                MachineStatus.step +=2;
                                    
                        break;

                        case STEP_25:
                            if (StatusTimer(T_WAIT_AUTOTEST_HEATER) == T_ELAPSED) {
                                Dos_Temperature_Enable = FALSE;
                                StopTimer(T_WAIT_AUTOTEST_HEATER);
                                StopTimer(T_WAIT_RELE_TIME);            
                                StartTimer(T_WAIT_RELE_TIME);   
                                TintingAct.HeaterResistance_state = OFF;
                                RISCALDATORE_OFF();                                
                                MachineStatus.step++;	                                
                            }
                        break;
                            
                        // End Autotest Cycle
                        case STEP_26:
                            procGUI.Autotest_Cycles_Number++;
                            if (procGUI.Autotest_Cycles_Number < TintingAct.Autotest_Total_Cycles_Number) {	
                                for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) 
                                    TintingAct.Autotest_Start[i] = 0;
                                Autotest_indx = 0xFF;
                                Autotest_Color_done = FALSE;
                                StartTimer(T_AUTOTEST_PAUSE);
                                MachineStatus.step++;							
                            }
                            // End Autotest
                            else {	
                                unsigned short crc;
                                crc = loadEETintHumidifier_Param();
                                if (TintingHumidifier.Humdifier_Type > 1) {
                                    if (TintingHumidifier.Humdifier_Type > 100)
                                        TintingHumidifier.Humdifier_Type = 100;
                                    TintingHumidifier.Humidifier_PWM = (unsigned char)(TintingHumidifier.Humdifier_Type/2);
                                    TintingHumidifier.Humdifier_Type = HUMIDIFIER_TYPE_2;  
                                } 
                                stopAllCircuitsAct();
                                MachineStatus.step+=2;
                            }	
                        break;

                        case STEP_27:
                            // Pause Time elapsed: Start e NEW cycle
                            if (StatusTimer(T_AUTOTEST_PAUSE) == T_ELAPSED) {						
                                StopTimer(T_AUTOTEST_PAUSE);
                                MachineStatus.step -=24;
                            }						
                        break;

                        // End Autotest
                        case STEP_28:
                            nextStatus = turnToState;												
                        break;

                        default:
                            HALT(); // unexpected 					
                        break;
                    }
                break;

                case EXIT_PH:
                break;
            }
        break; // AUTOTEST_ST 

    /************************************************************************ */
    /*                              COLOR_SUPPLY_ST                           */
    /************************************************************************ */                        
        case COLOR_SUPPLY_ST:            
            indicator = LIGHT_STEADY;
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    // disable auto COLD RESET upon ALARMs 
                    autoRecoveryFromAlarm = FALSE;
                    stopAllActuators();
//                    StopCleaningManage = TRUE; 
                    StopTimer(T_WAIT_BRUSH_PAUSE);
                    
                    if (TintingAct.Dosing_Temperature == DOSING_TEMP_PROCESS_DISABLED)
                        Dosing_Half_Speed = FALSE;
                    else if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && 
                         ((TintingAct.Dosing_Temperature/10) <= (TintingHumidifier.Temp_T_HIGH - TintingHumidifier.Heater_Hysteresis)) )
                        Dosing_Half_Speed = TRUE;
                    else if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && 
                         ((TintingAct.Dosing_Temperature/10) >= (TintingHumidifier.Temp_T_HIGH + TintingHumidifier.Heater_Hysteresis)) )
                        Dosing_Half_Speed = FALSE;
                    
                    // Calculations
                    if (procGUI.dispenserType == FILLING_SEQUENCE_200)
                        setColorSupply();
                    else if ( (procGUI.dispenserType == FILLING_SEQUENCE_20_100_80) || (procGUI.dispenserType == FILLING_SEQUENCE_20_180) )
                        setColorSupplyBasesColorant(20,FALSE);		  
                    else if ( (procGUI.dispenserType == FILLING_SEQUENCE_50_100_50) || (procGUI.dispenserType == FILLING_SEQUENCE_50_150) )
                        setColorSupplyBasesColorant(50,FALSE);		  

                	Filling_step = STEP_1;
                    // Double Group Continuous and Single Stroke Management 
                    DoubleGroupContinuousManagement();
                	// Calculates_Tinting_Colorants_Order();
            	    NEW_Calculates_Tinting_Colorants_Order();
                    // Start timers 
                    StartTimer(T_WAIT_START_SUPPLY);
                    // Timeout on dispensation 
                    Timer_Out_Supply_Duration = Timer_Out_Supply_Low;
                    if (Timer_Out_Supply_High > 0)
                      Durata[T_OUT_SUPPLY] = 3000000;	
                    else  {
                      if (Diag_Setup_Timer_Received == 1)
                          Durata[T_OUT_SUPPLY] = Timer_Out_Supply_Duration;				
                    }
                    StopTimer(T_OUT_SUPPLY);
                    StartTimer(T_OUT_SUPPLY);
                    Count_Timer_Out_Supply = 0;
                    // Reset recirculation and stirring FSMs for all used colors
                    resetStandbyProcesses();
#ifdef CLEANING_AFTER_DISPENSING
                    Enable_Cleaning = TRUE;
#endif                                                
                break;

                case RUN_PH:
                    // Note: periodic processes are *suspended* during dispensation 
                    // Timeout on dispensation start 
                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_ELAPSED)
                        setAlarm(TIMEOUT_SUPPLY_START);
                    // Timeout on dispensation completion
                    else if (StatusTimer(T_OUT_SUPPLY) == T_ELAPSED) {
                        Count_Timer_Out_Supply++;
                        StopTimer(T_OUT_SUPPLY);
                        if (Timer_Out_Supply_High == 0)
                            setAlarm(TIMEOUT_SUPPLY_FAILED);			  
                        else if (Count_Timer_Out_Supply >= Timer_Out_Supply_High) {
                            if (Timer_Out_Supply_Duration == 0)
                                setAlarm(TIMEOUT_SUPPLY_FAILED);
                            else {
                                Durata[T_OUT_SUPPLY] = Timer_Out_Supply_Duration;
                                Timer_Out_Supply_Duration = 0;
                                StartTimer(T_OUT_SUPPLY);
                            }
                        }	
                        else
                            StartTimer(T_OUT_SUPPLY);
            	    }

                    // Tinting Panel Open!	  
                    if (Panel_table_transition == LOW_HIGH) {
                        forceAlarm(TINTING_PANEL_TABLE_ERROR);
                        break;
                    }
                    // Bases Carriage Open
                    if (Bases_Carriage_transition == LOW_HIGH) {
                        forceAlarm(TINTING_BASES_CARRIAGE_ERROR);
                        break;
                    }
                    if (! checkAllRequiredCircuitsEnabled())
                        break; // Do not open AUTOCAP on errors 

                    switch (MachineStatus.step) {
                        case STEP_0:
                            // Wait till all Active Processes are terminated and CAN is PRESENT
                            if (!isAllCircuitsSupplyHome() || (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) != DARK) )
                                break;  

                            //--------------------------------------------------------------		
                            // 1.
                            // Basi presenti
                            // Autocap Disabilitato
                            // Coloranti Presenti 
                            // Start Ricirculation on BASES AND Start Ricirculation on COLORANTS 
                            if (isFormulaColorants() && isFormulaBases() && !isAutocapActEnabled() ) {
                                checkBasesDispensationAct(FALSE);
                                checkColorantDispensationAct(FALSE);
                                MachineStatus.step += 4;					
                            }	
                            //--------------------------------------------------------------
                            // 2.
                            // Basi presenti
                            // Autocap Abilitato
                            // Coloranti Presenti 
                            // NO Start Ricirculation on BASES AND Start Ricirculation on COLORANTS, OPEN AUTOCAP
                            else if (isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {				
                                checkColorantDispensationAct(FALSE);
                                openAutocapAct();
                                MachineStatus.step += 4;					
                            }	
                            //--------------------------------------------------------------			
                            // 3.
                            // Basi assenti
                            // Coloranti Presenti 				
                            // NO Start Ricirculation on BASES AND Start Ricirculation on COLORANTS 
                            else if (isFormulaColorants() && !isFormulaBases() ) {							
                                checkColorantDispensationAct(FALSE);
                                MachineStatus.step += 4;					
                            }	
                            //--------------------------------------------------------------			
                            // 4.
                            // Basi presenti
                            // Autocap Disabilitato
                            // Coloranti Assenti				
                            // Start Ricirculation on BASES AND NO Start Ricirculation on COLORANTS 
                            else if (!isFormulaColorants() && isFormulaBases() && !isAutocapActEnabled() ) {							
                                checkBasesDispensationAct(FALSE);
                                MachineStatus.step += 4;					
                            }	
                            //--------------------------------------------------------------				
                            // 5.
                            // Basi presenti
                            // Autocap Abilitato
                            // Coloranti Assenti				
                            // NO Start Ricirculation on BASES AND NO Start Ricirculation on COLORANTS, OPEN AUTOCAP
                            else if (!isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {			
                                openAutocapAct();
                                MachineStatus.step += 4;
                            }	
                            //--------------------------------------------------------------
                            // 6.
                            // Basi Assenti
                            // Coloranti Assenti				
                            //  End Duispensation
                            else 
                                MachineStatus.step += 14;					
                        break;

                        case STEP_1:
                        case STEP_2:
                        case STEP_3:
                        case STEP_4:
                            //--------------------------------------------------------------			
                            // 1.
                            // Basi Presenti
                            // Autocap Disabilitato
                            // Coloranti Presenti 
                            // Attesa completamento pre-ricircolo Coloranti e Basi
                            if (isFormulaColorants() && isFormulaBases() && !isAutocapActEnabled() ) {
                                checkBasesDispensationAct(FALSE);
                                checkColorantDispensationAct(FALSE);					
                                // Ricircolo Basi terminato
                                // Ricircolo Coloranti terminato
                                if (isAllBasesRecircBeforeFilling() && isAllBasesSupplyHome() && isAllColorantsSupplyHome() ) {							
                                    // Dispensazione Coloranti e Basi Partita
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){							
                                        checkBasesDispensationAct(TRUE);
                                        checkColorantDispensationAct(TRUE);					
                                        MachineStatus.step +=5;												
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);	
                                }
                                // Ricircolo Basi terminato
                                // Ricircolo Coloranti NON terminato
                                else if (isAllBasesRecircBeforeFilling() && isAllBasesSupplyHome() && !isAllColorantsSupplyHome() ) {							
                                    // Dispensazione Basi Partita
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){													
                                        checkBasesDispensationAct(TRUE);
                                        MachineStatus.step +=3;					
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);								
                                }
                                // Ricircolo Basi NON terminato
                                // Ricircolo Coloranti terminato
                                else if ( (!isAllBasesRecircBeforeFilling() || !isAllBasesSupplyHome() ) && isAllColorantsSupplyHome() ) {							
                                    // Dispensazione Coloranti Partita
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){																			
                                        checkColorantDispensationAct(TRUE);					
                                        MachineStatus.step +=4;					
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);								
                                }					
                            }	
                            //---------------------------------------------------------------------
                            // 2.
                            // Basi Presenti
                            // Autocap Abilitato
                            // Coloranti Presenti 
                            // Attesa completamento pre-ricircolo Coloranti e Apertura Autocap
                            else if (isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {				
                                checkColorantDispensationAct(FALSE);										
                                // Autocap Aperto
                                // Ricircolo Coloranti terminato
                                if (isAutocapActOpen() && isAllColorantsSupplyHome() ) {							
                                    // Dispensazione Coloranti Partita
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){																									
                                        checkColorantDispensationAct(TRUE);					
                                        MachineStatus.step +=2;
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);															
                                }
                                // Autocap NON ancora aperto
                                // Ricircolo Coloranti terminato
                                else if (!isAutocapActOpen() && isAllColorantsSupplyHome() ) {							
                                    // Dispensazione Coloranti Partita
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){																															
                                        checkColorantDispensationAct(TRUE);					
                                        MachineStatus.step ++;					
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);																						
                                }
                                // Autocap NON ancora aperto
                                // Ricircolo Coloranti NON terminato
                            }	
                            //---------------------------------------------------------------------				
                            // 3.
                            // Basi Assenti
                            // Coloranti Presenti 				
                            // Attesa completamento pre-ricircolo Coloranti
                            else if (isFormulaColorants() && !isFormulaBases() ) {
                                checkColorantDispensationAct(FALSE);										
                                // Ricircolo Coloranti terminato
                                if (isAllColorantsSupplyHome() ) {							
                                    // Dispensazione Coloranti Partita
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){																															
                                        checkColorantDispensationAct(TRUE);					
                                        MachineStatus.step +=5;
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);	
                                }
                            }	
                            //---------------------------------------------------------------------				
                            // 4.
                            // Basi presenti
                            // Autocap Disabilitato
                            // Coloranti Assenti				
                            // Attesa completamento pre-ricircolo Basi
                            else if (!isFormulaColorants() && isFormulaBases() && !isAutocapActEnabled() )	{
                                checkBasesDispensationAct(FALSE);
                                if (isAllBasesRecircBeforeFilling() && isAllBasesSupplyHome() ) {							
                                    // Dispensazione Basi Partita
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ) {																																					
                                        checkBasesDispensationAct(TRUE);
                                        MachineStatus.step +=5;					
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);							
                                }
                            }	
                            //---------------------------------------------------------------------				
                            // 5.
                            // Basi presenti
                            // Autocap Abilitato
                            // Coloranti Assenti				
                            // Attesa Apertura Autocap 
                            else if (!isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {
                                // Autocap Aperto					
                                if (isAutocapActOpen() ) 
                                    MachineStatus.step +=2;					
                                // Autocap NON ancora aperto
                            }	
                            //---------------------------------------------------------------------
                        break;

                        case STEP_5:
                            // 2.
                            // Basi Presenti
                            // Autocap Abilitato
                            // Coloranti Presenti 
                            // Autocap NON ancora Aperto
                            // Erogazione Coloranti Partita			
                            if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){																																					
                                checkColorantDispensationAct(TRUE);														
                                // Autocap Aperto
                                if (isAutocapActOpen() )
                                    MachineStatus.step ++;
                            }
                            else
                                setAlarm(CAN_ABSENT_DURING_DISPENSING);												
                        break;

                        case STEP_6:
                            // 2.
                            // Basi Presenti
                            // Autocap Abilitato
                            // Coloranti Presenti 
                            // Autocap Aperto
                            // Erogazione Coloranti Partita			
                            if (isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {
                                if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ) {																																					
                                    checkColorantDispensationAct(TRUE);																				
                                    // Attesa completamento Apertura
                                    if (! isAutocapActRunning()) {
                                        // Start pre-Ricircolo Basi
                                        checkBasesDispensationAct(FALSE);																										
                                        MachineStatus.step +=2;
                                    }	
                                }
                                else
                                    setAlarm(CAN_ABSENT_DURING_DISPENSING);																		
                            }
                            // 5.
                            // Basi presenti
                            // Autocap Abilitato
                            // Coloranti Assenti				
                            // Autocap Aperto	
                            else if (!isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() )  {
                                // Attesa completamento Apertura
                                if (! isAutocapActRunning()) {
                                    // Start pre-Ricircolo Basi
                                    checkBasesDispensationAct(FALSE);																										
                                    MachineStatus.step +=2;
                                }						
                            }				
                        break;

                        case STEP_7:
                            // 1.
                            // Basi Presenti
                            // Autocap Disabilitato
                            // Coloranti Presenti 
                            // Dispensazione Basi in corso - Attesa completamento pre-ricircolo Coloranti
                            if (isFormulaColorants() && isFormulaBases() && !isAutocapActEnabled() ) {
                                if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){
                                    checkBasesDispensationAct(TRUE);
                                    //  Attesa completamento pre-ricircolo Coloranti
                                    if(isAllColorantsSupplyHome() ) {	
                                        // Dispensazione Coloranti Partita
                                        checkColorantDispensationAct(TRUE);
                                        MachineStatus.step +=2;						
                                    }
                                }
                                else
                                    setAlarm(CAN_ABSENT_DURING_DISPENSING);																								
                            }
                        break;

                        case STEP_8:
                            // 1.
                            // Basi Presenti
                            // Autocap Disabilitato
                            // Coloranti Presenti 
                            // Dispensazione Coloranti in corso - Attesa completamento pre-ricircolo Basi
                            if (isFormulaColorants() && isFormulaBases() && !isAutocapActEnabled() ) {					
                                if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) )
                                {
                                    checkColorantDispensationAct(TRUE);
                                    checkBasesDispensationAct(FALSE);																				
                                    //  Attesa completamento pre-ricircolo Basi
                                    if (isAllBasesRecircBeforeFilling() && isAllBasesSupplyHome() ) {	
                                        // Dispensazione Basi Partita
                                        checkBasesDispensationAct(TRUE);
                                        MachineStatus.step ++;						
                                    }
                                }
                                else
                                    setAlarm(CAN_ABSENT_DURING_DISPENSING);							
                            }
                            // 2.
                            // Basi Presenti
                            // Autocap Abilitato
                            // Coloranti Presenti 
                            // Autocap Aperto
                            // Erogazione Coloranti in corso
                            // Attesa completamento pre-ricircolo Coloranti				
                            else if (isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {
                                if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ) {
                                    checkColorantDispensationAct(TRUE);
                                    checkBasesDispensationAct(FALSE);																				
                                    //  Attesa completamento pre-ricircolo Basi
                                    if (isAllBasesRecircBeforeFilling() && isAllBasesSupplyHome() ) {	
                                        // Dispensazione Basi Partita
                                        checkBasesDispensationAct(TRUE);
                                        MachineStatus.step ++;						
                                    }											
                                }
                                else
                                    setAlarm(CAN_ABSENT_DURING_DISPENSING);													
                            }
                            // 5.
                            // Basi presenti
                            // Autocap Abilitato
                            // Coloranti Assenti				
                            // Autocap Aperto	
                            // Attesa completamento pre-ricircolo Coloranti
                            else if (!isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {					
                                checkBasesDispensationAct(FALSE);
                                //  Attesa completamento pre-ricircolo Basi
                                if (isAllBasesRecircBeforeFilling() && isAllBasesSupplyHome() ) {
                                    // Dispensation started 
                                    if (StatusTimer(T_WAIT_START_SUPPLY) == T_RUNNING)
                                        StopTimer(T_WAIT_START_SUPPLY);

                                    if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){						
                                        // Dispensazione Basi Partita
                                        checkBasesDispensationAct(TRUE);
                                        MachineStatus.step ++;	
                                    }
                                    else
                                        setAlarm(CAN_ABSENT_DURING_DISPENSING);																				
                                }											
                            }	 				
                        break;			

                        case STEP_9:
                            if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){						
                                // Dispensation in corso di tutti i Coloranti e Basi
                                checkColorantDispensationAct(TRUE);	
                                checkBasesDispensationAct(TRUE);					
                //				if (isAllCircuitsSupplyRun()) {
                                    MachineStatus.step ++;
                //				}
                            }
                            else
                                setAlarm(CAN_ABSENT_DURING_DISPENSING);																				
                        break;

                        case STEP_10:
                            if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ){						
                                checkColorantDispensationAct(TRUE);	
                                // Quando la Dipsensazione della Basi è terminata Chiusura Autocap se Abilitato e se Filling method = "FILLING_SEQUENCE_200"
                                if ( (isAutocapActEnabled() && isFormulaBases() && isAllBasesSupplyEnd() && (statoAutoCap == AUTOCAP_CLOSED) && (procGUI.dispenserType == FILLING_SEQUENCE_200) )
                                                                                                        ||
                                     (isAutocapActEnabled() && isFormulaBases() && isAllBasesSupplyEnd() && (statoAutoCap == AUTOCAP_CLOSED) && (procGUI.dispenserType ==FILLING_SEQUENCE_20_180) && (Filling_step == STEP_2) )
                                                                                                        ||
                                     (isAutocapActEnabled() && isFormulaBases() && isAllBasesSupplyEnd() && (statoAutoCap == AUTOCAP_CLOSED) && (procGUI.dispenserType ==FILLING_SEQUENCE_50_150) && (Filling_step == STEP_2) )	) {					
                                    closeAutocapAct();
                                    MachineStatus.step ++;					
                                }
                                // Controllo fine dispensazione di tutti i circuiti
                                else if (isAllCircuitsSupplyEnd()) {
                                    stopAllCircuitsAct();
                                    MachineStatus.step +=4;
                                }
                            }
                            else
                                setAlarm(CAN_ABSENT_DURING_DISPENSING);																									
                        break;

                        case STEP_11:
                            if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ) {						
                                checkColorantDispensationAct(TRUE);	
                                // Wait for ACK 
                                MachineStatus.step += 2 ;
                                if (isAutocapActClose()) 
                                    MachineStatus.step ++ ;
                            }
                            else
                                setAlarm(CAN_ABSENT_DURING_DISPENSING);																														
                        break;

                        case STEP_12:
                            if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ) {									
                                checkColorantDispensationAct(TRUE);	
                                // Wait for completion 
                                if (! isAutocapActRunning()) 					
                                    MachineStatus.step ++ ;
                            }
                            else
                                setAlarm(CAN_ABSENT_DURING_DISPENSING);																																			
                        break;

                        case STEP_13:
                            if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) ) {												
                                checkColorantDispensationAct(TRUE);	
                                if (isAllCircuitsSupplyEnd()) {
                                    stopAllCircuitsAct();
                                    MachineStatus.step +=4;
                                }
                            }
                            else
                                setAlarm(CAN_ABSENT_DURING_DISPENSING);					
                        break;

                        case STEP_14:
                            // Wait for all circuits to be home 
                            if (isAllCircuitsSupplyHome()) {
                                // Dispensing Sequence: 100% Bases AND 100% Colorants
                                //----------------------------------------------------------
                                if (procGUI.dispenserType == FILLING_SEQUENCE_200) {
                                    if (!isAutocapActEnabled() )
                                        MachineStatus.step+=3;

                                    // If formula has only Colorants NO Autocap Close
                                    else if (!isFormulaBases() )				
                                        MachineStatus.step+=3;

                                    else if (statoAutoCap == AUTOCAP_CLOSED) {
                                        closeAutocapAct();
                                        MachineStatus.step ++;
                                    }
                                    else
                                        MachineStatus.step+=3;
                                }
                                //----------------------------------------------------------
                                else if (procGUI.dispenserType == FILLING_SEQUENCE_20_180) {
                                    // 20% Bases completed
                                    if (Filling_step == STEP_1) {	
                                        setColorSupplyBasesColorant(80,TRUE);
                                        DoubleGroupContinuousManagement();
                                        NEW_Calculates_Tinting_Colorants_Order();
                                        Filling_step = STEP_2;
                                        MachineStatus.step-=14;
                                    }
                                    // 20% Bases + 100% Colorants AND 80% Bases completed
                                    else {
                                        if (!isAutocapActEnabled() )
                                            MachineStatus.step+=3;

                                        // If formula has only Colorants NO Autocap Close
                                        else if (!isFormulaBases() )				
                                            MachineStatus.step+=3;

                                        else if (statoAutoCap == AUTOCAP_CLOSED) {
                                            closeAutocapAct();
                                            MachineStatus.step ++;
                                        }
                                        else
                                            MachineStatus.step+=3;
                                    }					
                                }
                                //----------------------------------------------------------
                                else if (procGUI.dispenserType == FILLING_SEQUENCE_20_100_80) {	
                                    // 20% Bases completed
                                    if (Filling_step == STEP_1) {	
                                        setColorSupplyBasesColorant(0,TRUE);
                                        DoubleGroupContinuousManagement();
                                        NEW_Calculates_Tinting_Colorants_Order();							
                                        Filling_step = STEP_2;
                                        MachineStatus.step-=14;
                                    }
                                    // 100% Coloants completed
                                    else if (Filling_step == STEP_2) {	
                                        setColorSupplyBasesColorant(80,FALSE);
                                        DoubleGroupContinuousManagement();
                                        NEW_Calculates_Tinting_Colorants_Order();														
                                        Filling_step = STEP_3;
                                        MachineStatus.step-=14;
                                    }
                                    // 20% Bases + 100% Colorants + 80% Bases completed
                                    else {
                                        if (!isAutocapActEnabled() )
                                            MachineStatus.step+=3;

                                        // If formula has only Colorants NO Autocap Close
                                        else if (!isFormulaBases() )				
                                            MachineStatus.step+=3;

                                        else if (statoAutoCap == AUTOCAP_CLOSED) {
                                            closeAutocapAct();
                                            MachineStatus.step ++;
                                        }
                                        else
                                            MachineStatus.step+=3;
                                    }					
                                }
                                //----------------------------------------------------------
                                else if (procGUI.dispenserType == FILLING_SEQUENCE_50_100_50) {	
                                    // 50% Bases completed
                                    if (Filling_step == STEP_1) {	
                                        setColorSupplyBasesColorant(0,TRUE);
                                        DoubleGroupContinuousManagement();
                                        NEW_Calculates_Tinting_Colorants_Order();														
                                        Filling_step = STEP_2;
                                        MachineStatus.step-=14;
                                    }
                                    // 100% Coloants completed
                                    else if (Filling_step == STEP_2) {	
                                        setColorSupplyBasesColorant(50,FALSE);
                                        DoubleGroupContinuousManagement();							
                                        NEW_Calculates_Tinting_Colorants_Order();																					
                                        Filling_step = STEP_3;
                                        MachineStatus.step-=14;
                                    }
                                    // 50% Bases + 100% Colorants + 50% Bases completed
                                    else {
                                        if (!isAutocapActEnabled() )
                                            MachineStatus.step+=3;

                                        // If formula has only Colorants NO Autocap Close
                                        else if (!isFormulaBases() )				
                                            MachineStatus.step+=3;

                                        else if (statoAutoCap == AUTOCAP_CLOSED) {
                                            closeAutocapAct();
                                            MachineStatus.step ++;
                                        }
                                        else
                                            MachineStatus.step+=3;
                                    }					
                                }
                                //----------------------------------------------------------
                                else if (procGUI.dispenserType == FILLING_SEQUENCE_50_150) {
                                    // 50% Bases completed
                                    if (Filling_step == STEP_1) {	
                                        setColorSupplyBasesColorant(50,TRUE);
                                        DoubleGroupContinuousManagement();							
                                        NEW_Calculates_Tinting_Colorants_Order();																					
                                        Filling_step = STEP_2;
                                        MachineStatus.step-=14;
                                    }
                                    // 50% Bases + 100% Colorants AND 50% Bases completed
                                    else {
                                        if (!isAutocapActEnabled() )
                                            MachineStatus.step+=3;

                                        // If formula has only Colorants NO Autocap Close
                                        else if (!isFormulaBases() )				
                                            MachineStatus.step+=3;

                                        else if (statoAutoCap == AUTOCAP_CLOSED) {
                                            closeAutocapAct();
                                            MachineStatus.step ++;
                                        }
                                        else
                                            MachineStatus.step+=3;
                                    }					
                                }
                                //----------------------------------------------------------					
                            }
                        break;

                        case STEP_15:
                            // Wait for ACK 
                            MachineStatus.step += 2 ;
                            if (isAutocapActClose()) 
                                MachineStatus.step ++ ;
                        break;

                        case STEP_16:
                            // Wait for completion 
                            if (! isAutocapActRunning()) 
                                MachineStatus.step ++ ;
                        break;

                        case STEP_17: 
                            MachineStatus.step ++;	                            
                        break;

                        case STEP_18: 
                            nextStatus = COLOR_RECIRC_ST;
                            StopTimer(T_OUT_SUPPLY);		  				
                        break;
                    } // switch (MachineStatus.step) 
                break;    

                case EXIT_PH:
                break;
            }
        break; // COLOR_SUPPLY_ST 

    /************************************************************************ */
    /*                              ALARM_ST                                  */
    /************************************************************************ */                        
        case ALARM_ST:            
            indicator = LIGHT_PULSE_FAST;
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    stopAllActuators();
                    StopCleaningManage = TRUE; 
                    indx_Clean = MAX_COLORANT_NUMBER;                    
                    StopTimer(T_WAIT_BRUSH_PAUSE);			                    
                break;

                case RUN_PH:
                    Can_Locator_Manager(OFF);
                    switch (MachineStatus.step) {
                        case STEP_0:
                            if (alarm() == FAILED_JUMP_TO_BOOT_TINTING_MASTER)
                                StartTimer(T_DELAY_WAIT_STOP);
                            // Tinting Panel Open!	  
                            if ( (alarm() != TINTING_PANEL_TABLE_ERROR) && (Panel_table_transition == LOW_HIGH) )
                                MachineStatus.step += 13;			  
                            // Bases Carriage Open
                            else if ( (alarm() != TINTING_BASES_CARRIAGE_ERROR) && (Bases_Carriage_transition == LOW_HIGH) )
                                MachineStatus.step += 13;			  
                            // Tinting Panel Open or Bases Carriage Open	  		
                            else if ( (alarm() == TINTING_PANEL_TABLE_ERROR) || (alarm() == TINTING_BASES_CARRIAGE_ERROR) )
                                MachineStatus.step += 13;	
                            // Initiate normal alarm recovery cycle 
                            else if (isAllCircuitsStopped()) {
                                intrTintingAct();
                                MachineStatus.step ++ ;
                            }    
                        break;

                        case STEP_1:                            
                            // Quite a long delay to ensure pumps are all idle 
                            if (StatusTimer(T_ALARM_RECOVERY) == T_HALTED) 
                                StartTimer(T_ALARM_RECOVERY);                       
                            else if (StatusTimer(T_ALARM_RECOVERY) == T_ELAPSED) 
                                MachineStatus.step ++ ;                       
                            break;

                        case STEP_2:
                        case STEP_3:
                        case STEP_4:
                        case STEP_5:
                        case STEP_6:
                              MachineStatus.step ++ ;
                        break;

                        case STEP_7:
                            if (! isAutocapActError()) {
                                // Close autocap 
                                closeAutocapAct();
                                MachineStatus.step ++;
                            }
                            else 
                              MachineStatus.step += 3; // Recovery                             
                        break;

                        case STEP_8:
                            // Wait for ACK 
                            if (isAutocapActClose())
                              MachineStatus.step ++ ;                          
                        break;

                        case STEP_9:
                            // wait for completion on CLOSE 
                            if (! isAutocapActRunning())
                                MachineStatus.step ++ ;
                            else if (isAutocapActError())
                                MachineStatus.step += 3 ; // Recirc and stirring 
                        break;

                        case STEP_10:
                            // Optional recovery on autocap only on error and container is present 
                            if (isAutocapActError() && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) )
                                MachineStatus.step ++;
                            else 
                                MachineStatus.step += 3; // skip 
                        break;

                        case STEP_11:
                            // Force HOMING on autocap 
                            if (! isAutocapActReady() )
                                intrAutocapAct();
                            else
                                initAutocapActHoming();
                            
                            MachineStatus.step ++ ;                            
                        break;

                        case STEP_12:
                            if (isAutocapActHomingCompleted() || isAutocapActError() )
                              MachineStatus.step ++ ;
                        break;

                        case STEP_13:
                            // Setup periodic processes
                            Stop_Process = FALSE;
                            procGUI.recirc_status = 0x000;            
                            procGUI.stirring_status = 0x0000;  
                    		cleaning_status = CLEAN_INIT_ST;		                            
                            if ( (Panel_table_transition != LOW_HIGH) && (Bases_Carriage_transition != LOW_HIGH) )
                                setColorRecirc();
                            bases_open = 0;
                            StartTimer(T_STANDBY_TIMEBASE);
                            // Make sure the timer is not active when entering next state
                            StopTimer(T_ALARM_AUTO_RESET);
                            MachineStatus.step ++;
                        break;

                        case STEP_14:                            
                            // Manage periodic processes 
                            if ( (Panel_table_transition != LOW_HIGH) && (Bases_Carriage_transition != LOW_HIGH) && (alarm() != USER_INTERRUPT) ) {
                                bases_open = 1;
                                if (isTintingEnabled() ) {
#ifndef CLEANING_AFTER_DISPENSING                                        
                                    if (alarm() != TINTING_BRUSH_READ_LIGHT_ERROR) {
                                        Cleaning_Manager();
                                        if (Clean_Activation == OFF) {
                                            // Manage periodic processes
                                            standbyProcesses();
                                            Temp_Process_Stop = TRUE;
                                        }    
                                        else if ( (Clean_Activation == ON) && (Temp_Process_Stop == TRUE) ) {
                                            Temp_Process_Stop = FALSE;
                                            stopAllActuators();
                                        }
                                    }
#else
                                    // Manage periodic processes
                                    standbyProcesses();
#endif            
                                }
                                else
                                    // Manage periodic processes
                                    standbyProcesses();                                
                            }
                            else {
                                if (bases_open == 1) {	
                                    bases_open = 0;
                                    stopAllActuators();
                                }
                            }
                            // Dosing Temperature: setting critical Temperature field
                            if ( (TintingHumidifier.Temp_Enable == TEMP_ENABLE) && (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && ((TintingAct.Dosing_Temperature/10) > TintingHumidifier.Temp_T_LOW) && 
                                  ((TintingAct.Dosing_Temperature/10) <= TintingHumidifier.Temp_T_HIGH))	
                                TintingAct.CriticalTemperature_state = ON;  
                            else 				
                                TintingAct.CriticalTemperature_state = OFF;  
                            
                            // Auto recovery management from ALARMs
                            if (autoRecoveryFromAlarm && isAlarmRecoverable()) {
                                if (StatusTimer(T_ALARM_AUTO_RESET) == T_HALTED)
                                    StartTimer(T_ALARM_AUTO_RESET);

                                else if (StatusTimer(T_ALARM_AUTO_RESET) == T_ELAPSED) {
                                    StopTimer(T_ALARM_AUTO_RESET);
                                    force_cold_reset = 1;
                                    StopCleaningManage = TRUE; 
                                    indx_Clean = MAX_COLORANT_NUMBER;
                                    StopTimer(T_WAIT_BRUSH_PAUSE);			                                                                    
                                    nextStatus = RESET_ST;
                                }
                            }
                        break;

                        default:
                        HALT(); // Unexpected 
                    } // Switch MachineStatus.step 

                    // JUMP_TO_BOOT Process causes an error:
                    if (alarm() == FAILED_JUMP_TO_BOOT_ACTUATOR) {
                        // After a WAITING Time send JUMP_TO_BOOT command to MAB	  
                        if (StatusTimer(T_DELAY_WAIT_STOP) == T_ELAPSED) {
                            StopTimer(T_DELAY_WAIT_STOP);
                            __asm__ volatile ("reset"); 
                        }	
                    }
                    // Overall
                    else if (isNewProcessingMsg()) {
                        switch(procGUI.typeMessage)
                        {
                            case RESET_MACCHINA:
                                StopCleaningManage = TRUE; 
                                indx_Clean = MAX_COLORANT_NUMBER;                                
                                StopTimer(T_WAIT_BRUSH_PAUSE);			                                                                
                                nextStatus = RESET_ST;
                            break;

                            case DIAGNOSTICA_MACCHINA:
                                StopCleaningManage = TRUE; 
                                indx_Clean = MAX_COLORANT_NUMBER;                                
                                StopTimer(T_WAIT_BRUSH_PAUSE);			                                                                                                
                                nextStatus = DIAGNOSTIC_ST;
                            break;
                        }
                        resetNewProcessingMsg();
                    } // isNewProcessingMsg()
                break;

                case EXIT_PH:
                break;
            }
        break; // ALARM_ST 

    /************************************************************************ */
    /*                         DIAGNOSTIC_ST                                  */
    /************************************************************************ */                        
        case DIAGNOSTIC_ST:            
            indicator = LIGHT_PULSE_SLOW;
            if ( isTintingEnabled() && (Punctual_Clean_Act == ON) && (TintingAct.PanelTable_state == OPEN) ) {
                Punctual_Clean_Act = OFF;
                TintingAct.Cleaning_status = 0x0000; 
                SPAZZOLA_OFF();              
                setAlarm(TINTING_PANEL_TABLE_ERROR);
            }
            switch (MachineStatus.phase) {
                case ENTRY_PH:
                    // Disable auto COLD RESET upon ALARMs 
                    autoRecoveryFromAlarm = FALSE;
                    diagResetIdleCounter();
                    initColorDiagProcesses();
                    stopAllActuators();
                    StopCleaningManage = TRUE;
                    StopTimer(T_WAIT_BRUSH_PAUSE);			                    
                break;

                case RUN_PH:                    
                    diagEvalIdleCounter();  
                    switch(MachineStatus.step) {
                        case STEP_0:                            
                            if (isNewProcessingMsg()) {
                                if (procGUI.typeMessage == RESET_MACCHINA)
                                    nextStatus = RESET_ST;
                                else if ( (procGUI.typeMessage == DIAG_ROTATING_TABLE_POSITIONING) && (Punctual_Clean_Act == OFF) ) {
                            		nextStatus  = ROTATING_ST;
                                    turnToState = DIAGNOSTIC_ST;	
                            	}
                                else if (procGUI.typeMessage == DIAG_AUTOTEST_SETTINGS) {
                                    nextStatus  = AUTOTEST_ST;
                                    turnToState = DIAGNOSTIC_ST;	
                                }		  
                                else if (procGUI.typeMessage == PAR_CURVA_CALIBRAZIONE_MACCHINA || procGUI.typeMessage == PAR_CIRCUITO_COLORANTE_MACCHINA) {
                                    diagResetIdleCounter();
                                    // Setup EEPROM writing variables 
                                    eeprom_byte = 0;
                                    eeprom_crc = 0;
                                    eeprom_i = 0;
                                    eeprom_j = 0;
                                    MachineStatus.step ++;
                                }
                                else if (procGUI.typeMessage == PAR_SLAVES_CONFIGURATION) {
                                    diagResetIdleCounter();
                                    // Setup EEPROM writing variables 
                                    eeprom_byte = 0;
                                    eeprom_crc = 0;
                                    eeprom_i = 0;
                                    eeprom_j = 0;
                                    // Prevent timeouts on newly activated actuators 
                                    resetSlaveRetries();
                                    MachineStatus.step ++;
                                }
                                else if (procGUI.typeMessage==DIAG_SETUP_HUMIDIFIER_TEMPERATURE_PROCESSES) {
                                    diagResetIdleCounter(); 
                                    // Setup EEPROM writing variables 
                                    eeprom_byte = 0;
                                    eeprom_crc = 0;
                                    eeprom_i = 0;
                                    eeprom_j = 0;
                        			MachineStatus.step ++;
                    		    }
                        		else if( (procGUI.typeMessage==UPDATE_TINTING_PUMP_SETTINGS) || (procGUI.typeMessage==UPDATE_TINTING_TABLE_SETTINGS) ||
                                         (procGUI.typeMessage==UPDATE_TINTING_CLEANING_SETTINGS) ) {
                                    diagResetIdleCounter();			  
                                    // Setup EEPROM writing variables 
                                    eeprom_byte = 0;
                                    eeprom_crc = 0;
                                    eeprom_i = 0;
                                    eeprom_j = 0;
                        			MachineStatus.step ++;
                        		}
                                else if (procGUI.typeMessage==DIAG_SETUP_PUMP_TYPE) {
                                    diagResetIdleCounter();
                                    // Setup EEPROM writing variables 
                                    eeprom_byte = 0;
                                    eeprom_crc = 0;
                                    eeprom_i = 0;
                                    eeprom_j = 0;
                                    MachineStatus.step ++;
                    		    }		 
                                else {                               
                                    switch (procGUI.typeMessage) {
                                        case DIAG_ATTIVA_AGITAZIONE_CIRCUITI:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step += 3;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                
                                            else {
                                                diagResetIdleCounter();
                                                DiagColorReshuffle();
                                                MachineStatus.step ++;
                                            }				
                                        break;

                                        case DIAG_ATTIVA_RICIRCOLO_CIRCUITI:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step += 3;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                                                            
                                            else {
                                                setColorRecirc();
                                                diagResetIdleCounter();
                                                DiagColorRecirc();
                                                MachineStatus.step ++;			  
                                            }				
                                        break;

                                        case DIAG_ATTIVA_DISPENSAZIONE_CIRCUITI:                                            
                                            // Temperature TOO LOW --> Can't Erogate
                                            if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && ((TintingAct.Dosing_Temperature/10) <=  TintingHumidifier.Temp_T_LOW) ){
                                                setAlarm(TEMPERATURE_TOO_LOW);
                                                MachineStatus.step += 2;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                                                            
                                            
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step += 2;			  
                                            }
                                            // Erogate ONLY if AUTOCAP is OPEN
                                            else if (isAutocapActEnabled() && (Run_Dispensing == 0) ) {					  
                                                for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
                                                    if (isDiagColorCircuitEn(i))
                                                        break;
                                                }
                                                indx = i;
                                                // 'indx' is a SLAVE of a DOUBLE GROUP
                                                if ((procGUI.circuit_pump_types[indx] == PUMP_DOUBLE) && (indx % 2 != 0) )
                                                    indx--;

                                                // 'indx' = Base --> Autocap Open is required if Erogation is starting
                                                if (isBaseCircuit(indx) && (isColorActHoming(indx)) ){							
                                                    if ( ( (procGUI.command == 1) && (isAutocapActOpen()) ) || (procGUI.command == 0) ) {
                                                        diagResetIdleCounter();
                                                        DiagColorSupply();
                                                        Run_Dispensing = 1;
                                                    }
                                                    else
                                                        MachineStatus.step += 2;
                                                }
                                                // 'indx' = Colorant --> No Autocap Open is required
                                                else if (!isBaseCircuit(indx) && (isColorReadyTintingModule(indx) || isTintingStopped() ) ) {
                                                    diagResetIdleCounter();
                                                    DiagColorSupply();
                                                    Run_Dispensing = 1;
                                                }						
                                            }
                                            else if (Run_Dispensing == 0){
                                                for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
                                                    if (isDiagColorCircuitEn(i))
                                                        break;
                                                }
                                                indx = i;
                                                // 'indx' is a SLAVE of a DOUBLE GROUP
                                                if ((procGUI.circuit_pump_types[indx] == PUMP_DOUBLE) && (indx % 2 != 0) )
                                                    indx--;

                                                // Base Erogation starting
                                                if (isBaseCircuit(indx) && (isColorActHoming(indx)) ){							
                                                    diagResetIdleCounter();
                                                    DiagColorSupply();
                                                    Run_Dispensing = 1;
                                                }
                                                // 'indx' = Colorant --> Colorant Erogation starting
                                                else if (!isBaseCircuit(indx) && (isColorReadyTintingModule(indx) || isTintingStopped()) ) {
                                                    diagResetIdleCounter();
                                                    DiagColorSupply();
                                                    Run_Dispensing = 1;
                                                }						
                                            }
                                            else if (Run_Dispensing == 1) {
                                                for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
                                                    if (isDiagColorCircuitEn(i))
                                                        break;
                                                }
                                                indx = i;
                                                if (isBaseCircuit(indx) && (procGUI.circuit_pump_types[indx] != PUMP_DOUBLE) && isColorActSupplyEnd(indx) )
                                                        MachineStatus.step ++;
                                                else if (isBaseCircuit(indx) && (procGUI.circuit_pump_types[indx] == PUMP_DOUBLE) && (indx % 2 == 0) && isColorActSupplyEnd(indx) )
                                                        MachineStatus.step ++;
                                                else if (isBaseCircuit(indx) && (procGUI.circuit_pump_types[indx] == PUMP_DOUBLE) && (indx % 2 != 0) && isColorActSupplyEnd(indx-1) )
                                                        MachineStatus.step ++;
                                                else if (!isBaseCircuit(indx) && isTintingSupplyEnd() ) 		
                                                        MachineStatus.step ++;
                                            }
                                        break;

                                        case DIAG_ATTIVA_EV_DISPENSAZIONE:
                                            diagResetIdleCounter();
                                            DiagColorSet_EV();
                                            MachineStatus.step ++;
                                        break;

                                        case DIAG_IMPOSTA_DISPENSAZIONE_CIRCUITI:
                                        case DIAG_IMPOSTA_RICIRCOLO_CIRCUITI:
                                            diagResetIdleCounter();
                                            MachineStatus.step ++;
                                        break;

                                        case DIAG_MOVIMENTAZIONE_AUTOCAP:
                                            diagResetIdleCounter();
                                            if (isAutocapActEnabled() ) {
                                                switch (procGUI.diag_motion_autocap) {
                                                    case AUTOCAP_CLOSED:
                                                        closeAutocapAct();
                                                        statoAutoCap = AUTOCAP_CLOSED;
                                                        if (turnToStandBy)
                                                            nextStatus = COLOR_RECIRC_ST;
                                                    break;

                                                    case AUTOCAP_OPEN:
                                                        openAutocapAct();
                                                        statoAutoCap = AUTOCAP_OPEN;
                                                        if (turnToStandBy)
                                                            nextStatus = COLOR_RECIRC_ST;
                                                    break;

                                                    default:
                                                    break;                                                    
                                                } // switch()
                                            }
                                            else if (turnToStandBy)
                                                nextStatus = COLOR_RECIRC_ST;                                                
                                        break; // 

                                        case DIAG_DISPENSATION_VOL:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step += 3;			  
                                            }
                                            // Bases Carriage Open
                                            else if (Bases_Carriage_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_BASES_CARRIAGE_ERROR);
                                                MachineStatus.step += 3;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                                                            
                                            
                                            // Temperature TOO LOW --> Can't Erogate
                                            if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && ((TintingAct.Dosing_Temperature/10) <= TintingHumidifier.Temp_T_LOW) ){
                                                setAlarm(TEMPERATURE_TOO_LOW);
                                                MachineStatus.step += 3;			  
                                            }	 
                                            else {
                                				diagResetIdleCounter();
                                                if (TintingAct.Dosing_Temperature == DOSING_TEMP_PROCESS_DISABLED)
                                                    Dosing_Half_Speed = FALSE;
                                                else if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && 
                                                     ((TintingAct.Dosing_Temperature/10) <= (TintingHumidifier.Temp_T_HIGH - TintingHumidifier.Heater_Hysteresis)) )
                                                    Dosing_Half_Speed = TRUE;
                                                else if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && 
                                                     ((TintingAct.Dosing_Temperature/10) >= (TintingHumidifier.Temp_T_HIGH + TintingHumidifier.Heater_Hysteresis)) )
                                                    Dosing_Half_Speed = FALSE;
                                                
                                                calcSupplyPar(colorAct[procGUI.id_color_circuit].vol_t, color_supply_par[procGUI.id_color_circuit].vol_mu,
                                                             color_supply_par[procGUI.id_color_circuit].vol_mc,procGUI.id_color_circuit);
                                				// Find a Single Group
                                                if ( (procGUI.circuit_pump_types[procGUI.id_color_circuit] != PUMP_DOUBLE) && 
                                                     (isColorReadyTintingModule(procGUI.id_color_circuit) || ((procGUI.id_color_circuit < 8) && isColorActHoming(procGUI.id_color_circuit)))) {
                                                    if ((colorAct[procGUI.id_color_circuit].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[procGUI.id_color_circuit].algorithm == ALG_ASYMMETRIC_CONTINUOUS)){
                                                        // If Dosing Temperature process is enabled and Temperature <= Temp_T_HIGH  --> Set Algorithm SINGLE STROKE
                                                        colorAct[procGUI.id_color_circuit].command.cmd = CMD_SUPPLY;							  						
                                                        // Colorant Circuit
                                                        if ((procGUI.id_color_circuit >= 8) && (procGUI.id_color_circuit <= 31) ) {
                                                            colorAct[procGUI.id_color_circuit].algorithm = COLOR_ACT_STROKE_OPERATING_MODE;
                                                            startSupplyContinuousTinting(procGUI.id_color_circuit);
                                                            #if defined NOLAB        
                                                                TintingAct.Color_Id = 1;       
                                                            #endif                                                                                                                                                                            
                                                        }
                                                        // Base Circuit
                                                        else if (procGUI.id_color_circuit < 8) 
                                                            setColorActMessage(DISPENSAZIONE_COLORE_CONT, procGUI.id_color_circuit);
                                                    }
                                                    else {
                                                        // Colorant Circuit
                                                        if ((procGUI.id_color_circuit >= 8) && (procGUI.id_color_circuit <= 31) ) {
                                                            startSupplyTinting(procGUI.id_color_circuit);
                                                            #if defined NOLAB        
                                                                procGUI.id_color_circuit = COLORANT_ID_OFFSET + 1;        
                                                            #endif                                                                                                                                                                                                    
                                                        }    
                                                        // Base Circuit
                                                        else if (procGUI.id_color_circuit < 8) 	
                                                            startSupplyColor(procGUI.id_color_circuit);									
                                                    }	
                                                }    
                                                // Find a Double Group with even index (0,2,4,6,): MASTER
                                                else if ( (procGUI.circuit_pump_types[procGUI.id_color_circuit] == PUMP_DOUBLE) && (procGUI.id_color_circuit%2 == 0) && (isColorActHoming(procGUI.id_color_circuit)) ) { 
                                                    if ((colorAct[procGUI.id_color_circuit].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[procGUI.id_color_circuit].algorithm == ALG_ASYMMETRIC_CONTINUOUS)) {
                                                        // If Dosing Temperature process is enabled and Temperature <= Temp_T_HIGH  --> Set Algorithm SINGLE STROKE
                                                        startSupplyColorContinuousDoubleGroupMaster(procGUI.id_color_circuit);
                                                    }
                                                    else
                                                        startSupplyColorDoubleGroupMaster(procGUI.id_color_circuit);							
                                                }						
                                                // Find a Double Group with odd index (1,3,5,7,): SLAVE
                                                else if ( (procGUI.circuit_pump_types[procGUI.id_color_circuit] == PUMP_DOUBLE) && (procGUI.id_color_circuit%2 != 0) && (isColorActHoming(procGUI.id_color_circuit-1)) ) {
                                                    if ((colorAct[procGUI.id_color_circuit].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[procGUI.id_color_circuit].algorithm == ALG_ASYMMETRIC_CONTINUOUS)) {
                                                        // If Dosing Temperature process is enabled and Temperature <= Temp_T_HIGH  --> Set Algorithm SINGLE STROKE
                                                        startSupplyColorContinuousDoubleGroupSlave(procGUI.id_color_circuit);
                                                    }
                                                    else						
                                                        startSupplyColorDoubleGroupSlave(procGUI.id_color_circuit);
                                                }												
                                                MachineStatus.step ++;
                                            }    
                                            break;
                                            
                                        case DIAG_RESET_EEPROM:
                                            // We need to reset the EEPROM  - ticket #108
                                            MachineStatus.step++;
                                            resetEEprom();
                                            eeprom_byte = 0;
                                            eeprom_crc = 0;
                                            eeprom_i = 0;
                                            eeprom_j = 0;
                                            indiceReset = 0;
                                            fasiCancellazione = 0;
                                            clearEEPROMInCorso = 1;
                                        break;
                                                
                                        case DIAG_SET_TINTING_PERIPHERALS:
                                            // Send Set Peripheral 
                                            if (isTintingReady() ){							
                                                diagResetIdleCounter();			
                                                TintingSetPeripheralAct();
                                                MachineStatus.step ++;
                                            }
                                            else if (isTintingActError() )                                    
                                                MachineStatus.step ++;                                            
                                        break;
                                        
                                        case DIAG_ROTATING_TABLE_STEPS_POSITIONING:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                                                                                                        
                                            // Send Rotating Table Steps Positioning
                                            else if (isTintingReady() ){							
                                                diagResetIdleCounter();			
                                                TintingPosizionamentoPassiTavola();
                                                MachineStatus.step ++;                                                    
                                            }
                                            else if (isTintingActError() )
                                                MachineStatus.step ++;                                            
                                        break;
				
                                        case DIAG_ROTATING_TABLE_SEARCH_POSITION_REFERENCE:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                                                                                                                                                    
                                            // Send Rotating Table Search Position Reference 
                                            if (isTintingReady() ) {							
                                                diagResetIdleCounter();			
                                                TintingRicercaRiferimentoTavola();
                                                MachineStatus.step ++;                                                    
                                            }
                                            else if (isTintingActError() )
                                                MachineStatus.step ++;
                                        break;

                                        case DIAG_ROTATING_TABLE_TEST:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                                                                                                                                                    
                                            // Send Rotating Table Test
                                            if ( isTintingReady() ) {							
                                                diagResetIdleCounter();
                                                TintingTestFunzionamnetoTavola();
                                                MachineStatus.step ++;                                                                                                   
                                            }
                                            else if (isTintingActError() )
                                                MachineStatus.step ++;                                            
                                        break;
				
                                        case ROTATING_TABLE_FIND_CIRCUITS_POSITION:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            else if (Punctual_Clean_Act == ON)
                                                MachineStatus.step ++;                                                                                                                                                                                    
                                            // Send Rotating Table Find Circuit Position
                                            if (isTintingReady() ) {							
                                                diagResetIdleCounter();			
                                                TintingAutoapprendimentoTavola();
                                                MachineStatus.step ++;                                                                                                    
                                            }
                                            else if (isTintingActError() )
                                                MachineStatus.step ++;                                                                                        
                                        break;
				
                                        case DIAG_COLORANT_ACTIVATION_CLEANING:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step += 3;			  
                                            }
                                            // Punctual Cleaning TURN - OFF
                                            else if ( (Punctual_Cleaning == ON) && (procGUI.command == OFF) ) {
                                                DiagColorClean();
                                                MachineStatus.step ++;                                                    
                                            }
                                            else if (TintingAct.Id_Punctual_Cleaning == COLORANT_NOT_VALID)
                                                MachineStatus.step += 3;			                                                  
                                            // Punctual Cleaning TURN - ON
                                            else if ( (Punctual_Cleaning == OFF) && (procGUI.command == ON) ) {
                                                if (isTintingReady()) {
                                                    if (step_Clean == 0)  {
                                                        step_Clean = 1;
                                                        diagResetIdleCounter();
                                                        stopAllActuators();                                                    
                                                    }
                                                    else if (step_Clean == 1) {
                                                        if (isAllCircuitsHome() ) {
                                                            step_Clean = 2;
                                                        }                                                    
                                                    }
                                                    else {
                                                        DiagColorClean();
                                                        MachineStatus.step ++;                                                    
                                                    }
                                                }
//                                              else                              
//                                                  MachineStatus.step ++;                                                        
                                            }
                                            else 
                                                MachineStatus.step ++;                                                                                                    
                                        break; 
                                        				
                                        case DIAG_JUMP_TO_BOOT:
                                            diagResetIdleCounter();
                                            nextStatus  = JUMP_TO_BOOT_ST;
                                            turnToState = DIAGNOSTIC_ST;
                                            MachineStatus.step ++;
                                        break;
                                    } // switch() 
                                } 
                                if ( (procGUI.typeMessage != DIAG_ATTIVA_DISPENSAZIONE_CIRCUITI) && (procGUI.typeMessage != DIAG_ROTATING_TABLE_TEST) && 
                                     (procGUI.typeMessage != ROTATING_TABLE_FIND_CIRCUITS_POSITION) && (procGUI.typeMessage != DIAG_COLORANT_ACTIVATION_CLEANING) && 
                                     (procGUI.typeMessage != DIAG_ROTATING_TABLE_SEARCH_POSITION_REFERENCE) && (procGUI.typeMessage != DIAG_ROTATING_TABLE_STEPS_POSITIONING) && 
                                     (procGUI.typeMessage != DIAG_SET_TINTING_PERIPHERALS) )
                                    resetNewProcessingMsg();
                            } // isNewProcessingMsg()
                            checkDiagColorSupplyEnd();
                            checkDiagColorRecircEnd();
                        break;
                        
                        case STEP_1:                            
                            // Processing a long command 
                            checkDiagColorSupplyEnd();
                            checkDiagColorRecircEnd();
                            switch (procGUI.typeMessage) {
                                case PAR_CURVA_CALIBRAZIONE_MACCHINA:
                                    eeprom_write_result = updateEECalibCurve(procGUI.id_calib_curve,procGUI.id_color_circuit);
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {                                        
                                        eeprom_read_result = updateEEParamCalibCurvesCRC();
                                        if (eeprom_read_result == EEPROM_READ_DONE) {
                                            MachineStatus.step ++;
                                        }
                                    }
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                        MachineStatus.step += 2;
                                break;

                                case PAR_CIRCUITO_COLORANTE_MACCHINA:
                                    eeprom_write_result = updateEEColorCircuit(procGUI.id_color_circuit);
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        eeprom_read_result = updateEEParamColorCircCRC();
                                        if (eeprom_read_result == EEPROM_READ_DONE)
                                            MachineStatus.step ++; 
                                    }
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                      MachineStatus.step += 2; 
                                break;

                                case PAR_SLAVES_CONFIGURATION:
                                    eeprom_write_result = updateEESlavesEn();
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        eeprom_read_result = updateEEParamSlavesEnCRC();
                                        if (eeprom_read_result == EEPROM_READ_DONE) {
#ifdef AUTOCAP_MMT                      
                                            if (isSlaveCircuitEn(AUTOCAP_ID-1) == TRUE) {
                                                procGUI.slaves_en[5] = procGUI.slaves_en[5] & 0xFE;
                                                autocap_enabled = TRUE;
                                            }
                                            else
                                                autocap_enabled = FALSE;                    
#endif                                            
                                            MachineStatus.step ++; 
                                        }    
                                    }
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                        MachineStatus.step += 2; 
                                break;

                                case DIAG_SETUP_PUMP_TYPE:
                                    eeprom_write_result = updateEECircuitPumpTypes();
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        eeprom_read_result = updateEECircuitPumpTypesCRC();
                                        if (eeprom_read_result == EEPROM_READ_DONE)
                                            MachineStatus.step ++;  
                                    }
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                        MachineStatus.step += 2;  
                                break;		  
		  
                                case UPDATE_TINTING_CLEANING_SETTINGS:
                                    eeprom_write_result = updateEETintCleaning();
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        updateEETintCleaning_CRC();
                                        MachineStatus.step ++; 
                                    }	
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                        MachineStatus.step += 2; 
                                break;
                                
                                case DIAG_SETUP_HUMIDIFIER_TEMPERATURE_PROCESSES:
                                    eeprom_write_result = updateEETintHumidifier();
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        updateEETintHumidifier_CRC();
                                        if (TintingHumidifier.Humdifier_Type > 1) {
                                            if (TintingHumidifier.Humdifier_Type > 100)
                                                TintingHumidifier.Humdifier_Type = 100;
                                            TintingHumidifier.Humidifier_PWM = (unsigned char)(TintingHumidifier.Humdifier_Type/2);
                                            TintingHumidifier.Humdifier_Type = HUMIDIFIER_TYPE_2;
                                        }                                        
                                        MachineStatus.step ++; 
                                    }	
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                        MachineStatus.step += 2; 
                                break;
   
                                case UPDATE_TINTING_PUMP_SETTINGS:
                                    eeprom_write_result = updateEETintPump();
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        updateEETintPump_CRC();
                                        MachineStatus.step ++; 
                                    }	
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                      MachineStatus.step += 2;
                                break;

                                case UPDATE_TINTING_TABLE_SETTINGS:
                                    eeprom_write_result = updateEETintTable();
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        updateEETintTable_CRC();
                                        MachineStatus.step ++; 
                                    }	
                                    else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                        MachineStatus.step += 2; 
                                break;

                                case DIAG_RESET_EEPROM:
                                    switch (fasiCancellazione) {
                                        case 0:
                                            // We delete the color supply parameters
                                            eeprom_write_result = updateEEColorCircuit(indiceReset);
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                eeprom_byte = 0;
                                                indiceReset++;
                                                if (indiceReset == N_SLAVES_COLOR_ACT) {
                                                    resetEEParamColorCircCRC();
                                                    fasiCancellazione++; // Ok 
                                                    eeprom_byte = 0;
                                                    eeprom_crc  = 0;
                                                    eeprom_i = 0;
                                                    eeprom_j = 0;
                                                    indiceReset = 0;
                                                }
                                            }
                                            else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                              MachineStatus.step += 2; 
                                        break;

                                        case 1:
                                            // We delete the calib curves
                                            eeprom_write_result = updateEECalibCurve(eeprom_i,indiceReset);
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                eeprom_i++;
                                                eeprom_byte = 0;
                                                if (eeprom_i == N_CALIB_CURVE) {
                                                    eeprom_i = 0;
                                                    indiceReset++;
                                                    if (indiceReset == N_SLAVES_COLOR_ACT) {
                                                        // We finish to write calib curves
                                                        resetEEParamCalibCurvesCRC();
                                                        fasiCancellazione++; 
                                                        eeprom_byte = 0;
                                                        eeprom_crc = 0;
                                                        eeprom_i = 0;
                                                        eeprom_j = 0;
                                                        indiceReset=0;
                                                    }
                                                }
                                            }
                                            else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                              MachineStatus.step += 2; 
                                        break;
                                        
                                        case 2:
                                            // We delete the active slaves
                                            eeprom_write_result = updateEESlavesEn();
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                fasiCancellazione++;
                                            }			
                                        break;
                                        
                                        case 3:
                                            fasiCancellazione++;
                                        break;

                                        case 4:
                                            fasiCancellazione++;
                                        break;		

                                        case 5:
                                            // We delete the Tinting module Humidifier
                                            eeprom_write_result = updateEETintHumidifier();
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                fasiCancellazione++; 
                                                eeprom_byte = 0;
                                            }
                                            else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                                MachineStatus.step += 2; // Err 			
                                        break;	
                                        
                                        case 6:
                                            // We delete the Tinting module Pump
                                            eeprom_write_result = updateEETintPump();
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                resetEETintPumpEEpromCRC();
                                                fasiCancellazione++; 
                                                eeprom_byte = 0;
                                            }
                                            else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                                MachineStatus.step += 2;  			
                                        break;	
                                        
                                        case 7:
                                            // We delete the Tinting module Table
                                            eeprom_write_result = updateEETintTable();
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                resetEETintTableEEpromCRC();
                                                fasiCancellazione++; 
                                                eeprom_byte = 0;
                                            }
                                            else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                                MachineStatus.step += 2;  			
                                        break;
                                        
                                        case 8:
                                            // We delete the Tinting module Clean
                                            eeprom_write_result = updateEETintCleaning();
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                resetEETintCleaningEEpromCRC();
                                                fasiCancellazione++; 
                                                eeprom_byte = 0;
                                            }
                                            else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                                MachineStatus.step += 2;  			
                                        break;
                                        
                                        case 9:
                                            // We delete the Pump Type
                                            eeprom_write_result = updateEECircuitPumpTypes();
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                fasiCancellazione = 0; 
                                                eeprom_byte = 0;
                                                MachineStatus.step ++; // Err 			
                                                __asm__ volatile ("reset");  
                                            }
                                            else if (eeprom_write_result == EEPROM_WRITE_FAILED)
                                                MachineStatus.step += 2; // Err 			
                                        break;									
                                    }
                                break;
		
                                default:
                                    if (isGUIStatusCmd()) {
                                        MachineStatus.step = STEP_0;
                                        resetNewProcessingMsg();
                                    }
                                    else if (isNewProcessingMsg() && (procGUI.typeMessage == RESET_MACCHINA) ) {
                                        resetNewProcessingMsg();
                                        nextStatus = RESET_ST;
                                    }
                                break;
                            } // switch (typeMessage) 
                        break;

                        // EEPROM update completed, clear inhibit reset flag to allow a full software reset
                        case STEP_2: // ok 
                        case STEP_3: // err 
                            inhibitReset = FALSE;
                            if (isGUIStatusCmd()) {
                                MachineStatus.step = STEP_0;
                                resetNewProcessingMsg();
                            }
                            else if (isNewProcessingMsg() && (procGUI.typeMessage == RESET_MACCHINA) ) {
                                resetNewProcessingMsg();
                                nextStatus = RESET_ST;
                            }                            
                        break;
                    } // Switch (MachineStatus.step)
                break;

                case EXIT_PH:
                break;
                
                default:
                break;
            } // Switch (MachineStatus.phase) 
        break;
	
    /************************************************************************ */
    /*                         JUMP_TO_BOOT_ST                                */
    /************************************************************************ */                        
        case JUMP_TO_BOOT_ST:
            indicator = LIGHT_STEADY;
            switch(MachineStatus.phase) {
                case ENTRY_PH:
                    autoRecoveryFromAlarm = FALSE;
                    initColorDiagProcesses();
                    diagResetIdleCounter();
                    Can_Locator_Manager(OFF);
                break;

            	case RUN_PH:
                    switch (MachineStatus.step) {
                		case STEP_0:
                            // Stop all Actuators 
                            stopAllActuators();
                            StopCleaningManage = TRUE;
                            indx_Clean = MAX_COLORANT_NUMBER;                            
                            Check_Neb_Error = FALSE;
                            StopTimer(T_WAIT_BRUSH_PAUSE);
                            StopTimer(T_WAIT_AIR_PUMP_TIME); 
                            StopTimer(T_TEST_RELE);
                            StopTimer(T_WAIT_GENERIC24V_TIME);
                            StopTimer(T_WAIT_NEB_ERROR);
                			StartTimer(T_DELAY_WAIT_STOP);                            
                            MachineStatus.step++;			
                        break;

                        case STEP_1:	
                            if (isAllCircuitsStopped() ) {
                                MachineStatus.step ++; 
                            }                                
                            else if (StatusTimer(T_DELAY_WAIT_STOP) == T_ELAPSED) {
                                StopTimer(T_DELAY_WAIT_STOP);
                                Status.step += 7;					
                            }                            
                        break;
			
                        case STEP_2:	
                            // Check if a Good TINTING MASTER BootLoader is present
//                            if ( ((BOOT_FW_VERSION() & 0xFF0000) == BOOT_CODE) && (BOOT_FW_VERSION() > TINTING_MASTER_BAD_BOOT_CODE) ){
                            if ( ((BL_VERSION & 0xFF0000) == BOOT_CODE) && (BL_VERSION > TINTING_MASTER_BAD_BOOT_CODE) ){
                            // Fix into "slaves_boot_ver[] BootLoader Firmware Version
                                for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
                                    if (isColorantActEnabled(i) && !isColorTintingModule(i))
                                        slaves_boot_ver[i] = slaves_boot_versions[i];
                                }
                                if (isAutocapActEnabled())
                                    slaves_boot_ver[AUTOCAP_ID - 1] = slaves_boot_versions[AUTOCAP_ID - 1];
                                
                                MachineStatus.step ++;
                            }		
                            // NO Boot Loader present or Bad BootLoader present --> Process terminates
                            else
                                MachineStatus.step += 7;			
                        break;

                        case STEP_3:
                            // Send JUMP_TO_BOOT command to all Circuits Enabled and with BootLoader present
                            for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
                                // If an Actuator (Colorant or Base) is not enabled or hasn't got BooLoader --> skip it 
                                if ( (!isColorantActEnabled(i)) || isColorTintingModule(i) || ((slaves_boot_ver[i] & 0xFF0000) != BOOT_CODE) || 
                                     (((slaves_boot_ver[i] & 0xFF0000) == BOOT_CODE) && (slaves_boot_ver[i] <= ACTUATOR_BAD_BOOT_CODE)) )
                                    continue;
                                JumpToBoot_ColorAct(i);
                            }    
#ifndef AUTOCAP_MMT
                            // Send JUMP_TO_BOOT command to AUTOCAP if Enabled and with BootLoader present
                            if ( (isAutocapActEnabled()) && ((slaves_boot_ver[AUTOCAP_ID - 1] & 0xFF0000) == BOOT_CODE) && (slaves_boot_ver[i] > ACTUATOR_BAD_BOOT_CODE)) 
                                JumpToBoot_AutocapAct();
#else
#endif                                                           
                            StartTimer(T_DELAY_JUMP_TO_BOOT);
                            MachineStatus.step ++;			
                        break;
			
                        case STEP_4:	
                            // Wait till JUMP_TO_BOOT command was correctly sent to all Actuators Enabled with Bootloader present
                            if (StatusTimer(T_DELAY_JUMP_TO_BOOT) == T_ELAPSED) {
                                StopTimer(T_DELAY_JUMP_TO_BOOT);
                                MachineStatus.step += 6;					
                            }
                            jump_boot = TRUE;
                            // Verify if all Actuators Enabled and with Bootloader present have received command JUMP_TO_BOOT
                            for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
                                if ( isColorantActEnabled(i) && !isColorTintingModule(i) && ((slaves_boot_ver[i] & 0xFF0000) == BOOT_CODE) && (slaves_boot_ver[i] > ACTUATOR_BAD_BOOT_CODE) && (colorAct[i].colorFlags.jump_to_boot != TRUE) ) {
                                    jump_boot = FALSE;	
                                    continue;
                                }	
                            }
#ifndef AUTOCAP_MMT
                            if ( (isAutocapActEnabled()) && ((slaves_boot_ver[AUTOCAP_ID - 1] & 0xFF0000) == BOOT_CODE) && (slaves_boot_ver[AUTOCAP_ID - 1] > ACTUATOR_BAD_BOOT_CODE) && (autocapAct.autocapFlags.jump_to_boot != TRUE) )
                                jump_boot = FALSE;			
#else
#endif                                                           
                            // All Actuators Enabled and with Bootloader present have received command JUMP_TO_BOOT
                            if (jump_boot == TRUE) {
                                StopTimer(T_DELAY_JUMP_TO_BOOT);
                                StartTimer(T_DELAY_BOOT_START);
                                MachineStatus.step ++;
                            }		
                        break;

                        case STEP_5:	
                            // Waiting time before to JUMP to MAB Boot Loader
                            if (StatusTimer(T_DELAY_BOOT_START) == T_ELAPSED) {	
                                StopTimer(T_DELAY_BOOT_START);
                                MachineStatus.step ++;
                            }			
                        break;

                        case STEP_6:
                            // Set the variabile at shared with BootLoader RAM address who tells to BootLoader that a JUMP_TO_BOOT command was previously send
                            jump_to_boot_done = (unsigned short) JUMP_TO_BOOT_DONE;
                            // JUMP to Boot Loader
                            __asm__ volatile ("reset");                                  
                            MachineStatus.step ++;
                        break;

                        case STEP_7:
                            // If we arrive here, we have a big problem!
                            forceAlarm(FAILED_JUMP_TO_BOOT_TINTING_MASTER);				
                        break;

                        case STEP_8:
                            // Timeout Error on Stop Process
                            forceAlarm(STOP_PROCESS_TIMEOUT_ERROR);				
                        break;

                        case STEP_9:
                            // End Process with Errors
                            forceAlarm(BOOTLOADER_TINTING_MASTER_ABSENT);				
                        break;

                        case STEP_10:
                            // End Process with Errors
                            forceAlarm(FAILED_JUMP_TO_BOOT_ACTUATOR);				
                        break;

                        default:		
                            HALT(); // unexpected 
                        break;		
                    }
                break; 
                
                case EXIT_PH:
                break;
            } // Switch (MachineStatus.phase) 
        break;
    }
    // These flags are always updated 
    procGUI.Container_presence = PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER);
    // -----------------------------------------------------------
    // Tinting Panel Management only if Tinting is Enabled
    if (isTintingEnabled() ) {
        Old_Panel_table_status = New_Panel_table_status;  
        New_Panel_table_status = TintingAct.PanelTable_state;  

        if ( (New_Panel_table_status == PANEL_CLOSE) && (Panel_table_transition == LOW_HIGH) && (New_Reset_Done == TRUE) ) 
            Panel_table_transition = HIGH_LOW;
        else if ( (New_Panel_table_status == PANEL_OPEN) && (Old_Panel_table_status == PANEL_CLOSE) ) {
            New_Reset_Done = FALSE; 
            Panel_table_transition = LOW_HIGH; 
        }
    }
    else
        Panel_table_transition = HIGH_LOW;         
// -----------------------------------------------------------  
    // Bases Carriage Management
    TintingAct.BasesCarriage_state = PhotocellStatus(BASES_CARRIAGE , FILTER);
    Old_Bases_Carriage_status = New_Bases_Carriage_status;  
    New_Bases_Carriage_status = TintingAct.BasesCarriage_state;  

    if ( (New_Bases_Carriage_status == CARRIAGE_CLOSE) && (Old_Bases_Carriage_status == CARRIAGE_OPEN) )
        Bases_Carriage_transition = HIGH_LOW;

    else if ( (New_Bases_Carriage_status == CARRIAGE_OPEN) && (Old_Bases_Carriage_status == CARRIAGE_CLOSE) ) 
        Bases_Carriage_transition = LOW_HIGH; 

    if ( (Bases_Carriage_transition == LOW_HIGH) && (MachineStatus.level == ALARM_ST) )
        TintingAct.BasesCarriageOpen = CARRIAGE_OPEN;					
    else
        TintingAct.BasesCarriageOpen = CARRIAGE_CLOSE;					
  // -----------------------------------------------------------      
//}
} // Run() 

static void changeStatus()
/**/
/*==========================================================================*/
/**
**   @brief  Gestisce la transizione tra le fasi
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    if (MachineStatus.phase == ENTRY_PH)
        MachineStatus.phase = RUN_PH;

    else if (MachineStatus.phase == RUN_PH && MachineStatus.level != nextStatus)
        MachineStatus.phase = EXIT_PH;

    else if (MachineStatus.phase == EXIT_PH) {
        MachineStatus.level = nextStatus;
        MachineStatus.phase = ENTRY_PH;
        MachineStatus.step  = STEP_0;
    }
} // changeStatus()

static void stopAllActuators(void)
/**/
/*===========================================================================*/
/**
**   @brief  Stop all actuators
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  stopAllCircuitsAct();

  if (isAutocapActEnabled())
	stopAutocapAct();
}

static void visualIndicator()
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

void statusManager(void)
/**/
/*==========================================================================*/
/**
**   @brief  sequencer of the module
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
   run();
   changeStatus();
   visualIndicator();
}

static void diagResetIdleCounter(void)
{
  diag_idle_counter = 0;
}

static void diagEvalIdleCounter(void)
{
    if (StatusTimer(T_DIAGNOSTIC_TIMEBASE) == T_HALTED)
      StartTimer(T_DIAGNOSTIC_TIMEBASE);

    else if (StatusTimer(T_DIAGNOSTIC_TIMEBASE) == T_ELAPSED) {
        if (LEAVE_DIAGNOSTIC_COUNT <= ++ diag_idle_counter) {
          force_cold_reset = 1;
          StopCleaningManage = TRUE; 
          indx_Clean = MAX_COLORANT_NUMBER;          
          StopTimer(T_WAIT_BRUSH_PAUSE);			                                          
          nextStatus = RESET_ST;
        }
        else 
          StartTimer(T_DIAGNOSTIC_TIMEBASE);
    }
}

/**/
/*==========================================================================*/
/**
**   @brief Can Locator Manager: Temporized TURN ON / OFF Laser depending on can sensor STATE 
**
**   @param  unsigned short mode: ON --> RUN - OFF --> OFFLINE
**
**   @return void
**/
/*==========================================================================*/
/**/
void Can_Locator_Manager(unsigned short mode)
{
	static unsigned short new_photo_status = OFF, old_photo_status = OFF, can_locator = OFF;

	if (mode == ON) {
		old_photo_status = new_photo_status;
			if (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK)
				new_photo_status = ON;
			else
				new_photo_status = OFF;			
		// OFF --> ON Transition
		if ( (new_photo_status == ON) && (old_photo_status == OFF) ) {
//		if ( (new_photo_status == OFF) && (old_photo_status == ON) ) {
			can_locator = ON;
			StopTimer(T_DELAY_LASER);
			StartTimer(T_DELAY_LASER);
			// Turn ON LASER
            LASER_BHL = 1;	
		}
		// Manage TIMER
		if ( (can_locator == ON) && (StatusTimer(T_DELAY_LASER) == T_ELAPSED) ) {
			can_locator = OFF;
			StopTimer(T_DELAY_LASER);
			// Turn OFF LASER
			LASER_BHL = 0;
		}
// OFF --> ON Transition
// Life test always ON
/*
if ( (new_photo_status == ON) && (old_photo_status == OFF) ) {
	can_locator = ON;
	// Turn ON LASER
	LASER_BHL = 1;	
}
*/
	}
	else {
//		new_photo_status = OFF;
		can_locator = OFF;
		StopTimer(T_DELAY_LASER);
		// Turn OFF LASER
		LASER_BHL = 0;
	}	
}
