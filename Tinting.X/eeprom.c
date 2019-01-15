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
#include "typedef.h"
#include "serialCom.h"
#include "eeprom.h"
#include "spimpol.h"
#include "spimpol.h"
#include <xc.h>

static unsigned char EEPROMWriteEnable(void);

unsigned char ReadStatus;

/* ===== DEFINIZIONE FUNZIONI GLOBALI ======================================= */

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
void EEPROMInit()
{
    // Set IOs directions for EEPROM SPI2
    EEPROM_SS_PORT  = 1;
    EEPROM_SCK_TRIS = 0;
    EEPROM_SDO_TRIS = 0;
    EEPROM_SDI_TRIS = 1;
#ifdef DEBUG_SLAVE
	BL_SPIMPolInit();
#endif
}

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
void EEPROMWriteByte(unsigned Data, unsigned Address)
{
    // wait for completion of previous write operation
    while(EEPROMReadStatus().Bits.WIP);

    EEPROMWriteEnable();
    mEEPROMSSLow();

    SPIMPolPut(EEPROM_CMD_WRITE);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Hi(Address));
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Lo(Address));
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Data);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    mEEPROMSSHigh();

    // wait for completion of previous write operation
    while(EEPROMReadStatus().Bits.WIP);
}

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
void EEPROMWriteByteNotBlocking(unsigned Data, unsigned Address)
{
    EEPROMWriteEnable();
    mEEPROMSSLow();

    SPIMPolPut(EEPROM_CMD_WRITE);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Hi(Address));
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Lo(Address));
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Data);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    mEEPROMSSHigh();
  
}



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
unsigned EEPROMReadByte(unsigned Address){
unsigned Temp;
    mEEPROMSSLow();

    SPIMPolPut(EEPROM_CMD_READ);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Hi(Address));
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(Lo(Address));
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    SPIMPolPut(0);
    SPIMPolIsTransmitOver();
    Temp = mSPIMPolGet();

    mEEPROMSSHigh();
    return Temp;
}

/************************************************************************
* Function: EEPROMWriteEnable()                                         *
*                                                                       *
* Preconditions: SPI module must be configured to operate with EEPROM.  *
*                                                                       *
* Overview: This function allows a writing into EEPROM. Must be called  *
* before every writing command.                                         *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/
static unsigned char EEPROMWriteEnable(void)
{
#ifdef DEBUG_SLAVE
  unsigned char ret_val = EEPROM_WRITE_DONE2;
#else
unsigned char ret_val = EEPROM_WRITE_DONE;

#endif

  mEEPROMSSLow();
  ret_val = SPIMPolPut(EEPROM_CMD_WREN);
  ret_val = SPIMPolIsTransmitOver();
  mSPIMPolGet();
  mEEPROMSSHigh();
  
  return ret_val;  
}

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
union _EEPROMStatus_ EEPROMReadStatus(){
char Temp;

    mEEPROMSSLow();
    SPIMPolPut(EEPROM_CMD_RDSR);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();
    SPIMPolPut(0);
    SPIMPolIsTransmitOver();
    Temp = mSPIMPolGet();
    mEEPROMSSHigh();

    return (union _EEPROMStatus_)Temp;
}

void EEPROMWritetatus(unsigned char status){

    mEEPROMSSLow();
    SPIMPolPut(EEPROM_CMD_WRSR);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();
    
    SPIMPolPut(status);
    SPIMPolIsTransmitOver();
    mSPIMPolGet();

    mEEPROMSSHigh();
}

/************************************************************************
* Function: EEPROMWriteArray()                                           *
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
void EEPROMWriteArray(unsigned short StartAddress, unsigned short Length, unsigned char *DataPtr)
{
unsigned short i;
unsigned short address;


//EEPROMWritetatus();
//ReadStatus = EEPROMReadStatus().Char;

address = StartAddress;

for (i=0; i < Length; i++)
	{
  /* Reset Watchdog*/
	ClrWdt();
  EEPROMWriteByte(DataPtr[i], address);	
	address++;
	}	
}/*end EEPROMWriteArray*/

/**/
/*===========================================================================*/
/** 
**   @brief  Writes an array in an E2 page
**
**   @param  StartAddress : E2 start address
**   @param  Length : Number of bytes to write  
**   @param  DataPtr : Pointer to the array
** 
**   @return Operation result
**/
/*===========================================================================*/
/**/
unsigned char EEPROMWriteArrayInPage(unsigned short StartAddress, unsigned char Length, unsigned char *DataPtr)
{
  unsigned short i;
#ifdef DEBUG_SLAVE
  unsigned char ret_val = EEPROM_WRITE_DONE2;
#else
unsigned char ret_val = EEPROM_WRITE_DONE;
#endif

  if (EEPROMReadStatus().Bits.WIP)
  {
    return EEPROM_BUSY;
  }
  
  ret_val = EEPROMWriteEnable();
  mEEPROMSSLow();
  
  ret_val = SPIMPolPut(EEPROM_CMD_WRITE);
  ret_val = SPIMPolIsTransmitOver();
  mSPIMPolGet();
  
  ret_val = SPIMPolPut(Hi(StartAddress));
  ret_val = SPIMPolIsTransmitOver();
  mSPIMPolGet();
  
  ret_val = SPIMPolPut(Lo(StartAddress));
  ret_val = SPIMPolIsTransmitOver();
  mSPIMPolGet();
   
  for (i=0; i < Length; i++)
	{
    ret_val = SPIMPolPut(*DataPtr);
    DataPtr++;
    ret_val = SPIMPolIsTransmitOver();
    mSPIMPolGet();
	}	
	
	mEEPROMSSHigh();
	return ret_val;
}/*end EEPROMWriteArrayInPage*/


/************************************************************************
* Function: EEPROMReadArray()                                           *
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

void EEPROMReadArray(unsigned short StartAddress, unsigned short Length, unsigned char *DataPtr)
{
unsigned short i;
unsigned short address;

address = StartAddress;

for (i=0; i < Length; i++)
	{
  /* Reset Watchdog*/
  ClrWdt();
  DataPtr[i] = EEPROMReadByte(address);	
	address++;
	}	
}	/*end EEPROMReadArray*/
