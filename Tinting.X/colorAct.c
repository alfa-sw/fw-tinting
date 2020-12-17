/* 
 * File:   colorAct.c
 * Author: michele.abelli
 * Description: Autocap management
 * Created on 20 Maggio 2019, 14.16
 */

#include <math.h>
#include <stdint.h>
#include "serialCom.h"
#include "serialCom_GUI.h"
#include "tintingManager.h"
#include "colorAct.h"
#include "eepromManager.h"
#include "p24FJ256GB110.h"
#include <string.h>
#include <stdlib.h>
#include "ram.h"
#include "define.h"
#include "gestIO.h"
#include "timerMg.h"
#include "mem.h"
#include "typedef.h"
#include "ErrorManager.h"

/**
 * Function prototypes
 */
static void startRecircAfterReset(unsigned char id);
static unsigned char getVolumeTableIndex(unsigned long volume,unsigned char mode);
static unsigned char useCalibCurvesPar(unsigned long volume,unsigned char id,unsigned char algorithm);
static void standByActuatorsControl(void);
static void diagActuatorsControl(void);
static unsigned char useCalibCurvesParContinous(unsigned long volume,unsigned char id,unsigned char algorithm,unsigned long *volSingle);
static void calcPosContinous(unsigned char ID);

/**
 * Function definitions
 */
void setColorActMessage(unsigned char packet_type, unsigned char slave_id)
/**/
/*============================================================================*/
/**
**   @brief Set the type of serial message to be send to  uC Syringe
**
**   @param packet_type type of packet
**
**   @return void
**/
/*============================================================================*/
/**/
{
    colorAct[slave_id].typeMessage = packet_type;
}

#ifndef CAR_REFINISHING_MACHINE

void makeColorActMessage(uartBuffer_t *txBuffer, unsigned char slave_id)
/**/
/*============================================================================*/
/**
**   @brief Create the serial message for uC Syringe
**
**   @param txBuffer pointer to the tx buffer
**   @param slave_id slave identifier
**
**   @return void
**/
/*============================================================================*/
/**/
{
    unsigned char idx = 0;
    // Local refs 
    colorAct_t* pColorAct = &colorAct[slave_id];
    color_supply_par_t* pSettings = &color_supply_par[slave_id];
    
     // Initialize tx frame, reserve extra byte for pktlen 
    FRAME_BEGIN( txBuffer, idx, slave_id + 1);

    STUFF_BYTE( txBuffer->buffer, idx, pColorAct->typeMessage);
    STUFF_BYTE( txBuffer->buffer, idx, pColorAct->command.cmd);

    switch (pColorAct->typeMessage)
    {
        case CONTROLLO_PRESENZA:
        case JUMP_TO_BOOT:  
        break;

        case POS_HOMING:
            // No. steps for home position, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pSettings->n_step_home));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pSettings->n_step_home));
            // Stroke length, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pSettings->n_step_stroke));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pSettings->n_step_stroke));           
            // Recirculation speed [rpm], 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pSettings->speed_recirc));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pSettings->speed_recirc));           
            // Pump Type, 8 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(procGUI.circuit_pump_types[slave_id]));            
        break;

        case AGITAZIONE_PITTURA:
        case AGITAZIONE_COLORE:
            if ( (pColorAct->command.cmd == CMD_RESH) )                
                setAttuatoreAttivo(slave_id,1);
            else
                setAttuatoreAttivo(slave_id,0);                
            // Stirring PWM pct 
            STUFF_BYTE( txBuffer->buffer, idx,pSettings->reshuffle_pwm_pct);
        break;
        
        case DISPENSAZIONE_COLORE_CONT:
          	setAttuatoreAttivo(slave_id,1);
            // Start position, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->posStart));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->posStart));
            // Stop position - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->posStop));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->posStop));
            // Cycle speed [rpm], 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_cycle_supply));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_cycle_supply));
            // No cycles , 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->numCicliDosaggio));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->numCicliDosaggio));
            // Single stroke data
            STUFF_BYTE( txBuffer->buffer, idx,LSB_LSW(pColorAct->n_step_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_LSW(pColorAct->n_step_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,LSB_MSW(pColorAct->n_step_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_MSW(pColorAct->n_step_cycle));
            // Cycle speed [rpm], 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_cycle));
            // No cycles (ignored in CONTINUOUS mode), 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_cycles));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_cycles));
            // Dispensation algorithm: 0 = STROKE, 1 = CONTINUOUS 
            STUFF_BYTE( txBuffer->buffer, idx,pColorAct->algorithm); 
            // BackStep info (enable, steps, speed), 40 bits 
            STUFF_BYTE( txBuffer->buffer, idx,pColorAct->en_back_step);
            
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_back_step));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_back_step));

            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_back_step));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_back_step));

            // No. steps backlash, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pSettings->n_step_backlash));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pSettings->n_step_backlash));
            // Delay between two cycles, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->delay_EV_off));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->delay_EV_off));
            // Suction speed [rpm], 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_suction));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_suction));
            // Stroke lenght, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_stroke));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_stroke));
            // Continuous stop point (ignored in STROKE mode), 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_continuous_end));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_continuous_end));
            // Forces base acts _not_ to perform stirring after dispensation. 
            if (slave_id < N_SLAVES_BASE_ACT)
                pColorAct->delay_resh_after_supply = 0;
            else
                pColorAct->delay_resh_after_supply = DELAY_RESHUFFLE_AFTER_SUPPLY_SEC;
            // Stirring duration after dispensation [secs] 
            STUFF_BYTE( txBuffer->buffer, idx,pColorAct->delay_resh_after_supply);
            // Stirring PWM pct 
            STUFF_BYTE( txBuffer->buffer, idx,pSettings->reshuffle_pwm_pct);
        break;

        case DISPENSAZIONE_BASE:
        case DISPENSAZIONE_COLORE:
        	setAttuatoreAttivo(slave_id,1);
            // No. steps cycle, 32 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB_LSW(pColorAct->n_step_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_LSW(pColorAct->n_step_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,LSB_MSW(pColorAct->n_step_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_MSW(pColorAct->n_step_cycle));
            // Cycle speed [rpm], 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_cycle));
            // No cycles (ignored in CONTINUOUS mode), 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_cycles));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_cycles));
            // Dispensation algorithm: 0 = STROKE, 1 = CONTINUOUS 
            STUFF_BYTE( txBuffer->buffer, idx,pColorAct->algorithm);
            // BackStep info (enable, steps, speed), 40 bits 
            STUFF_BYTE( txBuffer->buffer, idx,pColorAct->en_back_step);
            
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_back_step));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_back_step));

            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_back_step));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_back_step));
            // No. steps backlash, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pSettings->n_step_backlash));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pSettings->n_step_backlash));
            // Delay between two cycles, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->delay_EV_off));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->delay_EV_off));
            // Suction speed [rpm], 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_suction));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_suction));
            // Stroke lenght, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_stroke));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_stroke));
            // Continuous stop point (ignored in STROKE mode), 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_continuous_end));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_continuous_end));
            // Forces base acts _not_ to perform stirring after dispensation. 
            if (slave_id < N_SLAVES_BASE_ACT)
                pColorAct->delay_resh_after_supply = 0;
            else
                pColorAct->delay_resh_after_supply = DELAY_RESHUFFLE_AFTER_SUPPLY_SEC;
            // Stirring duration after dispensation [secs] 
            STUFF_BYTE( txBuffer->buffer, idx,pColorAct->delay_resh_after_supply);
            // Stirring PWM pct 
            STUFF_BYTE( txBuffer->buffer, idx,pSettings->reshuffle_pwm_pct);
        break;

        case RICIRCOLO_PITTURA:
        case RICIRCOLO_COLORE:
        case AGITAZIONE_RICIRCOLO_COLORE:
            if ( (pColorAct->command.cmd == CMD_RECIRC) || (pColorAct->command.cmd == CMD_RESH_RECIRC) )                
                setAttuatoreAttivo(slave_id,1);
            else
                setAttuatoreAttivo(slave_id,0);                
            // No. steps cycle, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_cycle));
            // Cycle speed [rpm], 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_cycle));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_cycle));
            // No. cycles, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_cycles));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_cycles));
            // Delay between two recirculation cycles [secs] 
            STUFF_BYTE( txBuffer->buffer, idx,pColorAct->recirc_pause);
            // Stirring PWM pct 
            STUFF_BYTE( txBuffer->buffer, idx,pSettings->reshuffle_pwm_pct);
        break;
        
        case DISPENSAZIONE_GRUPPO_DOPPIO:
            setAttuatoreAttivo(slave_id,1);
            // Algorithm: ALG_DOUBLE_GROUP
            STUFF_BYTE( txBuffer->buffer, idx,LSB(ALG_DOUBLE_GROUP));
            // Stroke lenght, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_stroke));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_stroke));
            // cycle speed [RPM] CHANNEL A, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx, LSB(pColorAct->speed_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx, MSB(pColorAct->speed_cycle_channel_A));
            // cycle speed [RPM] CHANNEL B, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx, LSB(pColorAct->speed_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx, MSB(pColorAct->speed_cycle_channel_B));
            // No step cycle CHANNEL A, 32 bits 
            STUFF_BYTE( txBuffer->buffer, idx, LSB_LSW(pColorAct->n_step_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx, MSB_LSW(pColorAct->n_step_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx, LSB_MSW(pColorAct->n_step_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx, MSB_MSW(pColorAct->n_step_cycle_channel_A));
            // No. cycles CHANNEL A, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx, LSB(pColorAct->n_cycles_channel_A ));
            STUFF_BYTE( txBuffer->buffer, idx, MSB(pColorAct->n_cycles_channel_A ));
            // No step cycle CHANNEL B, 32 bits 
            STUFF_BYTE( txBuffer->buffer, idx, LSB_LSW(pColorAct->n_step_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx, MSB_LSW(pColorAct->n_step_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx, LSB_MSW(pColorAct->n_step_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx, MSB_MSW(pColorAct->n_step_cycle_channel_B));
            // No. cycles CHANNEL B, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx, LSB(pColorAct->n_cycles_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx, MSB(pColorAct->n_cycles_channel_B));
        break;

        case DISPENSAZIONE_COLORE_CONT_GRUPPO_DOPPIO:
            setAttuatoreAttivo(slave_id,1);
            // Algorithm: ALG_DOUBLE_GROUP_CONTINUOUS 
            STUFF_BYTE( txBuffer->buffer, idx, LSB(ALG_DOUBLE_GROUP_CONTINUOUS));
            // Stroke lenght, 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_step_stroke));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_step_stroke));	
            // Homing speed [rpm], 16 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pSettings->speed_recirc));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pSettings->speed_recirc));
            // Start position CHANNEL A - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->posStart_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->posStart_A));	
            // Start position CHANNEL B - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->posStart_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->posStart_B));	
            // Stop position CHANNEL A - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->posStop_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->posStop_A));	
            // Stop position CHANNEL B - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->posStop_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->posStop_B));	
            // Continuous dosing speed CHANNEL A - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speedContinous_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speedContinous_A));	
            // Continuous dosing speed CHANNEL B - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speedContinous_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speedContinous_B));	
            // Continuous dosing cycles CHANNEL A - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->numCicliDosaggio_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->numCicliDosaggio_A));
            // Continuous dosing cycles CHANNEL B - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->numCicliDosaggio_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->numCicliDosaggio_B));	
            // No step cycle Single Stroke CHANNEL A, 32 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB_LSW(pColorAct->n_step_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_LSW(pColorAct->n_step_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx,LSB_MSW(pColorAct->n_step_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_MSW(pColorAct->n_step_cycle_channel_A));
            // No step cycle Single Stroke CHANNEL B, 32 bits 
            STUFF_BYTE( txBuffer->buffer, idx,LSB_LSW(pColorAct->n_step_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_LSW(pColorAct->n_step_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx,LSB_MSW(pColorAct->n_step_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB_MSW(pColorAct->n_step_cycle_channel_B));
            // Single Stroke dosing speed CHANNEL A - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_cycle_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_cycle_channel_A));	
            // Single Stroke dosing speed CHANNEL B - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->speed_cycle_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->speed_cycle_channel_B));	
            // Single Stroke dosing cycles CHANNEL A - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_cycles_channel_A));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_cycles_channel_A));	
            // Single Stroke dosing cycles CHANNEL B - 16 bits
            STUFF_BYTE( txBuffer->buffer, idx,LSB(pColorAct->n_cycles_channel_B));
            STUFF_BYTE( txBuffer->buffer, idx,MSB(pColorAct->n_cycles_channel_B));
        break;
        
        default:
            break;
    }
  // crc, pktlen taken care of here 
  FRAME_END( txBuffer, idx);
  // Master only 
  txBuffer->bufferFlags.txReady = TRUE;
} // makeColorActMessage() 

void decodeColorActMessage(uartBuffer_t *rxBuff, unsigned char slave_id)
/**/
/*==========================================================================*/
/**
**   @brief  Decode the serial message received from uC Syringe
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
    unionDWord_t tmpVol, tmpVer;
    unionWord_t tmpWord;

    typeMessage = rxBuff->buffer[idx ++];
    // Status and error code, 16 bits 
    status = rxBuff->buffer[idx ++];
    errorCode = rxBuff->buffer[idx ++];
    // Version number,32 bits 
    tmpVer.byte[0] = rxBuff->buffer[idx ++];
    tmpVer.byte[1] = rxBuff->buffer[idx ++];
    tmpVer.byte[2] = rxBuff->buffer[idx ++];
    tmpVer.byte[3] = 0; 
    // padding 
    slaves_sw_versions[slave_id] = tmpVer.udword;
    // Adjust slave id 
    slave_id -= B1_BASE_IDX;
    // Dispensated volume, 32 bits 
    tmpVol.byte[0] = rxBuff->buffer[idx ++];
    tmpVol.byte[1] = rxBuff->buffer[idx ++];
    tmpVol.byte[2] = rxBuff->buffer[idx ++];
    tmpVol.byte[3] = rxBuff->buffer[idx ++];
    colorAct[slave_id].totalVolume = tmpVol.udword;
    // Photoc and reserve indicators, 16 bits 
    colorAct[slave_id].photoc_home = rxBuff->buffer[idx ++];
    colorAct[slave_id].usw_reserve = rxBuff->buffer[idx ++];
    // 13-07-2017 - Used from Testing Bench
    colorAct[slave_id].usw_reserveDiag = rxBuff->buffer[idx ++];
    // Additional data for zero reading 
    tmpWord.byte[0] = rxBuff->buffer[idx ++];
    tmpWord.byte[1] = rxBuff->buffer[idx ++];
    colorAct[slave_id].photoc_zero_reading = tmpWord.sword;
    // Boot version number (major, minor, patch), 24  bits 
    tmpVer.byte[0] = rxBuff->buffer[idx ++];
    tmpVer.byte[1] = rxBuff->buffer[idx ++];
    tmpVer.byte[2] = rxBuff->buffer[idx ++];
    tmpVer.byte[3] = 0; // padding 
    slaves_boot_versions[slave_id] = tmpVer.udword;
    // Evaluate status flag 
    colorAct[slave_id].colorFlags.allFlags = 0L;
    switch (status)
    {
        case COLOR_INIT_ST:
            // Initialization 
            colorAct[slave_id].colorFlags.stopped = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 0);
        break;

        case COLOR_READY_ST:
            // Ready 
            setAttuatoreAttivo(slave_id,0);
            colorAct[slave_id].colorFlags.ready = TRUE;
            colorAct[slave_id].colorFlags.stopped = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 0);
        break;

        case COLOR_GO_HOMING_ST:
        case COLOR_SEARCH_HOMING_ST:
            setAttuatoreAttivo(slave_id,1);
            // The actuator is still on its way home 
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_HOMING_ST:
            setAttuatoreAttivo(slave_id,0);
            // When they get there, they're STOPPED and HOMING 
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.homing = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 0);
        break;

        case COLOR_STANDBY_END_ST:
            setAttuatoreAttivo(slave_id,1);
            colorAct[slave_id].colorFlags.recirc_end = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_SUPPLY_END_ST:
            setAttuatoreAttivo(slave_id,1);
            colorAct[slave_id].colorFlags.supply_end = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
          break;

        case COLOR_STANDBY_RUN_ST:
            setAttuatoreAttivo(slave_id,1);
            colorAct[slave_id].colorFlags.recirc_run = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_SUPPLY_RUN_ST:
            setAttuatoreAttivo(slave_id,1);
            colorAct[slave_id].colorFlags.supply_run = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
          break;

        case COLOR_HOMING_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.homing_pos_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_TOUT_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.tout_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
          break;

        case COLOR_RESET_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.reset_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_SUPPLY_CALC_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.supply_calc_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_SOFTWARE_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.software_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_OVERCURRENT_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.overcurrent_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_HOMING_BACK_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.homing_back_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_POS0_READ_LIGHT_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.pos0_read_light_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_END_STROKE_READ_DARK_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.end_stroke_read_dark_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_OPEN_LOAD_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.open_load_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;

        case COLOR_DRV_OVER_CURR_TEMP_ERROR_ST:
            colorAct[slave_id].colorFlags.stopped = TRUE;
            colorAct[slave_id].colorFlags.drv_over_curr_temp_error = TRUE;
            set_slave_status(B1_BASE_IDX + slave_id, 1);
        break;
        
        case COLOR_JUMP_TO_BOOT_ST:
          colorAct[slave_id].colorFlags.jump_to_boot = TRUE;
          set_slave_status(B1_BASE_IDX + slave_id, 0);
        break; 
        
        default:
        break;    
    }
    if(procGUI.circuit_pump_types[slave_id] == PUMP_DOUBLE) {        
        if (slave_id == Double_Group_0) {
            if ( ( (slave_id == 0) && ((procGUI.stirring_status & 0x01) == 1) ) || ( (slave_id == 2) && ((procGUI.stirring_status & 0x04) == 1) ) ) {
                setAttuatoreAttivo(slave_id,1); 
                set_slave_status(slave_id,1);
                if (Double_Group_1 != 64) {
                    setAttuatoreAttivo(Double_Group_1,1); 
                    set_slave_status(Double_Group_1,1);
                }
            }            
        }
        else if (slave_id == Double_Group_1) {
            if ( ( (slave_id == 0) && ((procGUI.stirring_status & 0x01) == 1) ) || ( (slave_id == 2) && ((procGUI.stirring_status & 0x04) == 1) ) ) {
                setAttuatoreAttivo(slave_id,1); 
                set_slave_status(slave_id,1);
                if (Double_Group_0 != 64) {
                    setAttuatoreAttivo(Double_Group_0,1); 
                    set_slave_status(Double_Group_0,1);
                }
            }            
        }
    }        
} // decodeColorActMessage() 

#endif
static double round(double f)
{
    double fl = floor(f);
    double cl = ceil(f);

    return (f - fl < cl - f)
        ? fl
        : cl ;
}

static unsigned char getVolumeTableIndex(unsigned long volume,
                                         unsigned char mode)
/**/
/*==========================================================================*/
/**
**   @brief Return calib table related to vol_r
**
**   @param ushort volume
**
**   @param mode COLOR_ACT_STROKE_OPERATING_MODE or COLOR_ACT_CONTINUOUS_OPERATING_MODE
**
**   @return  table id
**/
/*==========================================================================*/
/**/
{
    unsigned char i;
    unsigned short max_speed = 0;
    unsigned char ret = NO_TABLE_AVAILABLE;
    for (i = 0; i < N_CALIB_CURVE; ++ i) {
        calib_curve_par_t* pCurve = &calib_curve_par[i];
        if (
            // working either in STROKE mode... 
            ((COLOR_ACT_STROKE_OPERATING_MODE == mode && 
             (pCurve->algorithm == COLOR_ACT_ALGORITHM_SINGLE_STROKE || pCurve->algorithm == COLOR_ACT_ALGORITHM_DOUBLE_STROKE))
                    ||
            // ... or CONTINUOUS mode ... 
            (COLOR_ACT_CONTINUOUS_OPERATING_MODE == mode &&
            (pCurve->algorithm == COLOR_ACT_ALGORITHM_SYMMETRIC_CONTINUOUS || pCurve->algorithm == COLOR_ACT_ALGORITHM_ASYMMETRIC_CONTINUOUS))) &&
            // ... and the target volume is within table validity range ... 
                volume >= pCurve->vol_min && volume <  pCurve->vol_max &&
            // ... and this table is faster than what we already got ... 
                pCurve->speed_value > max_speed) {

                max_speed = pCurve->speed_value;
                ret = i;
        }
        else if ( (COLOR_ACT_STROKE_OPERATING_MODE == mode) &&
                   (pCurve->algorithm == COLOR_ACT_ALGORITHM_HIGH_RES_STROKE || pCurve->algorithm == COLOR_ACT_ALGORITHM_DOUBLE_STROKE) && 
                   (volume >= pCurve->vol_min) && (volume <  pCurve->vol_max) && (pCurve->speed_value > max_speed) ) {

            max_speed = pCurve->speed_value;
            ret = i;
        }		        
    }
    return ret;
}

static unsigned char useCalibCurvesParContinous(unsigned long volume,unsigned char id,unsigned char algorithm,unsigned long *volSingle)
{   
	unsigned char res;
	unsigned char i;
	// Local refs 
	colorAct_t* pColorAct = &colorAct[id];
    
	// Clear output value (see last comment at the bottom) 
	if (algorithm == COLOR_ACT_CONTINUOUS_OPERATING_MODE)
		pColorAct->n_step_cycle_supply = 0;
	else
		pColorAct->n_step_cycle = 0;

	// select calibration table 
	res = getVolumeTableIndex(volume, algorithm);
	if (res != NO_TABLE_AVAILABLE) {
        calib_curve_par_t* pCurve = &calib_curve_par[res];
        color_supply_par_t *pColor=&color_supply_par[id];
        // linear interpolation 
        for (i = 0; i < N_CALIB_POINTS - 1; ++ i) {
            unsigned long vol_1, vol_2;
            vol_1 = pCurve->vol_cc[i];
            vol_2 = pCurve->vol_cc[i+1];
            unsigned long step1, step2;
            step1 = pCurve->n_step[i];
            step2 = pCurve->n_step[i+1];
            // "volume" = vol_1 
            if (volume == vol_1) {
                if (algorithm == COLOR_ACT_CONTINUOUS_OPERATING_MODE)
                    pColorAct->n_step_cycle_supply = (unsigned long) step1;
                else 
                    pColorAct->n_step_cycle = (unsigned long) step1;
                break;
            }
    	    //  "volume" = vol_2 AND i" = N_CALIB_POINTS - 2 
            else if ((volume == vol_2) && (i == N_CALIB_POINTS - 2)) {
                if (algorithm == COLOR_ACT_CONTINUOUS_OPERATING_MODE)
                    pColorAct->n_step_cycle_supply = (unsigned long) step2;
                else 
                    pColorAct->n_step_cycle = (unsigned long) step2;
                break;
            }
    	    //  "volume" inside an interval vol1_1 - vol_2, OR "volume" < first interval, OR "volume" > last interval of the last N_CALIB_POINTS
            else  if ( (volume > vol_1 && volume < vol_2) || ((volume < vol_1) && (i == 0)) || ((volume > vol_2) && (vol_2 > 0) && (i == (N_CALIB_POINTS - 2))) ) {
                // Here we use double-precision floating point arithmetic. Which is fully supported by the PIC24 platform. Hell, why not ?!? 
                double v, v0, v1,ris;
                v = volume;
                v0 = vol_1;
                v1 = vol_2;
            	double s, s0, s1;
                s0 = step1;
                s1 = step2;

                if (algorithm == COLOR_ACT_CONTINUOUS_OPERATING_MODE) {
                    // I have to search the volume with a step number that is a multiple of the stroke length nearest to the target volume
                    double m,q;
        			m = (v1-v0)/(s1-s0);
                    q = v0-(m*s0);
                    unsigned short j;
                    unsigned char fine = 0;			  
                    j = 0;
                    do
                    {
                        s = s0+((double)pColor->n_step_stroke*(double)j);
                        ris = m*s+q;
                        if (ris > v) {
                            // E' la prima volta che succede quindi il volume da dispensare in continous è il volume precedente
                            if (j > 0) {
                                j--;
                                s = s0+((double)pColor->n_step_stroke*(double)j);
                            }
                            else
                                s=s0;

                            ris = m*s+q;
                            *volSingle = (unsigned long)(v-ris);
                            fine = 1;
                        }
                        else if (ris == v) {
                            *volSingle = 0;
                            fine = 1;
                        }
                        if (s > s1) {
                            // Finita ricerca
                            s = s1;
                            ris = s*m+q;
                            if (ris > v) {
                                *volSingle = (unsigned long)(ris-v);
                            }
                            else {
                                *volSingle = (unsigned long)(v-ris);
                            }
                            fine = 1;
                        }
                        j++;
                    } while (!fine);
                    pColorAct->n_step_cycle_supply = (unsigned long) s;
        		}
                else {
                    s = round((s0 * (v1 - v) + s1 * (v - v0)) / (v1 - v0));
                    pColorAct->n_step_cycle = (unsigned long) s;
                }		  
                // Interpolate the number of required steps. NOTE: while in
                // STROKE mode this calculation always pertains a _single_
                // stroke, in CONTINUOUS mode the calculated no. of steps is the
                // total number of steps to be performed. It will be up to the
                // actuator to determine how to perform a single run consisting
                // of that (possibly very large) number of steps. 
                break;
            }
    	    // "volume" > last interval of calibration table, but NOT of last N_CALIB_POINTS
        	else if ( (volume > vol_1) && (vol_1 > 0) && (vol_2 == 0) ) {
                vol_1 = pCurve->vol_cc[i-1];
                vol_2 = pCurve->vol_cc[i];
                step1 = pCurve->n_step[i-1];
                step2 = pCurve->n_step[i];
                // Here we use double-precision floating point
                // arithmetic. Which is fully supported by the PIC24
                // platform. Hell, why not ?!? 
                double v, v0, v1,ris;
                v  = volume;
                v0 = vol_1;
                v1 = vol_2;
                double s, s0, s1;
                s0 = step1;
                s1 = step2;
                if (algorithm == COLOR_ACT_CONTINUOUS_OPERATING_MODE) {
                    // I have to search the volume with a step number that is a multiple of the stroke length nearest to the target volume
                    double m,q;
                    m = (v1-v0)/(s1-s0);
                    q = v0-(m*s0);
                    unsigned short j;
                    unsigned char fine = 0;
                    j = 0;
                    do
                    {
                        s = s0+((double)pColor->n_step_stroke*(double)j);
                        ris = m*s+q;
                        if (ris > v) {
                            // E' la prima volta che succede quindi il volume da dispensare in continous è il volume precedente
                            if (j > 0) {
                                j--;
                                s = s0+((double)pColor->n_step_stroke*(double)j);
                            }
                            else
                                s = s0;
                            ris = m*s+q;
                            *volSingle = (unsigned long)(v-ris);
                            fine = 1;
                        }
                        else if (ris == v) {
                            *volSingle = 0;
                            fine = 1;
                        }
                        if (s > s1)
                        {
                            // Finita ricerca
                            s = s1;
                            ris = s*m+q;
                            if (ris > v)
                                *volSingle = (unsigned long)(ris-v);
                            else
                                *volSingle = (unsigned long)(v-ris);
                            fine = 1;
                        }
                        j++;
                    } while (!fine);
                    pColorAct->n_step_cycle_supply = (unsigned long) s;
                }
                else {
                  s = round((s0 * (v1 - v) + s1 * (v - v0)) / (v1 - v0));
                  pColorAct->n_step_cycle = (unsigned long) s;
                }
                // Interpolate the number of required steps. NOTE: while in
                // STROKE mode this calculation always pertains a _single_
                // stroke, in CONTINUOUS mode the calculated no. of steps is the
                // total number of steps to be performed. It will be up to the
                // actuator to determine how to perform a single run consisting
                // of that (possibly very large) number of steps. 
                break;
            }
        }
    } // else 
    // If n_step_cycle_supply is still zero cycle, no interpolation was
    // possible for this target volume. We'll raise a DATA_SUPPLY_ERROR ALARM in this case
    if (algorithm == COLOR_ACT_CONTINUOUS_OPERATING_MODE) {
        if (! pColorAct->n_step_cycle_supply)
          setAlarm(B1_DATA_SUPPLY_FAILED + id);
    }
    else {
        if (! pColorAct->n_step_cycle)
          setAlarm(B1_DATA_SUPPLY_FAILED + id);
    }
    return res;
}

static unsigned char useCalibCurvesPar(unsigned long volume,
                                       unsigned char id,
                                       unsigned char mode)
/**/
/*============================================================================*/
/**
**   @brief Calculate color parameters for a single dispensation
**   cycle.
**
**   @param volume, volume to be dispensed *in a single stroke* for STROKE
**   mode. In CONTINUOUS mode, `volume` is the overall target volume.
**
**   @param id the color actuator slave ID
**
**   @param mode COLOR_ACT_STROKE_OPERATING_MODE or COLOR_ACT_CONTINUOUS_OPERATING_MODE
**
**   @return chosen calibration table ID
**/
/*============================================================================*/
/**/
{    
    unsigned char res;
    unsigned char i;
    // Local refs
    colorAct_t* pColorAct = &colorAct[id];

    // Clear output value (see last comment at the bottom) 
    pColorAct->n_step_cycle_supply = 0;
    // Select calibration table 
    res = getVolumeTableIndex(volume, mode);
    if (res != NO_TABLE_AVAILABLE) {
        calib_curve_par_t* pCurve = & calib_curve_par[res];
        // Depending on calibration curve I set the dispensing algorithm
      	pColorAct->algorithm = pCurve->algorithm;	
        // Linear interpolation 
        for (i = 0; i < N_CALIB_POINTS - 1; ++ i) {
            unsigned long vol_1, vol_2;
            vol_1 = pCurve->vol_cc[i];
            vol_2 = pCurve->vol_cc[i+1];
            unsigned long step1, step2;
            step1 = pCurve->n_step[i];
            step2 = pCurve->n_step[i+1];
            // "volume" = vol_1 
            if (volume == vol_1) {
                pColorAct->n_step_cycle_supply = (unsigned long) step1;
                break;
            }
            //  "volume" = vol_2 AND i" = N_CALIB_POINTS - 2 
            else if ((volume == vol_2) && (i == N_CALIB_POINTS - 2)) {
                pColorAct->n_step_cycle_supply = (unsigned long) step2;
                break;
            }
            // "volume" inside an interval vol1_1 - vol_2, OR "volume" < first interval, OR "volume" > last interval of the last N_CALIB_POINTS
            else  if ( (volume > vol_1 && volume < vol_2) || ((volume < vol_1) && (i == 0)) || ((volume > vol_2) && (vol_2 > 0) && (i == (N_CALIB_POINTS - 2))) ) {
                // Here we use double-precision floating point
                // arithmetic. Which is fully supported by the PIC24 platform. Hell, why not ?!? 
                double v, v0, v1;
                v  = volume;
                v0 = vol_1;
                v1 = vol_2;
                double s, s0, s1;
                s0 = step1;
                s1 = step2;
                // Interpolate the number of required steps. NOTE: while in
                // STROKE mode this calculation always pertains a _single_
                // stroke, in CONTINUOUS mode the calculated no. of steps is the
                // total number of steps to be performed. It will be up to the
                // actuator to determine how to perform a single run consisting
                // of that (possibly very large) number of steps. 
                s = round((s0 * (v1 - v) + s1 * (v - v0)) / (v1 - v0));
                pColorAct->n_step_cycle_supply = (unsigned long) s;
                break;
            }
            // "volume" > last interval of calibration table, but NOT of last N_CALIB_POINTS
            else if ( (volume > vol_1) && (vol_1 > 0) && (vol_2 == 0) ) {
                vol_1 = pCurve->vol_cc[i-1];
                vol_2 = pCurve->vol_cc[i];
                step1 = pCurve->n_step[i-1];
                step2 = pCurve->n_step[i];
                // Here we use double-precision floating point
                // arithmetic. Which is fully supported by the PIC24
                // platform. Hell, why not ?!?
                double v, v0, v1;
                v  = volume;
                v0 = vol_1;
                v1 = vol_2;
                double s, s0, s1;
                s0 = step1;
                s1 = step2;

                // Interpolate the number of required steps. NOTE: while in
                // STROKE mode this calculation always pertains a _single_
                // stroke, in CONTINUOUS mode the calculated no. of steps is the
                // total number of steps to be performed. It will be up to the
                // actuator to determine how to perform a single run consisting
                // of that (possibly very large) number of steps. 
                s = round((s0 * (v1 - v) + s1 * (v - v0)) / (v1 - v0));
                pColorAct->n_step_cycle_supply = (unsigned long) s;
                break;		
            }
        }
    } // else 
    // If n_step_cycle_supply is still zero cycle, no interpolation was
    // possible for this target volume. We'll raise a DATA_SUPPLY_ERROR
    //ALARM in this case. 
    if (! pColorAct->n_step_cycle_supply)
        setAlarm(B1_DATA_SUPPLY_FAILED + id);
      return res;
}

static void calcPosContinous(unsigned char ID)
{
	long x, m, l,h,j;
	unsigned short cycles = 0;
	unsigned char fine=0;
	colorAct_t* pColorAct = &colorAct[ID];	
	x = (long)pColorAct->n_step_cycle_supply;
	m = (long)pColorAct->n_step_stroke;
	j = (long)pColorAct->n_step_stroke;

	h = 0;
	pColorAct->posStart = (long)pColorAct->n_step_continuous_end;
	if (pColorAct->algorithm == ALG_SYMMETRIC_CONTINUOUS) {
		cycles = 1;		
		do
		{
			x = x-(j-pColorAct->posStart);
			cycles++;
			h = x/m;
			cycles+=h;
			l = x%m;
			if ((cycles%2) == 0)
				// Se pari raggiungo la pos stop partendo da posend 
				pColorAct->posStop = j-l;
			else
				// Se dispari raggiungo la posstop partendo da 0
				pColorAct->posStop = l;
		
			if (pColorAct->posStop>=(j-100)) {
				// Sono troppo vicino alla fine mi devo spostare
				h = 300;
				x =(long)pColorAct->n_step_cycle;
				m =(long)pColorAct->n_step_stroke;
				j = (long)pColorAct->n_step_stroke;
				if ((cycles%2) == 0)
					pColorAct->posStart=(long)pColorAct->n_step_continuous_end+h;
				else
					pColorAct->posStart=(long)pColorAct->n_step_continuous_end-h;
				cycles = 1;				
			}
			else if (pColorAct->posStop<pColorAct->n_step_backlash)
			{
				// Sono troppo vicino allo 0 devo spostarmi avanti
				h = 300;
				x = (long)pColorAct->n_step_cycle;
				m = (long)pColorAct->n_step_stroke;
				j = (long) pColorAct->n_step_stroke;
				if ((cycles % 2)==0)
					pColorAct->posStart=(long) pColorAct->n_step_continuous_end-h;
				else
					pColorAct->posStart=(long) pColorAct->n_step_continuous_end+h;
				cycles = 1;				
			}	
			else
				fine = 1;
		} while (!fine);
	}
	else {
		cycles = 0;
		do
		{
			x = x-(j-pColorAct->posStart);
			cycles++;
			h = x/m;
			cycles+=(h*2)+2;
			l = x%m;
			if ((cycles%2)==0)
				// Se pari raggiungo la pos stop partendo da posend 
				pColorAct->posStop=j-l;
			else
				// Se dispari raggiungo la posstop partendo da 0
				pColorAct->posStop=l;
            
            if (pColorAct->posStop >= (j-100)) {
                // Sono troppo vicino alla fine mi devo spostare
                h = 300;
                x = (long)pColorAct->n_step_cycle;
                m = (long)pColorAct->n_step_stroke;
                j = (long)pColorAct->n_step_stroke;
                if ((cycles%2) == 0)
                    pColorAct->posStart = (long)pColorAct->n_step_continuous_end+h;
                else
                    pColorAct->posStart = (long)pColorAct->n_step_continuous_end-h;
                cycles = 1;				
			}
			else if (pColorAct->posStop<pColorAct->n_step_backlash) {
				// Sono troppo vicino allo 0 devo spostarmi avanti
				h = 300;
				x = (long)pColorAct->n_step_cycle;
				m = (long)pColorAct->n_step_stroke;
				j = (long) pColorAct->n_step_stroke;
				if ((cycles % 2)==0)
					pColorAct->posStart=(long) pColorAct->n_step_continuous_end-h;
				else
					pColorAct->posStart=(long) pColorAct->n_step_continuous_end+h;

				cycles = 1;
			}	
			else
				fine = 1;
		} while (!fine);
	}
	pColorAct->numCicliDosaggio = cycles;
}

void calcSupplyPar(unsigned long vol_t,unsigned long vol_mu,unsigned long vol_mc,unsigned char id)
/**/
/*==========================================================================*/
/**
**   @brief Calculate color parameters for supply procedure
**
**   @param vol_t  target volume
**   @param vol_mu useful stroke max volume
**   @param vol_mc minimum continuous volume
**   @param id color actuator identifier
**
**   @return void
**/
/*==========================================================================*/
/**/
{   
    unsigned char ndx = NO_TABLE_AVAILABLE;
    unsigned char tipoAlg;
    unsigned char ndxSingle=NO_TABLE_AVAILABLE;
    unsigned long singleStrokeVol=0;
  
    // Local refs 
    
    colorAct_t* pColorAct = &colorAct[id];
    // Fetch calibration data from EEPROM 
    loadEECalibCurves(id);
    
	if (isColorTintingModule(id))
		color_supply_par[id].n_step_stroke = TintingPump.N_steps_stroke;

    // 1. If target volume fits in a single stroke, use STROKE algorithms; 
    if (vol_t <= vol_mu) {
        pColorAct->n_cycles_supply = 1;
        ndx = useCalibCurvesPar(vol_t, id, COLOR_ACT_STROKE_OPERATING_MODE);
        if (ndx == NO_TABLE_AVAILABLE)
            return;
        calib_curve_par_t* pCurve = &calib_curve_par[ndx];
        if ( (pCurve->algorithm == COLOR_ACT_ALGORITHM_HIGH_RES_STROKE) || (pCurve->algorithm == COLOR_ACT_ALGORITHM_DOUBLE_STROKE) )
            tipoAlg = COLOR_ACT_HIGH_RES_STROKE_OPERATING_MODE;
        else
            tipoAlg = COLOR_ACT_STROKE_OPERATING_MODE;			
    }
    // 2. If target volume is greater than minimum continuous volume, switch to CONTINUOUS algorithms
    else if (vol_mc <= vol_t) {
        pColorAct->n_cycles_supply = 1; // ignored anyway ... 
        // Modifico la ricerca delle tabelle per sapere quali tabelle usare
        ndx = useCalibCurvesParContinous(vol_t, id, COLOR_ACT_CONTINUOUS_OPERATING_MODE,&singleStrokeVol);
        tipoAlg = COLOR_ACT_CONTINUOUS_OPERATING_MODE;
        // Adesso cerco il volume rimasto nelle tabelle single stroke
        if (singleStrokeVol != 0) {
            if (singleStrokeVol <= vol_mu) {
                pColorAct->n_cycles = 1;
                ndxSingle = useCalibCurvesParContinous(singleStrokeVol, id, COLOR_ACT_STROKE_OPERATING_MODE,&singleStrokeVol);
            }
            else {
                unsigned short n = (unsigned short)(singleStrokeVol/ vol_mu);
                // add an extra cycle if required 
                if (vol_t % vol_mu)
                  ++ n;

                pColorAct->n_cycles = n;
                ndxSingle= useCalibCurvesParContinous(singleStrokeVol/ n, id, COLOR_ACT_STROKE_OPERATING_MODE,&singleStrokeVol);
            }
    	}
        else {
            pColorAct->n_cycles = 0;
            ndxSingle = ndx;	// Messa per riuscire a passare il controllo sull'indice delle tabelle
        }
    }
    // 3. fallback to multiple, equally sized-strokes at maximum available speed with STROKE algorithms
    else {
        unsigned short n = (unsigned short)(vol_t / vol_mu);
        // Add an extra cycle if required 
        if (vol_t % vol_mu)
          ++ n;

        pColorAct->n_cycles_supply = n;
        ndx = useCalibCurvesPar(vol_t / n, id, COLOR_ACT_STROKE_OPERATING_MODE);
        tipoAlg = COLOR_ACT_STROKE_OPERATING_MODE;		
    } 

    if ((ndx == NO_TABLE_AVAILABLE) || ((tipoAlg==COLOR_ACT_CONTINUOUS_OPERATING_MODE) && (ndxSingle == NO_TABLE_AVAILABLE)))
        setAlarm(B1_DATA_SUPPLY_FAILED + id);        
    else if ( (tipoAlg==COLOR_ACT_CONTINUOUS_OPERATING_MODE) && (pColorAct->n_cycles > 1) && (isColorCircuit(id)) )
        setAlarm(B1_DATA_SUPPLY_FAILED + id);        
    else {
        // Local refs 
        color_supply_par_t* pSettings = &color_supply_par[id];
        dispensationAct_t* pDispensation = &dispensationAct[id];
        calib_curve_par_t* pCurve = &calib_curve_par[ndx];
        calib_curve_par_t* pCurveSingle = &calib_curve_par[ndxSingle];
        
        if ( (tipoAlg == COLOR_ACT_STROKE_OPERATING_MODE) || (tipoAlg == COLOR_ACT_HIGH_RES_STROKE_OPERATING_MODE) ) {
            // If Dosing Temperature process is enabled and Temperature <= Temp_T_HIGH --> Half Suction Speed
            if (Dosing_Half_Speed == TRUE) {
                pColorAct->speed_suction = pSettings->speed_suction / 2;
                pColorAct->speed_cycle_supply = pCurve->speed_value / 2;
                pDispensation->speed_cycle = pColorAct->speed_cycle_supply / 2;			
            }	
            else {
                pColorAct->speed_suction = pSettings->speed_suction;
                pColorAct->speed_cycle_supply = pCurve->speed_value;
                pDispensation->speed_cycle = pColorAct->speed_cycle_supply;			
            }		
		
            pColorAct->n_step_stroke = pSettings->n_step_stroke;
            pColorAct->n_step_continuous_end = pSettings->n_step_continuous_end;
            pColorAct->delay_EV_off = pSettings->delay_EV_off;
            // 2. Additional settings from calibration curve
            if (tipoAlg == COLOR_ACT_HIGH_RES_STROKE_OPERATING_MODE)
                pColorAct->algorithm = COLOR_ACT_ALGORITHM_HIGH_RES_STROKE;
            else
                pColorAct->algorithm = pCurve->algorithm;

            pColorAct->en_back_step = pCurve->en_back_step;
            pColorAct->n_step_back_step = pCurve->n_step_back_step;
            pColorAct->speed_back_step = pCurve->speed_back_step;
            // 3. update data for DIAG_GET_LAST_DISPENSATION_PARAMS command reply 
            pDispensation->n_step_cycle = pColorAct->n_step_cycle_supply;
            pDispensation->n_cycles = pColorAct->n_cycles_supply;
        }
        else {
            // If Dosing Temperature process is enabled and Temperature <= Temp_T_HIGH --> Half Suction Speed
             if (Dosing_Half_Speed == TRUE) {
                pColorAct->speed_suction = pSettings->speed_suction / 2;
                pColorAct->speed_cycle_supply = pCurve->speed_value / 2;
                pColorAct->speed_cycle = pCurveSingle->speed_value / 2;
                pDispensation->speed_cycle = pColorAct->speed_cycle_supply / 2;
            }
            else {
                pColorAct->speed_suction = pSettings->speed_suction;
                pColorAct->speed_cycle_supply = pCurve->speed_value;
                pColorAct->speed_cycle = pCurveSingle->speed_value;
                pDispensation->speed_cycle = pColorAct->speed_cycle_supply;			
            }			
		  
            pColorAct->n_step_stroke = pSettings->n_step_stroke;
            pColorAct->n_step_continuous_end = pSettings->n_step_continuous_end;
            pColorAct->delay_EV_off = pSettings->delay_EV_off;
            // 2. additional settings from calibration curve
            pColorAct->algorithm = pCurve->algorithm;
            // Salviamo i valori di velocità nel supply mettiamo il continous mentre nell'altro mettiamo il single stroke
            // vado a calcolare il numero di cicli presenti nel dosaggio continous
            calcPosContinous(id);
/*            
            pColorAct->en_back_step = pCurveSingle->en_back_step;
            pColorAct->n_step_back_step = pCurveSingle->n_step_back_step;
    	    pColorAct->speed_back_step = pCurveSingle->speed_back_step;	
*/
            pColorAct->en_back_step = pCurve->en_back_step;
            pColorAct->n_step_back_step = pCurve->n_step_back_step;		
            pColorAct->speed_back_step = pCurve->speed_back_step;
            
            // 3. update data for DIAG_GET_LAST_DISPENSATION_PARAMS command reply 
            pDispensation->n_step_cycle = pColorAct->n_step_cycle_supply+pColorAct->n_step_cycle;
            pDispensation->n_cycles = pColorAct->n_cycles;
        }
    } // ndx != NO_SPEED_TABLE_AVAILABLE 
}

void setColorSupply(void)
/**/
/*==========================================================================*/
/**
**   @brief  Set color parameters for supply procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{   
    unsigned char index, id;
    // Volume di dispensazione per ogni circuito
    for (index = 0; index < N_SLAVES_COLOR_ACT; index++) {
        colorAct[index].vol_t = procGUI.colors_vol[index];
        procGUI.dispensation_colors_id[index] = 0;
    }
    procGUI.dispensation_colors_number = procGUI.used_colors_number;
    // Set up supply parameters for all relevant colors 
    for (index = 0; index < procGUI.used_colors_number; index ++) {
        id  = procGUI.used_colors_id[index];
        procGUI.dispensation_colors_id[index] = procGUI.used_colors_id[index]; 

        calcSupplyPar(colorAct[id].vol_t, color_supply_par[id].vol_mu, color_supply_par[id].vol_mc,id);
        
        if( (procGUI.circuit_pump_types[id] == PUMP_DOUBLE) && (id%2 != 0) )			  
            continue;
        if (isBaseCircuit(id)) {
            // Is this really useful? 
            stopColorAct(id);
        }
    }
    procGUI.simultaneous_colors_nr = 0;
    procGUI.simultaneous_colors_status = 0;
    procGUI.recirc_before_supply_status = 0;
    for (index = 1; index < N_SLAVES_COLOR_ACT; index++)
        Erogation_done[index] = 0;	     
}

void setColorSupplyBasesColorant(unsigned short Bases_Vol, unsigned char Colorant)
/**/
/*==========================================================================*/
/**
**   @brief  Set 20% of BASES color parameters for sypply 
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{    
    unsigned char index, id, i;
    for (index = 0; index < N_SLAVES_COLOR_ACT; index++)
        procGUI.dispensation_colors_id[index] = 0;
    
    procGUI.dispensation_colors_number = 0;

    for (i = 0; i < procGUI.used_colors_number; ++ i) {
        if ( (isBaseCircuit(procGUI.used_colors_id[i]) ) && (Bases_Vol != 0) )
            procGUI.dispensation_colors_number++;
        else if ( (!isBaseCircuit(procGUI.used_colors_id[i]) ) && (Colorant == TRUE) ) 
            procGUI.dispensation_colors_number++;
    }	
    
    index = 0;    
    for (i = 0; i < procGUI.used_colors_number; ++ i) {
        if ( ((isBaseCircuit(procGUI.used_colors_id[i]))  && (Bases_Vol != 0)) ||
             ((!isBaseCircuit(procGUI.used_colors_id[i])) && (Colorant == TRUE)) ) { 
            procGUI.dispensation_colors_id[index] = procGUI.used_colors_id[i];
            index++;		
        } 
    } 
    for (index = 0; index < N_SLAVES_COLOR_ACT; index++) 
      if ( (isBaseCircuit(index)) && (Bases_Vol != 0) )
          colorAct[index].vol_t = (procGUI.colors_vol[index] * Bases_Vol / 100);
      else if ( (!isBaseCircuit(index)) && (Colorant == TRUE) ) 
          colorAct[index].vol_t = procGUI.colors_vol[index];

    // Set up supply parameters for all relevant colors 
    for (index = 0; index < procGUI.dispensation_colors_number; index ++) {
        id  = procGUI.dispensation_colors_id[index];
        calcSupplyPar(colorAct[id].vol_t,color_supply_par[id].vol_mu,color_supply_par[id].vol_mc,id);
    }
    procGUI.simultaneous_colors_nr = 0;
    procGUI.simultaneous_colors_status = 0;
    for (i=1; i<N_SLAVES_COLOR_ACT; i++)
        Erogation_done[i] = 0;    
    
    if ( (Bases_Vol == 20) || (Bases_Vol == 50) || (Bases_Vol == 80))
      procGUI.recirc_before_supply_status = 0;
}

void initColorStandbyProcesses(void)
/**/
/*==========================================================================*/
/**
**   @brief  Reset recirc and stirring FSMs for all color actuators
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{    
    int i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        recirc_act_fsm[i] = PROC_IDLE;
        recirc_counter[i] = 0;
        stirring_act_fsm[i] = PROC_IDLE;
        stirring_counter[i] = 0;
//        if (isColorTintingModule(i) ) {
//            cleaning_act_fsm[i] = PROC_IDLE;
//            cleaning_counter[i] = 0;
//        }
    }
	stirring_counter_tinting = 0;
	stirring_act_fsm_tinting = PROC_IDLE;		
    tinting_ricirc_active = OFF;
    // This timer is used as timebase for standby ops 
    StopTimer(T_STANDBY_TIMEBASE);
    // These timers are used to introduce an additional delay before
    // starting each actuator. This is intended to reduce load peaks on the power supply
    StopTimer(T_STANDBY_RECIRC_STARTUP);
    StopTimer(T_STANDBY_STIRRING_STARTUP);
}

void resetStandbyProcesses(void)
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
    unsigned char id;
    int i;
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        id = procGUI.dispensation_colors_id[ i ];
        resetStandbyProcessesSingle(id);
    } 
}

void setColorRecirc(void)
/**/
/*==========================================================================*/
/**
**   @brief  Set color parameters for recirculation procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{    
    int i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        // If circuit 'i' is a BASE of an ODD DOUBLE GROUP  --> NO Ricirculation
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) {
            colorAct[i].n_step_cycle = 0;
            colorAct[i].recirc_pause = 0;
            colorAct[i].speed_cycle  = 0;
            colorAct[i].n_cycles     = 0;
        }        
        if (isColorantActEnabled(i)) {
          colorAct[i].n_step_cycle = color_supply_par[i].n_step_stroke;
          colorAct[i].recirc_pause = color_supply_par[i].recirc_pause;
          colorAct[i].speed_cycle = color_supply_par[i].speed_recirc;
          colorAct[i].n_cycles = 9999; // we don't really need to get this right
        }
        else {
          colorAct[i].n_step_cycle = 0;
          colorAct[i].recirc_pause = 0;
          colorAct[i].speed_cycle = 0;
          colorAct[i].n_cycles = 0;
        }
    }
} // setColorRecirc() 

static int countRunningRecirc()
{    
    int ret = 0;
    int i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isColorantActEnabled(i))
          continue;
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) 
          continue;
        if (isSkipStandbyForError(i))
          continue;
        if (recirc_act_fsm[i] == PROC_RUNNING)
          ++ ret;
    }
    return ret;
}

static unsigned short latestReadyRecirc()
{    
    int i;
    unsigned short ret = 0;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isColorantActEnabled(i))
          continue;
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) 
          continue;
        if (isSkipStandbyForError(i))
          continue;
        if (recirc_act_fsm[i] == PROC_READY && recirc_counter[i] > ret)
          ret = recirc_counter[i];
    }
    return ret; 
}

static int countRunningStirring()
{    
    int i, ret = 0;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isColorantActEnabled(i))
          continue;
        if (isSkipStandbyForError(i))
          continue;
        if (stirring_act_fsm[i] == PROC_RUNNING)
          ++ ret;
    }
    return ret;
}

static unsigned short latestReadyStirring()
{    
    int i;
    unsigned short ret = 0;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isColorantActEnabled(i))
          continue;
        if (isSkipStandbyForError(i))
          continue;
        if (stirring_act_fsm[i] == PROC_READY && stirring_counter[i] > ret)
          ret = stirring_counter[i];
    }
    return ret;
}

static void standByRecirculation()
{    
    int i;
    unsigned short limit;
    // No checks on enabled acts here, as this is not necessary 
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isColorantActEnabled(i))
          continue;
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) 
          continue;
        if (isSkipStandbyForError(i))
          continue;
        if (recirc_act_fsm[i] == PROC_RUNNING) {
            // Switching from RUNNING to IDLE? 
            limit = (unsigned short)
            color_supply_par[i].recirc_duration * CONV_MIN_SEC;
//limit = 5;
            if (recirc_counter[i] >= limit) {
                recirc_counter[i] = 0;
                recirc_act_fsm[i] = PROC_IDLE; // No additional check 
            }
        }
        else if (recirc_act_fsm[i] == PROC_IDLE) {
            // Switching from IDLE to READY? 
            limit = (unsigned short)color_supply_par[i].recirc_window * CONV_MIN_SEC * CONV_TIME_UNIT_MIN;
//limit = 2;
            if (recirc_counter[i] >= limit) {
              recirc_counter[i] = 0;
              recirc_act_fsm[i] = PROC_READY; // no additional check 
            }
        }
        else if (recirc_act_fsm[i] == PROC_READY) {
            // Slightly harder, transition only if all of the following conditions hold:
            //   (a) #active < #max ;
            //   (b) MAX(counter) <= counter ;
            //   (c) Timer T_STANDBY_STARTUP is elapsed#if defined _TINTING
            if (isColorTintingModule(i)) {
                // Condition for Starting Ricirculation: 
                // 1. N° Ricirculating Circuits < N_MAX_SIMULTANEOUS_ACTS
                // 2, NO Colorant Tinting module Ricirculation Active
                // 3. NO Colorant Tinting module Cleaning Active
                if (isTintingReady()) {
                    if ( (countRunningRecirc() < N_MAX_SIMULTANEOUS_ACTS) && (tinting_ricirc_active == OFF) && 
                          (TintingAct.Cleaning_status == 0x0000) && (latestReadyRecirc() <= recirc_counter[i]) ) {
                        if (StatusTimer(T_STANDBY_RECIRC_STARTUP) == T_HALTED) {
                            StartTimer(T_STANDBY_RECIRC_STARTUP);
                        }
                        else if (StatusTimer(T_STANDBY_RECIRC_STARTUP) == T_ELAPSED) {
                            StopTimer(T_STANDBY_RECIRC_STARTUP);
                            recirc_counter[i] = 0;
                            recirc_act_fsm[i] = PROC_RUNNING;
                            tinting_ricirc_active = ON;
                        }
                    }
                }
            }
            else {
                if (countRunningRecirc() < N_MAX_SIMULTANEOUS_ACTS && latestReadyRecirc() <= recirc_counter[i]) {				
                    if (StatusTimer(T_STANDBY_RECIRC_STARTUP) == T_HALTED) {
                        StartTimer(T_STANDBY_RECIRC_STARTUP);
                    }
                    else if (StatusTimer(T_STANDBY_RECIRC_STARTUP) == T_ELAPSED) {
                        StopTimer(T_STANDBY_RECIRC_STARTUP);
                        recirc_counter[i] = 0;
                        recirc_act_fsm[i] = PROC_RUNNING;
                    }
                }
            }	
        }
    } 
}


static void standByStirring()
{    
    int i;
    unsigned short limit;
    // No checks on enabled acts here, as this is not necessary 
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        if (isSkipStandbyForError(i))
            continue;
    	if (isColorTintingModule(i))
            continue;		        
        if (stirring_act_fsm[i] == PROC_RUNNING) {
            // Switching from RUNNING to IDLE? 
            limit = (unsigned short)color_supply_par[i].reshuffle_duration;
//limit = 10;            
            if (stirring_counter[i] >= limit) {
                stirring_counter[i] = 0;
                stirring_act_fsm[i] = PROC_IDLE; // no additional check 
            }
        }
        else if (stirring_act_fsm[i] == PROC_IDLE) {
            // Switching from IDLE to READY? 
            limit = (unsigned short) color_supply_par[i].reshuffle_window * CONV_MIN_SEC * CONV_TIME_UNIT_MIN;
//limit = 10;            
            // limit = 0;
            if (stirring_counter[i] >= limit) {
                stirring_counter[i] = 0;
                stirring_act_fsm[i] = PROC_READY; // no additional check 
            }
        }
        else if (stirring_act_fsm[i] == PROC_READY) {
            // Slightly harder, transition only if all of the following
            // conditions hold:
            // (a) #active < #max ;
            // (b) MAX(counter) <= counter ;
            // (c) Timer T_STANDBY_STARTUP is elapsed
            if (countRunningStirring() < N_MAX_SIMULTANEOUS_ACTS && latestReadyStirring() <= stirring_counter[i]) {
                if (StatusTimer(T_STANDBY_STIRRING_STARTUP) == T_HALTED)
                    StartTimer(T_STANDBY_STIRRING_STARTUP);

                else if (StatusTimer(T_STANDBY_STIRRING_STARTUP) == T_ELAPSED) {
                    StopTimer(T_STANDBY_STIRRING_STARTUP);
                    stirring_counter[i] = 0;
                    stirring_act_fsm[i] = PROC_RUNNING;
                }
            }
        }
    }   
}

static void standByActuatorsControl()
{   
    int i;
	unsigned char cmd;	
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        // Base Circuit
        if (isBaseCircuit(i)) {				
            if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) 
                continue;
            // Recirculation + stirring 
            if (recirc_act_fsm[i] == PROC_RUNNING &&
                stirring_act_fsm[i] == PROC_RUNNING)
                cmd = CMD_RESH_RECIRC;
            // Recirculation only 
            else if (recirc_act_fsm[i] == PROC_RUNNING)
                cmd = CMD_RECIRC;
            // Stirring only 
            else if (stirring_act_fsm[i] == PROC_RUNNING)
                cmd = CMD_RESH;
            // Idle 
            else
                cmd = CMD_STOP;				
            controlRecircStirringColor(i, cmd);
        }
        // Colorant Circuit
        else {
            cmd = CMD_IDLE;
            // recirculation only 
            if (recirc_act_fsm[i] == PROC_RUNNING)
                cmd = CMD_RECIRC;
            // cleaning  
//				else if (cleaning_act_fsm[i] == PROC_RUNNING)
//					cmd = CMD_TINTING_CLEAN;
            // recirculation finished 
            else if ((procGUI.recirc_status & (1L << i)) > 0)
                cmd = CMD_STOP;
            // cleaning finished 
            else if ((TintingAct.Cleaning_status & (1L << i)) > 0)
                cmd = CMD_STOP;

            if (cmd != CMD_IDLE)
                controlRecircStirringColor(i, cmd);
        }				
    }		
}

void standbyProcesses(void)
/**/
/*==========================================================================*/
/**
**   @brief  Circuit Standby management
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{   
    int i;
    if (StatusTimer(T_STANDBY_TIMEBASE) == T_HALTED) {
      StartTimer(T_STANDBY_TIMEBASE);
    }
    else if (StatusTimer(T_STANDBY_TIMEBASE) == T_ELAPSED) {
        // Increment periodic processes counters 
        for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
            if (! isColorantActEnabled(i))
                continue;
            if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) 
                continue;
            if (isSkipStandbyForError(i))
                continue;
            if (isColorTintingModule(i)) {
                ++ recirc_counter[i] ;
//                ++ cleaning_counter[i] ;			
            }
            else {
                ++ recirc_counter[i] ;			
                ++ stirring_counter[i] ;
            }
        }
        ++ stirring_counter_tinting;		
        StartTimer(T_STANDBY_TIMEBASE);
    }
    // Periodic sub-tasks during STANDBY 
    standByRecirculation();
    standByStirring();
    standByActuatorsControl();
}

static void startRecircBeforeSupply(unsigned char id)
/**/
/*===========================================================================*/
/**
**   @brief  Activation of recirculation or stirring or recirculation + stirring
**
**   @param  unsigned char circuit id
**
**   @return void
**/
/*===========================================================================*/
/**/
{   
    if (color_supply_par[id].recirc_before_dispensation_n_cycles) {
        setColorActMessage(RICIRCOLO_COLORE, id);
        colorAct[id].command.cmd = CMD_RECIRC;

        colorAct[id].n_step_cycle = color_supply_par[id].n_step_stroke;
        colorAct[id].recirc_pause = 0;
        colorAct[id].speed_cycle = color_supply_par[id].speed_recirc;
        colorAct[id].n_cycles = color_supply_par[id].recirc_before_dispensation_n_cycles;
    }
    else 
        procGUI.recirc_before_supply_status |= (1L << id);
}

static void startRecircAfterReset(unsigned char id)
/**/
/*===========================================================================*/
/**
**   @brief  Activation of recirculation or stirring or recirculation + stirring
**
**   @param  unsigned char circuit id
**
**   @return void
**/
/*===========================================================================*/
/**/
{  
    setColorActMessage(RICIRCOLO_COLORE, id);
    colorAct[id].command.cmd = CMD_RECIRC;

    colorAct[id].n_step_cycle = color_supply_par[id].n_step_stroke;
    colorAct[id].recirc_pause = 0;
    colorAct[id].speed_cycle = color_supply_par[id].speed_recirc;
    colorAct[id].n_cycles = 1;
}

unsigned char isAllCircuitsSupplyRun(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all required circuits are done with dispensing
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
	unsigned short i, erogation_completed = TRUE;
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        if (!isBaseCircuit(procGUI.dispensation_colors_id[i]) ) {	
            if (Erogation_done[procGUI.dispensation_colors_id[i]] == FALSE)
                erogation_completed = FALSE;
        }
    }
	return erogation_completed;
}

unsigned char isAllBasesRecircBeforeFilling(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all Bases recirculated befeore filling
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{  
    unsigned char i, idx;
    unsigned char ret = TRUE;
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        idx = procGUI.dispensation_colors_id[i];
        if (! isColorantActEnabled(idx))
            continue;
        // If circuit 'idx' is a BASE of an ODD DOUBLE GROUP --> Recirculation check on MASTER 'idx-1'
        if ( isBaseCircuit(idx) && (procGUI.circuit_pump_types[idx] == PUMP_DOUBLE) && (idx%2 != 0) ) 
            idx--;
        if (isBaseCircuit(idx) && ! isRecircBeforeSupplyDone(idx))
            ret = FALSE;
    }
    return ret;
}

unsigned char isAllCircuitsSupplyEnd(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if supply is end in all color circuits
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i, idx;
    unsigned char ret = TRUE;
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        idx = procGUI.dispensation_colors_id[i];
        if (! isColorantActEnabled(idx))
          continue;
        if (!isBaseCircuit(idx)) {
            if (Erogation_done[idx] == FALSE)
                return FALSE;
        }	
        // If circuit 'idx' is a BASE of an ODD DOUBLE GROUP --> check on MASTER 'idx-1'
        if ( isBaseCircuit(idx) && (procGUI.circuit_pump_types[idx] == PUMP_DOUBLE) && (idx%2 != 0) )
            idx--;
        if ( isBaseCircuit(idx) && (!isColorActSupplyEnd(idx)) )
            ret = FALSE;
    }
    return ret;
}

unsigned char isAllCircuitsSupplyHome(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all supply circuits are at home
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i, idx;
    unsigned char ret = TRUE;
    
    if ( isTintingEnabled()&& !isTintingReady() )
        return FALSE;

    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        idx = procGUI.dispensation_colors_id[i];
        if (! isColorantActEnabled(idx))
          continue;
/*
        if (!isBaseCircuit(idx)) {
            if (! isColorReadyTintingModule(idx)) 	
                return FALSE;
        }
*/	
        // If circuit 'idx' is a BASE of an ODD DOUBLE GROUP --> check on MASTER 'idx-1'
        if ( isBaseCircuit(idx) && (procGUI.circuit_pump_types[idx] == PUMP_DOUBLE) && (idx%2 != 0) )
            idx--;
        if ( isBaseCircuit(idx) && (!isColorActHoming(idx)) )
            ret = FALSE;
    }
    return ret;
}

void controlRecircStirringColor(unsigned char id, unsigned char type_cmd)
/**/
/*===========================================================================*/
/**
**   @brief  Recirculation and Stirring control function
**
**   @param  unsigned char circuit id
**
**   @param  unsigned char circuit cmd id
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char circuit_indx, i;

    if (isBaseCircuit(id)) {	
        // Stirring ON 
        if (type_cmd == CMD_RESH) {
            if (Double_Group_0 == id) {
                if (DoubleGoup_Stirring_st == OFF) {
                    StopTimer(T_WAIT_AIR_PUMP_TIME);
                    StartTimer(T_WAIT_AIR_PUMP_TIME);
                    //StartTimer(T_WAIT_STIRRING_ON);                    
                    SetDoubleGroupStirring(0, ON);                    
                }      
            }
            else if (Double_Group_1 == id) {
                //StartTimer(T_WAIT_STIRRING_ON);
                SetDoubleGroupStirring(1, ON);
            }                
            else {
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_RESH;
            }
        }
        // Ricirculation ON
        else if (type_cmd == CMD_RECIRC) {         
            if (Double_Group_0 == id) {
                if (DoubleGoup_Stirring_st == ON) {
                    StopTimer(T_WAIT_AIR_PUMP_TIME);
                    StartTimer(T_WAIT_AIR_PUMP_TIME);
                }
                //StopTimer(T_WAIT_STIRRING_ON);                
                SetDoubleGroupStirring(0, OFF);
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_RECIRC;		
            }
            else if (Double_Group_1 == id) {  
                //StopTimer(T_WAIT_STIRRING_ON);                
                SetDoubleGroupStirring(1, OFF);
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_RECIRC;		
            }
            else { 
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_RECIRC;			
            }		
        }
        // Stirring and Ricirculation ON
        else if (type_cmd == CMD_RESH_RECIRC) {
            if (Double_Group_0 == id) {  
                if (DoubleGoup_Stirring_st == OFF) {
                    StopTimer(T_WAIT_AIR_PUMP_TIME);
                    StartTimer(T_WAIT_AIR_PUMP_TIME);
                    //StartTimer(T_WAIT_STIRRING_ON);                    
                    SetDoubleGroupStirring(0, ON);                    
                }                      
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_RECIRC;		
            }
            else if (Double_Group_1 == id) {  
                //StartTimer(T_WAIT_STIRRING_ON);
                SetDoubleGroupStirring(1, ON);
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_RECIRC;		
            }
            else { 
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_RESH_RECIRC;			
            }		
        }
        // Stop Stirring and Ricirculation
        else if (type_cmd == CMD_STOP) {
            if (Double_Group_0 == id) {  
                if (DoubleGoup_Stirring_st == ON) {
                    StopTimer(T_WAIT_AIR_PUMP_TIME);
                    StartTimer(T_WAIT_AIR_PUMP_TIME);
                    //StopTimer(T_WAIT_STIRRING_ON);                                
                    SetDoubleGroupStirring(0, OFF);                    
                }                                      
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_STOP;		
            }
            else if (Double_Group_1 == id) {  
                //StopTimer(T_WAIT_STIRRING_ON);                                
                SetDoubleGroupStirring(1, OFF);
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_STOP;		
            }
            else { 
                setColorActMessage(AGITAZIONE_RICIRCOLO_COLORE, id);
                colorAct[id].command.cmd = CMD_STOP;			
            }				
        }	
    }
    // Colorant Tinting Module
    else {
        if (type_cmd == CMD_RECIRC) {
            // Start Ricirculation
            TintingStartRecirc(id);	
            #if defined NOLAB        
                TintingAct.Color_Id = 1;        
            #endif                
        }
        else if (type_cmd == CMD_RESH) {
            // Start Stirring
            TintingStartStirring();
        }
    }
    switch(type_cmd)
    {
        case CMD_TINTING_SETUP_OUTPUT:
            // All 8 - 23 possible Ricirculation colorants are OFF 
            procGUI.recirc_status   &= ~(0xFFFF00);
            // All 8 - 23 possible Cleaning colorants are OFF 
            TintingAct.Cleaning_status = 0x000000; 
            // All 8 - 23 possible Stirring colorants are ON	
			for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if (TintingAct.Table_Colorant_En[i] == TRUE)
                    procGUI.stirring_status |= (1L << (i + TINTING_COLORANT_OFFSET));
            } 
//            procGUI.stirring_status |= (0xFFFF00);
        break;
        case CMD_RECIRC:
            if(isColorTintingModule(id) ) {
                procGUI.recirc_status 	&= ~(0xFFFF00);
                procGUI.recirc_status   |= (1L << id);
                procGUI.stirring_status &= ~(0xFFFF00);
                // All 8 - 23 possible Cleaning colorants are OFF 
                TintingAct.Cleaning_status = 0x000000; 
            }
            else {
                procGUI.recirc_status   |= (1L << id);
                procGUI.stirring_status &= ~(1L << id);
                // 'id' is a DOUBLE GROUP : set recirc_status also on SLAVE circuit
                if (procGUI.circuit_pump_types[id] == PUMP_DOUBLE) {
                    procGUI.recirc_status   |= (1L << (id+1));
                    procGUI.stirring_status &= ~(1L << (id+1));
                    if ( (id == Double_Group_0) && (Double_Group_1 != 64) ) {
                        procGUI.recirc_status   |= (1L << Double_Group_1);
                        procGUI.stirring_status &= ~(1L << Double_Group_1);
                        procGUI.recirc_status   |= (1L << (Double_Group_1+1));
                        procGUI.stirring_status &= ~(1L << (Double_Group_1+1));                        
                    }
                    else if ( (id == Double_Group_1) && (Double_Group_0 != 64) ) {
                        procGUI.recirc_status   |= (1L << Double_Group_0);
                        procGUI.stirring_status &= ~(1L << Double_Group_0);
                        procGUI.recirc_status   |= (1L << (Double_Group_0+1));
                        procGUI.stirring_status &= ~(1L << (Double_Group_0+1));                        
                    } 
                }	
            }	
        break;
        case CMD_RESH:
            if(isColorTintingModule(id) ) {
                procGUI.recirc_status   &= ~(1L << id);
                for (circuit_indx = 0; circuit_indx < N_SLAVES_COLOR_ACT; circuit_indx++) {
                    if ( isColorTintingModule(circuit_indx) && isColorantActEnabled(circuit_indx) )				
                        procGUI.stirring_status |= (1L << circuit_indx);		
                }			
            }
            else {
                procGUI.recirc_status   &= ~(1L << id);
                procGUI.stirring_status |= (1L << id);
                if (procGUI.circuit_pump_types[id] == PUMP_DOUBLE) {
                    procGUI.recirc_status   &= ~(1L << (id+1));
                    procGUI.stirring_status |= (1L << (id+1));
                    if ( (id == Double_Group_0) && (Double_Group_1 != 64) ) {
                        procGUI.recirc_status   &= ~(1L << Double_Group_1);
                        procGUI.stirring_status |= (1L << Double_Group_1);
                        procGUI.recirc_status   &= ~(1L << (Double_Group_1+1));
                        procGUI.stirring_status |= (1L << (Double_Group_1+1));                        
                    }
                    else if ( (id == Double_Group_1) && (Double_Group_0 != 64) ) {
                        procGUI.recirc_status   &= ~(1L << Double_Group_0);
                        procGUI.stirring_status |= (1L << Double_Group_0);
                        procGUI.recirc_status   &= ~(1L << (Double_Group_0+1));
                        procGUI.stirring_status |= (1L << (Double_Group_0+1));                        
                    }                     
                }	
            }		
        break;
        case CMD_RESH_RECIRC:
            procGUI.recirc_status   |= (1L << id);
            procGUI.stirring_status |= (1L << id);
            if (procGUI.circuit_pump_types[id] == PUMP_DOUBLE) {
                procGUI.recirc_status   |= (1L << (id+1));
                procGUI.stirring_status |= (1L << (id+1));
                if ( (id == Double_Group_0) && (Double_Group_1 != 64) ) {
                    procGUI.recirc_status   |= (1L << Double_Group_1);
                    procGUI.stirring_status |= (1L << Double_Group_1);
                    procGUI.recirc_status   |= (1L << (Double_Group_1+1));
                    procGUI.stirring_status |= (1L << (Double_Group_1+1));                        
                }
                else if ( (id == Double_Group_1) && (Double_Group_0 != 64) ) {
                    procGUI.recirc_status   |= (1L << Double_Group_0);
                    procGUI.stirring_status |= (1L << Double_Group_0);
                    procGUI.recirc_status   |= (1L << (Double_Group_0+1));
                    procGUI.stirring_status |= (1L << (Double_Group_0+1));                        
                }                                     
            }	
        break;
        case CMD_STOP:
            if(isColorTintingModule(id) ) {
                procGUI.recirc_status   &= ~(1L << id);
                procGUI.stirring_status &= ~(1L << id);
                TintingStop();
//                TintingStopProcess();                
            }
            else {
                procGUI.recirc_status   &= ~(1L << id);
                procGUI.stirring_status &= ~(1L << id);
                if (procGUI.circuit_pump_types[id] == PUMP_DOUBLE) {
                    procGUI.recirc_status   &= ~(1L << (id+1));
                    procGUI.stirring_status &= ~(1L << (id+1));
                    if ( (id == Double_Group_0) && (Double_Group_1 != 64) ) {
                        procGUI.recirc_status   &= ~(1L << Double_Group_1);
                        procGUI.stirring_status &= ~(1L << Double_Group_1);
                        procGUI.recirc_status   &= ~(1L << (Double_Group_1+1));
                        procGUI.stirring_status &= ~(1L << (Double_Group_1+1));                        
                    }
                    else if ( (id == Double_Group_1) && (Double_Group_0 != 64) ) {
                        procGUI.recirc_status   &= ~(1L << Double_Group_0);
                        procGUI.stirring_status &= ~(1L << Double_Group_0);
                        procGUI.recirc_status   &= ~(1L << (Double_Group_0+1));
                        procGUI.stirring_status &= ~(1L << (Double_Group_0+1));                        
                    }                                                         
                }
//                stopColorAct(id);
            }
        break;
        default:
        break;
    }
}

void checkCircuitsDispensationAct(unsigned char canDispensing)
/**/
/*==========================================================================*/
/**
**   @brief Manage recirc before dispensation and max number of
**   simultaneous dispensing colors
**
**   @param uchar dispensing enable condition : FALSE if only RECIRC
**   is admitted
**
**   @return void
**/
/*==========================================================================*/
/**/
{
/*    
    unsigned char i, i_circuit;
    unsigned char j, ricirc_indx, erog_indx, master_slave;
    static unsigned char Tinting_Wait_End;
    
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        i_circuit = procGUI.dispensation_colors_id[i];
        // ------------------------------------------------------------------------------------------------------------------------	
        // Double Group
        if (isBaseCircuit(i_circuit) && (procGUI.circuit_pump_types[i_circuit] == PUMP_DOUBLE) ) {  
            // 'i_circuit' is a MASTER
            if (i_circuit % 2 == 0) {
                j = 0;
                master_slave = DOUBLE_GROUP_MASTER_NO_SLAVE;
                // Find if also a SLAVE is present on Formula
                while (j < procGUI.dispensation_colors_number) { 
                    if (procGUI.dispensation_colors_id[j] == (i_circuit+1))
                        // also SLAVE circuit 'i_circuit+1' is present in Formula
                        master_slave = DOUBLE_GROUP_MASTER_WITH_SLAVE;
                        j++;
                }	
                ricirc_indx = i_circuit;
                erog_indx   = i_circuit;
            }
            // 'i_circuit' is a SLAVE
            else {
                j = 0;
                master_slave = DOUBLE_GROUP_SLAVE_NO_MASTER;
                // Find if also a MASTER is present in Formula 
                while (j < procGUI.dispensation_colors_number) { 
                    if (procGUI.dispensation_colors_id[j] == (i_circuit-1))
                        // also MASTER circuit 'i_circuit-1' is present in Formula
                        master_slave = DOUBLE_GROUP_SLAVE_WITH_MASTER;
                    j++;
                }
                ricirc_indx = i_circuit-1;
                if (master_slave == DOUBLE_GROUP_SLAVE_NO_MASTER)
                    erog_indx = i_circuit;
                else
                    erog_indx = i_circuit-1;				
            }		
        }
        // Colorant or Single Base
        else {  
            ricirc_indx = i_circuit;
            erog_indx   = i_circuit;
            master_slave = COLORANT_SINGLE_BASE;
        }
        // ------------------------------------------------------------------------------------------------------------------------
        if (master_slave != DOUBLE_GROUP_SLAVE_WITH_MASTER) {
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
    // 'i_circuit' is a BASE
            if (!isColorTintingModule(i_circuit) ) {
                if (isColorActHoming(ricirc_indx)) {
                    // pre-dispensation recirculation not yet completed? 
                    if (isRecircBeforeSupplyDone(ricirc_indx) == FALSE) {
                        if (isBaseCircuit(ricirc_indx))
                            startRecircBeforeSupply(ricirc_indx);

                        else if ((master_slave == COLORANT_SINGLE_BASE) && (procGUI.simultaneous_colors_nr < procGUI.simultaneous_colors_max)) {
                            startRecircBeforeSupply(ricirc_indx);
                            if (! isSimultaneousColorDispensing(ricirc_indx)) {
                                procGUI.simultaneous_colors_status |= (1L << ricirc_indx);
                                procGUI.simultaneous_colors_nr++;
                            }
                        }
                    }
                    // recirculation is now complete, dispense? 
                    else if (canDispensing) {
                        calcSupplyPar(colorAct[erog_indx].vol_t,
                              color_supply_par[erog_indx].vol_mu,
                              color_supply_par[erog_indx].vol_mc,
                              erog_indx);

                        colorAct[erog_indx].algorithm = Erogation_Type[erog_indx];				
                        if ((colorAct[erog_indx].algorithm==ALG_SYMMETRIC_CONTINUOUS) || (colorAct[erog_indx].algorithm==ALG_ASYMMETRIC_CONTINUOUS)) {
                            colorAct[erog_indx].command.cmd = CMD_SUPPLY;							  
                            //  Colorant or Single Base
                            if (master_slave == COLORANT_SINGLE_BASE)
                                setColorActMessage(DISPENSAZIONE_COLORE_CONT, erog_indx);
                            // Double Group Master with NO Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_NO_SLAVE)
                                startSupplyColorContinuousDoubleGroup(erog_indx);
                            // Double Group Master with Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_WITH_SLAVE)
                                startSupplyColorContinuousDoubleGroup(erog_indx);
                            // Double Group Slave with NO Master in Formula
                            else
                                startSupplyColorContinuousDoubleGroup(erog_indx-1);				
                        }
                        else {
                            //  Colorant or Single Base
                            if (master_slave == COLORANT_SINGLE_BASE)
                                startSupplyColor(erog_indx);				
                            // Double Group Master with NO Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_NO_SLAVE)
                                startSupplyColorDoubleGroup(erog_indx);
                            // Double Group Master with Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_WITH_SLAVE)
                                startSupplyColorDoubleGroup(erog_indx);
                            // Double Group Slave with NO Master in Formula
                            else
                                startSupplyColorDoubleGroup(erog_indx-1);				
                        }
                    }	
                }
                // pre-dispensation recirculation started? 
                else if (isColorActRecircRun(ricirc_indx)) 
                    procGUI.recirc_before_supply_status |= (1L << ricirc_indx);

                // pre-dispensation recirculation stopped? 
                else if (isColorActRecircEnd(i_circuit)) {
                    procGUI.recirc_before_supply_status |= (1L << i_circuit);
                    stopColorAct(i_circuit);
                }
            }
    // 'i_circuit' is a TINTING COLORANT
            else {			
                if ( ((procGUI.simultaneous_colors_status & (1L << ricirc_indx)) > 0) ||  
                     ((procGUI.simultaneous_colors_nr == 0) && (Erogation_done[erog_indx] == FALSE)) ) {					 
                    //if (isTintingHoming()) {
                    if (isTintingReady()) {				
                        Tinting_Wait_End = 1;
                        // pre-dispensation recirculation not yet completed? 			
                        if (isRecircBeforeSupplyDone(ricirc_indx) == FALSE) {
                            TintingStartRecircBeforeSupply(ricirc_indx);
                            #if defined NOLAB        
                                TintingAct.Color_Id = 1;       
                            #endif                             
                            if (! isSimultaneousColorDispensing(ricirc_indx)) {
                                procGUI.simultaneous_colors_status |= (1L << ricirc_indx);
                                procGUI.simultaneous_colors_nr++;
                            }
                        }
                        // recirculation is now complete, dispense? 
                        else if (canDispensing) {

                            calcSupplyPar(colorAct[erog_indx].vol_t,
                                  color_supply_par[erog_indx].vol_mu,
                                  color_supply_par[erog_indx].vol_mc,
                                  erog_indx);

                            colorAct[erog_indx].algorithm = Erogation_Type[erog_indx];			  
                            if ((colorAct[erog_indx].algorithm==ALG_SYMMETRIC_CONTINUOUS) || (colorAct[erog_indx].algorithm==ALG_ASYMMETRIC_CONTINUOUS)) {
                                colorAct[erog_indx].algorithm = COLOR_ACT_STROKE_OPERATING_MODE; 
                                startSupplyContinuousTinting(erog_indx);
                                #if defined NOLAB        
                                    TintingAct.Color_Id = 1;       
                                #endif                                                                            
                            }
                            else {
                                //  Colorant 
                                startSupplyTinting(erog_indx);
                                #if defined NOLAB        
                                    TintingAct.Color_Id = 1;       
                                #endif                                            
                            }    
                        }
                    }
                    // pre-dispensation recirculation started? 
                    else if (isTintingRicircRun())
                        procGUI.recirc_before_supply_status |= (1L << ricirc_indx);

                    // pre-dispensation recirculation stopped? 
                    else if (isTintingRicircEnd()) {
                        procGUI.recirc_before_supply_status |= (1L << i_circuit);
                        TintingStop();
                    }
                    // dispensation ended? 
                    else if (isTintingSupplyEnd() && (Tinting_Wait_End == 1) ) {
                        TintingStop();
    //					erog_end = TRUE;
                        Tinting_Wait_End = 0;
                        Erogation_done[erog_indx] = TRUE;						
                        if (isSimultaneousColorDispensing(ricirc_indx)) {
                            procGUI.simultaneous_colors_status &= ~(1L << ricirc_indx);
                                if (procGUI.simultaneous_colors_nr)
                                    -- procGUI.simultaneous_colors_nr;
                        }					
                    }
                }	
            }		
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
        }
    }
*/
} // checkCircuitsDispensationAct() 

void intrCircuitAct(unsigned char first_id, unsigned char last_id)
/**/
/*===========================================================================*/
/**
**   @brief  Activation of reset procedure for bases and colors circuits
**
**   @param first and last circuit id
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char i;    
    for (i = first_id; i <= last_id; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP -
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
            continue;
        // reset_mode => ! homing || error 
        if (! procGUI.reset_mode || ! isColorActHoming(i) || isColorActError(i))
            intrColorAct(i);
    }
}

void resetCircuitAct(unsigned char first_id, unsigned char last_id)
/**/
/*===========================================================================*/
/**
**   @brief  Activation of reset procedure for bases and colors circuits
**
**   @param first and last circuit id
**
**   @return void
**/
/*===========================================================================*/
/**/
{  
    unsigned char i;
    for (i = first_id; i <= last_id; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP 
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
            continue;
        // reset_mode => ! homing || error 
        if (! procGUI.reset_mode  || ! isColorActHoming(i) || isColorActError(i))
            posHomingColorAct(i);
    }
}

void stopColorActs(unsigned char first_id, unsigned char last_id)
/**/
/*===========================================================================*/
/**
**   @brief Stops all color actuators in given range
**
**   @param first and last circuit id
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char i;
    for (i = first_id; i <= last_id; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP 
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
            continue;
        // Stop an actuator even if it signals error 
        stopColorAct(i);
    }
}

unsigned char isBasesMotorCircuitsRunning(void)
/**/
/*==========================================================================*/
/**
**   @brief Check if some Bases Enabled has motor active 
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{   
    unsigned char i;
    unsigned char ret = FALSE;

    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        // Base Circuit
        if (isBaseCircuit(i)) {				
            if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) 
                continue;
            
            if (attuatoreAttivo[i] == TRUE)                
                ret = TRUE;                
        }    
    }
    return ret;
}

unsigned char isCircuitsHoming(unsigned char first_id, unsigned char last_id)
/**/
/*==========================================================================*/
/**
**   @brief Check if all circuits are at homing position. Ignore
**   circuits that are either: (a) disabled, (b) in error;
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{   
    unsigned char i;
    unsigned char ret = TRUE;

    for (i = first_id; i <= last_id; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP 
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
            continue;
        if (isColorActError(i))
            continue;
        if (! isColorActHoming(i))
            ret = FALSE;
    }
    return ret;
}

unsigned char isCircuitsReady(unsigned char first_id, unsigned char last_id)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all the required circuits have started homing
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i;
    unsigned char ret = TRUE;
    for (i = first_id; i <= last_id; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP 
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) ) 
            continue;
        if (! procGUI.reset_mode  && ! isColorActReady(i))
            ret = FALSE;
    }
    return ret;
}

 unsigned char isCircuitsStartedHoming(unsigned char first_id, unsigned char last_id)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all the required circuits have started homing
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i;
    unsigned char ret = TRUE;

    for (i = first_id; i <= last_id; ++ i) {
        if (! isColorantActEnabled(i))
            continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP 
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
            continue;
        // Ignore idle circuits (error state?) 
        if (! isColorCmdHoming(i))
           continue;
        if (isColorActStopped(i)) 
            ret = FALSE;         
        else 
            // Prevent continuous resets on this slave and signals pending recirculation after reset 
            checkHomingColorAct(i);
    }
    return ret;
}

unsigned char isCircuitsRecircEnd(unsigned char first_id, unsigned char last_id)
/**/
/*==========================================================================*/
/**
**   @brief Check if recirculation is terminated in all color circuits
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{ 
    unsigned char i;
    unsigned char ret = TRUE;
    for (i = first_id; i <= last_id; ++ i) {
        if (! isColorantActEnabled(i))
          continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP 
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
          continue;        
        if (isColorActError(i))
          continue;
        // Ignore actuators for which there is no running recirculation 
        if (! isColorCmdRec(i))
          continue;
        // The actual predicate 
        if (! isColorActRecircEnd(i))
          ret = FALSE;
    }
    return ret;
}

void recircResetCircuitAct(unsigned char first_id, unsigned char last_id)
/**/
/*===========================================================================*/
/**
**   @brief  Activation of reset procedure for bases and colors circuits
**
**   @param first and last circuit id
**
**   @return void
**/
/*===========================================================================*/
/**/
{ 
    unsigned char i;
    for (i = first_id; i <= last_id; ++ i) {
        // Ignore disabled actuators 
        if (! isColorantActEnabled(i))
            continue;
        // Circuit i' is a BASE of an ODD DOUBLE GROUP 
        if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
            continue;        
        // Ignore actuators for which there is no pending recirc 
        if (! isResetRecircPending(i))
            continue;
        // Perform one recirculation cycle after reset 
        startRecircAfterReset(i);
    }
}

unsigned char isAllCircuitsStopped(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all circuits are stopped
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i;
    unsigned char ret = TRUE;
    
    if (!isTintingStopped() ) 
        ret = FALSE;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (isColorCircuit(i) )
            continue;
        if (!isColorantActEnabled(i) )
            continue;	
        if( (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
            continue;
        if (! isColorActStopped(i))
            ret = FALSE;
    }
    return ret;
}

void stopAllCircuitsAct(void)
/**/
/*==========================================================================*/
/**
**   @brief  Stop all color actuators
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char i;
    
    if (Stop_Process == TRUE)
        TintingStopProcess();
    else if (procGUI.typeMessage != DIAG_MOVIMENTAZIONE_AUTOCAP)
        TintingStop();    
	
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (isColorTintingModule(i))
            continue;
        if (!isColorantActEnabled(i) )
          continue;	
        if( (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
          continue;

        setColorActMessage(CONTROLLO_PRESENZA, i);        
        colorAct[i].command.cmd = CMD_STOP;              
        procGUI.recirc_status &= ~(1L << i);            
        procGUI.stirring_status &= ~(1L << i);         
        if (Double_Group_0 == i) { 			
            procGUI.recirc_status &= ~(1L << (i+1));     
            procGUI.stirring_status &= ~(1L << (i+1));
            if (Double_Group_1 != 64) {
                procGUI.recirc_status &= ~(1L << Double_Group_1);            
                procGUI.stirring_status &= ~(1L << Double_Group_1);         
                procGUI.recirc_status &= ~(1L << (Double_Group_1+1));     
                procGUI.stirring_status &= ~(1L << (Double_Group_1+1));   
            } 
            if (DoubleGoup_Stirring_st == ON) {
                StopTimer(T_WAIT_AIR_PUMP_TIME);
                StartTimer(T_WAIT_AIR_PUMP_TIME);
            }
            //StopTimer(T_WAIT_STIRRING_ON);                            
            SetDoubleGroupStirring(0, OFF);	
        } 								
        else if (Double_Group_1 == i) { 		
            procGUI.recirc_status &= ~(1L << (i+1)); 
            procGUI.stirring_status &= ~(1L << (i+1));    
            if (Double_Group_0 != 64) {
                procGUI.recirc_status &= ~(1L << Double_Group_0);            
                procGUI.stirring_status &= ~(1L << Double_Group_0);         
                procGUI.recirc_status &= ~(1L << (Double_Group_0+1));     
                procGUI.stirring_status &= ~(1L << (Double_Group_0+1));   
            }
            //StopTimer(T_WAIT_STIRRING_ON);                            
            SetDoubleGroupStirring(1, OFF);		
        } 												
    }
}

unsigned char isAllCircuitsHome(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all supply circuits are at home
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i, idx;
    unsigned char ret = TRUE;

    if ( isTintingEnabled()&& !isTintingReady() )
        return FALSE;
    
    for (idx = 0; idx < N_SLAVES_COLOR_ACT; ++ idx) {
        if (! isColorantActEnabled(idx))
            continue;
        if (!isBaseCircuit(idx))
            continue;            
/*        
        if (!isBaseCircuit(idx)) {
            if (! isColorReadyTintingModule(idx)) 	
                return FALSE;
        }	
*/
        i = idx;
        // If circuit 'idx' is a BASE of an ODD DOUBLE GROUP --> check on MASTER 'idx-1'
        if ( isBaseCircuit(idx) && (procGUI.circuit_pump_types[idx] == PUMP_DOUBLE) && (idx%2 != 0) )
            i--;
        if ( isBaseCircuit(i) && (!isColorActHoming(i)) )
            ret = FALSE;
    }
    
    return ret;
}

unsigned char checkAllRequiredCircuitsEnabled()
/**/
/*==========================================================================*/
/**
**   @brief Check that every circuit which is required for the
**   dispensation is actually enabled. Raises an ALARM otherwise.
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i, idx;
    unsigned char ret = TRUE;
    for (i = 0; i < procGUI.used_colors_number; ++ i) {
        idx = procGUI.used_colors_id[i];
        if (! isColorantActEnabled(idx)) {
            setAlarm(DISABLED_REQUIRED_CIRCUIT_0_ERROR + idx);
            ret = FALSE;
            // Any other ALARM would be discarded anyway...
            break;
        }
    }
    return ret;
}

unsigned char isDiagColorCircuitEn(unsigned char circuitID)
/**/
/*==========================================================================*/
/**
**   @brief Check if circuit is both (a) enabled, (b) used in for
**   DIAG_ operation
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    return isColorantActEnabled(circuitID) && (procGUI.diag_color_en & (1L << circuitID)) ;
}

void initColorDiagProcesses(void)
/**/
/*==========================================================================*/
/**
**   @brief  Reset recirc and stirring FSMs for all color actuators
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{ 
    int i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        diag_recirc_act_fsm[i]   = PROC_IDLE;
        diag_stirring_act_fsm[i] = PROC_IDLE;
    }
}

static void diagActuatorsControl(void)
{    
    int i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        unsigned char cmd = CMD_IDLE;
        if (! isColorantActEnabled(i))
          continue;
        if (isSkipStandbyForError(i))
          continue;
        if ( (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 != 0) )
          continue;
        if (isColorTintingModule(i)) {
            if (! isDiagColorCircuitEn(i))
              continue;

            // Recirculation + stirring is NOT possible on Tinting Module
            if (diag_recirc_act_fsm[i] == PROC_RUNNING &&
                diag_stirring_act_fsm[i] == PROC_RUNNING) {
                diag_recirc_act_fsm[i]   = PROC_IDLE;
                diag_stirring_act_fsm[i] = PROC_IDLE;
            }
            // Recirculation it is possible on Tinting Module if it is READY, or a RICIRCULATION process is Acitve or just finished, or a CLEANING process is Active
            else if (diag_recirc_act_fsm[i] == PROC_RUNNING) {
                if (isTintingReady() || isTintingRicircEnd() )
                    cmd = CMD_RECIRC;
                else 
                    diag_recirc_act_fsm[i] = PROC_IDLE;
            }
            // Stirring it is possible on Tinting Module if it is READY, or a RICIRCULATION process is Acitve or just finished, or a CLEANING process is Active
            else if (diag_stirring_act_fsm[i] == PROC_RUNNING) {
                if (isTintingReady() || isTintingRicircEnd() )
                    cmd = CMD_RESH;
                else 
                    diag_stirring_act_fsm[i] = PROC_IDLE;
            }
            // Idle 
            else
               cmd = CMD_STOP;
        }
        else {
            // Recirculation + stirring 
            if (diag_recirc_act_fsm[i] == PROC_RUNNING &&
                diag_stirring_act_fsm[i] == PROC_RUNNING)
              cmd = CMD_RESH_RECIRC;
            // Recirculation only 
            else if (diag_recirc_act_fsm[i] == PROC_RUNNING)
              cmd = CMD_RECIRC;
            // Stirring only 
            else if (diag_stirring_act_fsm[i] == PROC_RUNNING)
              cmd = CMD_RESH;
            // Idle 
            else
                cmd = CMD_STOP;
        }	
        controlRecircStirringColor(i, cmd);
    }
}

void DiagColorSupply(void)
/**/
/*==========================================================================*/
/**
**   @brief  Diag color supply procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{   
    unsigned char i,j;
    
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isDiagColorCircuitEn(i))
          continue;
        // Find a Double Group with odd index (1,3,5,7,) : --> set MASTER index j = i - 1 
        if ( (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i % 2 != 0) ) 	
            j = i - 1;
        else 
            j = i;

        if (procGUI.command == OFF) {
            // Colorant Circuit
            if ( (j >= 8) && (j <= 31) )
                TintingStop();
            // Base Circuit
            else if (j < 8)
                stopColorAct(j);
        }
        else {
            if ( isColorReadyTintingModule(j) || ((j < 8) && isColorActHoming(j)) ) {
                // Filippo - check for continous dispensation - I have to verify that the step number was a multiple of the stroke length
                if ((colorAct[i].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[i].algorithm == ALG_ASYMMETRIC_CONTINUOUS)){
                    // Devo verificare che il numero di passi da fare sia un multiplo della corsa totale
                    if ((colorAct[i].n_step_cycle_supply % colorAct[i].n_step_stroke) != 0)
                        setAlarm(B1_SUPPLY_CALC_ERROR+i);
                    else {	
                        // I have to calculate the start position and the stop position and send the right command to the actuator
                        calcPosContinous(i);
                        colorAct[i].command.cmd = CMD_SUPPLY;	
                        colorAct[i].n_step_cycle=0;
                        if (procGUI.circuit_pump_types[i] != PUMP_DOUBLE)  {
                            // Colorant Circuit
                            if ( (i >= 8) && (i <= 31) ) {
                                colorAct[i].n_cycles = 0;	
                                startSupplyContinuousTinting(i);
                                #if defined NOLAB        
                                    TintingAct.Color_Id = 1;       
                                #endif                                                                                                            
                            }					
                            // Base Circuit
                            else if (i < 8)
                                setColorActMessage(DISPENSAZIONE_COLORE_CONT, i);						
                        }
                        // Find a Double Group with even index (0,2,4,6,)
                        else if ( (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i % 2 == 0) ) 	
                            startSupplyColorContinuousDoubleGroupMaster(i);
                        // Find a Double Group with odd index (1,3,5,7,)
                        else
                            startSupplyColorContinuousDoubleGroupSlave(i);
                    }
                }
                else {
                    if (procGUI.circuit_pump_types[i] != PUMP_DOUBLE) {
                        // Colorant Circuit
                        if ( (i >= 8) && (i <= 31) ) {
                            startSupplyTinting(i);
                            #if defined NOLAB        
                                TintingAct.Color_Id = 1;       
                            #endif                                                                        
                        }    
                        // Base Circuit
                        else if (i < 8)
                            startSupplyColor(i);
                    } 	
                    // Find a Double Group with even index (0,2,4,6,)
                    else if ( (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i % 2 == 0) ) 	
                        startSupplyColorDoubleGroupMaster(i);
                    // Find a Double Group with odd index (1,3,5,7,)
                   else
                        startSupplyColorDoubleGroupSlave(i);
                }
            }  
        }
    } 
}

void DiagColorRecirc()
/**/
/*==========================================================================*/
/**
**   @brief  Diag color recirculation and reshuffle procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{    
    unsigned char i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
      if (! isDiagColorCircuitEn(i))
        continue;
      diag_recirc_act_fsm[i] = (procGUI.command == ON)
        ? PROC_RUNNING
        : PROC_IDLE
        ;
    }
    diagActuatorsControl();
}

void DiagColorReshuffle(void)
/**/
/*==========================================================================*/
/**
**   @brief  Diag color reshuffle procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isDiagColorCircuitEn(i))
            continue;
        diag_stirring_act_fsm[i] = (procGUI.command == ON)
            ? PROC_RUNNING
            : PROC_IDLE
            ;
    }
    diagActuatorsControl();
}

void DiagColorClean (void)
/**/
/*==========================================================================*/
/**
**   @brief  Diag Clean procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    TintingPuliziaTavola();
    if (procGUI.command == ON) {
        if (Punctual_Clean_Act == OFF) {
            Punctual_Cleaning = ON;
            Punctual_Clean_Act = ON;
        } 
    }        
    else {
        // All 8 - 23 possible Cleaning colorants are OFF 
        TintingAct.Cleaning_status = 0x000000; 
        Punctual_Cleaning = OFF;        
   }     
}

void DiagColorSet_EV(void)
/**/
/*==========================================================================*/
/**
**   @brief  Diag color set EV dispensation procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (! isDiagColorCircuitEn(i))
            continue;
        if (procGUI.command == OFF)
            stopColorAct(i);
        else if (isColorActHoming(i))
            startSet_EV(i);
    }
}

void checkDiagColorSupplyEnd(void)
/**/
/*==========================================================================*/
/**
**   @brief  Stop color circuit at the end of supply procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{   
    unsigned char i;
    if (isColorSupllyEndTintingModule() )
		TintingStop();
  
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
    	if ( (isBaseCircuit(i) ) && isColorActSupplyEnd(i) ) 
            stopColorAct(i);
    }
}

void checkDiagColorRecircEnd(void)
/**/
/*==========================================================================*/
/**
**   @brief  Stop color circuit at the end of recirc procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{    
    unsigned char i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        if (isBaseCircuit(i) && isColorActRecircEnd(i))
            stopColorAct(i);
    }
}

void countDoubleGroupAct(void)
/**/
/*==========================================================================*/
/**
**   @brief  Find Double Group0 and Group1
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
	unsigned char i;	
	for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
		if (! isColorantActEnabled(i))
			continue;
		else if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 == 0) && (Double_Group_0 == 64) ) 
			Double_Group_0 = i;
		else if ( isBaseCircuit(i) && (procGUI.circuit_pump_types[i] == PUMP_DOUBLE) && (i%2 == 0) && (Double_Group_0 != 64) &&  (Double_Group_1 == 64) ) 
			Double_Group_1 = i;
	}		
}

void DoubleGroupContinuousManagement(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check and Management Double Groups Continuous and Single Stroke
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char index, id;
    // Set up supply parameters for all relevant colors 
    for (index = 0; index < procGUI.used_colors_number; index ++) {
        id  = procGUI.used_colors_id[index];
        // Find a Double Group with even index (0,2,4,6,) with amount > 0 
        if ( (procGUI.circuit_pump_types[id] == PUMP_DOUBLE) && (id%2 == 0) && (colorAct[id].vol_t > 0) ) { 	
            // Check if also odd index is present in Formula
            // --------------------------------------------------------------------------------------------------------------------------------------
//          if (procGUI.colors_vol[id+1] > 0) { 
            if (colorAct[id+1].vol_t> 0) { 
                // CONTINUOUS: MASTER and SLAVE
                if ( ( (colorAct[id].algorithm == ALG_SYMMETRIC_CONTINUOUS)     || (colorAct[id].algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) &&
                     ( (colorAct[id+1].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[id+1].algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) ) {
                    colorAct[id].algorithm = ALG_DOUBLE_GROUP_CONTINUOUS;
                    Erogation_Type[id] = ALG_SYMMETRIC_CONTINUOUS;				
                    colorAct[id].posStart_A = colorAct[id].posStart; 			
                    colorAct[id].posStop_A  = colorAct[id].posStop; 			
                    colorAct[id].speedContinous_A = colorAct[id].speed_cycle_supply; 
                    colorAct[id].numCicliDosaggio_A = colorAct[id].numCicliDosaggio; 
                    colorAct[id].n_step_cycle_channel_A = colorAct[id].n_step_cycle; 
                    colorAct[id].speed_cycle_channel_A = colorAct[id].speed_cycle;
                    colorAct[id].n_cycles_channel_A = colorAct[id].n_cycles; 	
                    colorAct[id].posStart_B = colorAct[id+1].posStart; 			
                    colorAct[id].posStop_B  = colorAct[id+1].posStop; 			
                    colorAct[id].speedContinous_B = colorAct[id+1].speed_cycle_supply;
                    colorAct[id].numCicliDosaggio_B = colorAct[id+1].numCicliDosaggio;
                    colorAct[id].n_step_cycle_channel_B = colorAct[id+1].n_step_cycle;
                    colorAct[id].speed_cycle_channel_B = colorAct[id+1].speed_cycle; 
                    colorAct[id].n_cycles_channel_B = colorAct[id+1].n_cycles;
    			}
                // CONTINUOUS: MASTER 
                else if ( (colorAct[id].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[id].algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) {			
                    colorAct[id].algorithm  = ALG_DOUBLE_GROUP_CONTINUOUS;				
                    Erogation_Type[id] = ALG_SYMMETRIC_CONTINUOUS;				
                    colorAct[id].posStart_A = colorAct[id].posStart; 					
                    colorAct[id].posStop_A  = colorAct[id].posStop; 			
                    colorAct[id].speedContinous_A = colorAct[id].speed_cycle_supply; 	
                    colorAct[id].numCicliDosaggio_A = colorAct[id].numCicliDosaggio; 	
                    colorAct[id].n_step_cycle_channel_A = colorAct[id].n_step_cycle; 	
                    colorAct[id].speed_cycle_channel_A = colorAct[id].speed_cycle;
                    colorAct[id].n_cycles_channel_A = colorAct[id].n_cycles; 
                    colorAct[id].posStart_B = 0; 					
                    colorAct[id].posStop_B  = 0; 		
                    colorAct[id].speedContinous_B = colorAct[id].speed_suction;
                    colorAct[id].numCicliDosaggio_B = 0;
                    colorAct[id].n_step_cycle_channel_B = colorAct[id+1].n_step_cycle_supply; 
                    colorAct[id].speed_cycle_channel_B = colorAct[id+1].speed_cycle_supply; 
                    colorAct[id].n_cycles_channel_B = colorAct[id+1].n_cycles_supply;
    			}
                // CONTINUOUS: SLAVE
                else if ( (colorAct[id+1].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[id+1].algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) {			
                    colorAct[id].algorithm  = ALG_DOUBLE_GROUP_CONTINUOUS;				
                    Erogation_Type[id] = ALG_SYMMETRIC_CONTINUOUS;				
                    colorAct[id].posStart_A = 0; 			
                    colorAct[id].posStop_A  = 0; 			
                    colorAct[id].speedContinous_A = colorAct[id].speed_suction;; 	
                    colorAct[id].numCicliDosaggio_A = 0; 
                    colorAct[id].n_step_cycle_channel_A = colorAct[id].n_step_cycle_supply;
                    colorAct[id].speed_cycle_channel_A = colorAct[id].speed_cycle_supply; 
                    colorAct[id].n_cycles_channel_A = colorAct[id].n_cycles_supply; 	
                    colorAct[id].posStart_B = colorAct[id+1].posStart; 					
                    colorAct[id].posStop_B  = colorAct[id+1].posStop; 		
                    colorAct[id].speedContinous_B = colorAct[id+1].speed_cycle_supply;
                    colorAct[id].numCicliDosaggio_B = colorAct[id+1].numCicliDosaggio;
                    colorAct[id].n_step_cycle_channel_B = colorAct[id+1].n_step_cycle; 
                    colorAct[id].speed_cycle_channel_B = colorAct[id+1].speed_cycle; 
                    colorAct[id].n_cycles_channel_B = colorAct[id+1].n_cycles;
    			}
                // SINGLE STROKE: MASTER & SLAVE
                else  {	
                    colorAct[id].algorithm = ALG_DOUBLE_GROUP;
                    Erogation_Type[id] = ALG_DOUBLE_GROUP;				
                    colorAct[id].speed_cycle_channel_A	= colorAct[id].speed_cycle_supply;		
                    colorAct[id].n_step_cycle_channel_A = colorAct[id].n_step_cycle_supply;
                    colorAct[id].n_cycles_channel_A = colorAct[id].n_cycles_supply;
                    colorAct[id].speed_cycle_channel_B	= colorAct[id+1].speed_cycle_supply;		
                    colorAct[id].n_step_cycle_channel_B = colorAct[id+1].n_step_cycle_supply;
                    colorAct[id].n_cycles_channel_B = colorAct[id+1].n_cycles_supply;				
                }
            }	
            // --------------------------------------------------------------------------------------------------------------------------------------
            // Only MASTER is present
            else {	
                // CONTINUOUS: MASTER 
                if ( (colorAct[id].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[id].algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) {			
                    colorAct[id].algorithm  = ALG_DOUBLE_GROUP_CONTINUOUS;				
                    Erogation_Type[id] = ALG_SYMMETRIC_CONTINUOUS;				
                    colorAct[id].posStart_A = colorAct[id].posStart; 					
                    colorAct[id].posStop_A  = colorAct[id].posStop; 			
                    colorAct[id].speedContinous_A = colorAct[id].speed_cycle_supply; 	
                    colorAct[id].numCicliDosaggio_A = colorAct[id].numCicliDosaggio; 	
                    colorAct[id].n_step_cycle_channel_A = colorAct[id].n_step_cycle; 	
                    colorAct[id].speed_cycle_channel_A = colorAct[id].speed_cycle;
                    colorAct[id].n_cycles_channel_A = colorAct[id].n_cycles; 
                    colorAct[id].posStart_B = 0; 					
                    colorAct[id].posStop_B  = 0; 		
                    colorAct[id].speedContinous_B = colorAct[id].speed_suction;
                    colorAct[id].numCicliDosaggio_B = 0;
                    colorAct[id].n_step_cycle_channel_B = 0; 
                    colorAct[id].speed_cycle_channel_B = colorAct[id].speed_suction; 
                    colorAct[id].n_cycles_channel_B = 0;
    			}
                // SINGLE STROKE: MASTER 
                else {
                    colorAct[id].algorithm = ALG_DOUBLE_GROUP;
                    Erogation_Type[id] = ALG_DOUBLE_GROUP;				
                    colorAct[id].n_step_stroke = colorAct[id].n_step_stroke;
                    colorAct[id].speed_cycle_channel_A	= colorAct[id].speed_cycle_supply;		
                    colorAct[id].n_step_cycle_channel_A = colorAct[id].n_step_cycle_supply;
                    colorAct[id].n_cycles_channel_A = colorAct[id].n_cycles_supply;
                    colorAct[id].speed_cycle_channel_B	= colorAct[id].speed_suction;		
                    colorAct[id].n_step_cycle_channel_B = 0;
                    colorAct[id].n_cycles_channel_B = 0;								
                }				
            }	
        }	
        // Find a Double Group with odd index (1,3,5,7) with amount > 0 where its even index has an amount = 0
        else if ( (procGUI.circuit_pump_types[id] == PUMP_DOUBLE) && (id%2 != 0) && (colorAct[id-1].vol_t == 0) ) {
            // CONTINUOUS: SLAVE 
            if ( (colorAct[id].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[id].algorithm == ALG_ASYMMETRIC_CONTINUOUS) ) {	
                colorAct[id-1].algorithm  = ALG_DOUBLE_GROUP_CONTINUOUS;				
                Erogation_Type[id] = ALG_SYMMETRIC_CONTINUOUS;				
                colorAct[id-1].n_step_stroke = colorAct[id].n_step_stroke;
                colorAct[id-1].posStart_A = 0; 			
                colorAct[id-1].posStop_A  = 0; 			
                colorAct[id-1].speedContinous_A   = colorAct[id].speed_suction; 	
                colorAct[id-1].numCicliDosaggio_A = 0; 
                colorAct[id-1].n_step_cycle_channel_A = 0; 
                colorAct[id-1].speed_cycle_channel_A = colorAct[id].speed_suction; 
                colorAct[id-1].n_cycles_channel_A = 0; 	
                colorAct[id-1].posStart_B = colorAct[id].posStart; 					
                colorAct[id-1].posStop_B  = colorAct[id].posStop; 		
                colorAct[id-1].speedContinous_B = colorAct[id].speed_cycle_supply;
                colorAct[id-1].numCicliDosaggio_B = colorAct[id].numCicliDosaggio;
                colorAct[id-1].n_step_cycle_channel_B = colorAct[id].n_step_cycle; 
                colorAct[id-1].speed_cycle_channel_B = colorAct[id].speed_cycle; 
                colorAct[id-1].n_cycles_channel_B = colorAct[id].n_cycles; 						
    	    }
            // SINGLE STROKE: SLAVE
            else {
                colorAct[id-1].algorithm = ALG_DOUBLE_GROUP;
                Erogation_Type[id] = ALG_DOUBLE_GROUP;				
                colorAct[id-1].n_step_stroke = colorAct[id].n_step_stroke;
                colorAct[id-1].speed_cycle_channel_A  = colorAct[id].speed_suction;		
                colorAct[id-1].n_step_cycle_channel_A = 0;
                colorAct[id-1].n_cycles_channel_A = 0;
                colorAct[id-1].speed_cycle_channel_B  = colorAct[id].speed_cycle_supply;		
                colorAct[id-1].n_step_cycle_channel_B = colorAct[id].n_step_cycle_supply;
                colorAct[id-1].n_cycles_channel_B = colorAct[id].n_cycles_supply;				
            }
        }
        else if  (procGUI.circuit_pump_types[id] != PUMP_DOUBLE) { 
            if (colorAct[id].vol_t > 0)
                Erogation_Type[id] = colorAct[id].algorithm;
    	}
    }
}

unsigned char isFormulaColorants(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if Formula contains Colorants 
**
**   @param  void
**
**   @return TRUE = Formula has Colorants / FALSE = NO Colorants in Formula
**/
/*==========================================================================*/
/**/
{
	unsigned char  index, id;
	
	for (index = 0; index < procGUI.used_colors_number; index ++) {
		id  = procGUI.used_colors_id[index];
		if (!isBaseCircuit(id) && (colorAct[id].vol_t > 0) )
			return TRUE;		
	}
	return FALSE;	
}

unsigned char isFormulaBases(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if Formula contains Bases 
**
**   @param  void
**
**   @return TRUE = Formula has Bases / FALSE = NO Bases in Formula
**/
/*==========================================================================*/
/**/
{
	unsigned char  index, id;
	
	for (index = 0; index < procGUI.used_colors_number; index ++) {
		id  = procGUI.used_colors_id[index];
		if (isBaseCircuit(id) && (colorAct[id].vol_t > 0))
			return TRUE;		
	}
	return FALSE;	
}

void checkBasesDispensationAct(unsigned char canDispensing)
/**/
/*==========================================================================*/
/**
**   @brief Manage recirc before dispensation and max number of
**   simultaneous dispensing colors
**
**   @param uchar dispensing enable condition : FALSE if only RECIRC
**   is admitted
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char i, i_circuit;
    unsigned char j, ricirc_indx, erog_indx, master_slave;
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        i_circuit = procGUI.dispensation_colors_id[i];
    // -------------------------------------------------------------------------
        // Double Group
        if (isBaseCircuit(i_circuit) && (procGUI.circuit_pump_types[i_circuit] == PUMP_DOUBLE) ) {  
            // 'i_circuit' is a MASTER
            if (i_circuit % 2 == 0) {
                j = 0;
                master_slave = DOUBLE_GROUP_MASTER_NO_SLAVE;
                // Find if also a SLAVE is present on Formula
                while (j < procGUI.dispensation_colors_number) { 
                    if (procGUI.dispensation_colors_id[j] == (i_circuit+1))
                        // also SLAVE circuit 'i_circuit+1' is present in Formula
                        master_slave = DOUBLE_GROUP_MASTER_WITH_SLAVE;
                        j++;
                }	
                ricirc_indx = i_circuit;
                erog_indx   = i_circuit;
            }
            // 'i_circuit' is a SLAVE
            else {
                j = 0;
                master_slave = DOUBLE_GROUP_SLAVE_NO_MASTER;
                // Find if also a MASTER is present in Formula 
                while (j < procGUI.dispensation_colors_number) { 
                    if (procGUI.dispensation_colors_id[j] == (i_circuit-1))
                        // also MASTER circuit 'i_circuit-1' is present in Formula
                        master_slave = DOUBLE_GROUP_SLAVE_WITH_MASTER;
                    j++;
                }
                ricirc_indx = i_circuit-1;
                if (master_slave == DOUBLE_GROUP_SLAVE_NO_MASTER)
                    erog_indx = i_circuit;
                else
                    erog_indx = i_circuit-1;				
            }		
        }
        // Colorant or Single Base
        else {  
            ricirc_indx = i_circuit;
            erog_indx   = i_circuit;
            master_slave = COLORANT_SINGLE_BASE;
        }
        // ---------------------------------------------------------------------
        if (master_slave != DOUBLE_GROUP_SLAVE_WITH_MASTER) {
            // 'i_circuit' is a BASE
            if (!isColorTintingModule(i_circuit) ) {
                if (isColorActHoming(ricirc_indx)) {
                    // pre-dispensation recirculation not yet completed? 
                    if (isRecircBeforeSupplyDone(ricirc_indx) == FALSE) {	
                        if (isBaseCircuit(i_circuit))
                            startRecircBeforeSupply(ricirc_indx);
                    }
                    // recirculation is now complete, dispense? 
                    else if (canDispensing) {
                        calcSupplyPar(colorAct[erog_indx].vol_t,
                              color_supply_par[erog_indx].vol_mu,
                              color_supply_par[erog_indx].vol_mc,
                              erog_indx);

                        colorAct[erog_indx].algorithm = Erogation_Type[erog_indx];				
                        if ((colorAct[erog_indx].algorithm == ALG_SYMMETRIC_CONTINUOUS) || (colorAct[erog_indx].algorithm == ALG_ASYMMETRIC_CONTINUOUS)) {
                            colorAct[erog_indx].command.cmd = CMD_SUPPLY;							  
                            //  Colorant or Single Base
                            if (master_slave == COLORANT_SINGLE_BASE)
                                setColorActMessage(DISPENSAZIONE_COLORE_CONT, erog_indx);
                            // Double Group Master with NO Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_NO_SLAVE)
                                startSupplyColorContinuousDoubleGroup(erog_indx);
                            // Double Group Master with Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_WITH_SLAVE)
                                startSupplyColorContinuousDoubleGroup(erog_indx);
                            // Double Group Slave with NO Master in Formula
                            else
                                startSupplyColorContinuousDoubleGroup(erog_indx-1);				
                        }
                        else {
                            //  Colorant or Single Base
                            if (master_slave == COLORANT_SINGLE_BASE)
                                startSupplyColor(erog_indx);				
                            // Double Group Master with NO Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_NO_SLAVE)
                                startSupplyColorDoubleGroup(erog_indx);
                            // Double Group Master with Slave in Formula
                            else if (master_slave == DOUBLE_GROUP_MASTER_WITH_SLAVE)
                                startSupplyColorDoubleGroup(erog_indx);
                            // Double Group Slave with NO Master in Formula
                            else
                                startSupplyColorDoubleGroup(erog_indx-1);				
                        }
                    }	
                }
                // pre-dispensation recirculation started? 
                else if (isColorActRecircRun(ricirc_indx)) {
                    procGUI.recirc_before_supply_status |= (1L << ricirc_indx);
                }
                // Pre-dispensation recirculation stopped? 
                else if (isColorActRecircEnd(ricirc_indx)) {
                    procGUI.recirc_before_supply_status |= (1L << ricirc_indx);
                    stopColorAct(ricirc_indx);			
                }
            }
        }
    }
} // checkBasesDispensationAct() 

void checkColorantDispensationAct(unsigned char canDispensing)
/**/
/*==========================================================================*/
/**
**   @brief Manage recirc before dispensation and max number of
**   simultaneous dispensing colors
**
**   @param uchar dispensing enable condition : FALSE if only RECIRC
**   is admitted
**
**   @return void
**/
/*==========================================================================*/
/**/
{
    unsigned char i, i_circuit, ricirc_indx, erog_indx;
    static unsigned char Tinting_Wait_End;

    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        i_circuit = procGUI.dispensation_colors_id[i];
        ricirc_indx = i_circuit;
        erog_indx   = i_circuit;
        // 'i_circuit' is a TINTING COLORANT
        if (isColorTintingModule(i_circuit)) {			
            if ( ((procGUI.simultaneous_colors_status & (1L << ricirc_indx)) > 0) ||  
                 ((procGUI.simultaneous_colors_nr == 0) && (Erogation_done[erog_indx] == FALSE)) ) {					 
                if (isTintingReady()) {				
                    Tinting_Wait_End = 1;
                    // pre-dispensation recirculation not yet completed? 			
                    if (isRecircBeforeSupplyDone(ricirc_indx) == FALSE) {
                        TintingStartRecircBeforeSupply(ricirc_indx);
                        #if defined NOLAB        
                            TintingAct.Color_Id = 1;       
                        #endif                                                     
                        if (! isSimultaneousColorDispensing(ricirc_indx)) {
                            procGUI.simultaneous_colors_status |= (1L << ricirc_indx);
                            procGUI.simultaneous_colors_nr++;
                        }
                    }
                    // recirculation is now complete, dispense? 
                    else if (canDispensing) {
                        calcSupplyPar(colorAct[erog_indx].vol_t,
                            color_supply_par[erog_indx].vol_mu,
                            color_supply_par[erog_indx].vol_mc,
                            erog_indx);

                        colorAct[erog_indx].algorithm = Erogation_Type[erog_indx];
                        if ((colorAct[erog_indx].algorithm==ALG_SYMMETRIC_CONTINUOUS) || (colorAct[erog_indx].algorithm==ALG_ASYMMETRIC_CONTINUOUS)) {
                            colorAct[erog_indx].algorithm = COLOR_ACT_STROKE_OPERATING_MODE; 
                            startSupplyContinuousTinting(erog_indx);
                            #if defined NOLAB        
                                TintingAct.Color_Id = 1;       
                            #endif                                                                                                                                        
                        }
                        else {
                            startSupplyTinting(erog_indx);
                            #if defined NOLAB        
                                TintingAct.Color_Id = 1;       
                            #endif                                                                        
                        }    
                    }
                }
                // pre-dispensation recirculation started? 
                else if (isTintingRicircRun())
                    procGUI.recirc_before_supply_status |= (1L << ricirc_indx);

                // pre-dispensation recirculation stopped? 
                else if (isTintingRicircEnd()) {
                    procGUI.recirc_before_supply_status |= (1L << i_circuit);
                    TintingStop();
                }
                // dispensation ended? 
                else if (isTintingSupplyEnd() && (Tinting_Wait_End == 1) ) {
                    TintingStop();
                    Tinting_Wait_End = 0;
                    Erogation_done[erog_indx] = TRUE;						
                    if (isSimultaneousColorDispensing(ricirc_indx)) {
                        procGUI.simultaneous_colors_status &= ~(1L << ricirc_indx);
                            if (procGUI.simultaneous_colors_nr)
                                -- procGUI.simultaneous_colors_nr;
                    }					
                }
            }	
        }		
    }
} // checkColorantDispensationAct()

unsigned char isAllBasesSupplyEnd(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if supply is end in all bases
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i, idx;
    unsigned char ret = TRUE;
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        idx = procGUI.dispensation_colors_id[i];
        if (! isColorantActEnabled(idx))
          continue;
        if (!isBaseCircuit(idx) ) 
          continue;	
        // If circuit 'idx' is a BASE of an ODD DOUBLE GROUP --> check on MASTER 'idx-1'
        if ( isBaseCircuit(idx) && (procGUI.circuit_pump_types[idx] == PUMP_DOUBLE) && (idx%2 != 0) )
          idx--;
        if ( isBaseCircuit(idx) && (!isColorActSupplyEnd(idx)) )
          ret = FALSE;
    }
    return ret;
}

unsigned char isAllBasesSupplyHome(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all supply Bases are at home
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i, idx;
    unsigned char ret = TRUE;
		
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        idx = procGUI.dispensation_colors_id[i];
        if (! isColorantActEnabled(idx))
            continue;
        // If circuit 'idx' is a BASE of an ODD DOUBLE GROUP --> check on MASTER 'idx-1'
        if ( isBaseCircuit(idx) && (procGUI.circuit_pump_types[idx] == PUMP_DOUBLE) && (idx%2 != 0) )
            idx--;
        if ( isBaseCircuit(idx) && (!isColorActHoming(idx)) )
            ret = FALSE;
    }
    return ret;
}

unsigned char isAllColorantsSupplyHome(void)
/**/
/*==========================================================================*/
/**
**   @brief  Check if all supply colorants are at home
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{
    unsigned char i, idx;
    unsigned char ret = TRUE;		
    for (i = 0; i < procGUI.dispensation_colors_number; ++ i) {
        idx = procGUI.dispensation_colors_id[i];
        if (! isColorantActEnabled(idx))
            continue;
  
        if (!isBaseCircuit(idx)) {
            if (! isColorReadyTintingModule(idx)) 	
                return FALSE;
        }
    }	
    return ret;    
}

unsigned char getAttuatoreAttivo(unsigned char attuatore)
{
	if (attuatore >= (N_SLAVES-1))
		return 0;
        
	return attuatoreAttivo[attuatore];
}
