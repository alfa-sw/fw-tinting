/* 
 * File:   eepromManager.c
 * Author: michele.abelli
 * Description: EEprom Processes management
 * Created on 6 Novembre 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "tintingManager.h"
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
#include <string.h> 

/* static function protos */
static unsigned short loadEEParamColorCircuits(void);
static unsigned short loadEEParamCalibCurves(void);
static unsigned short loadEETintPump_Param(void);
static unsigned short loadEETintTable_Param(void);
static unsigned short loadEETintClean_Param(void);
static unsigned short loadEECircuitPumpTypes(void);

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
    
    // Load Table Circuits Position 
    EEPROMReadArray(EE_CRC_VALUE_CIR_STEPS_POS, EE_CRC_SIZE,((unsigned char *) &readCrc));
    calcCrc = loadEEParamCirStepsPos();
    if (readCrc != calcCrc) {
        InitFlags.CRCCircuitStepsPosFailed = TRUE;
    }                
    else
        InitFlags.CRCCircuitStepsPosFailed = FALSE;
    
    // Load Circuit Settings
    EEPROMReadArray(EE_CRC_VALUE_COLOR_CIRCUIT_PAR, EE_CRC_SIZE,((unsigned char *) &readCrc));
    calcCrc = loadEEParamColorCircuits();
    if (readCrc != calcCrc)
      InitFlags.CRCParamColorCircuitFailed = TRUE;
    
    // Load Calibration data 
    EEPROMReadArray(EE_CRC_VALUE_CALIB_CURVES_PAR, EE_CRC_SIZE,((unsigned char *) &readCrc));								  
    calcCrc = loadEEParamCalibCurves();
    if (readCrc != calcCrc)
        InitFlags.CRCParamCalibCurvesFailed = TRUE;
        
    // Load Slaves enable mask 
    EEPROMReadArray(EE_CRC_VALUE_SLAVES_EN, EE_CRC_SIZE,((unsigned char *) &readCrc));
    calcCrc = loadEEParamSlavesEn();
    if (readCrc != calcCrc)
        InitFlags.CRCParamSlavesEnFailed = TRUE;

    // Load circuit Pump types 
    EEPROMReadArray(EE_CRC_VALUE_CIRCUIT_PUMP_TYPES, EE_CRC_SIZE,((unsigned char *) &readCrc));								  
    calcCrc = loadEECircuitPumpTypes();
    if (readCrc != calcCrc)
        InitFlags.CRCParamCircuitPumpTypesFailed = TRUE;
    
	// Load Humidifier parameters
	EEPROMReadArray(EE_CRC_VALUE_TINTING_HUM_PARAM_OFFSET, EE_CRC_SIZE,((unsigned char *) &readCrc));
	calcCrc = loadEETintHumidifier_Param();
	if (readCrc != calcCrc) 
		InitFlags.CRCParamHumidifier_paramFailed = TRUE;
    
	// Load Tinting Pump parameters
	EEPROMReadArray(EE_CRC_VALUE_TINT_PUMP_PARAM_OFFSET, EE_CRC_SIZE,((unsigned char *) &readCrc));
	calcCrc = loadEETintPump_Param();
	if (readCrc != calcCrc) 
		InitFlags.CRCParamTinting_Pump_paramFailed = TRUE;

	// Load Tinting Table parameters
	EEPROMReadArray(EE_CRC_VALUE_TINT_TABLE_PARAM_OFFSET, EE_CRC_SIZE,((unsigned char *) &readCrc));
	calcCrc = loadEETintTable_Param();
	if (readCrc != calcCrc) 
		InitFlags.CRCParamTinting_Table_paramFailed = TRUE;

	// Load Tintng Clean parameters
	EEPROMReadArray(EE_CRC_VALUE_TINT_CLEAN_PARAM_OFFSET, EE_CRC_SIZE,((unsigned char *) &readCrc));
	calcCrc = loadEETintClean_Param();
	if (readCrc != calcCrc) 
	{
		InitFlags.CRCParamTinting_Clean_paramFailed = TRUE;
	}    
    return TRUE;
}

// -----------------------------------------------------------------------------
//
//                      CIRCUITS STEPS POSITION
//
// -----------------------------------------------------------------------------
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

unsigned short loadEEParamCirStepsPos(void)
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

// -----------------------------------------------------------------------------
//
//                      CIRCUITS SETTINGS
//
// -----------------------------------------------------------------------------
unsigned char updateEEParamColorCircCRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates the color circuits parameters
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_READ_IN_PROGRESS;
    if (eeprom_i == 0)
      offset = EE_START_CIRCUIT_COLOR;

    if (eeprom_i < N_SLAVES_COLOR_ACT) {
        EEPROMReadArray(offset, sizeof(color_supply_par_t),((unsigned char *) &color_supply_par[eeprom_i]));
        offset += sizeof(color_supply_par_t);

        eeprom_crc = CRCarea((unsigned char *) &color_supply_par[eeprom_i],sizeof(color_supply_par_t), eeprom_crc);
        eeprom_i ++;
    }
    else {
      EEPROMWriteArray(EE_CRC_VALUE_COLOR_CIRCUIT_PAR, EE_CRC_SIZE,((unsigned char *) &eeprom_crc));
      ret_val = EEPROM_READ_DONE;
    }
    return ret_val;
}

unsigned char updateEEColorCircuit(unsigned char index)
/**/
/*===========================================================================*/
/**
**   @brief  Updates circuit color parameters
**
**   @param  color circuit index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
    if (!EEPROMReadStatus().Bits.WIP) {
        if (eeprom_byte == 0) {
            startAddress = EE_START_CIRCUIT_COLOR + ((unsigned short)(index))*sizeof(color_supply_par_t);
        }

        if (eeprom_byte < sizeof(color_supply_par_t)) {
            EEPROMWriteByteNotBlocking(((unsigned char *) &color_supply_par_writing)[eeprom_byte],startAddress);
            startAddress ++;
            eeprom_byte ++;
        }
        else {
            ret_val = EEPROM_WRITE_DONE;
        }
    }
    return ret_val;
}

static unsigned short loadEEParamColorCircuits(void)
/**/
/*===========================================================================*/
/**
**   @brief  Load all the color circuits parameters
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
{
    unsigned short crc;
    unsigned char i;
    crc = 0;
    startAddress = EE_START_CIRCUIT_COLOR;
    for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
        EEPROMReadArray(startAddress, sizeof(color_supply_par_t),(unsigned char *) &color_supply_par[i]);
        startAddress += sizeof(color_supply_par_t);
        crc = CRCarea((unsigned char *) &color_supply_par[i],sizeof(color_supply_par_t), crc);
    }
    return crc;
}

void resetEEParamColorCircCRC(void)
{
	eeprom_crc=1;
	EEPROMWriteArray(EE_CRC_VALUE_COLOR_CIRCUIT_PAR, EE_CRC_SIZE,((unsigned char *) &eeprom_crc));
	
}

// -----------------------------------------------------------------------------
//
//                      CALIBRATION DATA
//
// -----------------------------------------------------------------------------
unsigned char updateEEParamCalibCurvesCRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates the calib curces parameters
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_READ_IN_PROGRESS;
    if (eeprom_i == 0 && eeprom_j == 0)
      offset = EE_START_CALIB_CURVE;

    if (eeprom_i < N_SLAVES_COLOR_ACT) {
        if (eeprom_j < N_CALIB_CURVE) {
            EEPROMReadArray(offset, sizeof(calib_curve_par_t),((unsigned char *) &calib_curve_par[eeprom_j]));

            offset += sizeof(calib_curve_par_t);
            eeprom_crc = CRCarea((unsigned char *) &calib_curve_par[eeprom_j],sizeof(calib_curve_par_t), eeprom_crc);
            eeprom_j ++;
        }
        else {
            eeprom_i ++;
            eeprom_j = 0;
        }
    }
    else {
        EEPROMWriteArray(EE_CRC_VALUE_CALIB_CURVES_PAR, EE_CRC_SIZE,((unsigned char *) &eeprom_crc));
        ret_val = EEPROM_READ_DONE;
    }
    return ret_val;
}

unsigned char updateEECalibCurve(unsigned char i_curve, unsigned char i_circuit)
/**/
/*===========================================================================*/
/**
**   @brief  Updates calibration curve i_curve for circuit i_circuit
**
**   @param  curve index
**
**   @param  circuit index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
    if (!EEPROMReadStatus().Bits.WIP) 
    {
        if (eeprom_byte == 0) {
            startAddress = EE_START_CALIB_CURVE + ((unsigned short)(i_curve + i_circuit * N_CALIB_CURVE)) * sizeof(calib_curve_par_t);
        }
        if (eeprom_byte < sizeof(calib_curve_par_t)) {
            EEPROMWriteByteNotBlocking(((unsigned char *) &calib_curve_par_writing)[eeprom_byte],startAddress);
            startAddress ++;
            eeprom_byte ++;
        }
        else {
            ret_val = EEPROM_WRITE_DONE;
        }
    }
    return ret_val;
}

void loadEECalibCurves(unsigned char i_circuit)
/**/
/*===========================================================================*/
/**
**   @brief  load all calibration curves for circuit i_circuit
**
**   @param  curve index
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char j;
    startAddress = EE_START_CALIB_CURVE + ((unsigned short)(i_circuit * N_CALIB_CURVE)) * sizeof(calib_curve_par_t);
    for (j = 0; j < N_CALIB_CURVE; j ++) {
      EEPROMReadArray(startAddress, sizeof(calib_curve_par_t), (unsigned char *) &calib_curve_par[j]);
      startAddress += sizeof(calib_curve_par_t);
    }
}

static unsigned short loadEEParamCalibCurves(void)
/**/
/*===========================================================================*/
/**
**   @brief  Load all the color calibration curves
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
{
    unsigned short crc;
    unsigned char i, j;
    crc = 0;
    startAddress = EE_START_CALIB_CURVE;
    for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
        for (j = 0; j < N_CALIB_CURVE; j ++) {
            EEPROMReadArray(startAddress, sizeof(calib_curve_par_t),(unsigned char *) &calib_curve_par[j]);
            startAddress += sizeof(calib_curve_par_t);
            crc = CRCarea((unsigned char *) &calib_curve_par[j],sizeof(calib_curve_par_t), crc);
        }
    }
    return crc;
}

void resetEEParamCalibCurvesCRC(void)
{
	eeprom_crc=1;
	EEPROMWriteArray(EE_CRC_VALUE_CALIB_CURVES_PAR, EE_CRC_SIZE,((unsigned char *) &eeprom_crc));
}
// -----------------------------------------------------------------------------
//
//                      SLAVE ENABLE MAK
//
// -----------------------------------------------------------------------------
unsigned char updateEEParamSlavesEnCRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates the slaves en parameters
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_READ_IN_PROGRESS;
    if (eeprom_i == 0)
      offset = EE_START_SLAVES_EN;

    if (eeprom_i < N_SLAVES_BYTES) {
        EEPROMReadArray(offset, sizeof(unsigned char),(unsigned char *) &procGUI.slaves_en[eeprom_i]);
        offset += sizeof(unsigned char);
        eeprom_crc = CRCarea((unsigned char *) &procGUI.slaves_en[eeprom_i],sizeof(unsigned char), eeprom_crc);
        eeprom_i ++;
    }
    else {
        EEPROMWriteArray(EE_CRC_VALUE_SLAVES_EN, EE_CRC_SIZE,(unsigned char *) &eeprom_crc);
        ret_val = EEPROM_READ_DONE;
    }
    return ret_val;
}

unsigned char updateEESlavesEn(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates slaves enable configuration
**
**   @param  color circuit index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
    if (!EEPROMReadStatus().Bits.WIP) {
        if (eeprom_byte == 0)
            startAddress = EE_START_SLAVES_EN;
        if (eeprom_byte < N_SLAVES_BYTES) {
            EEPROMWriteByteNotBlocking(((unsigned char *) &en_slaves_writing)[eeprom_byte],startAddress);
            startAddress ++;
            eeprom_byte ++;
        }
        else {
            ret_val = EEPROM_WRITE_DONE;
        }
    }
    return ret_val;
}

unsigned short loadEEParamSlavesEn(void)
/**/
/*===========================================================================*/
/**
**   @brief  Load slaves enable
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
{
    unsigned short crc;
    unsigned char i;
    crc = 0;
    startAddress = EE_START_SLAVES_EN;    
    for (i = 0; i < N_SLAVES_BYTES; i ++) {
      EEPROMReadArray(startAddress, sizeof(unsigned char),(unsigned char *) &procGUI.slaves_en[i]);
      startAddress += sizeof(unsigned char);
      crc = CRCarea((unsigned char *) &procGUI.slaves_en[i],sizeof(unsigned char), crc);
    }
    return crc;
}

// -----------------------------------------------------------------------------
//
//                      CIRCUIT PUMP TYPES
//
// -----------------------------------------------------------------------------
unsigned char updateEECircuitPumpTypesCRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates circuit pump types crc
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
	unsigned char ret_val = EEPROM_READ_IN_PROGRESS;
    if (eeprom_i == 0)
        offset = EE_START_SLAVES_CIRCUIT_PUMP_TYPES;

    if (eeprom_i < N_SLAVES_COLOR_ACT) {
        EEPROMReadArray(offset, sizeof(unsigned short),(unsigned char *) &procGUI.circuit_pump_types[eeprom_i]);
        offset += sizeof(unsigned short);
        eeprom_crc = CRCarea((unsigned char *) &procGUI.circuit_pump_types[eeprom_i],sizeof(unsigned short), eeprom_crc);
        eeprom_i ++;
    }
    else {
        EEPROMWriteArray(EE_CRC_VALUE_CIRCUIT_PUMP_TYPES, EE_CRC_SIZE,(unsigned char *) &eeprom_crc);
        ret_val = EEPROM_READ_DONE;
    }
    return ret_val;
}

unsigned char updateEECircuitPumpTypes(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates Circuit Pump Types
**
**   @param  color circuit index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
    unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
    if (!EEPROMReadStatus().Bits.WIP) {
        if (eeprom_byte == 0)
            startAddress = EE_START_SLAVES_CIRCUIT_PUMP_TYPES;
        
        if (eeprom_byte < N_SLAVES_COLOR_ACT) {
            EEPROMWriteByteNotBlocking(((unsigned char *) &Pump_Types_Circuit_writing)[eeprom_byte],startAddress);
            startAddress ++;
            eeprom_byte ++;
          }
        else {
            ret_val = EEPROM_WRITE_DONE;
        }
    }
    return ret_val;    
}

static unsigned short loadEECircuitPumpTypes(void)
/**/
/*===========================================================================*/
/**
**   @brief  Load all Circuit Pump Types
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
{
    unsigned short crc;
    unsigned char i;
    crc = 0;
    startAddress = EE_START_SLAVES_CIRCUIT_PUMP_TYPES;
    for (i = 0; i < N_SLAVES_COLOR_ACT; i ++) {
      EEPROMReadArray(startAddress, sizeof(unsigned short),(unsigned char *) &procGUI.circuit_pump_types[i]);
      startAddress += sizeof(unsigned short);
      crc = CRCarea((unsigned char *) &procGUI.circuit_pump_types[i],sizeof(unsigned short), crc);
    }
    return crc;
}

// -----------------------------------------------------------------------------
//
//                      HUMIDIFIER PARAMETERS
//
// -----------------------------------------------------------------------------
void updateEETintHumidifier_CRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates the Tinting Humidifier offset parameters
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
	unsigned short offset;
	unsigned short crc;	
	crc = 0;
	offset = EE_START_VALUE_TINTING_HUM_PARAM_OFFSET;
	EEPROMReadArray(offset,sizeof(TintingHumidifier_t),(unsigned char *)&TintingHumidifier);
    offset += sizeof(TintingHumidifier_t);
    crc = CRCarea((unsigned char *)&TintingHumidifier,sizeof(TintingHumidifier_t), crc);
	EEPROMWriteArray(EE_CRC_VALUE_TINTING_HUM_PARAM_OFFSET, EE_CRC_SIZE,(unsigned char *) &crc);
}

unsigned char updateEETintHumidifier(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates Tint Humidifier parameters
**
**   @param  Update Tinting Humidifier parameters
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
	unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
	if (!EEPROMReadStatus().Bits.WIP) 
	{
		if (eeprom_byte == 0) 
			startAddress = EE_START_VALUE_TINTING_HUM_PARAM_OFFSET;

		if (eeprom_byte < sizeof(TintingHumidifier_t)) {
			EEPROMWriteByteNotBlocking(((unsigned char *) &TintingHumidifierWrite)[eeprom_byte],startAddress);
			startAddress ++;
			eeprom_byte ++;
		}
  		else 
            ret_val = EEPROM_WRITE_DONE;
	}
	return ret_val;
}

/**/
/*===========================================================================*/
/**
**   @brief  Load Humidifier parameters
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
unsigned short loadEETintHumidifier_Param(void)
{
	unsigned short crc;
	crc = 0;
	startAddress = EE_START_VALUE_TINTING_HUM_PARAM_OFFSET;
	EEPROMReadArray(startAddress, sizeof(TintingHumidifier_t ),(unsigned char *) &TintingHumidifier);
	startAddress+=sizeof(TintingHumidifier_t );
	crc = CRCarea((unsigned char *) &TintingHumidifier,sizeof(TintingHumidifier_t ), crc);
	return crc;
}

// -----------------------------------------------------------------------------
//
//                      TINTING PUMP PARAMETERS
//
// -----------------------------------------------------------------------------
void updateEETintPump_CRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates the Tinting Pump offset parameters
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
	
    unsigned short offset;
	unsigned short crc;
	crc = 0;
	offset = EE_START_VALUE_TINT_PUMP_PARAM_OFFSET;
	EEPROMReadArray(offset,sizeof(TintingPump_t),(unsigned char *)&TintingPump);
    offset += sizeof(TintingPump_t);
    crc = CRCarea((unsigned char *)&TintingPump,sizeof(TintingPump_t), crc);
	EEPROMWriteArray(EE_CRC_VALUE_TINT_PUMP_PARAM_OFFSET, EE_CRC_SIZE,(unsigned char *) &crc);
}

unsigned char updateEETintPump(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates Tinting Pump parameters
**
**   @param  color circuit index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
	unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
	if (!EEPROMReadStatus().Bits.WIP) 
	{
		if (eeprom_byte == 0) 
			startAddress = EE_START_VALUE_TINT_PUMP_PARAM_OFFSET;

		if (eeprom_byte < sizeof(TintingPump_t)) {
			EEPROMWriteByteNotBlocking(((unsigned char *) &TintingPumpWrite)[eeprom_byte],startAddress);
			startAddress ++;
			eeprom_byte ++;
		}
  		else 
			ret_val = EEPROM_WRITE_DONE;
	}
	return ret_val;
}

/**/
/*===========================================================================*/
/**
**   @brief  Load Tinting Pump parameters
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
static unsigned short loadEETintPump_Param(void)
{
	unsigned short crc;	
	crc = 0;
	startAddress = EE_START_VALUE_TINT_PUMP_PARAM_OFFSET;
	EEPROMReadArray(startAddress, sizeof(TintingPump_t ),(unsigned char *) &TintingPump);
	startAddress+=sizeof(TintingPump_t );
	crc = CRCarea((unsigned char *) &TintingPump,sizeof(TintingPump_t ), crc);
	return crc;
}

void resetEETintPumpEEpromCRC(void)
{
	eeprom_crc=1;
	EEPROMWriteArray(EE_CRC_VALUE_TINT_PUMP_PARAM_OFFSET, EE_CRC_SIZE,((unsigned char *) &eeprom_crc));	
}

// -----------------------------------------------------------------------------
//
//                      TINTING TABLE PARAMETERS
//
// -----------------------------------------------------------------------------
void updateEETintTable_CRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates the Tinting Table offset parameters
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
	unsigned short offset;
	unsigned short crc;
	crc = 0;
	offset = EE_START_VALUE_TINT_TABLE_PARAM_OFFSET;
	EEPROMReadArray(offset,sizeof(TintingTable_t),(unsigned char *)&TintingTable);
    offset += sizeof(TintingTable_t);
    crc = CRCarea((unsigned char *)&TintingTable,sizeof(TintingTable_t), crc);
	EEPROMWriteArray(EE_CRC_VALUE_TINT_TABLE_PARAM_OFFSET, EE_CRC_SIZE,(unsigned char *) &crc);
}

unsigned char updateEETintTable(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates Tinting Table parameters
**
**   @param  color circuit index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
	unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
	if (!EEPROMReadStatus().Bits.WIP) 
	{
		if (eeprom_byte == 0) 
			startAddress = EE_START_VALUE_TINT_TABLE_PARAM_OFFSET;

		if (eeprom_byte < sizeof(TintingTable_t)) 
		{
			EEPROMWriteByteNotBlocking(((unsigned char *) &TintingTableWrite)[eeprom_byte],startAddress);

			startAddress ++;
			eeprom_byte ++;
		}
  		else 
			ret_val = EEPROM_WRITE_DONE;
	}
	return ret_val;
}

/**/
/*===========================================================================*/
/**
**   @brief  Load Tinting Table parameters
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
static unsigned short loadEETintTable_Param(void)
{
	unsigned short crc;	
	crc = 0;
	startAddress = EE_START_VALUE_TINT_TABLE_PARAM_OFFSET;
	EEPROMReadArray(startAddress, sizeof(TintingTable_t ),(unsigned char *) &TintingTable);
	startAddress+=sizeof(TintingTable_t );
	crc = CRCarea((unsigned char *) &TintingTable,sizeof(TintingTable_t ), crc);
	return crc;    
}

void resetEETintTableEEpromCRC(void)
{
	eeprom_crc=1;
	EEPROMWriteArray(EE_CRC_VALUE_TINT_TABLE_PARAM_OFFSET, EE_CRC_SIZE,((unsigned char *) &eeprom_crc));	
}

// -----------------------------------------------------------------------------
//
//                      TINTING CLEANING PARAMETERS
//
// -----------------------------------------------------------------------------
void updateEETintCleaning_CRC(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates the Tinting Cleaning offset parameters
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
	unsigned short offset;
	unsigned short crc;
	crc = 0;
	offset = EE_START_VALUE_TINT_CLEAN_PARAM_OFFSET;
	EEPROMReadArray(offset,sizeof(TintingCleaning_t),(unsigned char *)&TintingClean);
    offset += sizeof(TintingCleaning_t);
    crc = CRCarea((unsigned char *)&TintingClean,sizeof(TintingCleaning_t), crc);
	EEPROMWriteArray(EE_CRC_VALUE_TINT_CLEAN_PARAM_OFFSET, EE_CRC_SIZE,(unsigned char *) &crc);
}

unsigned char updateEETintCleaning(void)
/**/
/*===========================================================================*/
/**
**   @brief  Updates Tinting Cleaning parameters
**
**   @param  color circuit index
**
**   @return EEPROM_WRITE_DONE: write ok
**           EEPROM_WRITE_IN_PROGRESS: write in progress
**           EEPROM_WRITE_FAILED: write fail
**/
/*===========================================================================*/
/**/
{
	unsigned char ret_val = EEPROM_WRITE_IN_PROGRESS;
	if (!EEPROMReadStatus().Bits.WIP) 
	{
		if (eeprom_byte == 0) 
			startAddress = EE_START_VALUE_TINT_CLEAN_PARAM_OFFSET;

		if (eeprom_byte < sizeof(TintingCleaning_t)) 
		{
			EEPROMWriteByteNotBlocking(((unsigned char *) &TintingCleanWrite)[eeprom_byte],startAddress);

			startAddress ++;
			eeprom_byte ++;
		}
  		else 
			ret_val = EEPROM_WRITE_DONE;
	}
	return ret_val;
}

/**/
/*===========================================================================*/
/**
**   @brief  Load Tinting Cleaning parameters
**
**   @param  void
**
**   @return computed CRC16 on parameters
**/
/*===========================================================================*/
/**/
static unsigned short loadEETintClean_Param(void)
{
	unsigned short crc;	
	crc = 0;
	startAddress = EE_START_VALUE_TINT_CLEAN_PARAM_OFFSET    ;
	EEPROMReadArray(startAddress, sizeof(TintingCleaning_t ),(unsigned char *) &TintingClean);
	startAddress+=sizeof(TintingCleaning_t );
	crc = CRCarea((unsigned char *) &TintingClean,sizeof(TintingCleaning_t ), crc);
	return crc;    
}

void resetEETintCleaningEEpromCRC(void)
{
	eeprom_crc=1;
	EEPROMWriteArray(EE_CRC_VALUE_TINT_CLEAN_PARAM_OFFSET, EE_CRC_SIZE,((unsigned char *) &eeprom_crc));	
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
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
	// First of all we reset the parameter value for all the strutcture
	for (i=0;i<N_SLAVES_COLOR_ACT;i++)
	{
		memset(&color_supply_par[i],0,sizeof(color_supply_par_t));
	}	
	memset(&color_supply_par_writing,0,sizeof(color_supply_par_t));

	for (i=0;i<N_CALIB_CURVE;i++)
	{
		memset(&calib_curve_par[i],0,sizeof(calib_curve_par_t));
	}
	memset(&calib_curve_par_writing,0,sizeof(calib_curve_par_t));

	for (i=0;i<N_SLAVES_BYTES;i++)
	{
		procGUI.slaves_en[i]=0;
	}

	for (i=0;i<N_SLAVES_COLOR_ACT;i++)
	{
		procGUI.circuit_pump_types[i]=0;
	}	
	
	memset(&TintingHumidifierWrite,0,sizeof(TintingHumidifier_t));
	memset(&TintingPumpWrite,0,sizeof(TintingPump_t));
	memset(&TintingTableWrite,0,sizeof(TintingTable_t));
	memset(&TintingCleanWrite,0,sizeof(TintingCleaning_t));	
}
