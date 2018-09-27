/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Stepper Motor management
 * Created on 16 luglio 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "stepper.h"
#include "timerMg.h"
#include "serialcom.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include <xc.h>
#include "typedef.h"

/*
*//*=====================================================================*//**
**      @brief Configurazione di un Motore Stepper 
**
**      @param input 'Motor_ID': tipo di Stepper
**                   'Resolution': risoluzione in passi  nella movimentazione del motore stepper
**                                 (1/1: 0, ½: 1, ¼: 2, 1/8: 3, 1/16: 4, 1/32: 5, 1/64: 6, 1/128: 7, 1/256: 8)  
**                   'AccDecCurrent': corrente (rms) alle fasi del motore stepper durante la movimentazione in rampa 
**                   'RunCurrent': corrente (rms) alle fasi del motore stepper durante la movimentazione a velocità costante 
**                   'HoldingCurrent': corrente (rms) alle fasi a motore fermo                                                                               
**                   'AccelerationRate': accelerazione in step/sec^2 per la rampa di accelerazione 
**                   'DecelerationRate': decelerazione in step/sec^2 per la rampa di decelerazione
**                   'AlarmsEnabled': maschera di abilitazione degli allarmi (0 = disabilitato, 1 = abilitato)
**                      bit0: Over-current detection
**                      bit1: Thermal shutdown
**                      bit2: Thermal warning
**                      bit3: Under voltage lock out
**                      bit4: Stall detection
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void ConfigStepper(unsigned short Motor_ID, unsigned short Resolution, unsigned short AccDecCurrent, unsigned short RunCurrent,
                   unsigned short HoldingCurrent, unsigned long AccelerationRate, unsigned long DecelerationRate, unsigned short AlarmsEnabled)
{

}

/*
*//*=====================================================================*//**
**      @brief Lettura maschera errori del motore stepper 'Motor_ID' 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**
**      @param ouput 'AlarmsError': maschera di stato degli allarmi (0 = nessun errore, 1 = errore)
**                      bit0: Over-current detection
**                      bit1: Thermal shutdown
**                      bit2: Thermal warning
**                      bit3: Under voltage lock out
**                      bit4: Stall detection 
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void ReadStepperError(unsigned short Motor_ID, unsigned short *AlarmsError)
{

}

/*
*//*=====================================================================*//**
**      @brief Impostazione della posizione di Home (Zero) del motore stepper 'Motor_ID'
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void SetStepperHomePosition(unsigned short Motor_ID)
{

}

/*
*//*=====================================================================*//**
**      @brief Lettura della posizione del motore stepper 'Motor_ID' in mezzi passi interi
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**
**      @retval posizione del motore stepper 'Motor_ID' rispetto alla posizione di Home espressa in mezzi passi interi
**
*//*=====================================================================*//**
*/
long GetStepperPosition(unsigned short Motor_ID)
{
   return 1; 
}

/*
*//*=====================================================================*//**
**      @brief Restituisce la velocità reale (se possibile, oppure quella logica) di movimentazione del motore stepper 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**
**      @retval velocità di movimentazione del motore stepper 'Motor_ID' in RPM
**
*//*=====================================================================*//**
*/
unsigned short GetStepperSpeed(unsigned short Motor_ID)
{
   return 1; 
}

/*
*//*=====================================================================*//**
**      @brief movimentazione del motore stepper 'Motor_ID', di 'Step_N' mezzi 
**             passi interi rispetto alla posizione corrente, alla velocità 'Speed' espressa in RPM 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**                   'Step_N': numero di passi (mezzi passi interi). Puo essere un numero negativo
**                   'Speed': velocità di movimentazione(rpm)
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void MoveStepper(unsigned short Motor_ID, long Step_N, unsigned short Speed)
{

}

/*
*//*=====================================================================*//**
**      @brief start movimentazione del motore stepper 'Motor_ID', alla velocità 'Speed' 
**             espressa in RPM, nella direzione 'Direction', per una durata 'Duration' sec, 
**             oppure se uguale a zero fino alla transizione 'Transition_Type' della 
**             Fotocellula 'PhotoType'. Se 'Duration' è uguale a zero e se NON viene mai 
**             intercettata la transizione della Fotocellula specificata, allora il Motore si muove continuativamente 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**                   'Speed': velocità di movimentazione(rpm)
**                   'Direction: direzione della movimentazione (CW, o CCW) (DIR_EROG, o DIR_SUCTION)
**                   'Transition_Type': tipo di transizione da intercettare (0 = LOW_HIGH, 1 = HIGH_LOW) 
**                                      con filtro attivato se 'Duration' è = 0                 
**                   'Photo_Type': tipo di Fotocellula
**                      0: Fotocellula Home
**                      1: Fotocellula Accoppiamento
**                      2: Fotocellula Apertura Valvola
**                      3: Fotocellula Chiusura Valvola
**                      4: Fotocellula Posizione Tavola
**                      5: Sensore Can Presence (Fotocellula o Ultrasuoni)
**                   'Duration':  durata di attivazione del Motore Stepper (sec). Essa ha la priorità su Transition_Type
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void StartStepper(unsigned short Motor_ID, unsigned short Speed, 
                  unsigned char Direction, unsigned char Transition_Type, unsigned short PhotoType, unsigned long Duration)
{

}

/*
*//*=====================================================================*//**
**      @brief Arresto immediato del motore stepper 'Motor_ID'  senza effettuare la rampa di decelerazione 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void StopStepper(unsigned short Motor_ID)
{
    
}

/*
*//*=====================================================================*//**
**      @brief Movimentazione del motore stepper 'Motor_ID', nella posizione di Home, alla velocità 'Speed' espressa in RPM 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**                   'Speed': velocità di movimentazione(rpm)
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void MoveStepperToHome(unsigned short Motor_ID, unsigned short Speed)
{
    
}

/*
*//*=====================================================================*//**
**      @brief Attivazione/Disattivazione 'Mode' del Motore DC selezionato 'Motor_ID'     
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Stirring Motor
**                      1: EV Motor
**                   'Mode': Disattivazione (=0) o Attivazione (=1)
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void DCMotorManagement(unsigned short Motor_ID, unsigned char Mode)
{
    
}

/*
*//*=====================================================================*//**
**      @brief Stato Coperto/Scoperto della Fotocellula "PhotoType", nella modalità di lettura "Filter" specificata      
**
**      @param input 'PhotoType': tipo di Fotocellula
**                          0: Fotocellula Home
**                          1: Fotocellula Accoppiamento
**                          2: Fotocellula Apertura Valvola
**                          3: Fotocellula Valvola
**                          4: Sensore Can Presence (Fotocellula o Ultrasuoni)
**                          5: Pannello Tavola
**                   'Filter': applicazione o meno del filtro in lettura Fotocellula (0 = NON applicato, 1 = applicato)
**
**      @retval stato della Fotocellula selezionata 'PhotoType' (0 = oscurata, 1 = NON oscurata)
**
*//*=====================================================================*//**
*/
unsigned char PhotocellStatus(unsigned short PhotoType, unsigned char Filter)
{
   return 1; 
}
