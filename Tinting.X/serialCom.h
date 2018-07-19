/* 
 * File:   serialCom.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 14.16
 */

#ifndef SERIALCOM_H
#define	SERIALCOM_H

/* reserved ASCII codes */
#define ASCII_STX (0x02)
#define ASCII_ETX (0x03)
#define ASCII_ESC (0x1B)

/* stuffying encodings */
#define ASCII_ZERO   ('0') /* ESC -> ESC ZERO  */
#define ASCII_TWO    ('2') /* STX -> ESC TWO   */
#define ASCII_THREE  ('3') /* ETX -> ESC THREE */

#define BUFFER_SIZE (60)

#define FRAME_LENGTH_BYTE_POS    (2)
#define FRAME_PAYLOAD_START      (1 + FRAME_LENGTH_BYTE_POS)
#define FRAME_BGN_OVERHEAD       (FRAME_PAYLOAD_START)

#define FRAME_CRC_LENGTH         (4)
#define FRAME_END_OVERHEAD       (1 + FRAME_CRC_LENGTH)
#define MIN_FRAME_SIZE           (1 + FRAME_BGN_OVERHEAD + FRAME_END_OVERHEAD)
#define MAX_FRAME_SIZE           (BUFFER_SIZE)

enum {
  /*  0 */ UNIVERSAL_ID, /* R1 only */
  /*  1 */ B1_BASE_ID,
  /*  2 */ B2_BASE_ID,
  /*  3 */ B3_BASE_ID,
  /*  4 */ B4_BASE_ID,
  /*  5 */ B5_BASE_ID,
  /*  6 */ B6_BASE_ID,
  /*  7 */ B7_BASE_ID,
  /*  8 */ B8_BASE_ID,
  /*  9 */ C1_COLOR_ID,
  /* 10 */ C2_COLOR_ID,
  /* 11 */ C3_COLOR_ID,
  /* 12 */ C4_COLOR_ID,
  /* 13 */ C5_COLOR_ID,
  /* 14 */ C6_COLOR_ID,
  /* 15 */ C7_COLOR_ID,
  /* 16 */ C8_COLOR_ID,
  /* 17 */ C9_COLOR_ID,
  /* 18 */ C10_COLOR_ID,
  /* 19 */ C11_COLOR_ID,
  /* 20 */ C12_COLOR_ID,
  /* 21 */ C13_COLOR_ID,
  /* 22 */ C14_COLOR_ID,
  /* 23 */ C15_COLOR_ID,
  /* 24 */ C16_COLOR_ID,
  /* 25 */ C17_COLOR_ID,
  /* 26 */ C18_COLOR_ID,
  /* 27 */ C19_COLOR_ID,
  /* 28 */ C20_COLOR_ID,
  /* 29 */ C21_COLOR_ID,
  /* 30 */ C22_COLOR_ID,
  /* 31 */ C23_COLOR_ID,
  /* 32 */ C24_COLOR_ID,
  /* 33 */ MOVE_X_AXIS_ID,
  /* 34 */ MOVE_Y_AXIS_ID,
  /* 35 */ STORAGE_CONTAINER1_ID,
  /* 36 */ STORAGE_CONTAINER2_ID,
  /* 37 */ STORAGE_CONTAINER3_ID,
  /* 38 */ STORAGE_CONTAINER4_ID,
  /* 39 */ PLUG_COVER_1_ID,
  /* 40 */ PLUG_COVER_2_ID,
  /* 41 */ AUTOCAP_ID,
  /* 42 */ SGABELLO,
  /* 43 */ HUMIDIFIER,
  /* 44 */ TINTING,
  /* 45 */ GENERIC_ACT13_ID,
  /* 46 */ GENERIC_ACT14_ID,
  /* 47 */ GENERIC_ACT15_ID,
  /* 48 */ GENERIC_ACT16_ID,
  N_SLAVES /* == 49 */
};

enum {
  /* 0 */ WAIT_STX,
  /* 1 */ WAIT_ID,
  /* 2 */ WAIT_LENGTH,
  /* 3 */ WAIT_DATA,
  /* 4 */ WAIT_CRC,
  /* 5 */ WAIT_ETX,
};

#define LSN(x) ((x) & 0x0F)           // Least Significant Nibble
#define MSN(x) (((x) & 0xF0) >> 4)    // Most Significant Nibble
#define LSB(x) ((x) & 0x00FF)         // Least Significant Byte
#define MSB(x) (((x) & 0xFF00) >> 8)  // Most Significant Byte
#define LSW(x) ((unsigned long)(x) & 0x0000FFFFL)         // Least Significant Word
#define MSW(x) (((unsigned long)(x) & 0xFFFF0000) >> 16)  // Most Significant Word
#define MSB_MSW(x) (((x) & 0xFF000000L) >> 24)  // Most Significant Byte of Most Significant Word
#define LSB_MSW(x) (((x) & 0x00FF0000L) >> 16)  // Least Significant Byte of Most Significant Word
#define MSB_LSW(x) (((x) & 0x0000FF00L) >> 8)   // Most Significant Byte of Least Significant Word
#define LSB_LSW(x) (((x) & 0x000000FFL))        // Least Significant Byte of Least Significant Word

typedef struct
{
  unsigned char buffer[BUFFER_SIZE];
  unsigned char length;
  unsigned char index;
  unsigned char status;
  unsigned char escape; // rx only
  union __attribute__ ((packed))
    {
      unsigned char allFlags;
      struct
      {
        unsigned char unused1     : 1;
        unsigned char txReady     : 1;
        unsigned char decodeDone  : 1;
        unsigned char rxCompleted : 1;
        unsigned char serialError : 1;
        unsigned char uartBusy    : 1;
        unsigned char startTx     : 1;
        unsigned char unused      : 1;
      };
    } bufferFlags;
} uartBuffer_t;

typedef struct
{
  void (*makeSerialMsg)(uartBuffer_t *, unsigned char);
  void (*decodeSerialMsg)(uartBuffer_t *,unsigned char);
  unsigned char numRetry;
} serialSlave_t;

extern void initSerialCom(void);
extern void serialCommManager(void);
extern void U1RX_InterruptHandler(void);
extern void U1TX_InterruptHandler(void);

#endif	/* SERIALCOM_H */

