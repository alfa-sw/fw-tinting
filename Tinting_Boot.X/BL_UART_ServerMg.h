/**/
/*============================================================================*/
/**
**      @file    BL_UART_ServerMg.h
**
**      @brief   BootLoader UART Server Manager module header file
**/
/*============================================================================*/
/**/
#ifndef __BL_UART_SERVERMG_H__
#define __BL_UART_SERVERMG_H__

typedef struct __attribute__ ((packed)) {
  unsigned char  typeMessage;
  unsigned long startAddress;
  unsigned long  address;
  unsigned short numDataPack;
  unsigned short idDataPackRx;
  unsigned short idDataPackTx;
  unsigned short idDataPackExpected;
  unsigned char  numDataBytesPack;
  unsigned short bufferData[BYTES2WORDS(RequestDataBlockSize)];
  unsigned char  errorType;
  unsigned char  num_retry_broadcast;
  unsigned char  IDtype;
  unsigned char  numSendingResetDeviceCmd;
  unsigned char  endProgramming;  
} progBoot_t;

typedef union __attribute__ ((packed)) {
  unsigned short uword;
  signed short   sword;
  unsigned char  byte[2];
} unionWord_t;


typedef union __attribute__ ((packed)) {
  unsigned long  udword;
  signed long    sdword;
  unsigned short word[2];
  unsigned char  byte[4];
} unionDWord_t;


/* Tipo di pacchetti MAB -> Actuators_BL */
enum {
  CMD_FW_UPLOAD      = 0x0F,
  DATA_FW_UPLOAD     = 0x10,
  END_DATA_FW_UPLOAD = 0x0A,
  GET_PROGRAM_DATA   = 0x0D,
  RESET_SLAVE        = 0x0E,
  CMD_FORCE_SLAVE_BL = 0x0B,
  CMD_FRMWR_REQUEST  = 0x09
};

/* Tipo di pacchetti Actuators_BL -> MAB */
enum
{
  ACK_FW_UPLOAD      = 0x11,
  NACK_FW_UPLOAD     = 0x12,
  SEND_PROGRAM_DATA  = 0x13,
  SEND_FRMWR_VERSION = 0x14
};

/* Tipo di errore */
enum
{
  PACK_LOST = 0x02,
};

#define isUART_FW_Upload_Ack()      (progBoot.typeMessage == ACK_FW_UPLOAD     && isNewProcessingMsg())
#define isUART_FW_Upload_Nack()     (progBoot.typeMessage == NACK_FW_UPLOAD    && isNewProcessingMsg())
#define isUART_Programming_Data()   (progBoot.typeMessage == SEND_PROGRAM_DATA && isNewProcessingMsg())
#define isUART_Reset()              (progBoot.typeMessage == RESET_SLAVE)
#define setEndProgramming()         (progBoot.endProgramming = TRUE)
#define isEndProgramming()          (progBoot.endProgramming == TRUE)
#define isUART_Slave_FW_Version()   (progBoot.typeMessage == SEND_FRMWR_VERSION && isNewProcessingMsg())
#define isUART_Master_FW_Version()  (BL_Master_Version == 1)

#define enBroadcastMessage()                                    \
  do {                                                          \
    progBoot.num_retry_broadcast = 0;                           \
    BL_slave_id = BROADCAST_ID;                                 \
  } while (0)

#define isMasterProgramming()                   \
  (progBoot.IDtype == PROGRAM_MAB)

/*===== TIPI LOCALI ==========================================================*/
/*===== DICHIARAZIONI LOCALI =================================================*/
/*===== VARIABILI LOCALI =====================================================*/
/*===== VARIABILI LOCALI =====================================================*/
/*===== COSTANTI LOCALI ======================================================*/

/*===== VARIABILI GLOBALI =====================================================*/
extern progBoot_t progBoot;

/*===== DICHIARAZIONE FUNZIONI GLOBALI =======================================*/
extern void setBootMessage(unsigned char packet_type);
extern void MakeBootMessage(uartBuffer_t *txBuffer, unsigned char slave_id);
extern void DecodeBootMessage(uartBuffer_t *rxBuffer,unsigned char slave_id);
extern void BL_UART_ServerMg(void);
extern void jump_to_appl(void);

#endif /* __BL_UART_SERVERMG_H__ */
