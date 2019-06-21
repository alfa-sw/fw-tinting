/**/
/*============================================================================*/
/**
**      @file    ram.h
**
**      @brief   Global data module
**/
/*============================================================================*/
/**/
#include "define.h"
#include "mem.h"
#include "serialCom.h"

#ifdef RAM_EXTERN_DISABLE
#define RAM_EXTERN
#else
#define RAM_EXTERN extern
#endif
/*****************************************************************************/

RAM_EXTERN char BL_StandAlone
#ifdef RAM_EXTERN_DISABLE
 = TRUE
#endif
;

RAM_EXTERN Stato BLState;

RAM_EXTERN volatile long boot_fw_version __attribute__((space(data),
                                                     address(__BL_SW_VERSION)));
													 
RAM_EXTERN unsigned short Set_Reset;

RAM_EXTERN USB_STATE_TYPE USB_Status;
RAM_EXTERN unsigned char  BL_slave_id;
RAM_EXTERN unsigned short BL_SLAVE_VERSION[3]; 
RAM_EXTERN unsigned short BL_MASTER_VERSION[3]; 
RAM_EXTERN unsigned short BL_Master_Version;
RAM_EXTERN unsigned short pippo, pippo1, pippo2, pippo3, pippo4, pippo5, pippo6;


