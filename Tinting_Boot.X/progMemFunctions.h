/**/
/*============================================================================*/
/**
 **      @file    progMemFunctions.h
 **
 **      @brief   progMemFunctions module header file
 **/
/*============================================================================*/
/**/
#ifndef __PROG_MEM_FUNCTIONS_H__
#define __PROG_MEM_FUNCTIONS_H__

extern void WriteFlashWord(long Addr,long Val);

extern DWORD ReadProgramMemory(DWORD address);

extern void WriteFlashSubBlock(DWORD StartAddress, unsigned short Size,
                               unsigned short * DataBuffer);

#define ERASE_FLASH_PAGES(first, last)                    \
  do {                                                    \
    for (i = (first); i <= (last); ++ i)                  \
      EraseFlashPage(i);                                  \
  } while (0)

extern void EraseFlashPage(unsigned char PageToErase);

#endif /* __PROG_MEM_FUNCTIONS_H__ */
