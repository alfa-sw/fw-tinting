/* 
 * File:   eepromManager.c
 * Author: michele.abelli
 * Description: EEprom Processes management
 * Created on 6 Novembre 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "statusManager.h"
#include "timerMg.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "typedef.h"
#include "eepromManager.h"
#include "eeprom.h"
#include "typedef.h"
#include "serialCom.h"
#include <xc.h>

/* static function protos */
static unsigned short loadEEParamCirStepsPos(void);

unsigned char checkEEprom(void)
/**/
/*============================================================================*/
/**
**   @brief  Check the integrity of the EEPROM memory
**
**   @param  void
**
**   @return void
**/
/*============================================================================*/
/**/
{
    unsigned short calcCrc, readCrc;
    /* Load slaves configuration into RAM */
    EEPROMReadArray(EE_CRC_VALUE_CIR_STEPS_POS, EE_CRC_SIZE,
                                    ((unsigned char *) &readCrc));

    calcCrc = loadEEParamCirStepsPos();
    if (readCrc != calcCrc)
        return FALSE;
    else
        return TRUE;
}

unsigned char updateEEParamCirStepsPosCRC(void)
/**/
/*============================================================================*/
/**
**   @brief  Updates Circuits Step Position values
**
**   @param  void
**
**   @return void
**/
/*============================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_READ_IN_PROGRESS;
    if (eeprom_i == 0)
      offset = EE_START_CIR_STEPS_POS;

    if (eeprom_i < (2*MAX_COLORANT_NUMBER)) {
        if (eeprom_i < MAX_COLORANT_NUMBER)  
          EEPROMReadArray(offset, sizeof(unsigned char),
                                      (unsigned char *) &TintingAct.Circuit_step_pos_cw[eeprom_i]);
        else
          EEPROMReadArray(offset, sizeof(unsigned char),
                                      (unsigned char *) &TintingAct.Circuit_step_pos_ccw[eeprom_i]);          
        offset += sizeof(unsigned char);

        if (eeprom_i < MAX_COLORANT_NUMBER)   
            eeprom_crc = CRCarea((unsigned char *) &TintingAct.Circuit_step_pos_cw[eeprom_i],
                                               sizeof(unsigned char), eeprom_crc);
        else
            eeprom_crc = CRCarea((unsigned char *) &TintingAct.Circuit_step_pos_ccw[eeprom_i],
                                           sizeof(unsigned char), eeprom_crc);
        
        eeprom_i ++;
    }
    else {
        EEPROMWriteArray(EE_CRC_VALUE_CIR_STEPS_POS, EE_CRC_SIZE,
                                       (unsigned char *) &eeprom_crc);
        ret_val = EEPROM_READ_DONE;
    }
    return ret_val;
}

unsigned char updateEECirStepsPos(void)
/**/
/*============================================================================*/
/**
**   @brief  Updates Circuits Step Position value
**
**   @param  Circuit Step Position index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*============================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
    if (!EEPROMReadStatus().Bits.WIP) {
        if (eeprom_byte == 0)
            startAddress = EE_START_CIR_STEPS_POS;

        if (eeprom_byte < (2*MAX_COLORANT_NUMBER)) {
            if (eeprom_i < MAX_COLORANT_NUMBER)  
                EEPROMWriteByteNotBlocking(((unsigned char *) &TintingAct.Circuit_step_pos_cw)[eeprom_byte],startAddress);
            else
                EEPROMWriteByteNotBlocking(((unsigned char *) &TintingAct.Circuit_step_pos_ccw)[eeprom_byte],startAddress);
                
            startAddress ++;
            eeprom_byte ++;
        }
        else {
            ret_val = EEPROM_WRITE_DONE;
        }
    }
    return ret_val;
}

static unsigned short loadEEParamCirStepsPos(void)
/**/
/*============================================================================*/
/**
**   @brief  Load Circuits Steps Position
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*============================================================================*/
/**/
{
    unsigned short crc;
    unsigned char i;
    crc = 0;
    startAddress = EE_START_CIR_STEPS_POS;

    for (i = 0; i < (2*MAX_COLORANT_NUMBER); i ++) {
        if (i < MAX_COLORANT_NUMBER)
            EEPROMReadArray(startAddress, sizeof(unsigned char),
                                      (unsigned char *) &TintingAct.Circuit_step_pos_cw[i]);
        else
            EEPROMReadArray(startAddress, sizeof(unsigned char),
                                      (unsigned char *) &TintingAct.Circuit_step_pos_ccw[i]);
            
        startAddress += sizeof(unsigned char);
        
        if (i < MAX_COLORANT_NUMBER)
            crc = CRCarea((unsigned char *) &TintingAct.Circuit_step_pos_cw[i],
                                    sizeof(unsigned char), crc);
        else
            crc = CRCarea((unsigned char *) &TintingAct.Circuit_step_pos_ccw[i],
                                    sizeof(unsigned char), crc);            
    }
    return crc;
}
void resetEEprom(void)
/**/
/*============================================================================*/
/**
**   @brief  RESET all EEprom Sectors
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*============================================================================*/
/**/
{
	unsigned short i;
	for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
      TintingAct.Circuit_step_pos_cw[i] = 0; 
      TintingAct.Circuit_step_pos_ccw[i] = 0;     
	}
}
