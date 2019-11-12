/* 
 * File:   autocapAct.c
 * Author: michele.abelli
 * Description: Autocap management
 * Created on 20 Maggio 2019, 14.16
 */

#include "serialCom.h"
#include "serialCom_GUI.h"
#include "tintingmanager.h"
#include "autocapAct.h"
#include "eepromManager.h"
#include "p24FJ256GB110.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ram.h"
#include "define.h"
#include "gestIO.h"
#include "timerMg.h"
#include "mem.h"
#include "typedef.h"
#include "ErrorManager.h"
#include "stepper.h"

static int autocapHomingStep;

void setAutocapActMessage(unsigned char packet_type)
/**/
/*==========================================================================*/
/**
**   @brief Set the type of serial message to be send to  uC autocap
**
**   @param packet_type type of packet
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  autocapAct.typeMessage = packet_type;
}

void initAutocapActHoming(void)
{
  autocapHomingStep = 0;
}

int isAutocapActHomingCompleted(void)
{
    int res = FALSE;
    switch (autocapHomingStep)
    {
        case 0:
            // HOMING can either start from READY_ST or CLOSE_ST 
            if (isAutocapActReady() || (isAutocapActClose() && ! isAutocapActRunning())) {
                StartTimer(T_AUTOCAP_HOMING);
                posHomingAutocapAct();
                autocapHomingStep ++ ;
            }
        break;

        case 1:
            if (isAutocapActHoming() && ! isAutocapActRunning()) {
                stopAutocapAct();
                autocapHomingStep ++ ;
            }
        break;
        
        case 2:
            // Terminates in CLOSE_ST
            if (isAutocapActClose() && ! isAutocapActRunning()) {
                StopTimer(T_AUTOCAP_HOMING);
                res = TRUE;
            }
        break;

  } // switch()

  if (StatusTimer(T_AUTOCAP_HOMING) == T_ELAPSED) {
    forceAlarm(AUTOCAP_HOMING_ERROR);
  }
  return res;
}

#ifndef AUTOCAP_MMT
void makeAutocapActMessage(uartBuffer_t *txBuffer, unsigned char slave_id)
/**/
/*==========================================================================*/
/**
**   @brief Create the serial message for uC Syringe
**
**   @param txBuffer pointer to the tx buffer
**   @param slave_id slave identifier
**
 * 
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char idx = 0;
    // Initialize tx frame, reserve extra byte for pktlen 
    FRAME_BEGIN( txBuffer, idx, slave_id + 1);

    STUFF_BYTE( txBuffer->buffer, idx, autocapAct.typeMessage);
    STUFF_BYTE( txBuffer->buffer, idx, autocapAct.command.cmd);

    switch (autocapAct.typeMessage)
    {
        case CONTROLLO_PRESENZA:
        case JUMP_TO_BOOT :  
        break;
	
        case POS_HOMING:
            if (isAutocapCmd(CMD_HOME)) {
                STUFF_BYTE( txBuffer->buffer, idx, LSB(autocapAct.n_step_home));
                STUFF_BYTE( txBuffer->buffer, idx, MSB(autocapAct.n_step_home));
            }
            else if (isAutocapCmd(CMD_PACK)) {
                // TODO: add params 
                Nop();
            }
        break;

        // Used for lifter as well
        case POSIZIONA_AUTOCAP:
            if (isAutocapCmd(CMD_OPEN) || isAutocapCmd(CMD_CLOSE)) {
                STUFF_BYTE( txBuffer->buffer, idx, LSB(autocapAct.n_step_move));
                STUFF_BYTE( txBuffer->buffer, idx, MSB(autocapAct.n_step_move));

                STUFF_BYTE( txBuffer->buffer, idx, LSB(autocapAct.speed_move));
                STUFF_BYTE( txBuffer->buffer, idx, MSB(autocapAct.speed_move));
            }
            else{
                // TODO: add params 
                Nop();
            }
        break;
        default:
        break;
    }
    // crc, pktlen taken care of here 
    FRAME_END( txBuffer, idx);
    // Master only 
    txBuffer->bufferFlags.txReady = TRUE;
} // makeMessage() 

void decodeAutocapActMessage(uartBuffer_t *rxBuffer, unsigned char slave_id)
/**/
/*==========================================================================*/
/**
**   @brief  Decode the serial message received from uC autocap
**
**   @param  *rxBuffer pointer to the rx buffer
**   @param  slave_id slave identifier
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  unsigned char status, typeMessage, errorCode;
  unsigned char idx = FRAME_PAYLOAD_START;
  unionDWord_t tmpVer;

  typeMessage = rxBuffer->buffer[idx ++];
  // Status and error code, 16 bits
  status = rxBuffer->buffer[idx ++];
  if (status == AUTOCAP_OPEN_ST)	  
	TintingAct.Autocap_Status = TINTING_AUTOCAP_OPEN;
  else if (status == AUTOCAP_CLOSE_ST)
	TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED;  
  else if ( (status == AUTOCAP_HOME_POS_ERROR_ST)  || (status == AUTOCAP_HOMING_ERROR_ST)   || (status == AUTOCAP_TOUT_ERROR_ST) ||
			 (status == AUTOCAP_PACKING_ERROR_ST)   || (status == AUTOCAP_SOFTWARE_ERROR_ST) || (status == AUTOCAP_DRV_OVER_CURR_TEMP_ERROR_ST) ||
			 (status == AUTOCAP_OPEN_LOAD_ERROR_ST) || (status == AUTOCAP_LIFTER_ERROR_ST) )
	TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;
  else 
	TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED;  			 
  
  errorCode = rxBuffer->buffer[idx ++];

  // Version number (major, minor, patch), 24  bits 
  tmpVer.byte[0] = rxBuffer->buffer[idx ++];
  tmpVer.byte[1] = rxBuffer->buffer[idx ++];
  tmpVer.byte[2] = rxBuffer->buffer[idx ++];
  tmpVer.byte[3] = 0; /* padding */
  slaves_sw_versions[slave_id] = tmpVer.udword;

  // Adjust slave id
  slave_id -= AUTOCAP_ID - 1 ;

  // Photoc indicator 
  autocapAct.photoc_home = rxBuffer->buffer[idx ++];

  // Boot version number (major, minor, patch), 24  bits
  tmpVer.byte[0] = rxBuffer->buffer[idx ++];
  tmpVer.byte[1] = rxBuffer->buffer[idx ++];
  tmpVer.byte[2] = rxBuffer->buffer[idx ++];
  tmpVer.byte[3] = 0; // padding 
  slaves_boot_versions[slave_id] = tmpVer.udword;
    
  // Evaluate status flags
  autocapAct.autocapFlags.allFlags = 0L;
  switch (status)
  {
    case AUTOCAP_INIT_ST:
      // Initialization 
      TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED;
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_READY_ST:
      // Ready 
      autocapAct.autocapFlags.ready = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_SEARCH_HOMING_ST:
      // Homing in progress 
      autocapAct.autocapFlags.homing = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED;      
      set_slave_status(AUTOCAP_ID-1, 1);
      break;

    case AUTOCAP_CLOSE_ST:
      // Autocap is CLOSED 
      autocapAct.autocapFlags.close = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED; 
      procGUI.Autocap_status = AUTOCAP_CLOSED; // CLOSED 
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_CLOSE_RUN_ST:
      // Autocamp is CLOSING 
      autocapAct.autocapFlags.close = TRUE;
      autocapAct.autocapFlags.running = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED;            
      set_slave_status(AUTOCAP_ID-1, 1);
      break;

    case AUTOCAP_OPEN_RUN_ST:
      // Autocap is OPENING 
      autocapAct.autocapFlags.open = TRUE;
      autocapAct.autocapFlags.running = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_CLOSED;            
      set_slave_status(AUTOCAP_ID-1, 1);
      break;

    case AUTOCAP_OPEN_ST:
      // Autocap is OPEN ( -> CAN LIFTER is retracted) 
      autocapAct.autocapFlags.open = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_OPEN; 
      procGUI.Autocap_status = AUTOCAP_OPEN; // OPEN 
      set_slave_status(AUTOCAP_ID-1, 0);
      break;
    /**
        * ERRORS
    */
    case AUTOCAP_HOME_POS_ERROR_ST:
      autocapAct.autocapFlags.home_pos_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_TOUT_ERROR_ST:
      autocapAct.autocapFlags.tout_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_HOMING_ERROR_ST:
      autocapAct.autocapFlags.homing_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_PACKING_ERROR_ST:
      autocapAct.autocapFlags.packing_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_LIFTER_ERROR_ST:
      autocapAct.autocapFlags.lifter_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_SOFTWARE_ERROR_ST:
      autocapAct.autocapFlags.software_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_DRV_OVER_CURR_TEMP_ERROR_ST:
      autocapAct.autocapFlags.over_curr_temp_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_OPEN_LOAD_ERROR_ST:
      autocapAct.autocapFlags.open_load_error = TRUE;
      TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
      set_slave_status(AUTOCAP_ID-1, 0);
      break;

    case AUTOCAP_JUMP_TO_BOOT:
      autocapAct.autocapFlags.jump_to_boot = TRUE;
      set_slave_status(AUTOCAP_ID-1, 0);
      break;
  } // switch() 
} // decodeMessage()

#else

void init_Autocap(void)
/*
*//*=====================================================================*//**
**      @brief Initialization Autocap Status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
{
	AutocapHomingStatus.step = STEP_0;
    AutocapStatus.level = AUTOCAP_INIT_ST;
    autocapHomingStep = 0;
    autocap_enabled = FALSE;
    // BRUSH Driver DRV8842 Reset 
    DRV8842_RESET();   
    #ifndef SKIP_FAULT_1
        StartTimer(T_DELAY_INIT_DONE);    
    #endif        
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
    unsigned char ret = FALSE;
    switch(AutocapHomingStatus.step) {
        case STEP_0:
            // Autocap OPEN
            if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == DARK)            
                AutocapHomingStatus.step ++;
            // Autocap NOT OPEN
            else
                AutocapHomingStatus.step +=4;                  
        break;

        case STEP_1:
            // Start Closing Autocap
            AUTOCAP_MMT_CLOSING();
            StartTimer(T_TIMEOUT_AUTOCAP);
            AutocapHomingStatus.step ++;
        break;

        case STEP_2:
            if (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                // After Photocell is DARK wait further "T_MOTOR_AUTOCAP_ON" time before to Turn off motor 
                StartTimer(T_MOTOR_AUTOCAP_ON);
                AutocapHomingStatus.step ++;
            }
            else if (StatusTimer (T_TIMEOUT_AUTOCAP) == T_ELAPSED) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();                
                AutocapHomingStatus.level = AUTOCAP_HOMING_ERROR_ST;
                AutocapHomingStatus.step +=4 ;                                
            }    
        break;

        case STEP_3:
            if (StatusTimer (T_MOTOR_AUTOCAP_ON) == T_ELAPSED) {
                AUTOCAP_MMT_STOPPED();
                StopTimer(T_MOTOR_AUTOCAP_ON);
                // Closing completed successfully
                if (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK) {
                    AutocapHomingStatus.step +=3 ;
                    AutocapHomingStatus.level = AUTOCAP_CLOSE_ST;
                }
                // Closing completed but Photocell is LIGHT!
                else {
                    AutocapHomingStatus.level = AUTOCAP_HOMING_ERROR_ST;
                    AutocapHomingStatus.step +=3 ;
                }                                    
            }                    
        break;

        case STEP_4:
            // Start Opening Autocap
            AUTOCAP_MMT_OPENING();
            StartTimer(T_TIMEOUT_AUTOCAP);
            AutocapHomingStatus.step ++;
        break;

        case STEP_5:
            if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == DARK) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();
                AutocapHomingStatus.step -=4 ;
            }
            else if (StatusTimer (T_TIMEOUT_AUTOCAP) == T_ELAPSED) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();                
                AutocapHomingStatus.level = AUTOCAP_HOMING_ERROR_ST;
                AutocapHomingStatus.step++ ;                                
            }                
        break;
         
        case STEP_6:        
            intrAutocapAct();
            // Done 
            ret = TRUE;            
        break;    
    } // switch() 
    return ret;
} // isHomingPositionReached() 

//static unsigned char isHomingPositionReached(void)
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
/*
{
    unsigned char ret = FALSE;
    switch(AutocapHomingStatus.step) {
        case STEP_0:
            // Autocap OPEN
            if (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == LIGHT)            
                AutocapHomingStatus.step ++;
            // Autocap CLOSED
            else
                AutocapHomingStatus.step +=4;                  
        break;

        case STEP_1:
            // Start Closing Autocap
            AUTOCAP_MMT_CLOSING();
            StartTimer(T_TIMEOUT_AUTOCAP);
            AutocapHomingStatus.step ++;
        break;

        case STEP_2:
            if (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK) {
                AUTOCAP_MMT_STOPPED();
                StopTimer(T_TIMEOUT_AUTOCAP);
                // After Photocell is DARK wait further "T_MOTOR_AUTOCAP_ON" time before to Turn off motor 
                StartTimer(T_MOTOR_AUTOCAP_ON);
                AutocapHomingStatus.step ++;
            }
            else if (StatusTimer (T_TIMEOUT_AUTOCAP) == T_ELAPSED) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();                
                AutocapHomingStatus.level = AUTOCAP_HOMING_ERROR_ST;
                AutocapHomingStatus.step += 4;                                
            }    
        break;

        case STEP_3:
            if (StatusTimer (T_MOTOR_AUTOCAP_ON) == T_ELAPSED) {
                AUTOCAP_MMT_STOPPED();
                StopTimer(T_MOTOR_AUTOCAP_ON);
                // Closing completed successfully
                if (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK)
                    AutocapHomingStatus.step += 3;
                // Closing completed but Photocell is LIGHT!
                else {
                    AutocapHomingStatus.level = AUTOCAP_HOMING_ERROR_ST;
                    AutocapHomingStatus.step += 3;
                }                                    
            }                    
        break;

        case STEP_4:
            // Start Opening Autocap
            AUTOCAP_MMT_OPENING();
            StartTimer(T_TIMEOUT_AUTOCAP);
            AutocapHomingStatus.step ++;
        break;

        case STEP_5:
            if (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == LIGHT) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();
                AutocapHomingStatus.step -=4 ;
            }
            else if (StatusTimer (T_TIMEOUT_AUTOCAP) == T_ELAPSED) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();                
                AutocapHomingStatus.level = AUTOCAP_HOMING_ERROR_ST;
                AutocapHomingStatus.step ++;                                
            }                
        break;
         
        case STEP_6:        
            intrAutocapAct();
            // Done 
            ret = TRUE;            
        break;    
    } // switch() 
    return ret;
} // isHomingPositionReached() 
*/
void autocap_Manager(void)
/**/
/*==========================================================================*/
/**
**   @brief  Manager for Autocap controlled directly from MMT board
**
**   
**   
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    static char Photo_Autocap_Open;
    
    autocapAct.autocapFlags.allFlags = 0;    
    
    if ( (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK) && (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == DARK) )  
        AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;
    
        switch(AutocapStatus.level) {
        case AUTOCAP_INIT_ST:
            TintingAct.Autocap_Status = AUTOCAP_CLOSED;
            set_slave_status(AUTOCAP_ID-1, 0);
            // Set winding current as 100% of the full scale current set by VRERF Input pin and sense resistance
            I2_BRUSH_ON();
            I3_BRUSH_ON();
            I4_BRUSH_ON();  
            DECAY = 0;
            Photo_Autocap_Open = FALSE;           
            AutocapStatus.level++;
        break;

        case AUTOCAP_READY_ST:
            // Actuator is ready for initialization
            autocapAct.autocapFlags.ready = TRUE;
            TintingAct.Autocap_Status = AUTOCAP_CLOSED;      
            set_slave_status(AUTOCAP_ID-1, 0);            
            // Homing cmd  
            if (isAutocapCmdHoming() ) {
                AutocapStatus.level = AUTOCAP_SEARCH_HOMING_ST;
                AutocapHomingStatus.step = STEP_0;
            }
        break;

        case AUTOCAP_SEARCH_HOMING_ST:
            // Homing in progress 
            autocapAct.autocapFlags.homing = TRUE;
            TintingAct.Autocap_Status = AUTOCAP_CLOSED;      
            set_slave_status(AUTOCAP_ID-1, 1); 
            // Searching AUTOCAP home position 
            if (isHomingPositionReached())
                AutocapStatus.level = AutocapHomingStatus.level;
        break;

        case AUTOCAP_CLOSE_ST:
            // Autocap is CLOSED 
            autocapAct.autocapFlags.close = TRUE;
            procGUI.Autocap_status = AUTOCAP_CLOSED;  
            set_slave_status(AUTOCAP_ID-1, 0);            
            // Initialization completed, ready for operation. AUTOCAP is closed 
            // Open cmd
//autocapAct.command.open = TRUE;            
            if (isAutocapCmdOpen()) {
                intrAutocapAct();
                if ( (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == DARK) || (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == LIGHT) )    
                    AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                                
                else {
                    AUTOCAP_MMT_OPENING();
                    StartTimer(T_TIMEOUT_AUTOCAP);
                    Photo_Autocap_Open = FALSE;
                    AutocapStatus.level ++ ;
                }
            }
            else if (isAutocapCmdClose() ) {                
                intrAutocapAct();
                if ( (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == DARK) || (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == LIGHT) )    
                    AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                                
            }
            else if (isAutocapCmdHoming() ) {
                AutocapStatus.level = AUTOCAP_SEARCH_HOMING_ST;
                AutocapHomingStatus.step = STEP_0;
            }
        break;

        case AUTOCAP_OPEN_RUN_ST:
            // Autocap is OPENING 
            autocapAct.autocapFlags.open = TRUE;
            autocapAct.autocapFlags.running = TRUE;
            TintingAct.Autocap_Status = AUTOCAP_CLOSED;            
            set_slave_status(AUTOCAP_ID-1, 1);            
            if ( (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == DARK) && (Photo_Autocap_Open == FALSE) ) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                StartTimer(T_MOTOR_AUTOCAP_ON);
                Photo_Autocap_Open = TRUE;
            }
            else if ( (StatusTimer (T_TIMEOUT_AUTOCAP) == T_ELAPSED) && (Photo_Autocap_Open == FALSE) ) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();
                AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                
            }
            else if ( (StatusTimer (T_MOTOR_AUTOCAP_ON) == T_ELAPSED) && (Photo_Autocap_Open == TRUE) ) {
                StopTimer(T_MOTOR_AUTOCAP_ON);
                Photo_Autocap_Open = FALSE;
                AUTOCAP_MMT_STOPPED();
                if (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == DARK)
                    AutocapStatus.level = AUTOCAP_OPEN_ST;
                else 
                    AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                                    
            }
        break;

        case AUTOCAP_OPEN_ST:
            // Autocap is OPEN
            autocapAct.autocapFlags.open = TRUE;
            procGUI.Autocap_status = AUTOCAP_OPEN; 
            set_slave_status(AUTOCAP_ID-1, 0);           
//autocapAct.command.close = TRUE;
            // Close cmd
            if (isAutocapCmdClose()) {
                intrAutocapAct();                
                if ( (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == LIGHT) || (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK) )    
                    AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                                
                else {
                    AUTOCAP_MMT_CLOSING();
                    StartTimer(T_TIMEOUT_AUTOCAP);
                    Photo_Autocap_Open = FALSE;
                    AutocapStatus.level ++ ;
                }
            }
            else if (isAutocapCmdOpen() ) {                
                intrAutocapAct();
                if ( (PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL , FILTER) == LIGHT) || (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK) )    
                    AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                                
            }            
            else if (isAutocapCmdHoming() ) {
                AutocapStatus.level = AUTOCAP_SEARCH_HOMING_ST;
                AutocapHomingStatus.step = STEP_0;
            }
        break;

        case AUTOCAP_CLOSE_RUN_ST:
            // Autocamp is CLOSING 
            autocapAct.autocapFlags.close = TRUE;
            autocapAct.autocapFlags.running = TRUE;
            TintingAct.Autocap_Status = AUTOCAP_CLOSED;            
            set_slave_status(AUTOCAP_ID-1, 1);            
            if ( (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK) && (Photo_Autocap_Open == FALSE) ) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                StartTimer(T_MOTOR_AUTOCAP_ON);
                Photo_Autocap_Open = TRUE;
            }
            else if ( (StatusTimer (T_TIMEOUT_AUTOCAP) == T_ELAPSED) && (Photo_Autocap_Open == FALSE) ) {
                StopTimer(T_TIMEOUT_AUTOCAP);
                AUTOCAP_MMT_STOPPED();
                AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                
            }
            else if ( (StatusTimer (T_MOTOR_AUTOCAP_ON) == T_ELAPSED) && (Photo_Autocap_Open == TRUE) ) {
                StopTimer(T_MOTOR_AUTOCAP_ON);
                Photo_Autocap_Open = FALSE;
                AUTOCAP_MMT_STOPPED();
                if (PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL , FILTER) == DARK)
                    AutocapStatus.level = AUTOCAP_CLOSE_ST;
                else 
                    AutocapStatus.level = AUTOCAP_HOME_POS_ERROR_ST;                                    
            }
        break;
        
        case AUTOCAP_DRV_OVER_CURR_TEMP_ERROR_ST:
            autocapAct.autocapFlags.over_curr_temp_error = TRUE;
            TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
            set_slave_status(AUTOCAP_ID-1, 0);
            AUTOCAP_MMT_STOPPED();
            Photo_Autocap_Open = FALSE;                                    
            // Homing cmd  
            if (isAutocapCmdHoming() ) {
                AutocapStatus.level = AUTOCAP_SEARCH_HOMING_ST;
                AutocapHomingStatus.step = STEP_0;
            }
        break;
        case AUTOCAP_HOMING_ERROR_ST:
            autocapAct.autocapFlags.homing_error = TRUE;
            TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;      
            set_slave_status(AUTOCAP_ID-1, 0);
            AUTOCAP_MMT_STOPPED();
            Photo_Autocap_Open = FALSE;                    
            // Homing cmd  
            if (isAutocapCmdHoming() ) {
                AutocapStatus.level = AUTOCAP_SEARCH_HOMING_ST;
                AutocapHomingStatus.step = STEP_0;
            }
        break;
        case AUTOCAP_HOME_POS_ERROR_ST:  
            autocapAct.autocapFlags.home_pos_error = TRUE;
            TintingAct.Autocap_Status = TINTING_AUTOCAP_ERROR;
            set_slave_status(AUTOCAP_ID-1, 0);            
            AUTOCAP_MMT_STOPPED();
            Photo_Autocap_Open = FALSE;            
            // Homing cmd  
            if (isAutocapCmdHoming() ) {
                AutocapStatus.level = AUTOCAP_SEARCH_HOMING_ST;
                AutocapHomingStatus.step = STEP_0;
            }
        break;
    } // switch()    
    // Check for DRV8842 ERRORS
#ifndef SKIP_FAULT_1
    if (isFault_1_Detection() && StatusTimer(T_DELAY_INIT_DONE) == T_ELAPSED)
        AutocapStatus.level = AUTOCAP_DRV_OVER_CURR_TEMP_ERROR_ST;        
#endif    
}

#endif    

