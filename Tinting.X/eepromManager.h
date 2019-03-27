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

// Circuits Steps Position Values: 0x0200 .. 0x0300 
#define EE_CRC_VALUE_CIR_STEPS_POS         (0x0200)
#define EE_START_CIR_STEPS_POS             (EE_CRC_VALUE_CIR_STEPS_POS + EE_CRC_SIZE)

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
unsigned char updateEECirStepsPos(void);
void updateEEParamCirStepsPosCRC(void);
void resetEEprom(void);
 
extern unsigned short CRCarea(unsigned char *pointer, unsigned short n_char,unsigned short CRCinit);

#endif /* _EEPROM_MANAGER_H_ */
