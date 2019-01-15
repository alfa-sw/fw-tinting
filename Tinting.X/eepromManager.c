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
    // Load slaves configuration into RAM 
    EEPROMReadArray(EE_CRC_VALUE_CIR_STEPS_POS, EE_CRC_SIZE,
                                    ((unsigned char *) &readCrc));

    calcCrc = loadEEParamCirStepsPos();
    if (readCrc != calcCrc)
        return FALSE;
    else
        return TRUE;
}

void updateEEParamCirStepsPosCRC(void)
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
    unsigned short crc;

    crc = 0;
    offset = EE_START_CIR_STEPS_POS;

	EEPROMReadArray(offset,sizeof(CircStepPosAct_t),(unsigned char *)&CircStepPosAct);
    offset += sizeof(CircStepPosAct_t);
    crc = CRCarea((unsigned char *)&CircStepPosAct,sizeof(CircStepPosAct_t), crc);
	EEPROMWriteArray(EE_CRC_VALUE_CIR_STEPS_POS, EE_CRC_SIZE,(unsigned char *) &crc);    
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
        
        if (eeprom_byte < sizeof(CircStepPosAct_t)) {
//                EEPROMWriteByte(((unsigned char *) &CircStepPosAct)[eeprom_byte],startAddress);
            EEPROMWriteByteNotBlocking(((unsigned char *) &CircStepPosAct)[eeprom_byte],startAddress);
            
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
    crc = 0;
    startAddress = EE_START_CIR_STEPS_POS;

	EEPROMReadArray(startAddress, sizeof(CircStepPosAct_t ),(unsigned char *) &CircStepPosAct);
	startAddress+=sizeof(CircStepPosAct_t );
	crc = CRCarea((unsigned char *) &CircStepPosAct,sizeof(CircStepPosAct_t ), crc);
    
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
      TintingAct.Circuit_step_pos[i] = 0;
      CircStepPosAct.Circ_Pos[i] = 0;
	}
}
