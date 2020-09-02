/* 
 * File:   serialCom.c
 * Author: michele.abelli
 * Description: Serial Communication 
 * Created on 16 luglio 2018, 14.18
 */

#include "serialCom.h"
#include "tintingmanager.h"
#include "p24FJ256GB110.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <xc.h>
#include "ram.h"
#include "define.h"
#include "gestIO.h"
#include "timerMg.h"
#include "mem.h"
#include "typedef.h"
#include "stepperParameters.h"
#include "serialcom_GUI.h"
#include "colorAct.h"
#include "autocapAct.h"

const unsigned char max_slave_retry[N_SLAVES-1] = {
  /* B1_BASE_IDX*/                5,
  /* B2_BASE_IDX*/                5,
  /* B3_BASE_IDX*/                5,
  /* B4_BASE_IDX*/                5,
  /* B5_BASE_IDX*/                5,
  /* B6_BASE_IDX*/                5,
  /* B7_BASE_IDX*/                5,
  /* B8_BASE_IDX*/                5,
  /* C1_COLOR_IDX*/               5,
  /* C2_COLOR_IDX*/               5,
  /* C3_COLOR_IDX*/               5,
  /* C4_COLOR_IDX*/               5,
  /* C5_COLOR_IDX*/               5,
  /* C6_COLOR_IDX*/               5,
  /* C7_COLOR_IDX*/               5,
  /* C8_COLOR_IDX*/               5,
  /* C0_COLOR_IDX*/               5,
  /* C10_COLOR_IDX*/              5,
  /* C11_COLOR_IDX*/              5,
  /* C12_COLOR_IDX*/              5,
  /* C13_COLOR_IDX*/              5,
  /* C14_COLOR_IDX*/              5,
  /* C15_COLOR_IDX*/              5,
  /* C16_COLOR_IDX*/              5,
  /* C17_COLOR_IDX*/              5,
  /* C18_COLOR_IDX*/              5,
  /* C19_COLOR_IDX*/              5,
  /* C28_COLOR_IDX*/              5,
  /* C21_COLOR_IDX*/              5,
  /* C22_COLOR_IDX*/              5,
  /* C23_COLOR_IDX*/              5,
  /* C24_COLOR_IDX*/              5,
  /*MOVE_X_AXIS_IDX*/             5,
  /*MOVE_Y_AXIS_IDX*/             5,
  /*STORAGE_CONTAINER1_IDX*/      5,
  /*STORAGE_CONTAINER2_IDX*/      5,
  /*STORAGE_CONTAINER3_IDX*/      5,
  /*STORAGE_CONTAINER4_IDX*/      5,
  /*PLUG_COVER_1_IDX*/            5,
  /*PLUG_COVER_2_IDX*/            5,
  /*MOVE_AUTOCAP_IDX*/            5,
  /*SGABELLO*/ 			          5,
  /*HUMIDIFIER*/ 		          5,
  /*TINTING*/ 			          5,
  /*GENERIC_ACT13_IDX*/           5,
  /*GENERIC_ACT14_IDX*/           5,
  /*GENERIC_ACT15_IDX*/           5,
  /*GENERIC_ACT16_IDX*/           5,
};

const unsigned short /*__attribute__((space(psv), section ("CRCTable")))*/ CRC_TABLE[256] = {
  0x0,0x0C0C1,0x0C181,0x140,0x0C301,0x3C0,0x280,0x0C241,
  0x0C601,0x6C0,0x780,0x0C741,0x500,0x0C5C1,0x0C481,0x440,
  0x0CC01,0x0CC0,0x0D80,0x0CD41,0x0F00,0x0CFC1,0x0CE81,0x0E40,
  0x0A00,0x0CAC1,0x0CB81,0x0B40,0x0C901,0x9C0,0x880,0x0C841,
  0x0D801,0x18C0,0x1980,0x0D941,0x1B00,0x0DBC1,0x0DA81,0x1A40,
  0x1E00,0x0DEC1,0x0DF81,0x1F40,0x0DD01,0x1DC0,0x1C80,0x0DC41,
  0x1400,0x0D4C1,0x0D581,0x1540,0x0D701,0x17C0,0x1680,0x0D641,
  0x0D201,0x12C0,0x1380,0x0D341,0x1100,0x0D1C1,0x0D081,0x1040,
  0x0F001,0x30C0,0x3180,0x0F141,0x3300,0x0F3C1,0x0F281,0x3240,
  0x3600,0x0F6C1,0x0F781,0x3740,0x0F501,0x35C0,0x3480,0x0F441,
  0x3C00,0x0FCC1,0x0FD81,0x3D40,0x0FF01,0x3FC0,0x3E80,0x0FE41,
  0x0FA01,0x3AC0,0x3B80,0x0FB41,0x3900,0x0F9C1,0x0F881,0x3840,
  0x2800,0x0E8C1,0x0E981,0x2940,0x0EB01,0x2BC0,0x2A80,0x0EA41,
  0x0EE01,0x2EC0,0x2F80,0x0EF41,0x2D00,0x0EDC1,0x0EC81,0x2C40,
  0x0E401,0x24C0,0x2580,0x0E541,0x2700,0x0E7C1,0x0E681,0x2640,
  0x2200,0x0E2C1,0x0E381,0x2340,0x0E101,0x21C0,0x2080,0x0E041,
  0x0A001,0x60C0,0x6180,0x0A141,0x6300,0x0A3C1,0x0A281,0x6240,
  0x6600,0x0A6C1,0x0A781,0x6740,0x0A501,0x65C0,0x6480,0x0A441,
  0x6C00,0x0ACC1,0x0AD81,0x6D40,0x0AF01,0x6FC0,0x6E80,0x0AE41,
  0x0AA01,0x6AC0,0x6B80,0x0AB41,0x6900,0x0A9C1,0x0A881,0x6840,
  0x7800,0x0B8C1,0x0B981,0x7940,0x0BB01,0x7BC0,0x7A80,0x0BA41,
  0x0BE01,0x7EC0,0x7F80,0x0BF41,0x7D00,0x0BDC1,0x0BC81,0x7C40,
  0x0B401,0x74C0,0x7580,0x0B541,0x7700,0x0B7C1,0x0B681,0x7640,
  0x7200,0x0B2C1,0x0B381,0x7340,0x0B101,0x71C0,0x7080,0x0B041,
  0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
  0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
  0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
  0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
  0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
  0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
  0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
  0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
};

void initBuffer(uartBuffer_t *buffer);
unsigned short CRCarea(unsigned char *pointer, unsigned short n_char,unsigned short CRCinit);
static serialSlave_t serialSlave;

#ifndef CAR_REFINISHING_MACHINE
static void resetBuffer(uartBuffer_t *buffer);

static uartBuffer_t rxBufferSlave;
static uartBuffer_t txBufferSlave;
static unsigned char currentSlave, deviceID;

static unsigned char getNextSlave(unsigned char lastSlave);
static unsigned char monitor_slave;

static void rebuildMessage(unsigned char receivedByte);
unsigned char IS_VALID_ID(unsigned char id);
#endif
void stuff_byte(unsigned char *buf, unsigned char *ndx, char c);


void initBuffer(uartBuffer_t *buffer)
/*
*//*=====================================================================*//**
**      @brief init buffer
**
**      @param buffer pointer to the buffer
**
**      @retval void
*//*=====================================================================*//**
*/
{
  memset(buffer->buffer, 0, BUFFER_SIZE);
  buffer->bufferFlags.allFlags = 0;

  buffer->status = WAIT_STX;
  buffer->index = 0;
  buffer->length = 0;
  buffer->escape = FALSE;
}

void set_slave_comm(unsigned short index)
/**/
/*===========================================================================*/
/**
**   @brief set GUI slave comm
**
**   @param index, the slave index
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned short addr, ofs;
    addr = index / 32;
    ofs  = index % 32;
    procGUI.slave_comm[addr] |= (1L << ofs);
}

unsigned short CRCarea(unsigned char *pointer, unsigned short n_char,unsigned short CRCinit)
/*
**=============================================================================
**
**      Oggetto        : Calcola CRC di una zona di byte specificata
**                       dai parametri di ingresso
**
**      Parametri      : pointer      Indirizzo iniziale dell'area
**                                    da controllare
**                       n_char       Numero dei bytes da includere nel calcolo
**                       CRCinit      Valore iniziale di CRC ( = 0 se n_char
**                                    copre l'intera zona da verificare,
**                                    = CRCarea della zona precedente se
**                                    si sta procedendo a blocchi
**
**      Ritorno        : CRCarea      Nuovo valore del CRC calcolato
**
**      Vers. - autore : 1.0  nuovo   G. Comai
**
**=============================================================================
*/
{

/* La routine proviene dalla dispensa "CRC Fundamentals", pagg. 196, 197. */

/* Nota sull'algoritmo: dato un vettore, se ne calcoli il CRC_16: se
   si accodano i 2 bytes del CRC_16 a tale vettore (low byte
   first!!!), il CRC_16 calcolato sul vettore così ottenuto DEVE
   valere zero.  Tale proprietà può essere sfruttata nelle
   comunicazione seriali per verificare che un messaggio ricevuto,
   contenente in coda i 2 bytes del CRC_16 (calcolati dal
   trasmettitore), sia stato inviato correttamente: il CRC_16,
   calcolato dal ricevente sul messaggio complessivo deve valere
   zero. */

  unsigned long i;
  unsigned short index;
//  unsigned char psv_shadow;

  /* save the PSVPAG */
//  psv_shadow = PSVPAG;

  /* set the PSVPAG for accessing CRC_TABLE[] */
//  PSVPAG = __builtin_psvpage (CRC_TABLE);

  for (i = 0; i < n_char; i++)
  {
    index = ( (CRCinit ^ ( (unsigned short) *pointer & 0x00FF) ) & 0x00FF);
    CRCinit = ( (CRCinit >> 8) & 0x00FF) ^ CRC_TABLE[index];
    pointer = pointer + 1;
    /* Reset Watchdog*/
    // ClrWdt();
  } /* end for */

  /* restore the PSVPAG for the compiler-managed PSVPAG */
//  PSVPAG = psv_shadow;

  return CRCinit;
} /* end CRCarea */

void stuff_byte(unsigned char *buf, unsigned char *ndx, char c)
/**/
/*===========================================================================*/
/**
**   @brief Writes c at the ndx-th position of buf, performing byte
**   stuffying if necessary. (*ndx) is incremented accordingly.
**
**   @param buf, the output buffer
**   @param ndx, a pointer to the current writing position in the output buffer
**   @param c, the character to be written
**
**/
/*===========================================================================*/
/**/
{
	unsigned char appoggio;
	appoggio=*ndx;
    // STX --> ESC TWO, ETX --> ESC THREE 
    if ((c == ASCII_STX) || (c == ASCII_ETX)) {
        buf[appoggio++]=ASCII_ESC;
        buf[appoggio++]=c + ASCII_ZERO;
    }
    // ESC --> ESC ZERO 
    else if (c == ASCII_ESC) {
        buf[appoggio++]=ASCII_ESC;
        buf[appoggio++]=ASCII_ZERO;
    }
    // Regular char, nothing fancy here 
    else
        buf[appoggio++]=c;
    
    *ndx=appoggio;
}

unsigned char CHECK_CRC16(uartBuffer_t *buf)
{
	unsigned short crc;
	unsigned short appoggio;

	appoggio=CRCarea(buf->buffer, buf->length, NULL);
	crc=(unsigned short)((buf->buffer[buf->index - 4]-0x20) << 0xC);
	crc|=(unsigned short)((buf->buffer[buf->index - 3]-0x20) << 0x8);
	crc|=(unsigned short)((buf->buffer[buf->index - 2]-0x20) << 0x4);
	crc|=(unsigned short)((buf->buffer[buf->index - 1]-0x20));

	if (crc==appoggio)
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}

void unstuffMessage(uartBuffer_t *buffer)
/**/
/*===========================================================================*/
/**
**   @brief Performs byte unstuffying on rx buffer. This function is a
**   private service of rebuildMessage()
**
**/
/*===========================================================================*/
/**/
{
  unsigned char i, j, c;

  /* skip 3 bytes from frame head: [ STX, ID, LEN ] */
  unsigned char *p = buffer->buffer + FRAME_PAYLOAD_START;

  /* i is the read index, j is the write index. For each iteration, j
   * is always incremented by 1, i may be incremented by 1 or 2,
   * depending on whether p[i] is a stuffed character or not. At the
   * end of the cycle (length bytes read) j is less than or equal to
   * i. (i - j) is the amount that must be subtracted to the payload
   * length. */

  i = j = 0;
  while (i < buffer->length) {
    c = *(p + i);
    ++ i;

    if (c == ASCII_ESC)
    {
      c = *(p + i) - ASCII_ZERO;
      ++ i;

      if (!c)
      {
        *(p + j) = ASCII_ESC;
      }
      else
      {
        *(p + j) = c;
      }
    }
    else
    {
      *(p + j) = c;
    }

    ++ j;
  }

  /* done with unstuffying, now fix payload length. */
  buffer->length -= (i - j);
}

#ifndef CAR_REFINISHING_MACHINE

static void resetBuffer(uartBuffer_t *buffer)
{
	buffer->bufferFlags.allFlags = 0;
	
	buffer->status = WAIT_STX;
	buffer->index = 0;
	buffer->length = 0;
	buffer->escape = FALSE;
}

void initSerialCom(void)
/*
*//*=====================================================================*//**
**      @brief Set UART3 registers; reset receiver and transmission
**             buffers and flags. Start the FIRST_LINK timer window
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
    unsigned char i;

    for (i = 0; i < N_SLAVES-1; i++) {
      numErroriSerial[i] = 0;
    }
    // Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    // EN TX
    LATDbits.LATD11 = 1;
    TRISDbits.TRISD11 = OUTPUT;
    // EN RX
    LATDbits.LATD12 = 0;
    TRISDbits.TRISD12 = INPUT;
    // UART3 ENABLE MULTIPROCESSOR RD13
    RS485_DE = 0;
    TRISDbits.TRISD13  = OUTPUT;
	// BaudRate = 115200; Clock Frequency = 32MHz; 
    U3BRG = 33;    
    // UARTEN = Disabled - USIDL = Continue module operation in Idle Mode - IREN = IrDa Encoder and Decoder disabled - RTSMD = UxRTS pin in Flow Control Mode
    // UEN1:UEN0 = UxTX and UxRX pins are enabled and used - WAKE = No Wake-up enabled - LPBACK = Loopback mode is disabled - ABAUD = Baud rate measurement disabled or completed
    // RXINV = UxRX Idle state is '1' - BRGH = High-Speed mode - PDSEL = 8-bit data, no parity - STSEL = One Stop bit 
    U3MODE = 0x08;
    U3STA = 0x00;
    // Enabling UARTEN bit
    U3MODEbits.UARTEN  = 1;      
    // Interrupt when last char is tranferred into TSR Register: so transmit buffer is empty
    U3STAbits.UTXISEL1 = 0;
    U3STAbits.UTXISEL0 = 1;
    // Transmit Enable
    U3STAbits.UTXEN = 1; 
    // Reset Interrupt flags
    IFS5bits.U3RXIF = 0;
    IFS5bits.U3TXIF = 0;
    // Start RX
    IEC5bits.U3RXIE = 1;
        
    initBuffer(&rxBufferSlave);
    initBuffer(&txBufferSlave);
}

static void rebuildMessage(unsigned char receivedByte)
/**/
/*=====================================================================*/
/**
**      @brief Called by  _U1RXInterrupt: update the rx buffer  with
**             subsequent received bytes
**
**      @param receivedByte received bytes
**
**      @retval void
**/
/*=====================================================================*/
/**/
{
    if (! IS_ERROR(rxBufferSlave)) {
        switch(rxBufferSlave.status){
            case WAIT_STX:
                if (receivedByte == ASCII_STX) {
                    STORE_BYTE_MIO( rxBufferSlave, receivedByte );
                    rxBufferSlave.status = WAIT_ID;
                    monitor_slave = currentSlave;
                }
            break;

            case WAIT_ID:
                STORE_BYTE_MIO( rxBufferSlave, receivedByte );
                deviceID = REMOVE_OFFSET(receivedByte);                
                if (! IS_VALID_ID(deviceID)) {
                  resetBuffer(&rxBufferSlave);
                }
                else {
                  rxBufferSlave.status = WAIT_LENGTH;
                }
            break;

            case WAIT_LENGTH:
                STORE_BYTE_MIO( rxBufferSlave, receivedByte );
                if ( receivedByte < ADD_OFFSET( MIN_FRAME_SIZE) || receivedByte > ADD_OFFSET( MAX_FRAME_SIZE) ) {
                    resetBuffer(&rxBufferSlave);
                }
                else {
                    // The length embedded in the frame takes into account the
                    // entire frame length, for ease of implementation of tx/rx
                    // code. Here we discard the final 5 bytes (4 CRC + ETX). Later
                    // on, after the crc check, we'll be able to discard also the
                    // initial overhead [ STX, ID, LEN ] */
                    rxBufferSlave.length  = REMOVE_OFFSET(receivedByte);
                    rxBufferSlave.length -= FRAME_END_OVERHEAD;
                    rxBufferSlave.status = WAIT_DATA;
                }
            break;

            case WAIT_DATA:
                // Check stuffying encoding 
                if (IS_ESCAPE(rxBufferSlave)) {
                    // ESC ZERO --> ESC, ESC TWO --> STX, ESC THREE --> ETX 
                    if (receivedByte != ASCII_ZERO && receivedByte != ASCII_TWO && receivedByte != ASCII_THREE) 
                        // Ilegal encoding detected 
                        resetBuffer(&rxBufferSlave);
   
                    CLEAR_ESCAPE(rxBufferSlave);
                }
                else {
                    if (receivedByte == ASCII_ESC)
                        SIGNAL_ESCAPE(rxBufferSlave);
                }
                STORE_BYTE_MIO( rxBufferSlave, receivedByte );
                if (rxBufferSlave.index == rxBufferSlave.length)
                    rxBufferSlave.status = WAIT_CRC;
            break;

            case WAIT_CRC:
                // Received four CRC bytes? 
                STORE_BYTE_MIO( rxBufferSlave, receivedByte );
                if (rxBufferSlave.index == FRAME_CRC_LENGTH + rxBufferSlave.length) 
                    rxBufferSlave.status = WAIT_ETX;
            break;

            case WAIT_ETX:
                if (rxBufferSlave.bufferFlags.rxCompleted)
                    break;
                if (receivedByte != ASCII_ETX || ! CHECK_CRC16(&rxBufferSlave)) {
                    resetBuffer(&rxBufferSlave);
                    Nop();
                    Nop();
                }
                else {
                    STORE_BYTE_MIO(rxBufferSlave, receivedByte);
                    rxBufferSlave.length -= FRAME_PAYLOAD_START;
                    // Frame ok, answer from current slave? 
                    rxBufferSlave.deviceletto=deviceID;
                    rxBufferSlave.deviceDedotto=SLAVE_DEVICE_ID(monitor_slave);
                    if (deviceID == SLAVE_DEVICE_ID(monitor_slave)) {
                        // After the CRC check, we can now "unstuff" the payload 
                        unstuffMessage(&rxBufferSlave);
                        rxBufferSlave.bufferFlags.rxCompleted = TRUE;
                        // Set corresponding bit in comm status word (DEBUG only) 
                        set_slave_comm(monitor_slave);
                    }
                    else
                      // Not an error, discard msg 
                      resetBuffer(&rxBufferSlave);
                }
            break;

            default:
                resetBuffer(&rxBufferSlave);
            break;
        } // switch 
    } // RebuildMessage() 
}

unsigned char IS_VALID_ID(unsigned char id)
{
	if (((0 < (id)) && ((id) <= (N_SLAVES-1))) || ((100 < (id)) && ((id) <= 100 + (N_SLAVES-1))))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void updateSerialComFunct_Act(unsigned char currentSlave)
/**/
/*===========================================================================*/
/**
 **   @brief point to decode/make message function releted to currentslave
 **
 **   @param slave id
 **
 **   @return void
 **/
/*===========================================================================*/
/**/
{
    switch (currentSlave) {

    #ifndef AUTOCAP_MMT        
        case (AUTOCAP_ID - 1):
                serialSlave.makeSerialMsg = &makeAutocapActMessage;
                serialSlave.decodeSerialMsg = &decodeAutocapActMessage;
                serialSlave.lastMsg[currentSlave] =
                autocapAct.typeMessage;
        break;
    #endif                            
        
        default: // BASE COLOR acts 
            serialSlave.makeSerialMsg = &makeColorActMessage;
            serialSlave.decodeSerialMsg = &decodeColorActMessage;
            serialSlave.lastMsg[currentSlave] =
            colorAct[currentSlave].typeMessage;
        break;
   }
} // updateSerialComFunct_Act() 

static void makeMessage_Act()
/*
*//*=====================================================================*//**
**   @brief Management of the fixed time window Display-slaves:
**          if the answer from the Involved slave is received,
**          the following actions are performed:
**             - update the serialSlave struct for the subsequent slave
**               interrogated,
**             - make the packet to be transmitted, calling the message
**               make function related to the new slave
**          If the time window is elapsed without answer and the number
**          of retries is lower than admitted:
**             - increase the number of retries for the current slave
**             - send again the message to the same slave
**
**  @retval void
**
*//*=====================================================================*//**
*/
{
    if (StatusTimer(T_FIRST_LINK_ACT_TIMER) == T_HALTED) {
        // Start first link timer 
        StartTimer(T_FIRST_LINK_ACT_TIMER);
    }
    if (StatusTimer (T_SLAVE_WINDOW_TIMER) == T_HALTED) {
        // Interrogate the first slave 
        currentSlave = B1_BASE_IDX;
        if (isSlaveCircuitEn(currentSlave) == FALSE)
            currentSlave = getNextSlave(currentSlave);
//        if (currentSlave != 0) {
            txBufferSlave.bufferFlags.startTx = TRUE;
            updateSerialComFunct_Act(currentSlave);
//        }
    }
    else if ((rxBufferSlave.bufferFlags.decodeDone == TRUE) || (StatusTimer(T_SLAVE_WINDOW_TIMER) == T_ELAPSED))  {
        if  ((rxBufferSlave.bufferFlags.decodeDone == TRUE) || (! isSlaveCircuitEn(currentSlave)) || ((StatusTimer(T_SLAVE_WINDOW_TIMER) == T_ELAPSED) &&
              (StatusTimer(T_FIRST_LINK_ACT_TIMER) == T_RUNNING))) {
            serialSlave.numRetry[currentSlave] = 0;
            serialSlave.answer[currentSlave] = TRUE;
            numErroriSerial[currentSlave]=0;
        }
        // Avoid overflows ! 
        else if (StatusTimer(T_SLAVE_WINDOW_TIMER) == T_ELAPSED && serialSlave.numRetry[currentSlave] < max_slave_retry[currentSlave]) {
            resetBuffer(&rxBufferSlave);
            ++ serialSlave.numRetry[currentSlave];
        }
        // Trace the number of error also when it goes through the alarm limit
        if (StatusTimer(T_SLAVE_WINDOW_TIMER)==T_ELAPSED) {
            resetBuffer(&rxBufferSlave);
            if (numErroriSerial[currentSlave]<NUM_MAX_ERROR_TIMEOUT)
                numErroriSerial[currentSlave]++;
        }
        // Interrogate the next slave
        if ((StatusTimer(T_DELAY_INTRA_FRAMES) == T_HALTED) || (StatusTimer(T_DELAY_INTRA_FRAMES) == T_ELAPSED))  {
            rxBufferSlave.bufferFlags.decodeDone = FALSE;
            currentSlave = getNextSlave(currentSlave);
//            if (currentSlave != 0) {
                txBufferSlave.bufferFlags.startTx = TRUE;
                serialSlave.answer[currentSlave] = FALSE;
                updateSerialComFunct_Act(currentSlave);
//            }            
        }
    } 
    if (txBufferSlave.bufferFlags.startTx == TRUE) {
        StopTimer(T_DELAY_INTRA_FRAMES);
        initBuffer(&txBufferSlave);
        serialSlave.makeSerialMsg(&txBufferSlave,currentSlave);
        txBufferSlave.bufferFlags.txReady = TRUE;
        NotRunningTimer(T_SLAVE_WINDOW_TIMER);
    }
}

static void decodeMessage_Act()
/**/
/*===========================================================================*/
/**
**   @brief decode the received message, calling the decode
**             function related to the Involved slave: call to
**             serialSlave->decodeSerialMsg(&rxBuffer)
**
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    if (rxBufferSlave.bufferFlags.rxCompleted == TRUE) {
        serialSlave.decodeSerialMsg(&rxBufferSlave,currentSlave);
        initBuffer(&rxBufferSlave);
        rxBufferSlave.bufferFlags.decodeDone = TRUE;
        StartTimer(T_DELAY_INTRA_FRAMES);
    }
}

static void sendMessage_Act()
/**/
/*===========================================================================*/
/**
**   @brief Start the transmission, enabling the UART 3 transmission
**             flag and filling the UART3 tx buffer with the first byte
**             to be transmitted
**
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    if (txBufferSlave.bufferFlags.txReady == TRUE) {
        if (txBufferSlave.bufferFlags.uartBusy != TRUE && txBufferSlave.length <= BUFFER_SIZE) {
            // Enable Tx multiprocessor line
            RS485_DE = 1;

            // Pulisco l'interrupt flag della trasmissione
            IFS5bits.U3TXIF = 0;

            // Abilito il flag UARTx En
            IEC5bits.U3TXIE = 1;

            // Scarico il primo byte nel buffer di trasmissione : Write data byte to lower byte of UxTXREG word Take control of buffer
            txBufferSlave.bufferFlags.uartBusy = TRUE;

            if (U3STAbits.TRMT) 
                U3TXREG = txBufferSlave.buffer[txBufferSlave.index ++];
            else {
                U3MODEbits.UARTEN = 0;
                initSerialCom();
            }
        }
    }
    else if (txBufferSlave.bufferFlags.txReady == FALSE && txBufferSlave.bufferFlags.uartBusy == FALSE && StatusTimer(T_SLAVE_WINDOW_TIMER) == T_NOT_RUNNING)
        StartTimer(T_SLAVE_WINDOW_TIMER);
}

static unsigned char getNextSlave(unsigned char lastSlave)
/**/
/*===========================================================================*/
/**
**   @brief Returns next slave to be interrogated
**
**   @param void
**
**   @return slave id
**
**/
/*===========================================================================*/
/**/
{
    unsigned char lastIndex;

    if ((serialSlave.priority[lastSlave] == SC_SLOW_PRIORITY) && (serialSlave.priority[fastIndex] == SC_FAST_PRIORITY))
        return(fastIndex);
    else {
        // L'ultimo pacchetto è un SC_FAST_PRIORITY, si ricerca il successivo SC_FAST_PRIORITY, altrimenti se sono finiti, si spedisce un SC_SLOW_PRIORITY
        lastIndex = fastIndex;
        fastIndex = (fastIndex+1)%(N_SLAVES-1);

#ifdef AUTOCAP_MMT        
        // Search next high priority slave
        while ( ((fastIndex == (AUTOCAP_ID-1)) || (!isSlaveCircuitEn(fastIndex)) || isSlaveJumpToBootSent(fastIndex) || (serialSlave.priority[fastIndex] != SC_FAST_PRIORITY)) && (fastIndex !=lastIndex)) {
          fastIndex = (fastIndex+1)%(N_SLAVES-1);
        }
#else
        // Search next high priority slave
        while ( ((!isSlaveCircuitEn(fastIndex)) || isSlaveJumpToBootSent(fastIndex) || (serialSlave.priority[fastIndex] != SC_FAST_PRIORITY)) && (fastIndex !=lastIndex)) {
          fastIndex = (fastIndex+1)%(N_SLAVES-1);
        }
#endif        
        if (fastIndex <=lastIndex) {
            // All high priority slaves have been interrogated: Search next slow priority slave
            lastIndex = slowIndex;
            slowIndex = (slowIndex+1)%(N_SLAVES-1);

#ifdef AUTOCAP_MMT  
            while ( ( (slowIndex == (AUTOCAP_ID-1)) || (!isSlaveCircuitEn(slowIndex)) || isSlaveJumpToBootSent(slowIndex) || ( (procGUI.circuit_pump_types[slowIndex] == PUMP_DOUBLE) && (slowIndex%2 != 0) ) || 
                   (isColorCircuit(slowIndex)) || (serialSlave.priority[slowIndex]  != SC_SLOW_PRIORITY)) && (slowIndex !=lastIndex))
#else
            while ( ((!isSlaveCircuitEn(slowIndex)) || isSlaveJumpToBootSent(slowIndex) || ( (procGUI.circuit_pump_types[slowIndex] == PUMP_DOUBLE) && (slowIndex%2 != 0) ) || 
                   (isColorCircuit(slowIndex)) || (serialSlave.priority[slowIndex]  != SC_SLOW_PRIORITY)) && (slowIndex !=lastIndex))
#endif                
            {
                slowIndex = (slowIndex+1)%(N_SLAVES-1);
            }
            if ((slowIndex != lastIndex) || (serialSlave.priority[fastIndex]  != SC_FAST_PRIORITY) || ((serialSlave.priority[lastSlave] == SC_FAST_PRIORITY) && (serialSlave.priority[slowIndex]  == SC_SLOW_PRIORITY)))
            {
                Nop();
                Nop();                
                return(slowIndex);
            }
            else {
                Nop();
                Nop();
                return(fastIndex);
            }
        }
        else {
            Nop();
            Nop();
            return (fastIndex);
        }
    }
} // getNextSlave() 

int isSlaveJumpToBootSent(int slave_id)
/**/
/*==========================================================================*/
/**
**   @brief  Return TRUE if slave has Received JumpToBoot command
**
**   @param  void
**
**   @return TRUE/FALSE
**/
/*==========================================================================*/
/**/
{	
	if ((slave_id >= B1_BASE_IDX) && (slave_id <= (C24_COLOR_ID - 1)) ) {
		if (colorAct[slave_id].colorFlags.jump_to_boot == TRUE)
			return TRUE;
		else
			return FALSE;			
	}
#ifndef NOLAB	
	else if (slave_id == (AUTOCAP_ID - 1)) {	
		if (autocapAct.autocapFlags.jump_to_boot == TRUE)
			return TRUE;
		else
			return FALSE;			
	}
#endif	
	else
		return TRUE;		
}

static void updateMsgPriority()
/**/
/*===========================================================================*/
/**
**   @brief Update serialSlave.priority[i] 
**          
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    int i;
    for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
        switch (colorAct[i].typeMessage) {
            case DISPENSAZIONE_BASE:
            case DISPENSAZIONE_COLORE:
            case DISPENSAZIONE_COLORE_CONT:
            case DISPENSAZIONE_GRUPPO_DOPPIO:
            case DISPENSAZIONE_COLORE_CONT_GRUPPO_DOPPIO:
                serialSlave.priority[i] = SC_FAST_PRIORITY;
            break;

            default:
                serialSlave.priority[i] = SC_SLOW_PRIORITY;
            break;
        }
    } 
} // updateMsgPriority() 

void serialCommManager_Act()
/**/
/*===========================================================================*/
/**
**   @brief Sequencer of the module
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    decodeMessage_Act();
    makeMessage_Act();
    sendMessage_Act();
    updateMsgPriority();
}

/******************************************************************************/
/****************************** Interrupt Routine *****************************/
/******************************************************************************/

void U3TX_InterruptHandler(void)
/*
*//*=====================================================================*//**
**      @brief Interrupt in tx della UART1
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
    if (_U3TXIE && _U3TXIF) {
        _U3TXIF = 0;        
        if (txBufferSlave.index >= txBufferSlave.length) {
            // Disable Tx multiprocessor line
            RS485_DE = 0;
            // Disabilito il flag UARTx En
            IEC5bits.U3TXIE = 0;
            txBufferSlave.bufferFlags.uartBusy = FALSE;
            txBufferSlave.bufferFlags.txReady = FALSE;
        }
        else
            U3TXREG = txBufferSlave.buffer[txBufferSlave.index ++];
    }
}

void U3RX_InterruptHandler(void)
/*
 *//*=====================================================================*//**
**      @brief Interrupt in tx della UART2
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
    register unsigned char flushUart;
    countBuffRx485 = 0;    
    if (_U3RXIE && _U3RXIF) {
        _U3RXIF = 0;

        // Overrun Error
        if (U3STAbits.OERR) {
            // Segnalazione Overrun Error 
            U3STAbits.OERR = 0;
            // When there's a communication error I reset the buffer immediately
            resetBuffer(&rxBufferSlave);
        }
        
        // Framing Error
        if (U3STAbits.FERR) {
            flushUart = U3RXREG;
            // Segnalazione Framing Error 
            // When there's a communication error I reset the buffer immediately
            resetBuffer(&rxBufferSlave);
        }        
        // Parity Error Check absent
        while (U3STAbits.URXDA && countBuffRx485 < URXREG_NUM_BYTES) {
            flushUart = U3RXREG;
            rebuildMessage(flushUart);    
            countBuffRx485++;
        }              
//        rebuildMessage(U3RXREG);       
    }
}

#endif

int isSlaveCircuitEn(int slave_id)
/**/
/*==========================================================================*/
/**
**   @brief  Return TRUE if slave is present
**
**   @param  void
**
**   @return TRUE/FALSE
**/

/*==========================================================================*/
/**/
{
  return procGUI.slaves_en[slave_id / 8] & 1 << slave_id % 8;
}

int isSlaveTimeout(int i)
{
    if (serialSlave.numRetry[i] >= max_slave_retry[i])
        return TRUE;
    else
        return FALSE;        
}

unsigned char getNumErroriSerial(unsigned char slave)
{
	if (slave>=(N_SLAVES-1))
	{
		return 0;
	}
	return numErroriSerial[slave];
}

void setAttuatoreAttivo(unsigned char attuatore,unsigned char value)
{
	if (attuatore>=(N_SLAVES-1))
		return;
	attuatoreAttivo[attuatore] = value;
}

void resetSlaveRetries()
{
    int i;
    for (i = 0; i < (N_SLAVES-1); ++ i) {
        serialSlave.answer[i]   = FALSE;
        serialSlave.numRetry[i] = 0;
    }
}
