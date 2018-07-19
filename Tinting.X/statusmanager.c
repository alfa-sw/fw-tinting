/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB106.h"
#include "statusmanager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include <xc.h>
#include "typedef.h"
/*
*//*=====================================================================*//**
**      @brief Initialization status
**
**      @param void
**
                                                                              * 
**      @retval void
**
*//*=====================================================================*//**
*/
void initStatusManager(void)
{
	Status.level = TINTING_INIT_ST;
}

/*
*//*=====================================================================*//**
**      @brief Updates general status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void StatusManager(void)
{

}
