/* 
 * File:   spi.c
 * Author: mballerini
 *
 * Created on 24 ottobre 2018, 17.13
 */

/*===== INCLUSIONI ========================================================== */
#include "p24FJ256GB110.h"
#include "define.h"
#include "spi.h"


/* ===== MACRO LOCALI ====================================================== */


/* ===== TIPI LOCALI ======================================================= */
/* ===== VARIABILI LOCALI ================================================== */
/* ===== COSTANTI LOCALI =================================================== */
/* ===== PROTOTIPI FUNZIONI LOCALI ========================================= */
void spi_remapping(unsigned char spi_index)
{
    switch (spi_index)
    {
        case SPI_1: //DRIVER
           //SDI1 = RP13
            RPINR20bits.SDI1R = SDI1_IO; 
            TRISBbits.TRISB2 = 1;
            //RP0 = SCK1
            RPOR0bits.RP0R = SCK1_IO;
            TRISBbits.TRISB0 = 0;
            //RP1 = SDO1
            AD1PCFGLbits.PCFG1 = 1;
            AD1PCFGLbits.PCFG2 = 1;
            RPOR0bits.RP1R = SDO1_IO;
            TRISBbits.TRISB1 = 0;
            
            break;
        
        case SPI_2: // EEPROM
            //SDI2 = RP26
            RPINR22bits.SDI2R = SDI2_IO; 
            TRISGbits.TRISG7 = 1;
            //RP21 = SCK2
            RPOR10bits.RP21R = SCK2_IO;
            TRISGbits.TRISG6 = 0;
            //RP19 = SDO2
            RPOR9bits.RP19R = SDO2_IO;
            TRISGbits.TRISG8 = 0;
            break;
            
        case SPI_3: //Sensore di temperatura
            //SDI3 = RP4
            //RPINR28bits.SDI3R = SDI3_IO; 
            //RP21 = SCK3
            //RPOR11bits.RP23R = SCK3_IO;
            //RP19 = SDO3
            //RPOR5bits.RP11R = SDO3_IO;
            break;    
    }
}

void spi_init(unsigned char spi_index)
/**/
/*===========================================================================*/
/**
**   @brief This function is used for initializing the SPI module.
**
**          Preconditions: TRIS bits of SCK and SDO should be made
**          output.  TRIS bit of SDI should be made input. TRIS bit of
**          Slave Chip Select pin (if any used) should be made output.
**
**   @param  spi_index index of SPI peripheral
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  switch (spi_index)
  {
  case SPI_1:
    SPI1STAT = 0;
    SPI1CON2 = 0;

    SPI1CON1 = (SPIM_PPRE | SPIM_SPRE); 
    SPI1STATbits.SPIEN = 0;
    SPI1CON1bits.MSTEN = 1; // 1: Master mode, 0: slave mode
    SPI1CON1bits.MODE16 = 0; // 0: communication is byte-wide, 1: word-wide
    SPI1CON1bits.CKE = 0; // Clock Edge Select bit, it depends on the slave
    SPI1CON1bits.CKP = 1;  // Clock Polarity Select bit. it depends on the slave
    SPI1CON1bits.SMP = 1; // Data Input Sample Phase. it depends on the slave
    
    //fSCK= 5 MHz
    //SPI1CON1bits.SPRE = 0; // 3 bit to set secondary Prescale, 0=8:1
    //SPI1CON1bits.SPRE = 3; // 2 bit to set primary Prescale, 3= 1:1
    //IEC0bits.SPI1IE = 0;
    //IFS0bits.SPI1IF = 0;
   
    SPI1STATbits.SPIEN = 1;
    break;

  case SPI_2:
    SPI2STAT = 0;
    SPI2CON2 = 0;

    // SPI2CON1 = (SPIM_PPRE | SPIM_SPRE); 
    SPI2STATbits.SPIEN = 0;
    SPI2CON1bits.MSTEN = 1; // 1: Master mode, 0: slave mode
    SPI2CON1bits.MODE16 = 0; // 0: communication is byte-wide, 1: word-wide
    SPI2CON1bits.CKE = 0; // Clock Edge Select bit, it depends on the slave
    SPI2CON1bits.CKP = 1;  // Clock Polarity Select bit. it depends on the slave
    SPI2CON1bits.SMP = 1; // Data Input Sample Phase. it depends on the slave
    
    //fSCK= 5 MHz
    //SPI2CON1bits.SPRE = 0; // 3 bit to set secondary Prescale, 0=8:1
    //SPI2CON1bits.SPRE = 3; // 2 bit to set primary Prescale, 3= 1:1
    //IEC0bits.SPI2IE = 0;
    //IFS0bits.SPI2IF = 0;
   
    SPI2STATbits.SPIEN = 1;
    break;
    
  case SPI_3:
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
    break;    
    
  }

}

void SpiSendData(Spi_buffer_t * txBuffer)
/*
*//*=====================================================================*//**
**      @brief Send data
**
**      @param  txBuffer pointer to tx buffer
**
**      @retval void
**
*//*=====================================================================*//**
*/
{
  while(SPI1STATbits.SPITBF);
  // write  data
  SPI1BUF =txBuffer->buffer[txBuffer->index++];
  
}

void SpiReadData(Spi_buffer_t * rxBuffer)
/*
*//*=====================================================================*//**
**      @brief Read data
**
**      @param  rxBuffer pointer to rx buffer
**
**      @retval void
**
*//*=====================================================================*//**
*/
{
  while(!SPI1STATbits.SPIRBF);//while(! _SPIRBF);
  //read data
  rxBuffer->buffer[rxBuffer->index++] = SPI1BUF;
}


void SpiSendByte(uint8_t byte)
/*
*//*=====================================================================*//**
**      @brief Send 1 byte
**
**      @param  byte to send
**
**      @retval void
**
*//*=====================================================================*//**
*/
{
  while(SPI1STATbits.SPITBF);
  // write  data
  SPI1BUF = byte;
    
}

uint8_t SpiRecvByte(void)
/*
*//*=====================================================================*//**
**      @brief receive 1 byte
**
**      @param  void
**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
**      @retval response
**
*//*=====================================================================*//**
*/
{

    while(!SPI1STATbits.SPIRBF);//while(! _SPIRBF);
  //read data
  return SPI1BUF;
    
}