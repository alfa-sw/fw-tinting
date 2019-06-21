/**/
/*============================================================================*/
/**
**      @file    main.h
**
**      @brief   acts bootloader main module header file
**
**      @version Alfa Color Tester
**/
/*============================================================================*/
/**/
#ifndef __MAIN_H__
#define __MAIN_H__

#include "Macro.h"

extern void BL_ServerMg(void);

extern unsigned short CRCarea(unsigned char *pointer, unsigned short n_char,
                              unsigned short CRCinit);

#define BL_ForceStandAlone()                    \
  do {                                          \
    BL_StandAlone = BL_FORCED_STAND_ALONE;      \
  } while (0)

#define isBL_ForceStandAlone()                  \
  (BL_StandAlone == BL_FORCED_STAND_ALONE)

#endif /* __MAIN_H__ */
