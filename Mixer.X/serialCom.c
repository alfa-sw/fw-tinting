/* 
 * File:   serialCom.c
 * Author: michele.abelli
 * Description: Serial Communication 
 * Created on 16 luglio 2018, 14.18
 */

#include "serialCom.h"
#include "statusmanager.h"
#include "p24FJ256GB110.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ram.h"
#include "define.h"
#include "gestIO.h"
#include "timerMg.h"
#include "mem.h"
#include "typedef.h"
#include "stepperParameters.h"

#define SW_VERSION (0x50000)

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

static uartBuffer_t rxBuffer;
static uartBuffer_t txBuffer;
static serialSlave_t serialSlave;
static unsigned char deviceID;

static void initBuffer(uartBuffer_t *buffer);
static void resetBuffer(uartBuffer_t *buffer);
static void rebuildMessage(unsigned char receivedByte);
void STORE_BYTE(uartBuffer_t *buf, unsigned char c);
unsigned char IS_VALID_ID(unsigned char id);
unsigned char CHECK_CRC16(uartBuffer_t *buf);
static void unstuffMessage();
void MakeTintingMessage(uartBuffer_t *txBuffer, unsigned char slave_id);
void stuff_byte(unsigned char *buf, unsigned char *ndx, char c);
unsigned short CRCarea(unsigned char *pointer, unsigned short n_char,unsigned short CRCinit);
void DecodeTintingMessage(uartBuffer_t *rxBuffer, unsigned char slave_id);
static void decodeMessage(void);
static void makeMessage (void);
static void sendMessage(void);

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
    // UARTEN = Disabled - USIDL = Continue module operation in Idle Mode - IREN = IrDa Encoder and Decoder disabled - RTSMD = UxRTS pin in Flow Control Mode
    // UEN1:UEN0 = UxTX and UxRX pins are enabled and used - WAKE = No Wake-up enabled - LPBACK = Loopback mode is disabled - ABAUD = Baud rate measurement disabled or completed
    // RXINV = UxRX Idle state is '1' - BRGH = High-Speed mode - PDSEL = 8-bit data, no parity - STSEL = One Stop bit 
    U3MODE = 0x08;

    U3STA = 0x00;
	// BaudRate = 115200; Clock Frequency = 32MHz; 
    U3BRG = 33;
    
    // Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    // EN TX
    LATDbits.LATD11 = 1;
    TRISDbits.TRISD11 = OUTPUT;
    // EN RX
    LATDbits.LATD12 = 0;
    TRISDbits.TRISD12 = INPUT;
    
    // Enabling UARTEN bit
    U3MODEbits.UARTEN = 1;      
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
    
    // UART3 ENABLE MULTIPROCESSOR RD13
    RS485_DE = 0;
// -----------------------------------------------------------------------------
  initBuffer(&rxBuffer);
  initBuffer(&txBuffer);

  serialSlave.makeSerialMsg=&MakeTintingMessage;
  serialSlave.decodeSerialMsg=&DecodeTintingMessage;
}


static void initBuffer(uartBuffer_t *buffer)
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

static void resetBuffer(uartBuffer_t *buffer)
{
	buffer->bufferFlags.allFlags = 0;
	
	buffer->status = WAIT_STX;
	buffer->index = 0;
	buffer->length = 0;
	buffer->escape = FALSE;

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

  if (!rxBuffer.bufferFlags.serialError)
  {
    switch(rxBuffer.status)
    {

    case WAIT_STX:
      if (receivedByte == ASCII_STX)
      {
        STORE_BYTE(&rxBuffer, receivedByte);
        rxBuffer.status = WAIT_ID;
      }
      break;

    case WAIT_ID:
      STORE_BYTE(&rxBuffer, receivedByte);
      deviceID = receivedByte-0x20;
      if (!IS_VALID_ID(deviceID))
      {
		  resetBuffer(&rxBuffer);					  \
      }
      else
      {
        rxBuffer.status = WAIT_LENGTH;
      }
      break;

    case WAIT_LENGTH:
      STORE_BYTE(&rxBuffer, receivedByte);
      if ( receivedByte < (MIN_FRAME_SIZE+0x20) || receivedByte > ( MAX_FRAME_SIZE+0x20) )
      {
		  resetBuffer(&rxBuffer);					  \
      }
      else
      {
        /* The length embedded in the frame takes into account the
         * entire frame length, for ease of implementation of tx/rx
         * code. Here we discard the final 5 bytes (4 CRC + ETX). Later
         * on, after the crc check, we'll be able to discard also the
         * initial overhead [ STX, ID, LEN ] */
        rxBuffer.length  = receivedByte-0x20;
        rxBuffer.length -= FRAME_END_OVERHEAD;

        rxBuffer.status = WAIT_DATA;
      }
      break;

    case WAIT_DATA:
      /* check stuffying encoding */
      if (rxBuffer.escape)
      {
        /* ESC ZERO --> ESC, ESC TWO --> STX, ESC THREE --> ETX */
        if (receivedByte != ASCII_ZERO &&
            receivedByte != ASCII_TWO &&
            receivedByte != ASCII_THREE)
        {
          /* Ilegal encoding detected */
		  resetBuffer(&rxBuffer);
        }
		
		rxBuffer.escape = FALSE;
      }
      else
      {
        if (receivedByte == ASCII_ESC)
        {
          rxBuffer.escape = TRUE;
        }
      }

      STORE_BYTE(&rxBuffer, receivedByte);
      if (rxBuffer.index == rxBuffer.length)
      {
        rxBuffer.status = WAIT_CRC;
      }
      break;

    case WAIT_CRC:
      STORE_BYTE(&rxBuffer, receivedByte);

      /* received four CRC bytes? */
      if (rxBuffer.index == FRAME_CRC_LENGTH + rxBuffer.length)
      {
        rxBuffer.status = WAIT_ETX;
      }
      break;

    case WAIT_ETX:
      if (receivedByte != ASCII_ETX || ! CHECK_CRC16(&rxBuffer))
      {
		// Filippo - when there's a communication error I reset the buffer immediately
		resetBuffer(&rxBuffer); 					\
      }
      else
      {
        STORE_BYTE(&rxBuffer, receivedByte);
        rxBuffer.length -= FRAME_PAYLOAD_START;

        /* frame ok, we can now "unstuff" the payload */
        if (deviceID == slave_id)
        {
          rxBuffer.bufferFlags.rxCompleted = TRUE;
        }

        unstuffMessage();

        if (! rxBuffer.bufferFlags.rxCompleted)
        {
		  resetBuffer(&rxBuffer);					  \
        }
      }
      break;

    default:
	  resetBuffer(&rxBuffer); 					\
    } /* switch */
  } /* if (! IS_ERROR) */
} /* rebuildMessage() */

void STORE_BYTE(uartBuffer_t *buf, unsigned char c)
{
	buf->buffer[buf->index++]=c;
	if (buf->index>=BUFFER_SIZE)
	{
		resetBuffer(buf);
	}
}

unsigned char IS_VALID_ID(unsigned char id)
{
	if (((0 < (id)) && ((id) <= N_SLAVES)) || ((100 < (id)) && ((id) <= 100 + N_SLAVES)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
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

static void unstuffMessage()
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
  unsigned char *p = rxBuffer.buffer + FRAME_PAYLOAD_START;

  /* i is the read index, j is the write index. For each iteration, j
   * is always incremented by 1, i may be incremented by 1 or 2,
   * depending on whether p[i] is a stuffed character or not. At the
   * end of the cycle (length bytes read) j is less than or equal to
   * i. (i - j) is the amount that must be subtracted to the payload
   * length. */

  i = j = 0;
  while (i < rxBuffer.length) {
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
  rxBuffer.length -= (i - j);
}

void MakeTintingMessage(uartBuffer_t *txBuffer, unsigned char slave_id)
/*
*//*=====================================================================*//**
**      @brief Create the serial message for MABrd
**
**      @param txBuffer pointer to the tx buffer
**
**      @param slave_id slave identifier
**
**      @retval void
**
*//*=====================================================================*//**
*/
{
  unsigned char idx = 0;
  
  // initialize tx frame, reserve extra byte for pktlen 

  txBuffer->buffer[idx++]=ASCII_STX;
  txBuffer->buffer[idx++]=(100 + slave_id)+0x20;
  idx++;	/* reserved for pktlen */

  stuff_byte(txBuffer->buffer, &idx, TintingAct.typeMessage);
  stuff_byte(txBuffer->buffer, &idx, Status.level);
  stuff_byte(txBuffer->buffer, &idx, Status.errorCode); 

  // Application program version number (24 bits) 
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(SW_VERSION));
  stuff_byte(txBuffer->buffer, &idx, MSB_LSW(SW_VERSION));
  stuff_byte(txBuffer->buffer, &idx, LSB_MSW(SW_VERSION));

  #if defined NO_BOOTLOADER
    /* BootLoader Firmware Version (24 bits) */
    stuff_byte( txBuffer->buffer, &idx, 0xFF);
    stuff_byte( txBuffer->buffer, &idx, 0xFF);
    stuff_byte( txBuffer->buffer, &idx, 0xFF);
  #else
    // Bootloader version number (24 bits) 
    stuff_byte(txBuffer->buffer, &idx, LSB_LSW(__BL_SW_VERSION));
    stuff_byte(txBuffer->buffer, &idx, MSB_LSW(__BL_SW_VERSION));
    stuff_byte(txBuffer->buffer, &idx, LSB_MSW(__BL_SW_VERSION));
  #endif
  // Humidifier process Temperature
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.Temperature));
  stuff_byte(txBuffer->buffer, &idx, MSB_LSW(TintingAct.Temperature));
  // Humidifier process RH Humidity
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.RH));
  stuff_byte(txBuffer->buffer, &idx, MSB_LSW(TintingAct.RH));
  // Dosing Temperature
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.Dosing_Temperature));
  stuff_byte(txBuffer->buffer, &idx, MSB_LSW(TintingAct.Dosing_Temperature));
  // Water level State
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.WaterLevel_state));
  // Mixer Photocells status
  // Mixer Homing Photocel "HOME_PHOTOCELL"  --> bit0
  // Mixer Jar Photocell "JAR_PHOTOCELL"     --> bit1
  // Mixer Door Open Photocell "DOOR_OPEN_PHOTOCELL" --> bit2
  // Mixer Autocap Open Photocell "AUTOCAP_OPEN_PHOTOCELL" --> bit3  
  // Mixer Autocap Lifter Down Photocell "AUTOCAP_LIFTER_DOWN_PHOTOCELL" --> bit4    
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.Photocells_state));
  // Door Microswitch State
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.Door_Microswitch));
  // Jar Available
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.Jar_presence));     
  // SetHighCurrent + Mixing + DoorOpen State --> 0 = Not Active,  1 = Active, 2 = END Successfully
  stuff_byte(txBuffer->buffer, &idx, LSB_LSW(TintingAct.SetHighCurrent_Mixing_DoorOpen_state));       
  /* crc, pktlen taken care of here */
  unionWord_t crc;													  
																
  /* fix pkt len */ 												
  txBuffer->buffer [FRAME_LENGTH_BYTE_POS] = 	(FRAME_END_OVERHEAD + idx)+0x20;						  
  txBuffer->length = ( FRAME_END_OVERHEAD + (idx));					  
																	  
  /* crc16, sent one nibble at the time, w/ offset, big-endian */	  

  crc.uword = CRCarea(txBuffer->buffer, idx, NULL);
  txBuffer->buffer[idx++]=MSN(crc.byte[1])+0x20;
  txBuffer->buffer[idx++]=LSN( crc.byte[1])+0x20;   
  txBuffer->buffer[idx++]=MSN( crc.byte[0])+0x20;   
  txBuffer->buffer[idx++]=LSN( crc.byte[0])+0x20;
																	  
  /* ETX = frame end */ 											  
  txBuffer->buffer[idx++]=ASCII_ETX;	
}

void DecodeTintingMessage(uartBuffer_t *rxBuffer, unsigned char slave_id)
/*
*//*=====================================================================*//**
**      @brief Decode the serial message received from MABrd
**
**      @param rxBuffer pointer to the rx buffer
**
**      @param slave_id slave identifier
**
**      @retval void
**
*//*=====================================================================*//**
*/
{
  unsigned char idx = FRAME_PAYLOAD_START;
  unsigned short TintingCommand, i;
  unionWord_t tmpWord;
  unionDWord_t tmpDWord;

  /* suppress warnings */
  (void) tmpWord;
  (void) tmpDWord;
  /* suppress warning */
  (void) slave_id;

  TintingAct.typeMessage = rxBuffer->buffer[idx ++];
  TintingCommand = rxBuffer->buffer[idx ++];
  
  switch(TintingCommand)
  {
    case CMD_TINTING_STOP:          
        // tinting_stop
        TintingAct.command.cmd = 0x01;
        break;
    case CMD_TINTING_INTR:          
        // tinting_intr
        TintingAct.command.cmd = 0x02;
        break;
    case CMD_TINTING_HOME:          
        // tinting_home
        TintingAct.command.cmd = 0x04;
        break;        
    case CMD_TINTING_PACKING:          
        // tinting_packing
        TintingAct.command.cmd = 0x08;
        break;        
    case CMD_TINTING_OPEN:          
        // tinting_open
        TintingAct.command.cmd = 0x10;
        break;
    case CMD_TINTING_CLOSE:          
        // tinting_close
        TintingAct.command.cmd = 0x20;
        break;
    case CMD_TINTING_EXTEND:          
        // tinting_extend
        TintingAct.command.cmd = 0x40;
        break;
    case CMD_TINTING_RETRACT:          
        // tinting_retract
        TintingAct.command.cmd = 0x80;
        break;
        
    default:
        TintingAct.command.cmd = 0x0000;
        break;
  }        

  switch (TintingAct.typeMessage)
  {
    case JUMP_TO_BOOT:
        Status.level = TINTING_JUMP_TO_BOOT;
        Start_Jump_Boot = 0;
    break;

    case CONTROLLO_PRESENZA:
        Check_Presence = TRUE; 
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++];        
//Tinting.Autocap_Status = AUTOCAP_CLOSED; 
        TintingAct.Autocap_Status = rxBuffer->buffer[idx ++];
        TintingAct.Check_Door_Open = rxBuffer->buffer[idx ++];        
    break;

    case POS_HOMING:
        TintingAct.Homing_type = rxBuffer->buffer[idx ++];
//        TintingAct.Homing_type = 0;

        // Autocap Homing
        if (TintingAct.Homing_type > 0) {
            TintingAct.Autocap_Enable = TRUE;
            if (isColorCmdHome()) {
                tmpWord.byte[0] = rxBuffer->buffer[idx ++];
                tmpWord.byte[1] = rxBuffer->buffer[idx ++];
                TintingAct.n_step_homing = tmpWord.sword * (unsigned long)CORRECTION_AUTOCAP_STEP_RES * 2;  
            }
            else if (isColorCmdPacking()) {
                // TODO: add params 
                Nop();
            }
        }  
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++];        
        TintingAct.Check_Jar_presence = TRUE;
    break;
    
    case MISCELAZIONE_PRODOTTO: 
        TintingAct.Check_Jar_presence = FALSE;
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++];        
    break;
          
    case TEST_FUNZIONAMENTO_MIXER:
        TintingAct.Check_Jar_presence = FALSE;
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++];        
    break;

    case USCITA_BARATTOLO:
        TintingAct.Check_Jar_presence = rxBuffer->buffer[idx ++];    
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++];     
    break;

    case IMPOSTA_CORRENTE_MANTENIMENTO_MOTORE_MIXER:
        // Deve essere mandato dalla MAB dop lo Scarico Positivo.
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++]; 
        TintingAct.Check_Door_Open = rxBuffer->buffer[idx ++];
    break;    
       
    case MISCELAZIONE_USCITA_BARATTOLO: 
        // Deve essere mandato dalla MAB dop lo Scarico Positivo.
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++];        
    break;
    
    case IMPOSTA_USCITE_MIXER:
        // Type of Output:
        // WaterPump  = 1
        // Nebulizer_Heater = 2
        // HeaterResistance = 4
        // MixerMotor  = 8  
        // DoorMotor = 16
        // AutocapMotor = 32
        // AutocapLifter = 64
        PeripheralAct.Peripheral_Types.bytePeripheral = rxBuffer->buffer[idx ++];
        // Enable/Disable Output
        TintingAct.Output_Act = rxBuffer->buffer[idx ++];          
    break;      

    case SETUP_PARAMETRI_MIXER:
        // Autocap Enable
        TintingAct.Autocap_Enable = rxBuffer->buffer[idx ++];
        // Motor Enable
        TintingAct.Motor_Enable = rxBuffer->buffer[idx ++];
        // Door Enable
        TintingAct.Door_Enable = rxBuffer->buffer[idx ++];        
        // Timeout on Door Open
        TintingAct.Time_Door_Open = rxBuffer->buffer[idx ++];
		// 1sec = 500
		Durata[T_WAIT_DOOR_OPEN] = TintingAct.Time_Door_Open * 500;	
        // Mixer Cycles N        
        TintingAct.Mixer_N_cycle = rxBuffer->buffer[idx ++];
        for (i = 0; i < MIXER_N_PROFILE; i++) {
			tmpWord.byte[0] = rxBuffer->buffer[idx ++];
			tmpWord.byte[1] = rxBuffer->buffer[idx ++];
			// Mixer Speed of step 'i'
            TintingAct.Mixer_Speed[i] = tmpWord.uword;			
			// Mixer Duration of step 'i'
            tmpWord.byte[0] = rxBuffer->buffer[idx ++];
			tmpWord.byte[1] = rxBuffer->buffer[idx ++];
			TintingAct.Mixer_Duration[i] = tmpWord.uword;			
            // Mixer Direction of step 'i'
            TintingAct.Mixer_Direction[i] = rxBuffer->buffer[idx ++];        
        } 
pippo = 1;        
    break;
    
    case SETUP_PARAMETRI_UMIDIFICATORE:
        // Humidifier process Enable / Disable
        HumidifierAct.Humidifier_Enable = rxBuffer->buffer[idx ++];
        // Humidifier Type
        HumidifierAct.Humdifier_Type = rxBuffer->buffer[idx ++];
        // THOR process
        if (HumidifierAct.Humdifier_Type > 1) {
            if (HumidifierAct.Humdifier_Type > 100)
                HumidifierAct.Humdifier_Type = 100;
            HumidifierAct.Humidifier_PWM = (unsigned char)(TintingAct.Humdifier_Type/2);
            HumidifierAct.Humdifier_Type = HUMIDIFIER_TYPE_2;
        }        
        // Humidifier Multiplier
        tmpWord.byte[0] = rxBuffer->buffer[idx ++];
        tmpWord.byte[1] = rxBuffer->buffer[idx ++];
        HumidifierAct.Humidifier_Multiplier = tmpWord.sword;
        // Starting Humidifier Period
        tmpWord.byte[0] = rxBuffer->buffer[idx ++];
        tmpWord.byte[1] = rxBuffer->buffer[idx ++];
        HumidifierAct.Humidifier_Period = tmpWord.sword;
        // Humidifier Nebulizer Duration with AUTOCAP OPEN
        tmpWord.byte[0] = rxBuffer->buffer[idx ++];
        tmpWord.byte[1] = rxBuffer->buffer[idx ++];
        HumidifierAct.AutocapOpen_Duration = tmpWord.sword;
        // Humidifier Nebulizer Period with AUTOCAP OPEN
        tmpWord.byte[0] = rxBuffer->buffer[idx ++];
        tmpWord.byte[1] = rxBuffer->buffer[idx ++];
        HumidifierAct.AutocapOpen_Period = tmpWord.sword;
        // Temperature controlled Dosing process Enable / Disable
        HumidifierAct.Temp_Enable = rxBuffer->buffer[idx ++];
        // Temperature Type
        HumidifierAct.Temp_Type = rxBuffer->buffer[idx ++];
        // Temperature controlled Dosing process Period 
        tmpWord.byte[0] = rxBuffer->buffer[idx ++];
        tmpWord.byte[1] = rxBuffer->buffer[idx ++];
        HumidifierAct.Temp_Period = tmpWord.sword;
        // LOW Temperature threshold value 
        HumidifierAct.Temp_T_LOW = rxBuffer->buffer[idx ++];
        // HIGH Temperature threshold value 
        HumidifierAct.Temp_T_HIGH = rxBuffer->buffer[idx ++];
        // Heater Activation 
        HumidifierAct.Heater_Temp = rxBuffer->buffer[idx ++];
        // Heater Hysteresis 
        HumidifierAct.Heater_Hysteresis = rxBuffer->buffer[idx ++];
    break;

#if defined AUTOCAP_ACTUATOR
    case POSIZIONA_AUTOCAP:
        if (isColorCmdOpen() || isColorCmdClose()) {
            tmpWord.byte[0] = rxBuffer->buffer[idx ++];
            tmpWord.byte[1] = rxBuffer->buffer[idx ++];
            TintingAct.n_step_move = tmpWord.sword * CORRECTION_AUTOCAP_STEP_RES * 2;

            tmpWord.byte[0] = rxBuffer->buffer[idx ++];
            tmpWord.byte[1] = rxBuffer->buffer[idx ++];
            TintingAct.speed_move = tmpWord.sword;  
        }
        else if (isColorCmdExtend() || isColorCmdRetract()) {
            // TODO: add params 
            Nop();
        }
        TintingAct.Machine_Motors = rxBuffer->buffer[idx ++];        
      break;
#endif    
    default:
    break;
  } /* switch() */
}

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
  /* STX --> ESC TWO, ETX --> ESC THREE */
  if ((c == ASCII_STX) || (c == ASCII_ETX))
  {
    buf[appoggio++]=ASCII_ESC;
    buf[appoggio++]=c + ASCII_ZERO;
  }
  /* ESC --> ESC ZERO */
  else if (c == ASCII_ESC)
  {
    buf[appoggio++]=ASCII_ESC;
    buf[appoggio++]=ASCII_ZERO;
  }
  /* Regular char, nothing fancy here */
  else
  {
    buf[appoggio++]=c;
  }
  *ndx=appoggio;
  
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
   first!!!), il CRC_16 calcolato sul vettore cos� ottenuto DEVE
   valere zero.  Tale propriet� pu� essere sfruttata nelle
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
  if (_U3TXIE && _U3TXIF)
  {
    _U3TXIF = 0;

    if (txBuffer.index >= txBuffer.length)
    {
      // Disable Tx multiprocessor line
      RS485_DE = 0;
      // Disable Transmission Interrupt
      IEC5bits.U3TXIE = 0;
      txBuffer.bufferFlags.uartBusy = FALSE;
      txBuffer.bufferFlags.txReady = FALSE;
      // End Trasmissione: se era arrivato comando di JUMP_TO_BOOT ne abilito la partenza
	  if (Status.level == TINTING_JUMP_TO_BOOT)
		 Start_Jump_Boot = 1; 
    }
    else
    {
      U3TXREG = txBuffer.buffer[txBuffer.index++];
    }
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
  
  if (_U3RXIE && _U3RXIF)
  {
    _U3RXIF = 0;

    /*Overrun Error*/
    if (U3STAbits.OERR)
    {
      /* Segnalazione Overrun Error */
      U3STAbits.OERR = 0;
	  // When there's a communication error I reset the buffer immediately
	  resetBuffer(&rxBuffer);					  \
    }

    /*Framing Error*/
    if (U3STAbits.FERR)
    {
      flushUart = U3RXREG;
      /* Segnalazione Framing Error */
	  // When there's a communication error I reset the buffer immediately
	  resetBuffer(&rxBuffer);					  \
    }
    // Parity Error Check absent
    
	rebuildMessage(U3RXREG);
  }
}

void serialCommManager(void)
/*
*//*=====================================================================*//**
**      @brief Sequencer of the module
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
  decodeMessage();
  makeMessage();
  sendMessage();
}

static void decodeMessage(void)
/*
*//*=====================================================================*//**
**      @brief decode the received message, calling the decode
**             function related to the Involved slave: call to
**             serialSlave->decodeSerialMsg(&rxBuffer)
**
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
  if (rxBuffer.bufferFlags.rxCompleted == TRUE) {
    if (StatusTimer(T_DELAY_INTRA_FRAMES) == T_HALTED)
      StartTimer(T_DELAY_INTRA_FRAMES);

    else if (StatusTimer(T_DELAY_INTRA_FRAMES) == T_ELAPSED) {
      serialSlave.decodeSerialMsg(&rxBuffer,slave_id);
      initBuffer(&rxBuffer);
      rxBuffer.bufferFlags.decodeDone = TRUE;

      StopTimer(T_DELAY_INTRA_FRAMES);
    }
  }
}

static void makeMessage (void)
/*
*//*=====================================================================*//**
**      @brief Management of the fixed time window Display-slaves:
**             if the answer from the Involved slave is received,
**             the following actions are performed:
**             - update the serialSlave struct for the subsequent slave
**               interrogated,
**             - make the packet to be transmitted, calling the message
**               make function related to the new slave
**             If the time window is elapsed without answer and the number
**             of retries is lower than admitted:
**             - increase the number of retries for the current slave
**             - send again the message to the same slave
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
  if (rxBuffer.bufferFlags.decodeDone == TRUE &&
      txBuffer.bufferFlags.uartBusy == FALSE) {

    rxBuffer.bufferFlags.decodeDone = FALSE;
    initBuffer(&txBuffer);

    serialSlave.makeSerialMsg(&txBuffer,slave_id);

    txBuffer.bufferFlags.txReady = TRUE;
    StartTimer(T_SLAVE_WAIT_TIMER);
    StopTimer(T_SLAVE_WAIT_LINK_TIMER);
  }

  else if (rxBuffer.bufferFlags.decodeDone == FALSE &&
           StatusTimer(T_SLAVE_WAIT_TIMER) == T_HALTED)

    StartTimer(T_SLAVE_WAIT_TIMER);
}

static void sendMessage(void)
/*
*//*=====================================================================*//**
**      @brief Start the transmission, enabling the UART 3 transmission
**             flag and filling the UART3 tx buffer with the first byte
**             to be transmitted
**
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
  if(txBuffer.bufferFlags.txReady == TRUE)
  {
    if ((txBuffer.bufferFlags.uartBusy == TRUE) || (txBuffer.length > BUFFER_SIZE))
    {
      return;
    }
    // Enable Tx multiprocessor line
    RS485_DE = 1;
    // Clear TX Interrupt flag
    IFS5bits.U3TXIF = 0;
    // Enable Tx Interrupt
    IEC5bits.U3TXIE = 1;

    // Scarico il primo byte nel buffer di trasmissione : Write data byte to lower byte of UxTXREG word
    // Take control of buffer
    txBuffer.bufferFlags.uartBusy = TRUE;

    // TRMT is set when U1TSR register and buffer is empty: wait 
    while(U3STAbits.TRMT == 0);
    U3TXREG = txBuffer.buffer[txBuffer.index++];    
  }
}