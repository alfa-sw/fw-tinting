/* 
 * File:   mem.h
 * Author: michele.abelli
 * Description: Memory addresses header file
 * Created on 16 luglio 2018, 14.18
 */

#ifndef MEM_H
#define	MEM_H

#define SW_VERSION (0x40001)
// MGB - MMT Protocol Version 
// Incremented by 1 for every protocol modification (new commands, or modifications)
#define PROTOCOL_VERSION (0x01)
// BootLoader Version identification code
#define BOOT_CODE (0x010000)
// Last BootLoader Bad Version
#define MAB_BAD_BOOT_CODE			(0x010000)
#define ACTUATOR_BAD_BOOT_CODE		(0x010500)
#define CANLIFTER_BAD_BOOT_CODE		(0x010600)
#define HUMIDIFIER_BAD_BOOT_CODE	(0x010700)
#define TINTING_BAD_BOOT_CODE		(0x010800)

#define JUMP_TO_BOOT_DONE (0xAA)

// Boot Loader Start address
#define __BOOT_GOTO_ADDR "0x0400"
/* -- Program memory macros -------------------------------------------------- */
#define __APPL_CODE_BASE (0x2C00L)
#define __APPL_CODE_END  (0x2A800L)

#define __APPL_CODE_CRC   (__APPL_CODE_BASE + 0x2)
#define __APPL_CODE_BEGIN (__APPL_CODE_BASE + 0x4)

// Interrupt vector addresses
#define __APPL_T1    (__APPL_CODE_BASE + 0x14)  //  0x2C14 
#define __APPL_U2RX1 (__APPL_CODE_BASE + 0x3A)  //  0x2C3A 
#define __APPL_U2TX1 (__APPL_CODE_BASE + 0x60)  //  0x2C60 
#define __APPL_U3RX1 (__APPL_CODE_BASE + 0x86)  //  0x2C86 
#define __APPL_U3TX1 (__APPL_CODE_BASE + 0xAC)  //  0x2CAC 
#define __APPL_SPI1  (__APPL_CODE_BASE + 0xD2)  //  0x2CD2 
#define __APPL_SPI2  (__APPL_CODE_BASE + 0xF8)  //  0x2CF8 
#define __APPL_SPI3  (__APPL_CODE_BASE + 0x11E) //  0x2D1E 
#define __APPL_I2C3  (__APPL_CODE_BASE + 0x144) //  0x2D44 

/* -- Data memory macros ----------------------------------------------------- */
#define __APPL_DATA_BASE 0x1010
// This location is used to store Slave Address
#define __BL_TEST_RESULTS_ADDR (__APPL_DATA_BASE - 0x12)
// This location is used to store Boot Loader Firmware Version
#define __BL_SW_VERSION (__APPL_DATA_BASE - 0x18)
#define __JMP_BOOT_ADDR (__APPL_DATA_BASE - 0x1C)

#define SLAVE_ADDR() (*(PtrTestResults))
#define BOOT_FW_VERSION() (*(BootPtrTestResults))
extern volatile const unsigned long *BootPtrTestResults;

#endif	/* MEM_H */

