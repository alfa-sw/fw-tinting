/**/
/* 
 * File:   eeprom.h
 * Author: michele.abelli
 * Description: Manage EEprom
 * Created on 6 Novembre 2018, 15.19
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

/*===== TIPI LOCALI ==========================================================*/
enum  {
#ifndef DEBUG_SLAVE
  EEPROM_WRITE_DONE,
#else
  EEPROM_WRITE_DONE2,
#endif
  EEPROM_BUSY,
  EEPROM_SPI_FAILURE,
};

/************************************************************************
 * Structure STATREG and union _EEPROMStatus_                            *
 *                                                                       *
 * Overview: Provide a bits and byte access to EEPROM status value.      *
 *                                                                       *
 ************************************************************************/
#ifdef DEBUG_SLAVE
struct  STATREG{
  char    WIP:1;
  char    WEL:1;
  char    BP0:1;
  char    BP1:1;
  char    RESERVED:3;
  char    WPEN:1;
};

union _EEPROMStatus_{
  struct  STATREG Bits;
  char    Char;
};
#else
#endif

/*===== MACRO LOCALI =========================================================*/

/************************************************************************
 * EEPROM Commands                                                       *
 *                                                                       *
 ************************************************************************/
#define EEPROM_PAGE_SIZE    (unsigned)64
#define EEPROM_PAGE_MASK    (unsigned)0x003f
#define EEPROM_CMD_READ     (unsigned)0b00000011
#define EEPROM_CMD_WRITE    (unsigned)0b00000010
#define EEPROM_CMD_WRDI     (unsigned)0b00000100
#define EEPROM_CMD_WREN     (unsigned)0b00000110
#define EEPROM_CMD_RDSR     (unsigned)0b00000101
#define EEPROM_CMD_WRSR     (unsigned)0b00000001

/************************************************************************
 * Aliases for IOs registers related to SPI connected to EEPROM          *
 *                                                                       *
 ************************************************************************/

// PIN mapping form gestIO.h
#define EEPROM_SS_PORT      EEPROM_CS
#define EEPROM_SCK_TRIS     SCK_EEPROM
#define EEPROM_SDO_TRIS     SDO_EEPROM
#define EEPROM_SDI_TRIS     SDI_EEPROM

/************************************************************************
 * Macro: Lo                                                             *
 *                                                                       *
 * Preconditions: None                                                   *
 *                                                                       *
 * Overview: This macro extracts a low byte from a 2 byte word.          *
 *                                                                       *
 * Input: None.                                                          *
 *                                                                       *
 * Output: None.                                                         *
 *                                                                       *
 ************************************************************************/
#define Lo(X)   (unsigned char)(X&0x00ff)

/************************************************************************
 * Macro: Hi                                                             *
 *                                                                       *
 * Preconditions: None                                                   *
 *                                                                       *
 * Overview: This macro extracts a high byte from a 2 byte word.         *
 *                                                                       *
 * Input: None.                                                          *
 *                                                                       *
 * Output: None.                                                         *
 *                                                                       *
 ************************************************************************/
#define Hi(X)   (unsigned char)((X>>8)&0x00ff)

/************************************************************************
 * Macro: mEEPROMSSLow                                                   *
 *                                                                       *
 * Preconditions: SS IO must be configured as output.                    *
 *                                                                       *
 * Overview: This macro pulls down SS line                               *
 *           to start a new EEPROM operation.                            *
 *                                                                       *
 * Input: None.                                                          *
 *                                                                       *
 * Output: None.                                                         *
 *                                                                       *
 ************************************************************************/
#define mEEPROMSSLow()      EEPROM_SS_PORT=0;

/************************************************************************
 * Macro: mEEPROMSSHigh                                                  *
 *                                                                       *
 * Preconditions: SS IO must be configured as output.                    *
 *                                                                       *
 * Overview: This macro set SS line to high level                        *
 *           to start a new EEPROM operation.                            *
 *                                                                       *
 * Input: None.                                                          *
 *                                                                       *
 * Output: None.                                                         *
 *                                                                       *
 ************************************************************************/
#define mEEPROMSSHigh()     EEPROM_SS_PORT=1;

/************************************************************************
 * Function: EEPROMInit                                                  *
 *                                                                       *
 * Preconditions: SPI module must be configured to operate with          *
 *                 parameters: Master, MODE16 = 0, CKP = 1, SMP = 1.     *
 *                                                                       *
 * Overview: This function setup SPI IOs connected to EEPROM.            *
 *                                                                       *
 * Input: None.                                                          *
 *                                                                       *
 * Output: None.                                                         *
 *                                                                       *
 ************************************************************************/
 void EEPROMInit();

/************************************************************************
 * Function: EEPROMReadStatus()                                          *
 *                                                                       *
 * Preconditions: SPI module must be configured to operate with  EEPROM. *
 *                                                                       *
 * Overview: This function reads status register from EEPROM.            *
 *                                                                       *
 * Input: None.                                                          *
 *                                                                       *
 * Output: Status register value.                                        *
 *                                                                       *
 ************************************************************************/
union _EEPROMStatus_ EEPROMReadStatus();

/************************************************************************
 * Function: EEPROMWriteByte()                                           *
 *                                                                       *
 * Preconditions: SPI module must be configured to operate with  EEPROM. *
 *                                                                       *
 * Overview: This function writes a new value to address specified.      *
 *                                                                       *
 * Input: Data to be written and address.                                *
 *                                                                       *
 * Output: None.                                                         *
 *                                                                       *
 ************************************************************************/
 void EEPROMWriteByte(unsigned Data, unsigned Address);

/************************************************************************
 * Function: EEPROMReadByte()                                            *
 *                                                                       *
 * Preconditions: SPI module must be configured to operate with  EEPROM. *
 *                                                                       *
 * Overview: This function reads a value from address specified.         *
 *                                                                       *
 * Input: Address.                                                       *
 *                                                                       *
 * Output: Data read.                                                    *
 *                                                                       *
 ************************************************************************/
unsigned EEPROMReadByte(unsigned Address);

void EEPROMWriteArray(unsigned short StartAddress,
                             unsigned short Length,
                             unsigned char *DataPtr);

 void EEPROMReadArray(unsigned short StartAddress,
                            unsigned short Length,
                            unsigned char *DataPtr);

 unsigned char EEPROMWriteArrayInPage(unsigned short StartAddress,
                                            unsigned char Length,
                                            unsigned char *DataPtr);
 void EEPROMWriteByteNotBlocking(unsigned Data, unsigned Address);

#endif /* __EEPROM_H__  */
