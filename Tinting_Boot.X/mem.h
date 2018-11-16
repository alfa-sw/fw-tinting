/********************************************************************************
 **
 **      Filename     : mem.h
 **
 **      Description  : Memory addresses header file
 **
 **   ===========================================================================
 */
#ifndef MEM_H_DEFINED
#define MEM_H_DEFINED

/* -- Program memory macros -------------------------------------------------- */
#define __BL_CODE_BASE (0x0000L)
#define __BL_CODE_END  (0x2BFEL)

#define __APPL_CODE_BASE (0x2C00L)
#define __APPL_GOTO_ADDR "0x2C04"

#define __BL_CODE_CRC   (__BL_CODE_END)

/* -- Data memory macros ----------------------------------------------------- */
#define __APPL_DATA_BASE (0x1000)
#define __APPL_DATA_END  (0x4000)

/* This location is used to forward the 485 index to the application.          */
#define __SLAVE_INDEX_ADDR (__APPL_DATA_BASE - 0x2)
#define __BL_SW_VERSION    (__APPL_DATA_BASE - 0x8)

/* -- Interrupt handlers ----------------------------------------------------- */
#define __APPL_T1    (__APPL_CODE_BASE + 0x14)  /*  0x2C14 */
#define __APPL_U2RX1 (__APPL_CODE_BASE + 0x1E)  /*  0x2C1E */
#define __APPL_U2TX1 (__APPL_CODE_BASE + 0x28)  /*  0x2C28 */
#define __APPL_U3RX1 (__APPL_CODE_BASE + 0x32)  /*  0x2C32 */
#define __APPL_U3TX1 (__APPL_CODE_BASE + 0x3C)  /*  0x2C3C */
#define __APPL_SPI1  (__APPL_CODE_BASE + 0x46)  /*  0x2C46 */
#define __APPL_SPI2  (__APPL_CODE_BASE + 0x50)  /*  0x2C50 */
#define __APPL_SPI3  (__APPL_CODE_BASE + 0x5A)  /*  0x2C5A */
#define __APPL_I2C3  (__APPL_CODE_BASE + 0x64)  /*  0x2C64 */

#endif /* MEM_H_DEFINED */
