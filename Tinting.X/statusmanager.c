
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
    static unsigned long slaves_boot_ver[N_SLAVES];
    unsigned char jump_boot;
    static unsigned char bases_open;
    static unsigned short Autotest_indx, Autotest_dosing_amount, Autotest_Color_done, Stop_Autotest; 
    static unsigned short TintingCmdSent = 0;
    static unsigned char New_Tinting_Cmd;
    rc = 0; // suppress warning
    
    switch(MachineStatus.level) {
    /************************************************************************ */
    /*                              INIT_ST                                   */
    /************************************************************************ */
        case INIT_ST:            
            indicator = LIGHT_OFF;
            for (i = 0; i < N_SLAVES; i++) 
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
                        Timer_Out_Supply_Low = 60000;
                        Diag_Setup_Timer_Received = 1;
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
                break; // ENTRY_PH 

                case RUN_PH:
                    // Wait for initial delay, before even considering anhything else 
                    if (StatusTimer(T_WAIT_START_RESET) != T_ELAPSED)
                        break;

                    if (isDeviceGlobalError()) {
                        // Perform *at most* one software reset (this check is necessary in order to avoid reset loop) 
                        if (! inhibitReset)
                            //Reset();
                            __asm__ volatile("GOTO 0x00");

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
            	  
                    if (force_cold_reset)
                        procGUI.reset_mode = 0;
		
                    switch (MachineStatus.step) {
                        case STEP_0:
                            Stop_Process = FALSE;	  
                            // Wait for all actuators to stop 
                            if (isAllCircuitsStopped())
                                MachineStatus.step ++;	
                        break;

                        case STEP_1:
                            //  Find Double Groups 0 and 1 if present (THOR machine, maximum 2 doubles groups)
                            countDoubleGroupAct();
                            // HUMIDIFIER
                            if (TintingAct.Humdifier_Type > 1) {
                                if (TintingAct.Humdifier_Type > 100)
                                    TintingAct.Humdifier_Type = 100;
                                TintingAct.Humidifier_PWM = (unsigned char)(TintingAct.Humdifier_Type/2);
                                TintingAct.Humdifier_Type = HUMIDIFIER_TYPE_2;
                            }
                            //--------------------------------------------------
                            //PUMP
                            // Passi da fotocellula madrevite coperta a fotocellula ingranamento coperta
                            TintingAct.Step_Accopp = TintingAct.Step_Accopp * (unsigned long)CORRECTION_PUMP_STEP_RES;
                            // Passi a fotoellula ingranamento coperta per ingaggio circuito
                            TintingAct.Step_Ingr = TintingAct.Step_Ingr * (unsigned long)CORRECTION_PUMP_STEP_RES;
                            // Passi per recupero giochi
                            TintingAct.Step_Recup = TintingAct.Step_Recup * (unsigned long)CORRECTION_PUMP_STEP_RES;
                            // Passi a fotocellula madrevite coperta per posizione di home
                            TintingAct.Passi_Madrevite = TintingAct.Passi_Madrevite * (unsigned long)CORRECTION_PUMP_STEP_RES;
                            // Passi per raggiungere la posizione di start ergoazione in alta risoluzione
                            TintingAct.Passi_Appoggio_Soffietto = TintingAct.Passi_Appoggio_Soffietto * (unsigned long)CORRECTION_PUMP_STEP_RES;
                            // Passi da posizione di home/ricircolo (valvola chiusa) a posizone di backstep (0.8mm))
                            TintingAct.Step_Valve_Backstep = TintingAct.Step_Valve_Backstep * (unsigned long)CORRECTION_PUMP_STEP_RES + (unsigned long)STEP_VALVE_OFFSET;
                            // N. steps in una corsa intera
                            TintingAct.N_steps_stroke = TintingAct.N_steps_stroke * (unsigned long)CORRECTION_PUMP_STEP_RES; 
                            //--------------------------------------------------
                            // TABLE
                            // Passi corrispondenti ad un giro completa di 360° della tavola
                            TintingAct.Steps_Revolution = TintingAct.Steps_Revolution * (unsigned long)CORRECTION_TABLE_STEP_RES;
                            // Tolleranza in passi corrispondente ad una rotazione completa di 360° della tavola
                            TintingAct.Steps_Tolerance_Revolution = TintingAct.Steps_Tolerance_Revolution * (unsigned long)CORRECTION_TABLE_STEP_RES;
                            // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
                            TintingAct.Steps_Reference = TintingAct.Steps_Reference * (unsigned long)CORRECTION_TABLE_STEP_RES;
                            // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
                            TintingAct.Steps_Tolerance_Reference = TintingAct.Steps_Tolerance_Reference * (unsigned long)CORRECTION_TABLE_STEP_RES;
                            // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito 
                            TintingAct.Steps_Circuit = TintingAct.Steps_Circuit * (unsigned long)CORRECTION_TABLE_STEP_RES;
                            // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito
                            TintingAct.Steps_Tolerance_Circuit = TintingAct.Steps_Tolerance_Circuit * (unsigned long)CORRECTION_TABLE_STEP_RES;
                            // Maschera abilitazione cloranti Tavola
                            TintingTable.Colorant_1 = procGUI.slaves_en[1];
                            TintingTable.Colorant_2 = procGUI.slaves_en[2];
                            TintingTable.Colorant_3 = procGUI.slaves_en[3];
                            //--------------------------------------------------
                            MachineStatus.step ++;                            
                        break;

                        //
                        // BASE PUMPS (B1 - B8)
                        // ***********************
                        case STEP_2:
                            // INTR + HOMING B1 - B8 
                            if (isCircuitsReady(B1_BASE_IDX, B8_BASE_IDX)) {
                              resetCircuitAct(B1_BASE_IDX, B8_BASE_IDX);
                              MachineStatus.step ++;
                            }
                            else {
                              intrCircuitAct(B1_BASE_IDX, B8_BASE_IDX);
                            }
                        break;

                        case STEP_3:
                            // Wait for acts B1 - B8 to start homing 
                            if (isCircuitsStartedHoming(B1_BASE_IDX, B8_BASE_IDX)) {
                                MachineStatus.step ++;
                            }
                        break;

                        case STEP_4:
                            // Wait for acts B1 - B8 to complete homing
                            if (isCircuitsHoming(B1_BASE_IDX, B8_BASE_IDX)) {
                                recircResetCircuitAct(B1_BASE_IDX, B8_BASE_IDX);
                                MachineStatus.step ++;
                            }
                        break;

                        case STEP_5:
                            // Wait for acts B1 - B8 to complete first recirculation 
                            if (isCircuitsRecircEnd(B1_BASE_IDX, B8_BASE_IDX)) {
                                stopColorActs(B1_BASE_IDX, B8_BASE_IDX);
                                MachineStatus.step ++;
                            }
                        break;

                        //
                        // COLORANT PUMPS (C1 - C8)
                        // ***********************
                        case STEP_6:

                            if(isTintingReady() || isTintingActError() ) {
                                posHomingTintingAct();				
                                MachineStatus.step ++;
                            }
                            else
                                intrTintingAct();							
                        break;

                        case STEP_7:
                            if (!isTintingStopped() ) {
                                checkHomingTintingAct();    
                                MachineStatus.step ++;
                            }		
                        break;

                        case STEP_8:
                            if (isTintingHoming() ) {
                                intrTintingAct();	
                                MachineStatus.step++;
                            }       
                        break;

                        case STEP_9:
                            if (isTintingReady() )
                                idleTintingAct();
                            else
                                break;		

                        case STEP_10:
                            New_Reset_Done = TRUE;
                            // Fast forward 
                            MachineStatus.step ++;
                        break;

                        //
                        // AUTOCAP
                        // ***********************
                        case STEP_11:
                            // Autocap HOMING if act is enabled and not closed or we're doing a COLD RESET
                            if (isAutocapActEnabled() && (! isAutocapActClose() || ! procGUI.reset_mode)) {
                                intrAutocapAct();
                                MachineStatus.step ++ ;
                            }
                            else {
                                MachineStatus.step += 3; // skip 
                            }
                        break;

                        case STEP_12:
                            // Initialize autocap HOMING 
                            if (isAutocapActReady()) {
                                initAutocapActHoming();
                                MachineStatus.step ++ ;
                            }
                        break;

                        case STEP_13:
                            // Wait for homing to complete 
                            if (isAutocapActHomingCompleted() || isAutocapActError()) {
                                MachineStatus.step ++;
                                statoAutoCap = AUTOCAP_CLOSED;
                            }
                        break;

                        case STEP_14: 
                            // RESET cycle completed 
                            nextStatus = COLOR_RECIRC_ST;
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
                    if ( (TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE) && (isTinting_RH_error()) ) {
                        forceAlarm(RH_ERROR);
                         break;
                    }
                    if ( (TintingHumidifier.Temp_Enable == TEMP_ENABLE) && (isTinting_Temperature_error()) ) {
                        TintingAct.CriticalTemperature_state = OFF;  
                        forceAlarm(TEMPERATURE_ERROR);
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
                    // Manage periodic processes 
                    standbyProcesses();
                    // Can Locator Manager Active
                    Can_Locator_Manager(ON);
		
                    if (isNewProcessingMsg()) {		  
                        switch(procGUI.typeMessage) {
                            case RESET_MACCHINA:
                                Can_Locator_Manager(OFF);
                                nextStatus = RESET_ST;
                            break;
                            case DISPENSAZIONE_COLORE_MACCHINA:
                                // Temperature TOO LOW --> Can't Erogate
                                if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && ((TintingAct.Dosing_Temperature/10) <= TintingHumidifier.Temp_T_LOW) ){
                                    setAlarm(TEMPERATURE_TOO_LOW);
                                    break;
                                }
                                Can_Locator_Manager(OFF);
                                nextStatus = COLOR_SUPPLY_ST;
                            break;
                            case DIAGNOSTICA_MACCHINA:
                                Can_Locator_Manager(OFF);
                                nextStatus = DIAGNOSTIC_ST;
                                turnToStandBy = 0;
                            break;
                            case DIAG_ROTATING_TABLE_POSITIONING: 
                                Can_Locator_Manager(OFF);
                                nextStatus = ROTATING_ST;
                                turnToState = DIAGNOSTIC_ST;				
                            break;
                            case DIAG_AUTOTEST_SETTINGS: 
                                Can_Locator_Manager(OFF);
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
                            nextStatus = DIAGNOSTIC_ST;
                            Can_Locator_Manager(OFF);		  			
                		}
              		    else if ( (procGUI.typeMessage == DIAG_ATTIVA_RICIRCOLO_CIRCUITI) || (procGUI.typeMessage == DIAG_ATTIVA_AGITAZIONE_CIRCUITI) )	 {
                            if (New_Tinting_Cmd == FALSE) {
                                stopAllActuators();
                                New_Tinting_Cmd = TRUE;
                            }					
                            else if ( (New_Tinting_Cmd == TRUE) && isAllCircuitsHome() ) {
                                New_Tinting_Cmd = FALSE;
                                nextStatus = DIAGNOSTIC_ST;
                                Can_Locator_Manager(OFF);		  
                            }
                   	    }	
                        else if (((procGUI.typeMessage>=PAR_CURVA_CALIBRAZIONE_MACCHINA) && (procGUI.typeMessage<=DIAG_JUMP_TO_BOOT)) && 
                                  (procGUI.typeMessage!=CAN_LIFTER_MOVEMENT) ) {
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
                            if (isTintingReady() ) {	
                                if ( ( (TintingAct.Circuit_Engaged != 0) && (TintingAct.Refilling_Angle == 0) ) || (TintingAct.Refilling_Angle != 0) )						
                                    MachineStatus.step++;							
                            }
                            else if (StatusTimer(T_WAIT_TABLE_POSITIONING) == T_ELAPSED) 
                            {
                                StopTimer(T_WAIT_TABLE_POSITIONING);
                                setAlarm(TINTING_TIMEOUT_TABLE_MOVE_ERROR);
                            }
                        break;

                        case STEP_4:
                            if (isGUIStatusCmd()) 
                            {
                                StopTimer(T_WAIT_TABLE_POSITIONING);
                                resetNewProcessingMsg();
                                MachineStatus.step = STEP_0;	
                            }
                            else if (isGUIAbortCmd())
                                setAlarm(USER_INTERRUPT);							
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
                    else if (New_Panel_table_status == PANEL_OPEN) {
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
                                else
                                    MachineStatus.step+= 3;
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

                        // End Autotest Cycle
                        case STEP_22:
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
                                stopAllCircuitsAct();
                                MachineStatus.step+=2;
                            }	
                        break;

                        case STEP_23:
                            // Pause Time elapsed: Start e NEW cycle
                            if (StatusTimer(T_AUTOTEST_PAUSE) == T_ELAPSED) {						
                                StopTimer(T_AUTOTEST_PAUSE);
                                MachineStatus.step -=20;
                            }						
                        break;

                        // End Autotest
                        case STEP_24:
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
                    // calculations
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
                      Durata[T_OUT_SUPPLY] = 60000;	
                    else  {
                      if (Diag_Setup_Timer_Received == 1)
                          Durata[T_OUT_SUPPLY] = Timer_Out_Supply_Duration;				
                    }
                    StopTimer(T_OUT_SUPPLY);
                    StartTimer(T_OUT_SUPPLY);
                    Count_Timer_Out_Supply = 0;
                    // Reset recirculation and stirring FSMs for all used colors
                    resetStandbyProcesses();
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
                            // Wait till all Active Processes are terminated
                            if (!isAllCircuitsSupplyHome())
                                break;  

                            //--------------------------------------------------------------		
                            // 1.
                            // Basi presenti
                            // Autocap Disabilitato
                            // Coloranti Presenti 
                            // Start Ricirculation on BASES AND Start Ricirculation on COLORANTS 
                            if (isFormulaColorants() && isFormulaBases() && !isAutocapActEnabled() ) {
                                if (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) {													
                                    checkBasesDispensationAct(FALSE);
                                    checkColorantDispensationAct(FALSE);
                                }	
                                MachineStatus.step += 4;					
                            }	
                            //--------------------------------------------------------------
                            // 2.
                            // Basi presenti
                            // Autocap Abilitato
                            // Coloranti Presenti 
                            // NO Start Ricirculation on BASES AND Start Ricirculation on COLORANTS, OPEN AUTOCAP
                            else if (isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {				
                                if (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) {							
                                    checkColorantDispensationAct(FALSE);
                                    openAutocapAct();
                                }
                                MachineStatus.step += 4;					
                            }	
                            //--------------------------------------------------------------			
                            // 3.
                            // Basi assenti
                            // Coloranti Presenti 				
                            // NO Start Ricirculation on BASES AND Start Ricirculation on COLORANTS 
                            else if (isFormulaColorants() && !isFormulaBases() ) {							
                                if (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) 		
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
                                if (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) {							
                                    checkBasesDispensationAct(FALSE);
                                }
                                MachineStatus.step += 4;					
                            }	
                            //--------------------------------------------------------------				
                            // 5.
                            // Basi presenti
                            // Autocap Abilitato
                            // Coloranti Assenti				
                            // NO Start Ricirculation on BASES AND NO Start Ricirculation on COLORANTS, OPEN AUTOCAP
                            else if (!isFormulaColorants() && isFormulaBases() && isAutocapActEnabled() ) {			
                                if ( ( (procGUI.check_can_dispensing == FALSE) && (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK) ) || (procGUI.check_can_dispensing == TRUE) )					
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
                break;

                case RUN_PH:
                    Can_Locator_Manager(OFF);
                    switch (MachineStatus.step) {
                        case STEP_0:
                            if (alarm() == FAILED_JUMP_TO_BOOT_MAB) {
                                StartTimer(T_DELAY_WAIT_STOP);
                            }
                            // Tinting Panel Open!	  
                            if ( (alarm() != TINTING_PANEL_TABLE_ERROR) && (Panel_table_transition == LOW_HIGH) ) {
                                stopAllActuators();
                                MachineStatus.step += 13;			  
                            }
                            // Bases Carriage Open
                            else if ( (alarm() != TINTING_BASES_CARRIAGE_ERROR) && (Bases_Carriage_transition == LOW_HIGH) ) {
                                stopAllActuators();
                                MachineStatus.step += 13;			  
                            }
                            // Tinting Panel Open or Bases Carriage Open	  		
                            else if ( (alarm() == TINTING_PANEL_TABLE_ERROR) || (alarm() == TINTING_BASES_CARRIAGE_ERROR) )
                                MachineStatus.step += 13;	
                            // Initiate normal alarm recovery cycle 
                            else if (isAllCircuitsStopped())
                                MachineStatus.step ++ ;	  				
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
                            if ( (Panel_table_transition != LOW_HIGH) && (Bases_Carriage_transition != LOW_HIGH) ) {
                                bases_open = 1;
                                standbyProcesses();
                            }
                            else {
                                if (bases_open == 1) {	
                                    bases_open = 0;
                                    stopAllActuators();
                                }
                            }
                            intrTintingAct();
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
                            __asm__ volatile ("goto " "0x0400");
                        }	
                    }
                    // Overall
                    else if (isNewProcessingMsg()) {
                        switch(procGUI.typeMessage)
                        {
                            case RESET_MACCHINA:
                                nextStatus = RESET_ST;
                            break;

                            case DIAGNOSTICA_MACCHINA:
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
            indicator = LIGHT_STEADY;
            switch (MachineStatus.phase) {
                case ENTRY_PH:
                    // Disable auto COLD RESET upon ALARMs 
                    autoRecoveryFromAlarm = FALSE;
                    diagResetIdleCounter();
                    initColorDiagProcesses();
                    stopAllActuators();
                    TintingCmdSent = 0;
                break;

                case RUN_PH:                    
                    diagEvalIdleCounter();  
                    switch(MachineStatus.step) {
                        case STEP_0:                            
                            if (isNewProcessingMsg()) {
                                if (procGUI.typeMessage == RESET_MACCHINA)
                                    nextStatus = RESET_ST;
                                else if (procGUI.typeMessage == DIAG_ROTATING_TABLE_POSITIONING) {
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
                                         (procGUI.typeMessage==DIAG_COLORANT_CLEANING_SETTINGS) ) {
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
                                            // Temperature TOO LOW --> Can't Erogate
                                            if ( (TintingAct.Dosing_Temperature != DOSING_TEMP_PROCESS_DISABLED) && ((TintingAct.Dosing_Temperature/10) <= TintingHumidifier.Temp_T_LOW) ){
                                                setAlarm(TEMPERATURE_TOO_LOW);
                                                MachineStatus.step += 3;			  
                                            }	 
                                            else {
                                				diagResetIdleCounter();
                                                calcSupplyPar(colorAct[procGUI.id_color_circuit].vol_t, color_supply_par[procGUI.id_color_circuit].vol_mu,
                                                             color_supply_par[procGUI.id_color_circuit].vol_mc,procGUI.id_color_circuit);
                                				// Find a Single Group
                                                if ( (procGUI.circuit_pump_types[procGUI.id_color_circuit] != PUMP_DOUBLE) && 
                                                     (isColorReadyTintingModule(procGUI.id_color_circuit) || ((procGUI.id_color_circuit < 8) && isColorActHoming(procGUI.id_color_circuit)))) {
                                                    if ( (procGUI.circuit_pump_types[procGUI.id_color_circuit] != PUMP_DOUBLE) && (isColorActHoming(procGUI.id_color_circuit)) ) { 
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
                                            if (isTintingReady() || isTintingSetPeripheralAct() ) {							
                                                diagResetIdleCounter();			
                                                TintingSetPeripheralAct();
                                                MachineStatus.step ++;
                                            }
                                            else
                                                MachineStatus.step ++;
                                        break;
                                        
                                        case DIAG_ROTATING_TABLE_STEPS_POSITIONING:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            // Send Rotating Table Steps Positioning
                                            else if (isTintingReady() && !isTintingActError() ){							
                                                diagResetIdleCounter();			
                                                TintingPosizionamentoPassiTavola();
                                                MachineStatus.step ++;                                                    
                                            }
                                            else
                                                MachineStatus.step ++;
                                        break;
				
                                        case DIAG_ROTATING_TABLE_SEARCH_POSITION_REFERENCE:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            // Send Rotating Table Search Position Reference 
                                            if (isTintingReady() && !isTintingActError() ) {							
                                                diagResetIdleCounter();			
                                                TintingRicercaRiferimentoTavola();
                                                MachineStatus.step ++;                                                    
                                            }
                                            else
                                                MachineStatus.step ++;
                                        break;

                                        case DIAG_ROTATING_TABLE_TEST:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                TintingCmdSent = 0;
                                                StopTimer(T_SEND_PARAMETERS);
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            // Send Rotating Table Test
                                            if (isTintingReady() && !isTintingActError() ){							
                                                diagResetIdleCounter();			
                                                TintingTestFunzionamnetoTavola();
                                            }
                                            else 
                                                MachineStatus.step ++;
                                        break;
				
                                        case ROTATING_TABLE_FIND_CIRCUITS_POSITION:
                                            // Tinting Panel Open!	  
                                            if (Panel_table_transition == LOW_HIGH) {
                                                forceAlarm(TINTING_PANEL_TABLE_ERROR);
                                                MachineStatus.step ++;			  
                                            }
                                            // Send Rotating Table Find Circuit Position
                                            if (isTintingReady() || isTintingActError() ) {							
                                                diagResetIdleCounter();			
                                                TintingAutoapprendimentoTavola();
                                            }
                                            else 
                                                MachineStatus.step ++;
                                        break;
				
                                        case DIAG_COLORANT_ACTIVATION_CLEANING:
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
                                if ( (procGUI.typeMessage != DIAG_ATTIVA_DISPENSAZIONE_CIRCUITI) && (TintingCmdSent == 0) )
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
                                        if (eeprom_read_result == EEPROM_READ_DONE)
                                            MachineStatus.step ++; 
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
                                        if (eeprom_read_result == EEPROM_READ_DONE)
                                            MachineStatus.step ++; 
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
		  
                                case DIAG_COLORANT_CLEANING_SETTINGS:
                                    MachineStatus.step ++; 
                                break;
			
                                case DIAG_SETUP_HUMIDIFIER_TEMPERATURE_PROCESSES:
                                    eeprom_write_result = updateEETintHumidifier();
                                    if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                        updateEETintHumidifier_CRC();
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
                                            fasiCancellazione++;
                                        break;
                                        
                                        case 9:
                                            // We delete the Pump Type
                                            eeprom_write_result = updateEECircuitPumpTypes();
                                            if (eeprom_write_result == EEPROM_WRITE_DONE) {
                                                fasiCancellazione = 0; 
                                                eeprom_byte = 0;
                                                MachineStatus.step ++; // Err 			
                                                //Reset();
                                                __asm__ volatile("GOTO 0x00");
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
                            // Send STOP command to all Actuators Enabled
                            for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
                                if (isColorantActEnabled(i) && !isColorTintingModule(i))
                                    stopColorAct(i);
                            }
                            if (isAutocapActEnabled())
                                stopAutocapAct();
                            TintingStop();	
                            StartTimer(T_DELAY_WAIT_STOP);
                            MachineStatus.step++;			
                        break;

                        case STEP_1:	
                            // Wait till STOP command was correctly executed to all Actuators Enabled 
                            if (StatusTimer(T_DELAY_WAIT_STOP) == T_ELAPSED) {
                                StopTimer(T_DELAY_WAIT_STOP);
                                MachineStatus.step += 7;					
                            }
                            jump_boot = TRUE;
                            // Check if All Actuators Enabled are STOPPED
                            for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
                                if ( (isColorantActEnabled(i)) && !isColorActStopped(i) && !isColorTintingModule(i) ) {
                                    jump_boot = FALSE;	
                                    continue;
                                }	
                            }
                            // Check if All Actuators Enabled are STOPPED
                            for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
                                if ( (isColorantActEnabled(i)) && !isColorActStopped(i) ) {
                                    jump_boot = FALSE;	
                                    continue;
                                }	
                            }
                //			if ( (isAutocapActEnabled()) && ! )
                //				jump_boot = FALSE;	
                            if (!isTintingStopped() )
                                jump_boot = FALSE;			
                            // All Actuators Enabled are correctly STOPPED
                            if (jump_boot == TRUE) {
                                StartTimer(T_DELAY_WAIT_STOP);
                                MachineStatus.step ++;
                            }			
                        break;
			
                        case STEP_2:	
                            // Check if a Good MAB BootLoader is present
                            if ( ((BOOT_FW_VERSION() & 0xFF0000) == BOOT_CODE) && (BOOT_FW_VERSION() > MAB_BAD_BOOT_CODE) ){
                                // Fix into "slaves_boot_ver[] BootLoader Firmware Version
                                for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
                                    if (isColorantActEnabled(i) && !isColorTintingModule(i))
                                        slaves_boot_ver[i] = slaves_boot_versions[i];
                                }
                                if (isAutocapActEnabled())
                                    slaves_boot_ver[AUTOCAP_ID - 1] = slaves_boot_versions[AUTOCAP_ID - 1];
                                slaves_boot_ver[TINTING - 1] = slaves_boot_versions[TINTING - 1];
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
                                if ( (!isColorantActEnabled(i)) || !isColorTintingModule(i) || ((slaves_boot_ver[i] & 0xFF0000) != BOOT_CODE) || 
                                     (((slaves_boot_ver[i] & 0xFF0000) == BOOT_CODE) && (slaves_boot_ver[i] <= ACTUATOR_BAD_BOOT_CODE)) )
                                    continue;
                                JumpToBoot_ColorAct(i);
                            }    
                            // Send JUMP_TO_BOOT command to AUTOCAP if Enabled and with BootLoader present
                            if ( (isAutocapActEnabled()) && ((slaves_boot_ver[AUTOCAP_ID - 1] & 0xFF0000) == ACTUATOR_BAD_BOOT_CODE) )
                                JumpToBoot_AutocapAct();			
                            // Send JUMP_TO_BOOT command to TINTING if Enabled and with BootLoader present
                            if ( ((slaves_boot_ver[TINTING - 1] & 0xFF0000) == TINTING_BAD_BOOT_CODE) )
                                TintingJumpToBoot();			
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
                            if ( (isAutocapActEnabled()) && ((slaves_boot_ver[AUTOCAP_ID - 1] & 0xFF0000) == BOOT_CODE) && (slaves_boot_ver[AUTOCAP_ID - 1] > ACTUATOR_BAD_BOOT_CODE) && (autocapAct.autocapFlags.jump_to_boot != TRUE) )
                                jump_boot = FALSE;			
                            if ( ((slaves_boot_ver[TINTING - 1] & 0xFF0000) == BOOT_CODE) && (slaves_boot_ver[TINTING - 1] > TINTING_BAD_BOOT_CODE) && (TintingAct.TintingFlags.tinting_jump_to_boot != TRUE) )
                                jump_boot = FALSE;			
                            // All Actuators Enabled and with Bootloader present have received command JUMP_TO_BOOT
                            if (jump_boot == TRUE) {
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
                            //__asm__ volatile ("goto " __BOOT_GOTO_ADDR);
                            __asm__ volatile ("goto " "0x0400");			
                            MachineStatus.step ++;
                        break;

                        case STEP_7:
                            // If we arrive here, we have a big problem!
                            setAlarm(FAILED_JUMP_TO_BOOT_MAB);				
                        break;

                        case STEP_8:
                            // Timeout Error on Stop Process
                            setAlarm(STOP_PROCESS_TIMEOUT_ERROR);				
                        break;

                        case STEP_9:
                            // End Process with Errors
                            setAlarm(BOOTLOADER_MAB_ABSENT);				
                        break;

                        case STEP_10:
                            // End Process with Errors
                            setAlarm(FAILED_JUMP_TO_BOOT_ACTUATOR);				
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
    procGUI.Container_presence = (PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER) == DARK);
    // -----------------------------------------------------------
    // Tinting Panel Management
    Old_Panel_table_status = New_Panel_table_status;  
    New_Panel_table_status = TintingAct.PanelTable_state;  

    if ( (New_Panel_table_status == PANEL_CLOSE) && (Panel_table_transition == LOW_HIGH) && (New_Reset_Done == TRUE) ) 
        Panel_table_transition = HIGH_LOW;
    else if ( (New_Panel_table_status == PANEL_OPEN) && (Old_Panel_table_status == PANEL_CLOSE) ) {
        New_Reset_Done = FALSE; 
        Panel_table_transition = LOW_HIGH; 
    }	
    // Bases Carriage Management
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
			PORTEbits.RE6 = 1;	
		}
		// Manage TIMER
		if ( (can_locator == ON) && (StatusTimer(T_DELAY_LASER) == T_ELAPSED) ) {
			can_locator = OFF;
			StopTimer(T_DELAY_LASER);
			// Turn OFF LASER
			PORTEbits.RE6 = 0;
		}
// OFF --> ON Transition
// Life test always ON
/*
if ( (new_photo_status == ON) && (old_photo_status == OFF) ) {
	can_locator = ON;
	// Turn ON LASER
	PORTEbits.RE6 = 1;	
}
*/
	}
	else {
//		new_photo_status = OFF;
		can_locator = OFF;
		StopTimer(T_DELAY_LASER);
		// Turn OFF LASER
		PORTEbits.RE6 = 0;
	}	
}
