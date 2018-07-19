/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Table Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB106.h"
#include "tableManager.h"
#include "statusManager.h"
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
void initTableStatusManager(void)
{
	Status.level = TINTING_INIT_ST;
}

/*
*//*=====================================================================*//**
**      @brief Table Initialization parameters
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void initTableParam(void)
{
  // Passi corrispondenti ad un giro completa di 360° della tavola
  TintingAct.Steps_Revolution = STEPS_REVOLUTION;
  // Tolleranza in passi corrispondente ad una rotazione completa di 360° della tavola
  TintingAct.Steps_Tolerance_Revolution = STEPS_TOLERANCE_REVOLUTION;
  // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
  TintingAct.Steps_Reference = STEPS_REFERENCE;
  // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
  TintingAct.Steps_Tolerance_Reference = STEPS_TOLERANCE_REFERENCE;
  // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito 
  TintingAct.Steps_Circuit = STEPS_CIRCUIT;
  // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito
  TintingAct.Steps_Tolerance_Circuit = STEPS_TOLERANCE_CIRCUIT;
  // Velocità massima di rotazione della tavola rotante
  TintingAct.High_Speed_Rotating_Table = HIGH_SPEED_ROTATING_TABLE;
  // Velocità minima di rotazione della tavola rotante
  TintingAct.Low_Speed_Rotating_Table = LOW_SPEED_ROTATING_TABLE;
  // Distanza in passi tra il circuito di riferimento e la spazzola
  TintingAct.Steps_Cleaning = STEPS_CLEANING;
  
  // Cleaning Duration (sec))
  TintingAct.Cleaning_duration = CLEANING_DURATION;
  // Cleaning Pause (min))
  TintingAct.Cleaning_pause = CLEANING_PAUSE;  
}    

/*
*//*=====================================================================*//**
**      @brief Updates Table status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void TableManager(void)
{

}


