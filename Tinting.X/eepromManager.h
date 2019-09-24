/* 
 * File:   eepromManager.h
 * Author: michele.abelli
 * Description: Manage EEprom
 * Created on 6 Novembre 2018, 15.19
 */

#ifndef _EEPROM_MANAGER_H_
#define _EEPROM_MANAGER_H_

/** EEPROM locations */
#define EE_CRC_SIZE   2 /* CRC16 */

// -----------------------------------------------------------------------------
// Circuits Steps Position Values: 0x0200 .. 0x0300 
#define EE_CRC_VALUE_CIR_STEPS_POS         (0x0200)
#define EE_START_CIR_STEPS_POS             (EE_CRC_VALUE_CIR_STEPS_POS + EE_CRC_SIZE)

// Circuit settings (pump acts) 0x300 .. 0x6C1 
#define EE_CRC_VALUE_COLOR_CIRCUIT_PAR (0x300)
#define EE_START_CIRCUIT_COLOR         (EE_CRC_VALUE_COLOR_CIRCUIT_PAR + EE_CRC_SIZE)

// Calibration curves (pump acts) 0x700 .. 0x4301
#define EE_CRC_VALUE_CALIB_CURVES_PAR  (0x700)
#define EE_START_CALIB_CURVE           (EE_CRC_VALUE_CALIB_CURVES_PAR + EE_CRC_SIZE)

// Slaves configuration 0x4350 .. 0x4357
#define EE_CRC_VALUE_SLAVES_EN         (0x4350)
#define EE_START_SLAVES_EN             (EE_CRC_VALUE_SLAVES_EN + EE_CRC_SIZE)

//  Humidifier 1.0 / 2.0 / settings 0x4400 .. 0x441E 
#define EE_CRC_VALUE_TINTING_HUM_PARAM_OFFSET    (0x4400)
#define EE_START_VALUE_TINTING_HUM_PARAM_OFFSET  (EE_CRC_VALUE_TINTING_HUM_PARAM_OFFSET + EE_CRC_SIZE)

// Circuit Pump Types 0x4500 .. 0x4580 
#define EE_CRC_VALUE_CIRCUIT_PUMP_TYPES	(0x4500)
#define EE_START_SLAVES_CIRCUIT_PUMP_TYPES  (EE_CRC_VALUE_CIRCUIT_PUMP_TYPES + EE_CRC_SIZE)

// Tinting Pump parameters 0x4750 - 0x4786
#define EE_CRC_VALUE_TINT_PUMP_PARAM_OFFSET      (0x4750)
#define EE_START_VALUE_TINT_PUMP_PARAM_OFFSET    (EE_CRC_VALUE_TINT_PUMP_PARAM_OFFSET + EE_CRC_SIZE)

// Tinting Table parameters 0x4790 - 0x47B8
#define EE_CRC_VALUE_TINT_TABLE_PARAM_OFFSET      (0x4790)
#define EE_START_VALUE_TINT_TABLE_PARAM_OFFSET    (EE_CRC_VALUE_TINT_TABLE_PARAM_OFFSET + EE_CRC_SIZE)

// Tinting Clean parameters 0x4800 - 0x4850
#define EE_CRC_VALUE_TINT_CLEAN_PARAM_OFFSET      (0x4800)
#define EE_START_VALUE_TINT_CLEAN_PARAM_OFFSET    (EE_CRC_VALUE_TINT_CLEAN_PARAM_OFFSET + EE_CRC_SIZE)	
// -----------------------------------------------------------------------------

// Maximum CRC Reading Errors admitted
#define MAX_EEPROM_RETRIES 5

#define ENABLE_EEPROM()     \
  do {                      \
    EEPROM_CS = 0;          \
  } while (0)

#define DISABLE_EEPROM()    \
  do {                      \
    EEPROM_CS = 1;          \
  } while (0)

/* EEprom Sectors */
enum {
  ALL_EEPROM,		
  EEPROM_COLOR_CIRCUIT,	
  EEPROM_CALIB_CURVE,		
  EEPROM_SLAVES_ENABLE,	
  EEPROM_XY_AXIS_OFFSET,
  EEPROM_CAN_LIFTER,		
  EEPROM_HUMIDIFIER,		
};

/* EEprom Sectors */
enum {
  EEPROM_ERASE,		
  EERPOM_SET_DEFAULT,
};
	  
	  
	  
enum {
  EEPROM_WRITE_IN_PROGRESS,
  EEPROM_WRITE_DONE,
  EEPROM_WRITE_FAILED,
};

enum {
  EEPROM_READ_IN_PROGRESS,
  EEPROM_READ_DONE,
  EEPROM_READ_FAILED,
};

typedef struct {
  unsigned char sector;
  unsigned char action;
} EEprom_t;

unsigned char checkEEprom(void);
unsigned short loadEEParamCirStepsPos(void);
unsigned short loadEEParamSlavesEn(void);

void resetEEParamColorCircCRC();
void resetEEParamCalibCurvesCRC();
void resetEETintPumpEEpromCRC(void);
void resetEETintTableEEpromCRC(void);
void resetEETintCleaningEEpromCRC(void);

unsigned char updateEECirStepsPos(void);
unsigned char updateEEColorCircuit(unsigned char index);
unsigned char updateEECalibCurve(unsigned char i_curve, unsigned char i_circuit);
unsigned char updateEESlavesEn(void);
unsigned char updateEECircuitPumpTypes(void);
unsigned char updateEETintHumidifier(void);
unsigned char updateEETintPump(void);
unsigned char updateEETintTable(void);
unsigned char updateEETintCleaning(void);
unsigned short loadEETintHumidifier_Param(void);

void updateEEParamCirStepsPosCRC(void);
unsigned char updateEEParamColorCircCRC(void);
unsigned char updateEEParamCalibCurvesCRC(void);
unsigned char updateEEParamSlavesEnCRC(void);
unsigned char updateEECircuitPumpTypesCRC(void);
void updateEETintHumidifier_CRC(void);
void updateEETintPump_CRC(void);
void updateEETintTable_CRC(void);
void updateEETintCleaning_CRC(void);

void resetEEprom(void);
 
void loadEECalibCurves(unsigned char i_circuit);

extern unsigned short CRCarea(unsigned char *pointer, unsigned short n_char,unsigned short CRCinit);

#endif /* _EEPROM_MANAGER_H_ */
