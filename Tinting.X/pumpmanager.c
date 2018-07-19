/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Pump Processes management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB106.h"
#include "pumpManager.h"
#include "statusManager.h"
#include "timerMg.h"
#include "serialcom.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "typedef.h"
#include <xc.h>

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
void initPumpStatusManager(void)
{
	Status.level = TINTING_INIT_ST;
}

/*
*//*=====================================================================*//**
**      @brief Pump Initialization parameters
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void initPumpParam(void)
{
  // Passi da fotocellula madrevite coperta a fotocellula ingranamento coperta
  TintingAct.Step_Accopp = STEP_ACCOPP;
  // Passi a fotoellula ingranamento coperta per ingaggio circuito
  TintingAct.Step_Ingr = STEP_INGR;
  // Passi per recupero giochi
  TintingAct.Step_Recup = STEP_RECUP;
  // Passi a fotocellula madrevite coperta per posizione di home
  TintingAct.Passi_Madrevite = PASSI_MADREVITE;
  // Passi per raggiungere la posizione di start ergoazione in alta risoluzione
  TintingAct.Passi_Appoggio_Soffietto = PASSI_APPOGGIO_SOFFIETTO;
  // Velocità da fotocellula madrevite coperta a fotocellula ingranamento coperta
  TintingAct.V_Accopp = V_ACCOPP;
  // Velocità a fotoellula ingranamento coperta per ingaggio circuito
  TintingAct.V_Ingr = V_INGR;
  // Velocità per raggiungere la posizione di start ergoazione in alta risoluzione
  TintingAct.V_Appoggio_Soffietto = V_APPOGGIO_SOFFIETTO;
  // Passi da posizione di home (valvola chiusa) a posizone di valvola aperta su foro grande
  TintingAct.Step_Valve_Open_Big = STEP_VALVE_OPEN_BIG;
  // Passi da posizione di home (valvola chiusa) a posizone di valvola aperta su foro piccolo
  TintingAct.Step_Valve_Open_Small = STEP_VALVE_OPEN_SMALL;
  // Velocità di apertura/chiusura valvola
  TintingAct.Speed_Valve = SPEED_VALVE;
  // N. steps in una corsa intera
  TintingAct.N_steps_stroke = N_STEPS_STROKE;   
}    

/*
*//*=====================================================================*//**
**      @brief Updates Pump status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void PumpManager(void)
{

}

