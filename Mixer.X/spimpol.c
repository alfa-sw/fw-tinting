/* 
 * File:   spimpol.c
 * Author: michele.abelli
 * Description: This implements a generic library functionality to support SPI Master * for dsPIC/PIC24 family
 * Created on 6 Novembre 2018, 15.19
 */

/*===== INCLUSIONI ========================================================= */
#include "p24FJ256GB110.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "typedef.h"
#include "eepromManager.h"
#include "typedef.h"
#include "serialCom.h"
#include "eeprom.h"
#include "spimpol.h"
#include <xc.h>

/************************************************************************
* Function: SPIMPolInit                                                 *
*                                                                       *
* Preconditions: TRIS bits of SCK and SDO should be made output.        *
* TRIS bit of SDI should be made input. TRIS bit of Slave Chip Select   *
* pin (if any used) should be made output. Overview This function is    *
* used for initializing the SPI module. It initializes the module       *
* according to Application Maestro options.                             *
*                                                                       *
* Input: Application Maestro options                                    *
*                                                                       *
* Output: None                                                          *
*                                                                       *
************************************************************************/
void BL_SPIMPolInit()
{
/*
    // MSTEN Master; DISSDO disabled; PPRE 64:1; SPRE 8:1; MODE16 disabled; SMP Middle; DISSCK disabled; CKP Idle:Low, Active:High; CKE Idle to Active; SSEN disabled; 
    // SPI Clock Frequency: 7.8125kHz
    // Communication is byte wide, MODE16 disabled
    SPI3CON1 = 0x20;
    // SPIFSD disabled; SPIBEN enabled; SPIFPOL disabled; SPIFE disabled; FRMEN disabled; 
    // Enhanced Buffer Master Mode selected, SPIBEN enabled
    SPI3CON2 = 0x01;
    // SPITBF disabled; SISEL SPI_INT_SPIRBF; SPIRBF disabled; SPIROV disabled; SPIEN enabled; SRXMPT disabled; SPISIDL disabled; 
    SPI3STAT = 0x800C;
    // Al momento NON usiamo gli Interrupt
    // Enable alt 3 SPI3 Interrupts
    //    SPITXI: SPI3TX - SPI3 Transfer Done
    //    Priority: 1
//    IPC2bits.SPI3TXIP = 1;
    //    SPII: SPI3 - SPI3 General
    //    Priority: 1
//    IPC2bits.SPI3IP = 1;
    //    SPIRXI: SPI3RX - SPI3 Receive Done
    //    Priority: 1
//    IPC14bits.SPI3RXIP = 1;
*/    
// ---------------------------------------------    
  SPISTAT = 0;
  SPICON = (SPIM_PPRE|SPIM_SPRE);
  SPICONbits.MSTEN = 1;
  SPICON2 = 0;
  SPICONbits.MODE16 = SPIM_MODE16;
  SPICONbits.CKE = SPIM_CKE;
  SPICONbits.CKP = SPIM_CKP;
  SPICONbits.SMP = SPIM_SMP;
  SPIINTENbits.SPIIE = 0;
  SPIINTFLGbits.SPIIF = 0;
  SPISTATbits.SPIEN = 1;
}

/************************************************************************
* Function SPIMPolPut                                                   *
*                                                                       *
* Preconditions: 'SPIMPolInit' should have been called.                 *
* Overview: in non Blocking Option this function sends the byte         *
* over SPI bus and checks for Write Collision; in Blocking Option       *
* it waits for a free transmission buffer.                              *
*                                                                       *
* Input: Data to be sent.                                               *
*                                                                       *
* Output: 'This function returns ‘0’  on proper initialization of *
* transmission and ‘SPIM_STS_WRITE_COLLISION’ on occurrence of    *
* the Write Collision error.                                            *
*                                                                       *
************************************************************************/
unsigned char SPIMPolPut(unsigned Data)
{
  unsigned char i;

  for (i=0; i<128; i++)
    {
      if (SPISTATbits.SPITBF == 0)
        {
          SPIBUF = Data;
          return 0;
        }
    }

  return SPIM_STS_WRITE_COLLISION;
}

/************************************************************************
* Function: SPIMPolIsTransmitOver                                       *
*                                                                       *
* Preconditions: ‘SPIMPolPut’ should have been called.            *
* Overview: in non Blocking Option this function checks whether         *
* the transmission of the byte is completed; in Blocking Option         *
* it waits till the transmission of the byte is completed.              *
*                                                                       *
* Input: None                                                           *
*                                                                       *
* Output: in Blocking Option none and in non Blocking Option            *
* it returns: ’0’ - if the transmission is over,                  *
* SPIM_STS_TRANSMIT_NOT_OVER - if the transmission is not yet over.     *
*                                                                       *
************************************************************************/
unsigned char SPIMPolIsTransmitOver()
{
  unsigned char i;

  for (i=0; i<128; i++)
    {
      if (SPISTATbits.SPIRBF != 0)
        {
          return 0;
        }
    }

  return SPIM_STS_TRANSMIT_NOT_OVER;
}
