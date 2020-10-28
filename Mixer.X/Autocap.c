
/* 
 * File:   autocap.c
 * Author: michele.abelli
 * Description: Autocap Processes management
 * Created on 16 aprle 2020
 */

#include "p24FJ256GB110.h"
#include "statusManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "stepper.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "typedef.h"
#include "stepperParameters.h"
#include "stepper.h"
#include "spi.h"
#include "L6482H.h"
#include <xc.h>

#if defined AUTOCAP_ACTUATOR

static unsigned char isHomingPositionReached(void);
static unsigned char isPackingPositionReached(void);


void autocapStatusManager(void)
/*
*//*=====================================================================*//**
**      @brief Updates AUTOCAP status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
{
    static signed long Steps_Todo;
    unsigned char retvalue;
    
//----------------------------------------------------------------------------
    Status_Board_Autocap.word = GetStatus(MOTOR_AUTOCAP);

    // Check for Motor Autocap Error
    if (Status_Board_Autocap.Bit.OCD == 0) {
        Autocap.errorCode = TINTING_AUTOCAP_MOTOR_OVERCURRENT_ERROR_ST;
        Autocap.step = AUTOCAP_ERROR_ST;
    }  
    if(Status_Board_Autocap.Bit.UVLO == 0) { //|| (Status_Board_Autocap.Bit.UVLO_ADC == 0) ) {
        Autocap.errorCode = TINTING_AUTOCAP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
        Autocap.step = AUTOCAP_ERROR_ST;
    }
    else if ( (Status_Board_Autocap.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Autocap.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
        Autocap.errorCode = TINTING_AUTOCAP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
        Autocap.step = AUTOCAP_ERROR_ST;
    }  
  //----------------------------------------------------------------------------    
if (Autocap_Enabled) {
    
    switch(Autocap.step) {
        case AUTOCAP_INIT_ST:
            // Initialization
            // Set winding current as 100% of the full scale current set by VRERF Input pin and sense resistance
            I2_BRUSH_ON();
            I3_BRUSH_ON();
            I4_BRUSH_ON();  
            DECAY = 0;
            DRV8842_STOP_RESET();            
            Autocap_Enabled = FALSE;
            Autocap.step++;
        break;

        case AUTOCAP_READY_ST:
            // Actuator is ready for Initialization 
            // Packing  
            if (isColorCmdPacking()) {
                StartTimer(T_CAN_LIFTER_RESET);
                Autocap.step = AUTOCAP_SEARCH_PACKING_ST;
                Autocap.level = STEP_0;                
            }
            // Homing
            else if (isColorCmdHome() && (TintingAct.Homing_type > 0) ) {
                StartTimer(T_RESET);
                Autocap.step = AUTOCAP_SEARCH_HOMING_ST;
                Autocap.level = STEP_0;
            }
        break;
	
        case AUTOCAP_SEARCH_PACKING_ST:
            // Searching can lifter pack position 
            retvalue = isPackingPositionReached();
            if (retvalue == TRUE) {
                StopTimer(T_CAN_LIFTER_RESET);
                Autocap.step = AUTOCAP_PACKED_ST;
            }
            else if (StatusTimer(T_CAN_LIFTER_RESET) == T_ELAPSED) {
                Autocap.errorCode = TINTING_AUTOCAP_PACKING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;
            }
        break;

        case AUTOCAP_PACKED_ST:
            //  Wait for stop to return to READY 
            if (isColorCmdStop()) {
                Autocap_Enabled = FALSE;
                Autocap.step = AUTOCAP_READY_ST;
            }
            else if (isColorCmdHome() && (TintingAct.Homing_type > 0) ) {
                StartTimer(T_RESET);
                Autocap.step = AUTOCAP_SEARCH_HOMING_ST;
                Autocap.level = STEP_0;                
            } 
        break;

        case AUTOCAP_SEARCH_HOMING_ST:
            // Searching AUTOCAP home position 
            retvalue = isHomingPositionReached();
            if (retvalue == PROC_OK) {
                StopTimer(T_RESET);
                Autocap.step ++ ;
            }
            else if (retvalue == PROC_FAIL) {
                StopTimer(T_RESET);
                Autocap.errorCode = TINTING_AUTOCAP_HOMING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;                
            }
            else if (StatusTimer(T_RESET) == T_ELAPSED) {
                StopTimer(T_RESET);                
                Autocap.errorCode = TINTING_AUTOCAP_HOMING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;
            }
        break;

        case AUTOCAP_CLOSE_ST:
            // Initialization completed, ready for operation. AUTOCAP is closed 
            if (isColorCmdOpen()) {
                // Sanity check 
                if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL, FILTER) == DARK) {    
                    Autocap.errorCode = TINTING_AUTOCAP_HOMING_ERROR_ST;
                    Autocap.step = AUTOCAP_ERROR_ST;
                }
                else {
                    SetStepperHomePosition(MOTOR_AUTOCAP); 
                    StopTimer(T_WAIT_AUTOCAP_OPENING);
                    StartTimer(T_WAIT_AUTOCAP_OPENING);                    
                    // Move to reach Autocap Open Position
                    StartStepper(MOTOR_AUTOCAP, TintingAct.speed_move, CW, LIGHT_DARK, AUTOCAP_OPEN_PHOTOCELL, 0);
                    Autocap.step = AUTOCAP_OPEN_RUN_ST ;
                }
            }
            else if (isColorCmdHome() && (TintingAct.Homing_type > 0) ) {
                StartTimer(T_RESET);
                Autocap.step = AUTOCAP_SEARCH_HOMING_ST;
                Autocap.level = STEP_0;                
            }
            else if (isColorCmdPacking()) {
                StartTimer(T_CAN_LIFTER_RESET);
                Autocap.step = AUTOCAP_SEARCH_PACKING_CLOSED_ST;
                Autocap.level = STEP_0;                
            }
        break;

        case AUTOCAP_OPEN_RUN_ST:
            // Opening AUTOCAP
            if (Status_Board_Autocap.Bit.MOT_STATUS == 0){
                SetStepperHomePosition(MOTOR_AUTOCAP);
                StopTimer(T_MOTOR_WAITING_TIME);            
                Autocap.step++;
            }
            // Autocap Open Photocell never DARK
            else if ((signed long)GetStepperPosition(MOTOR_AUTOCAP) <=- (signed long)MAX_STEP_AUTOCAP_OPEN) {
                StopTimer(T_WAIT_AUTOCAP_OPENING);            
                Autocap.errorCode = TINTING_AUTOCAP_HOMING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;
            }
            else if (StatusTimer(T_WAIT_AUTOCAP_OPENING)==T_ELAPSED) {
                StopTimer(T_WAIT_AUTOCAP_OPENING);
                Autocap.errorCode = TINTING_AUTOCAP_HOMING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;
            }                                
        break;

        case AUTOCAP_OPEN_ST:
            CAN_LIFTER_IDLE(); // turn DC motor OFF 

            // AUTOCAP is open 
            if (isColorCmdClose()) {
                // Sanity check 
                if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL, FILTER) == LIGHT) {  
                    Autocap.errorCode = TINTING_AUTOCAP_HOMING_ERROR_ST;
                    Autocap.step = AUTOCAP_ERROR_ST;
                }
                else {
                    Steps_Todo = -(signed long)TintingAct.n_step_move;
                    MoveStepper(MOTOR_AUTOCAP, Steps_Todo, TintingAct.speed_move);                     
                    Autocap.step ++ ;
                }
            }
            else if (isColorCmdExtend()) {
                CAN_LIFTER_FWD(); // Turn DC motor ON 
                StartTimer(T_CAN_LIFTER_OPERATION);
                Autocap.step = AUTOCAP_EXTEND_RUN_ST;
            }
            else if (isColorCmdPacking()) {
                StartTimer(T_CAN_LIFTER_RESET);
                Autocap.step = AUTOCAP_SEARCH_PACKING_CLOSED_ST;
                Autocap.level = STEP_0;                
            }            
        break;

        case AUTOCAP_CLOSE_RUN_ST:
            // AUTOCAP is closing
            if (Status_Board_Autocap.Bit.MOT_STATUS == 0){
                // Must be LIGHT now 
                if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL, FILTER) == DARK) { 
                    Autocap.errorCode = TINTING_AUTOCAP_HOME_POS_ERROR_ST;
                    Autocap.step = AUTOCAP_ERROR_ST;
                }
                else
                    Autocap.step = AUTOCAP_CLOSE_ST;
            }
        break;

        case AUTOCAP_SEARCH_PACKING_CLOSED_ST:
            // Searching can lifter pack position (from CLOSE_ST) 
            retvalue = isPackingPositionReached();            
            if (retvalue == TRUE) {
                StopTimer(T_CAN_LIFTER_RESET);
                Autocap.step = AUTOCAP_PACKED_CLOSED_ST;
            }
            else if (StatusTimer(T_CAN_LIFTER_RESET) == T_ELAPSED) {
                Autocap.errorCode = TINTING_AUTOCAP_PACKING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;                
            }
        break;

        case AUTOCAP_PACKED_CLOSED_ST:
            //  Wait for stop to return to CLOSE_ST 
            if (isColorCmdStop())
                Autocap.step = AUTOCAP_CLOSE_ST;
        break;

        case AUTOCAP_EXTEND_RUN_ST:
            // Can lifter extending 
            if (StatusTimer(T_CAN_LIFTER_OPERATION) == T_ELAPSED) {
                if (PhotocellStatus(AUTOCAP_LIFTER_PHOTOCELL, FILTER) == LIGHT) {
                    // Can lifter extended 
                    CAN_LIFTER_IDLE(); // Turn DC motor OFF                  
                    Autocap.step ++ ;
                }
                else {
                    Autocap.errorCode = TINTING_AUTOCAP_LIFTER_ERROR_ST;
                    Autocap.step = AUTOCAP_ERROR_ST; 
                }
            }
        break;

        case AUTOCAP_EXTEND_ST:
            // Can lifter extended 
            CAN_LIFTER_IDLE(); // Turn DC motor OFF 

            if (isColorCmdRetract()) {
                CAN_LIFTER_BWD(); // Turn DC motor ON 
                StartTimer(T_CAN_LIFTER_OPERATION);
                Autocap.step ++ ;
            }
            else if (isColorCmdPacking()) {
                StartTimer(T_CAN_LIFTER_RESET);
                Autocap.step = AUTOCAP_SEARCH_PACKING_CLOSED_ST;
                Autocap.level = STEP_0;                
            }                        
        break;

        case AUTOCAP_RETRACT_RUN_ST:
            // Can lifter retracting 
            if (PhotocellStatus(AUTOCAP_LIFTER_PHOTOCELL, FILTER) == DARK)
                Autocap.step = AUTOCAP_OPEN_ST ;
            else if (StatusTimer(T_CAN_LIFTER_OPERATION) == T_ELAPSED) {
                Autocap.errorCode = TINTING_AUTOCAP_LIFTER_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;
            }    
        break;

        case AUTOCAP_ERROR_ST:
            // Error recovery, turn everything OFF. An INTR command is required to get out of these states. 
            HardHiZ_Stepper(MOTOR_AUTOCAP);
            CAN_LIFTER_IDLE();
            // AUTOCAP is open 
            if (isColorCmdClose()) {
                // Sanity check 
                if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL, FILTER) == DARK) {  
                    Steps_Todo = -(signed long)TintingAct.n_step_move;
                    MoveStepper(MOTOR_AUTOCAP, Steps_Todo, TintingAct.speed_move);                     
                    Autocap.step  = AUTOCAP_CLOSE_RUN_ST ;
                }
            } 
            else if (isColorCmdRetract()) {
                CAN_LIFTER_BWD(); // Turn DC motor ON 
                StartTimer(T_CAN_LIFTER_OPERATION);
                Autocap.step = AUTOCAP_RETRACT_RUN_ST;
            }
        break;
        
        default:
            
        break;
    } // switch() 
}     
#ifndef SKIP_FAULT_1
    if (isFault_1_Detection() && StatusTimer(T_DELAY_INIT_DONE) == T_ELAPSED)
        Autocap.errorCode = TINTING_AUTOCAP_DRV_OVER_CURR_TEMP_ERROR_ST;        
#endif    

    // INTR From any state but INIT -> READY 
/*
    if (Autocap.step != AUTOCAP_INIT_ST && Autocap.step != AUTOCAP_CLOSE_ST && Autocap.step != AUTOCAP_OPEN_ST && isColorCmdIntr()) {
        Autocap.step = AUTOCAP_READY_ST;
    } 
 */ 
    if ( (Autocap.step == AUTOCAP_ERROR_ST) && isColorCmdIntr() ) {
        Autocap.step = AUTOCAP_READY_ST;
    }    
}

static unsigned char isHomingPositionReached(void)
/**/
/*===========================================================================*/
/**
**   @brief Return TRUE if homing position found
**
**   @param void
**
**   @return TRUE/FALSE
**/
/*===========================================================================*/
/**/
{
    unsigned char ret = PROC_RUN;
    static signed long Steps_Todo;
//----------------------------------------------------------------------------
    Status_Board_Autocap.word = GetStatus(MOTOR_AUTOCAP);

    // Check for Motor Autocap Error
    if (Status_Board_Autocap.Bit.OCD == 0) {
        Autocap.errorCode = TINTING_AUTOCAP_MOTOR_OVERCURRENT_ERROR_ST;
        return PROC_FAIL;
    }  
    if(Status_Board_Autocap.Bit.UVLO == 0) { //|| (Status_Board_Autocap.Bit.UVLO_ADC == 0) ) {
        Autocap.errorCode = TINTING_AUTOCAP_MOTOR_UNDER_VOLTAGE_ERROR_ST;
        return PROC_FAIL;
    }
    else if ( (Status_Board_Autocap.Bit.TH_STATUS == UNDER_VOLTAGE_LOCK_OUT) || (Status_Board_Autocap.Bit.TH_STATUS == THERMAL_SHUTDOWN_DEVICE) ) {
        Autocap.errorCode = TINTING_AUTOCAP_MOTOR_THERMAL_SHUTDOWN_ERROR_ST;
        return PROC_FAIL;
   }  
  //----------------------------------------------------------------------------    
    switch(Autocap.level) {
        // Initialize
        case STEP_0:            
            StopStepper(MOTOR_AUTOCAP);
            SetStepperHomePosition(MOTOR_AUTOCAP); 
            Autocap.level ++;
        break;

        // Start: search backwards     
        case STEP_1:            
            StartStepper(MOTOR_AUTOCAP, (unsigned short)AUTOCAP_SPEED_SEARCH_PHOTOC, CW, LIGHT_DARK, AUTOCAP_OPEN_PHOTOCELL, 0);
            StartTimer(T_WAIT_AUTOCAP_OPENING);
            Autocap.level ++;
        break;

        // Moving backwards 
        case STEP_2:    
            if (Status_Board_Autocap.Bit.MOT_STATUS == 0){            
                StopTimer(T_WAIT_AUTOCAP_OPENING);
                StartTimer(T_WAIT_AUTOCAP_OPENING);
                SetStepperHomePosition(MOTOR_AUTOCAP);
                StartStepper(MOTOR_AUTOCAP, (unsigned short)AUTOCAP_SPEED_SEARCH_PHOTOC, CCW, DARK_LIGHT, AUTOCAP_OPEN_PHOTOCELL, 0);
                Autocap.level ++;                
            }
            // Autocap Open Photocell never DARK
            else if ((signed long)GetStepperPosition(MOTOR_AUTOCAP) <= -(signed long)MAX_STEP_AUTOCAP_OPEN) {
                HardHiZ_Stepper(MOTOR_AUTOCAP);
                StopTimer(T_WAIT_AUTOCAP_OPENING);            
                ret = PROC_FAIL;
            }
            else if (StatusTimer(T_WAIT_AUTOCAP_OPENING)==T_ELAPSED) {
                HardHiZ_Stepper(MOTOR_AUTOCAP);
                StopTimer(T_WAIT_AUTOCAP_OPENING);
                ret = PROC_FAIL;                           
            }                                
        break;

        // Moving forward 
        case STEP_3:  
            if (Status_Board_Autocap.Bit.MOT_STATUS == 0){            
                StopTimer(T_WAIT_AUTOCAP_OPENING);
                SetStepperHomePosition(MOTOR_AUTOCAP);                 
                Steps_Todo = -(signed long)TintingAct.n_step_homing;
                // Move to reach Autocap Home Position
                MoveStepper(MOTOR_AUTOCAP, Steps_Todo, (unsigned short)AUTOCAP_SPEED_HOMING);                            
                Autocap.level ++;                
            }
            // Autocap Open Photocell never DARK
            else if ((signed long)GetStepperPosition(MOTOR_AUTOCAP) >= (signed long)MAX_STEP_AUTOCAP_OPEN) {
                HardHiZ_Stepper(MOTOR_AUTOCAP);
                StopTimer(T_WAIT_AUTOCAP_OPENING);            
                ret = PROC_FAIL;                           
            }
            else if (StatusTimer(T_WAIT_AUTOCAP_OPENING)==T_ELAPSED) {
                HardHiZ_Stepper(MOTOR_AUTOCAP);
                StopTimer(T_WAIT_AUTOCAP_OPENING);
                ret = PROC_FAIL;                           
            }                                
        break;

        // Waiting for position 
        case STEP_4:  
            if (Status_Board_Autocap.Bit.MOT_STATUS == 0){            
                SetStepperHomePosition(MOTOR_AUTOCAP);
                if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL, FILTER) == DARK) {  
                    HardHiZ_Stepper(MOTOR_AUTOCAP); 
                    Autocap.errorCode = TINTING_AUTOCAP_HOMING_ERROR_ST;
                    ret = PROC_FAIL;                           
                }
                else
                    Autocap.level ++;
            }
        break;

        case STEP_5:
            ret = PROC_OK;
        break;
    } // switch() 

    return ret;
} // isHomingPositionReached() 

static unsigned char isPackingPositionReached(void)
/**/
/*===========================================================================*/
/**
**   @brief Return TRUE if packing position found
**
**   @param void
**
**   @return TRUE/FALSE
**/
/*===========================================================================*/
/**/
{
    unsigned char ret = FALSE;

    switch(Autocap.level)
    {
        case STEP_0:
            // Initialized 
            StopTimer(T_CAN_LIFTER_RESET);
            StartTimer(T_CAN_LIFTER_RESET);
            if (PhotocellStatus(PHOTO_AUTOCAP_LIFTER, FILTER) == DARK) { 
                // move forward until we see LIGHT 
                CAN_LIFTER_FWD();
            }
            Autocap.level ++;
        break;

        case STEP_1:
            // Waiting for LIGHT 
            if (StatusTimer(T_CAN_LIFTER_RESET) == T_ELAPSED) {
                StopTimer(T_CAN_LIFTER_RESET);
                Autocap.errorCode = TINTING_AUTOCAP_PACKING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;
            }
            else if (PhotocellStatus(PHOTO_AUTOCAP_LIFTER, FILTER) == LIGHT) { 
                StopTimer(T_CAN_LIFTER_RESET);
                StartTimer(T_CAN_LIFTER_RESET);
                // move backward until we see DARK 
                CAN_LIFTER_BWD();
                Autocap.level ++;
            }
        break;

        case STEP_2:
            if (StatusTimer(T_CAN_LIFTER_RESET) == T_ELAPSED) {
                StopTimer(T_CAN_LIFTER_RESET);
                Autocap.errorCode = TINTING_AUTOCAP_PACKING_ERROR_ST;
                Autocap.step = AUTOCAP_ERROR_ST;
            }
            // Returning to DARK 
            else if (PhotocellStatus(PHOTO_AUTOCAP_LIFTER, FILTER) == DARK) { 
                StopTimer(T_CAN_LIFTER_RESET);
                // stop lifter 
                CAN_LIFTER_IDLE();
                // got to DARK 
                ret = TRUE;
            }
        break;
    } 
    return ret;
} // isPackingPositionReached()

#endif