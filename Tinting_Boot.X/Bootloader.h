/**/
/*============================================================================*/
/**
**      @file    Bootloader.h
**
**      @brief   BootLoader shared text module header file
**/
/*============================================================================*/
/**/

#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

/** Bootloader function pointers */

typedef struct {
  /** Pointer to the RunBootloader function */

  unsigned short (*ptrBL_CRCarea)(unsigned char *pointer, unsigned short n_char,
                                  unsigned short CRCinit);

  unsigned short (*ptrBL_CRCareaFlash)(unsigned long address, unsigned long n_word,
                                       unsigned short CRCinit);
} BootloaderPointers_T;

#endif /* __BOOTLOADER_H__ */
