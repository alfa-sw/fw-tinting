/**/
/*============================================================================*/
/**
**      @file    Macro.h
**
**      @brief   Global macro definitions
**/
/*============================================================================*/
/**/

#ifndef __MACRO_H__
#define __MACRO_H__

#define BL_SW_VERSION 0x010801

// #define WORD_SIZE                            0x02

#define BYTES2WORDS(x)                          \
  ((x) / 2)

#define HALT()                                  \
  do {                                          \
    Sleep();                                    \
  } while (1)

/* BLStandAlone values */
#define BL_WAIT_CHECK_STAND_ALONE               0
#define BL_NO_STAND_ALONE                       1
#define BL_STAND_ALONE                          2
#define BL_FORCED_STAND_ALONE                   3

/* Number of bytes in the "Data" field of a standard request to/from
   the PC.  Must be an even number from 2 to 56. */
#define RequestDataBlockSize   56

#define FALSE 0
#define TRUE  1

#if ! defined DEBUG_SLAVE
#  define DISABLE_WDT()                         \
  do {                                          \
    _SWDTEN = 0;                                \
  } while (0)

#  define ENABLE_WDT()                          \
  do {                                          \
    ClrWdt();                                   \
    _SWDTEN = 1;                                \
  } while (0)

#else
#define DISABLE_WDT()
#define ENABLE_WDT()
#endif

#endif /* __MACRO_H__ */
