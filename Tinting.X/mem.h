/* 
 * File:   mem.h
 * Author: michele.abelli
 * Description: Memory addresses header file
 * Created on 16 luglio 2018, 14.18
 */

#ifndef MEM_H
#define	MEM_H

// Boot Loader Start address
#define __BOOT_GOTO_ADDR "0x0400"
/* -- Program memory macros -------------------------------------------------- */
#define __APPL_CODE_BASE (0x2C00L)
#define __APPL_CODE_END  (0x2A800L)

#define __APPL_CODE_CRC   (__APPL_CODE_BASE + 0x2)
#define __APPL_CODE_BEGIN (__APPL_CODE_BASE + 0x4)

// Interrupt vector addresses
#define  __APPL_INT0   (__APPL_CODE_BASE + 0x14)  /*  0x2C14 */
#define  __APPL_T2     (__APPL_CODE_BASE + 0x1E)  /*  0x2C1E */
#define  __APPL_U1RX1  (__APPL_CODE_BASE + 0x28)  /*  0x2C28 */
#define  __APPL_U1TX1  (__APPL_CODE_BASE + 0x32)  /*  0x2C32 */
#define  __APPL_CN     (__APPL_CODE_BASE + 0x3C)  /*  0x2C3C */
#define  __APPL_T4     (__APPL_CODE_BASE + 0x46)  /*  0x2C46 */
#define  __APPL_U2RX1  (__APPL_CODE_BASE + 0x50)  /*  0x2C50 */
#define  __APPL_U2TX1  (__APPL_CODE_BASE + 0x5A)  /*  0x2C5A */
#define  __APPL_MI2C2  (__APPL_CODE_BASE + 0x64)  /*  0x2C64 */
#define  __APPL_U3RX1  (__APPL_CODE_BASE + 0x6E)  /*  0x2C6E */
#define  __APPL_U3TX1  (__APPL_CODE_BASE + 0x78)  /*  0x2C78 */
#define  __APPL_USB1   (__APPL_CODE_BASE + 0x82)  /*  0x2C82 */
#define  __APPL_IC1    (__APPL_CODE_BASE + 0x8C)  /*  0x2CBC */
#define  __APPL_IC2    (__APPL_CODE_BASE + 0x96)  /*  0x2C96 */
/* -- Data memory macros -------------------------------------------------- */
#define __APPL_DATA_BASE 0x1010
/* Data dwelling in the BL memory, not altered by app init */
#define __BL_TEST_RESULTS_ADDR (__APPL_DATA_BASE - 0x12)
/* This location is used to store whiche program is actually active: BOOT or APPLICATION PROGRAM */
#define __PROGRAM_ACTIVE_ADDR (__APPL_DATA_BASE - 0x14)
#define __BL_SW_VERSION (__APPL_DATA_BASE - 0x18)

#define SLAVE_ADDR() (*(PtrTestResults))
#define BOOT_FW_VERSION() (*(BootPtrTestResults))
//extern volatile const unsigned long *PtrTestResults;
extern volatile const unsigned long *BootPtrTestResults;

#endif	/* MEM_H */

