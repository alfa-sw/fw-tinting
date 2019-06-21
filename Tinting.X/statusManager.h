/* 
 * File:   statusManager.h
 * Author: michele.abelli
 * Description: Manage Process Status
 * Created on 31 Maggio 2019, 16.55
 */

#ifndef _STATUS_MANAGER_H_
#define _STATUS_MANAGER_H_

#include "define.h"
#include "errorManager.h"

/**
 * -- Exported functions  -------------------------------------------------------
 */
extern void Can_Locator_Manager(unsigned short mode);
extern void set_slave_comm(unsigned short index);
/**
 * -- Sequencers ----------------------------------------------------------------
 */
extern void statusManager(void);

#endif /* _STATUS_MANAGER_H_ */
