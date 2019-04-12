/**/
/*============================================================================*/
/**
**      @file      SerialCom.c
**
**      @brief     Modulo di comunicazione seriale
**
**      @version   Alfa Color Tester
**/
/*============================================================================*/
/**/

#include "Macro.h"
#include "ram.h"
#include "serialCom.h"
#include "BL_UART_ServerMg.h"
#include "p24FJ256GB110.h"
#include "timerMg.h"
#include "mem.h"

//#include "UART.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief Stores a byte into RX uartBuffer, performs boundary
 * checking.
 *
 * @param uartBuffer_t buf, RX buffer
 * @param char c, the character to be stored
 */
#define STORE_BYTE(buf, c)                      \
  do {                                          \
    (buf).buffer[(buf).index ++ ] = (c);        \
    if ((buf).index >= BUFFER_SIZE)             \
    {                                           \
      SIGNAL_ERROR();                           \
    }                                           \
  } while(0)

/**
 * @brief Resets the receiver
 */
#define RESET_RECEIVER()                           \
  do {                                             \
    BL_initBuffer(&rxBuffer);                      \
  } while (0)

/**
 * @brief Last char was an escape?
 */
#define IS_ESCAPE()                             \
  (rxBuffer.escape != FALSE)

/**
 * @brief Signal last char was an escape
 */
#define SIGNAL_ESCAPE()                         \
  do {                                          \
    rxBuffer.escape = TRUE;                     \
  } while (0)

/**
 * @brief Signal last char was not an escape
 */
#define CLEAR_ESCAPE()                          \
  do {                                          \
    rxBuffer.escape = FALSE;                    \
  } while (0)

/**
 * @brief Serial error?
 */
#define IS_ERROR()                              \
  (rxBuffer.bufferFlags.serialError != FALSE)

#define SIGNAL_ERROR()                          \
  do {                                          \
    rxBuffer.bufferFlags.serialError = TRUE;    \
  } while (0)

#define IS_FROM_DISPLAY(id)                     \
  ((id) < 100)

#define IS_FROM_SLAVE(id)                       \
  ((id) > 100)

#if defined (__PIC24FJ256GB110__)
#define ENABLE_TX_MULTIPROCESSOR LATDbits.LATD13
#elif defined(__PIC24FJ256GB106__)
#define ENABLE_TX_MULTIPROCESSOR LATCbits.LATC14
#else
#define ENABLE_TX_MULTIPROCESSOR LATAbits.LATA8
#endif

/** TRIS ***********************************************************/
#define INPUT_PIN           1
#define OUTPUT_PIN          0

uartBuffer_t  rxBuffer;
uartBuffer_t  txBuffer;
serialSlave_t serialSlave;
unsigned char deviceID;

/* ===== PROTOTIPI FUNZIONI LOCALI ========================================= */
static void BL_makeMessage(void);
static void BL_decodeMessage(void);
static void BL_sendMessage(void);
static void BL_initBuffer(uartBuffer_t*);

static void BL_unstuffMessage();
static void BL_rebuildMessage(unsigned char);

/* ===== DEFINIZIONE FUNZIONI LOCALI ======================================= */
static void BL_unstuffMessage()
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

static void BL_rebuildMessage(unsigned char receivedByte)
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
  #ifdef FORCE_SERIAL_PACKET_LOSS
  static int _last_msg_from_display_was_lost;
  #endif

  if (! IS_ERROR())
  {
    switch(rxBuffer.status)
    {

    case WAIT_STX:
      if (receivedByte == ASCII_STX)
      {
        STORE_BYTE(rxBuffer, receivedByte);
        rxBuffer.status = WAIT_ID;
      }
      break;

    case WAIT_ID:
      STORE_BYTE(rxBuffer, receivedByte);
      deviceID = REMOVE_OFFSET(receivedByte);
      if (! IS_VALID_ID(deviceID))
      {
        SIGNAL_ERROR();
      }
      else
      {
        rxBuffer.status = WAIT_LENGTH;
      }
      break;

    case WAIT_LENGTH:
      STORE_BYTE(rxBuffer, receivedByte);
      if ( receivedByte < ADD_OFFSET( MIN_FRAME_SIZE) ||
           receivedByte > ADD_OFFSET( MAX_FRAME_SIZE) )
      {
        SIGNAL_ERROR();
      }
      else
      {
        /* The length embedded in the frame takes into account the
         * entire frame length, for ease of implementation of tx/rx
         * code. Here we discard the final 5 bytes (4 CRC + ETX). Later
         * on, after the crc check, we'll be able to discard also the
         * initial overhead [ STX, ID, LEN ] */
        rxBuffer.length  = REMOVE_OFFSET(receivedByte);
        rxBuffer.length -= FRAME_END_OVERHEAD;

        rxBuffer.status = WAIT_DATA;
      }
      break;

    case WAIT_DATA:
      /* check stuffying encoding */
      if (IS_ESCAPE())
      {
        /* ESC ZERO --> ESC, ESC TWO --> STX, ESC THREE --> ETX */
        if (receivedByte != ASCII_ZERO &&
            receivedByte != ASCII_TWO &&
            receivedByte != ASCII_THREE)
        {
          /* Ilegal encoding detected */
          SIGNAL_ERROR();
        }
        CLEAR_ESCAPE();
      }
      else
      {
        if (receivedByte == ASCII_ESC)
        {
          SIGNAL_ESCAPE();
        }
      }

      STORE_BYTE(rxBuffer, receivedByte);
      if (rxBuffer.index == rxBuffer.length)
      {
        rxBuffer.status = WAIT_CRC;
      }
      break;

    case WAIT_CRC:
      STORE_BYTE(rxBuffer, receivedByte);

      /* received four CRC bytes? */
      if (rxBuffer.index == FRAME_CRC_LENGTH + rxBuffer.length)
      {
        rxBuffer.status = WAIT_ETX;
      }
      break;

    case WAIT_ETX:
      #ifdef FORCE_SERIAL_PACKET_LOSS
      if ( IS_FROM_DISPLAY( deviceID ) && deviceID == MASTER_DEVICE_ID(BL_slave_id)
                                       && (_last_msg_from_display_was_lost =
                                          !_last_msg_from_display_was_lost))
      #else
      if (receivedByte != ASCII_ETX || ! CHECK_CRC16(&rxBuffer))
      #endif
      {
        SIGNAL_ERROR();
      }
      else {
        STORE_BYTE(rxBuffer, receivedByte);
        rxBuffer.length -= FRAME_PAYLOAD_START;

        /* frame ok, we can now "unstuff" the payload */
        BL_unstuffMessage();

        /* This is ugly! the slave won't accept the rx as completed
         * unless the IDs match. This will in turn signal a comm error
         * that will be recovered below. */
        if (deviceID == MASTER_DEVICE_ID(BL_slave_id) || (deviceID == BROADCAST_ID))
          rxBuffer.bufferFlags.rxCompleted = TRUE;

        if (! rxBuffer.bufferFlags.rxCompleted)
          SIGNAL_ERROR();
      }
      break;

    default:
      SIGNAL_ERROR();
      break;
    } /* switch */
  } /* if (! IS_ERROR) */

  if (IS_ERROR())
    RESET_RECEIVER();
} /* rebuildMessage() */

static void BL_makeMessage(void)
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
  if (txBuffer.bufferFlags.txRequest == TRUE && txBuffer.bufferFlags.uartBusy == FALSE) {
    txBuffer.bufferFlags.txRequest = FALSE;
    BL_initBuffer(&txBuffer);

    serialSlave.makeSerialMsg(&txBuffer,BL_slave_id);
    txBuffer.bufferFlags.txReady = TRUE;
  }
}

static void BL_decodeMessage(void)
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
  // A seconda dello stato e dello slave interrogato, chiamo le differenti
  // funzioni di decodifica
  if (rxBuffer.bufferFlags.rxCompleted == TRUE)
  {
    if (StatusTimer(T_DELAY_INTRA_FRAMES) == T_HALTED)
      StartTimer(T_DELAY_INTRA_FRAMES);

    else if (StatusTimer(T_DELAY_INTRA_FRAMES) == T_ELAPSED) {
      serialSlave.decodeSerialMsg(&rxBuffer, BL_slave_id);
      BL_initBuffer(&rxBuffer);

      // rxBuffer.bufferFlags.decodeDone = TRUE;
      StopTimer(T_DELAY_INTRA_FRAMES);
    }
  }
}

static void BL_sendMessage(void)
/*
*//*=====================================================================*//**
**      @brief Start the transmission, enabling the UART 3
**             transmission flag and filling the UART3 tx buffer with
**             the first byte to be transmitted
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/

{
  if (txBuffer.bufferFlags.txReady == TRUE) {

    if (txBuffer.bufferFlags.uartBusy == TRUE || txBuffer.length > BUFFER_SIZE)
      return;

    // Enable Tx multiprocessor line
    RS485_DE = 1;     
    // Clear TX interrupt flag 
    IFS5bits.U3TXIF = 0;
    // Enable UART1 TX
    IEC5bits.U3TXIE = 1;
    // Write first data byte to lower byte of UxTXREG word. Take control of buffer 
    txBuffer.bufferFlags.uartBusy = TRUE;

    while(U3STAbits.TRMT == 0)
      ;

    U3TXREG = txBuffer.buffer[txBuffer.index ++];
  }
}


static void BL_initBuffer(uartBuffer_t *buffer)
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

void BL_initSerialCom(void)
/*
*//*=====================================================================*//**
**      @brief Set UART3 registers; reset rreceiver and transmission
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

    BL_initBuffer(&rxBuffer);
    BL_initBuffer(&txBuffer);

    serialSlave.makeSerialMsg=&MakeBootMessage;
    serialSlave.decodeSerialMsg=&DecodeBootMessage;
}

void BL_serialCommManager(void)
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
  BL_decodeMessage();
  BL_makeMessage();
  BL_sendMessage();
}

void __attribute__((__interrupt__, no_auto_psv)) _AltU3RXInterrupt(void)
/*
 *//*=====================================================================*//**
**      @brief Interrupt in tx della UART3
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
  register unsigned char flushUart;

  if (_U3RXIE && _U3RXIF) {
    _U3RXIF = 0;

    // Overrun Error 
    if (U3STAbits.OERR) {
      /* signal Overrun Error */
      U3STAbits.OERR = 0;
      SIGNAL_ERROR();
    }
    // Framing Error 
    if (U3STAbits.FERR) {
      flushUart = U3RXREG;
      // signal Framing Error 
      SIGNAL_ERROR();
    }
    BL_rebuildMessage(U3RXREG);
  }
}

void BL_stuff_byte(unsigned char *buf, unsigned char *ndx, char c)
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
  /* STX --> ESC TWO, ETX --> ESC THREE */
  if ((c == ASCII_STX) || (c == ASCII_ETX)) {
    WRITE_BYTE(buf, *ndx, ASCII_ESC);
    WRITE_BYTE(buf, *ndx, c + ASCII_ZERO);
  }

  /* ESC --> ESC ZERO */
  else if (c == ASCII_ESC) {
    WRITE_BYTE(buf, *ndx, ASCII_ESC);
    WRITE_BYTE(buf, *ndx, ASCII_ZERO);
  }

  /* Regular char, nothing fancy here */
  else {
    WRITE_BYTE(buf, *ndx, c);
  }
}

/******************************************************************************/
/****************************** Interrupt Routine *****************************/
/******************************************************************************/

void __attribute__((__interrupt__, no_auto_psv)) _AltU3TXInterrupt(void)
/*
*//*=====================================================================*//**
**      @brief Interrupt in tx della UART3
**
**      @param void
**
**      @retval void
*//*=====================================================================*//**
*/
{
  if (_U3TXIE && _U3TXIF) {
    // clear TX interrupt flag on UART3 
    _U3TXIF = 0;
    if (txBuffer.index == txBuffer.length) {
      // Disable Tx multiprocessor line
      RS485_DE = 0;
      // Disable Transmission Interrupt

      IEC5bits.U3TXIE = 0;
      txBuffer.bufferFlags.uartBusy = FALSE;
      txBuffer.bufferFlags.txReady = FALSE;
if (txBuffer.buffer[1] != 0xB0)
pippo = 1;    
    }
    else {
      U3TXREG = txBuffer.buffer[txBuffer.index ++];
    }
  }
}
