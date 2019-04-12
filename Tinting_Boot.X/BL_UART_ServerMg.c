/**/
/*========================================================================== */
/**
 **      @file    BL_UART_ServerMg.c
 **
 **      @brief   I/O management
 **/
/*========================================================================== */
/**/

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "p24FJ256GB110.h"
#include "define.h"
#include "serialCom.h"
#include "MACRO.h"
#include "ram.h"
#include "progMemFunctions.h"
#include "Timermg.h"
#include "BL_UART_ServerMg.h"

progBoot_t progBoot;
DWORD_VAL FlashMemoryValue;

static void FW_Upload_Proc(void);
static void setBootMessage(unsigned char packet_type);

void MakeBootMessage(uartBuffer_t *txBuffer, unsigned char slave_id)
/*
*//*=====================================================================*//**
**  @brief Create the serial message for  MABRD
**
**  @param txBuffer pointer to the tx buffer
**

**  @param slave_id slave identifier
**
**  @retval void
**
*//*=====================================================================*//**
*/
{
  unsigned char idx = 0;
  /* initialize tx frame, reserve extra byte for pktlen */
  FRAME_BEGIN(txBuffer, idx, 100 + slave_id);
  STUFF_BYTE( txBuffer->buffer, idx, progBoot.typeMessage);

  switch (progBoot.typeMessage)
  {
    unsigned char i;

    case ACK_FW_UPLOAD:
      STUFF_BYTE( txBuffer->buffer, idx, LSB(progBoot.idDataPackRx));
      STUFF_BYTE( txBuffer->buffer, idx, MSB(progBoot.idDataPackRx));
	  if (Set_Reset == 1)
		  Set_Reset = 2;
      break;

    case NACK_FW_UPLOAD:
      STUFF_BYTE( txBuffer->buffer, idx, LSB(progBoot.idDataPackRx));
      STUFF_BYTE( txBuffer->buffer, idx, MSB(progBoot.idDataPackRx));

      STUFF_BYTE( txBuffer->buffer, idx, progBoot.errorType);
      break;

    case SEND_PROGRAM_DATA:
      for (i = 0;i < BYTES2WORDS(RequestDataBlockSize); ++ i) {
        STUFF_BYTE( txBuffer->buffer, idx, LSB(progBoot.bufferData[i]));
        STUFF_BYTE( txBuffer->buffer, idx, MSB(progBoot.bufferData[i]));
      }
      break;

    case SEND_FRMWR_VERSION:
	  STUFF_BYTE( txBuffer->buffer, idx, LSB_LSW(BL_SW_VERSION));
      STUFF_BYTE( txBuffer->buffer, idx, MSB_LSW(BL_SW_VERSION));
      STUFF_BYTE( txBuffer->buffer, idx, LSB_MSW(BL_SW_VERSION));
      break;
  }

  FRAME_END( txBuffer, idx);
}

void DecodeBootMessage(uartBuffer_t *rxBuffer, unsigned char slave_id)
/*
*//*=====================================================================*//**
**  @brief Create the serial message from MABRD
**
**  @param rxBuffer, the UART RX buffer
**
**  @param slave_id slave identifier <unused>
**
**  @retval void
**
*//*=====================================================================*//**
*/
{
  unsigned char idx = FRAME_PAYLOAD_START;
  unionWord_t  tmpWord1;
  unionDWord_t tmpWord2;
  unsigned char i;

  /* suppress warning */
  (void) slave_id;
  tmpWord1.uword  = 0;  
  tmpWord2.udword = 0;
  progBoot.typeMessage = rxBuffer->buffer[idx ++];
  switch (progBoot.typeMessage)
  {
  case CMD_FW_UPLOAD:
    tmpWord2.byte[0] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[1] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[2] = rxBuffer->buffer[idx ++];    
    progBoot.startAddress = tmpWord2.udword;
    
    tmpWord1.byte[0] = rxBuffer->buffer[idx ++];
    tmpWord1.byte[1] = rxBuffer->buffer[idx ++];
    progBoot.numDataPack = tmpWord1.uword;    
    break;

  case DATA_FW_UPLOAD:
    tmpWord1.byte[0] = rxBuffer->buffer[idx ++];
    tmpWord1.byte[1] = rxBuffer->buffer[idx ++];
    progBoot.idDataPackRx = tmpWord1.uword;

    tmpWord2.byte[0] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[1] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[2] = rxBuffer->buffer[idx ++];    
    progBoot.address = tmpWord2.udword;

if ( (progBoot.address >= 0x2C00) && (progBoot.address <= 0x2C08) )    
    pippo = 1;
    
    progBoot.numDataBytesPack = rxBuffer->buffer[idx ++];

    for (i = 0; i < BYTES2WORDS(RequestDataBlockSize); ++ i) {
      tmpWord1.byte[0] = rxBuffer->buffer[idx ++];
      tmpWord1.byte[1] = rxBuffer->buffer[idx ++];
      progBoot.bufferData[i]= tmpWord1.uword;
    }
    break;

  case END_DATA_FW_UPLOAD:
    tmpWord1.byte[0] = rxBuffer->buffer[idx ++];
    tmpWord1.byte[1] = rxBuffer->buffer[idx ++];
    progBoot.idDataPackRx = tmpWord1.uword;

    tmpWord2.byte[0] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[1] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[2] = rxBuffer->buffer[idx ++];    
    progBoot.address = tmpWord2.udword;
    
    progBoot.numDataBytesPack = rxBuffer->buffer[idx ++];

//    for (unsigned char i = 0; i < BYTES2WORDS(progBoot.numDataBytesPack); ++ i) {
    for (i = 0; i < BYTES2WORDS(progBoot.numDataBytesPack); ++ i) {
      tmpWord1.byte[0] = rxBuffer->buffer[idx ++];
      tmpWord1.byte[1] = rxBuffer->buffer[idx ++];
      progBoot.bufferData[i]= tmpWord1.uword;
    }
    break;

  case GET_PROGRAM_DATA:
    progBoot.numDataBytesPack = rxBuffer->buffer[idx ++];
    tmpWord2.byte[0] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[1] = rxBuffer->buffer[idx ++];
    tmpWord2.byte[2] = rxBuffer->buffer[idx ++];    
    progBoot.address = tmpWord2.udword;
    break;

  case RESET_SLAVE:
break;      
  case CMD_FORCE_SLAVE_BL:
break;
  case CMD_FRMWR_REQUEST:
    break;

  default:
    break;
  }
  setNewProcessingMsg();
}

static void FW_Upload_Proc(void)
/**/
/*==========================================================================*/
/**
**   @brief  Firmware upload procedure
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  unsigned char i;

  switch (BLState.step) {

  case ERASE_DEVICE:
    /* Erase operation will take a while... */
    DISABLE_WDT();
    ERASE_FLASH_PAGES(FIRST_PG_APPL, LAST_PG_APPL);
    ENABLE_WDT();

    BLState.step = WAIT_DATA_PACKET;
    setBootMessage(ACK_FW_UPLOAD);

    progBoot.address = progBoot.startAddress;
    progBoot.idDataPackExpected = 1;
    break;

  case WAIT_DATA_PACKET:
    if (isUART_FW_Upload_Data()) {

      if (progBoot.idDataPackRx == progBoot.idDataPackExpected)
        BLState.step = PROGRAM_DEVICE;

      else {
        setPackError(PACK_LOST);
        setBootMessage(NACK_FW_UPLOAD);
        BLState.livello = UART_FW_UPLOAD_FAILED;
      }

      resetNewProcessingMsg();
    }

    else if (isUART_FW_UploadEnd_Data()) {

      if (progBoot.idDataPackRx == progBoot.idDataPackExpected)
        BLState.step = PROGRAM_END;

      else {
        setPackError(PACK_LOST);
        setBootMessage(NACK_FW_UPLOAD);
        BLState.livello = UART_FW_UPLOAD_FAILED;
      }

      resetNewProcessingMsg();
    }

    else if (isUART_FW_GetProgramData()) {
      BLState.step = GET_DATA;
      resetNewProcessingMsg();
    }

    else if (isUART_Reset()) {
      BLState.step = RESET_SYSTEM;
      resetNewProcessingMsg();
	}

    else if (isUART_FW_Upload_Cmd()) {
      BLState.step = ERASE_DEVICE;
      resetNewProcessingMsg();
    }

	else if (isUART_Firmware_Request_Cmd()) {
	   resetNewProcessingMsg();
	   setBootMessage(SEND_FRMWR_VERSION);
	}
    break;

  case PROGRAM_DEVICE:
    /* Write operation will take a while ... */
    DISABLE_WDT();
    WriteFlashSubBlock(progBoot.address,
                       BYTES2WORDS(progBoot.numDataBytesPack),
                       progBoot.bufferData);
    ENABLE_WDT();

    setBootMessage(ACK_FW_UPLOAD);
    //progBoot.address+=(progBoot.numDataBytesPack/WORDSIZE);

//    if (progBoot.idDataPackRx < progBoot.numDataPack) {
      ++ progBoot.idDataPackExpected;
      BLState.step = WAIT_DATA_PACKET;
//    }
    break;

  case PROGRAM_END:

    /* Write operation will take a while ... */
    DISABLE_WDT();
    WriteFlashSubBlock(progBoot.address,
                       BYTES2WORDS(progBoot.numDataBytesPack),
                       progBoot.bufferData);
    ENABLE_WDT();

    setBootMessage(ACK_FW_UPLOAD);
    //progBoot.address+=(progBoot.numDataBytesPack/WORD_SIZE);

//    if (progBoot.idDataPackRx < progBoot.numDataPack) {
      ++ progBoot.idDataPackExpected;
      BLState.step = WAIT_DATA_PACKET;
//    }
   break;

  case GET_DATA:      
    for(i = 0; i < (progBoot.numDataBytesPack/2); i = i+2) {
      FlashMemoryValue.Val = ReadProgramMemory((DWORD) (progBoot.address + i));

      progBoot.bufferData[BYTES2WORDS(RequestDataBlockSize) + i -
                          BYTES2WORDS(progBoot.numDataBytesPack)] =
        (unsigned short)FlashMemoryValue.word.LW;
      // set the phantom byte to 0 
      FlashMemoryValue.byte.MB = 0x00;

      // upper word that contains the phantom byte 
      progBoot.bufferData[BYTES2WORDS(RequestDataBlockSize) + i + 1 -
                          BYTES2WORDS(progBoot.numDataBytesPack)] =
       (unsigned short)FlashMemoryValue.word.HW;
	}
    setBootMessage(SEND_PROGRAM_DATA);
    BLState.step = WAIT_DATA_PACKET;
    break;

	case RESET_SYSTEM:
		setBootMessage(ACK_FW_UPLOAD);
		Set_Reset = 1;	
		BLState.step = DO_RESET;
	break;

	case DO_RESET:
		if (Set_Reset == 2) {
			/* Write operation will take a while ... */
			DISABLE_WDT();
			//  WriteFlashWord(BL_STAND_ALONE_CHECK, 0x00000000L);
			//ENABLE_WDT();
			//Reset();
			BL_StandAlone = CheckApplicationPresence(BL_STAND_ALONE_CHECK);
			if (BL_StandAlone == BL_NO_STAND_ALONE) {
				Nop();
				Nop();
				jump_to_appl(); /* goodbye */
			} 
			else {
				Set_Reset = 0;
				ENABLE_WDT();
			}	
		} 	
	
	break;	
  } /* switch (BLState.step) */
} /* FW_Upload_Proc() */

static void setBootMessage(unsigned char packet_type)
/**/
/*==========================================================================*/
/**
**   @brief Request to send the packet_type serial message
**
**   @param packet_type type of packet
**
**   @return void
**/
/*==========================================================================*/
/**/
{
  progBoot.typeMessage = packet_type;
  setTxRequestMsg();
}

void BL_UART_ServerMg(void)
/**/
/*==========================================================================*/
/**
**   @brief  sequencer of the module
**
**   @param  void
**
**   @return void
**/
/*==========================================================================*/
/**/
{

  switch (BLState.livello)
  {
  case INIT:
    /* Ready for upload command */
    if (isUART_FW_Upload_Cmd()) {
		BLState.livello = UART_FW_UPLOAD;
		BLState.step = ERASE_DEVICE;
		resetNewProcessingMsg();
    }
	else if (isUART_FW_GetProgramData()) {
		BLState.livello = UART_FW_UPLOAD;
		BLState.step = GET_DATA;
		resetNewProcessingMsg();
    }
	else if (isUART_Reset()) {
		BLState.livello = UART_FW_UPLOAD;
		BLState.step = RESET_SYSTEM;
		resetNewProcessingMsg();
    }
	else if (isUART_Firmware_Request_Cmd()) { 
		resetNewProcessingMsg();
		setBootMessage(SEND_FRMWR_VERSION);
	}
    break;

  case UART_FW_UPLOAD:
		FW_Upload_Proc();
    break;

  case UART_FW_UPLOAD_FAILED:
		HALT();
    break;
  }
}
