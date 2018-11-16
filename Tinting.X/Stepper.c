/* 
 * File:   statusmanager.h
 * Author: michele.abelli
 * Description: Stepper Motor management
 * Created on 16 luglio 2018, 14.16
 */

#include "timerMg.h"
#include "serialcom.h"
#include "gestio.h"
#include "typedef.h"
#include "L6482H_type.h"
#include "L6482H.h"
#include "p24FJ256GB110.h"

//#include "define.h"


static cSPIN_RegsStruct_TypeDef * cSPIN_RegsStruct;
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
**                      bit4: ADC Under Voltage
**                      bit5: Unused
**                      bit6: Switch turn-on event
**                      bit7: Command error                                                                                                                                                          
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void ConfigStepper(unsigned short Motor_ID, unsigned short Resolution, unsigned short AccDecCurrent, unsigned short RunCurrent,
                   unsigned short HoldingCurrent, unsigned long AccelerationRate, unsigned long DecelerationRate, unsigned short AlarmsEnabled)
{
    
   SPI_Set_Slave(Motor_ID);                       //Motor ID SPI selection 
    
   switch (Resolution)
    {
        case 0: cSPIN_RegsStruct->STEP_MODE = 8; //  1/1
        break;
        case 1: cSPIN_RegsStruct->STEP_MODE = 9; // 1/2
        break;
        case 2: cSPIN_RegsStruct->STEP_MODE = 10; // 1/4
        break;
        case 3: cSPIN_RegsStruct->STEP_MODE = 11; // 1/8
        break;
        case 4: cSPIN_RegsStruct->STEP_MODE = 12; // 1/16
        break;
    }   
   
    cSPIN_Set_Param(cSPIN_STEP_MODE, cSPIN_RegsStruct->STEP_MODE);

    cSPIN_RegsStruct->TVAL_ACC = AccDecCurrent ;            
    cSPIN_Set_Param(cSPIN_TVAL_ACC, cSPIN_RegsStruct->TVAL_ACC);
    cSPIN_RegsStruct->TVAL_DEC = AccDecCurrent ;            
    cSPIN_Set_Param(cSPIN_TVAL_DEC, cSPIN_RegsStruct->TVAL_DEC);
    cSPIN_RegsStruct->TVAL_RUN = RunCurrent ;               
    cSPIN_Set_Param(cSPIN_TVAL_RUN, cSPIN_RegsStruct->TVAL_RUN);
    cSPIN_RegsStruct->TVAL_HOLD = HoldingCurrent ;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct->TVAL_HOLD);
    cSPIN_RegsStruct->ACC = AccelerationRate ;              
    cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct->ACC);
    cSPIN_RegsStruct->DEC = DecelerationRate ;              
    cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct->DEC);
                
    cSPIN_RegsStruct->ALARM_EN = AlarmsEnabled ;
    cSPIN_Set_Param(cSPIN_ALARM_EN, cSPIN_RegsStruct->ALARM_EN);  
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
    //Motor ID SPI selection 
    cSPIN_Get_Param(cSPIN_STATUS);

    
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
    //Motor ID SPI selection
    //cSPIN_SetHome ( uint8_t deviceId)  //Ricerca homing come su SCCB
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
    // Motor ID SPI selection 
    cSPIN_Get_Param(cSPIN_ABS_POS);
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
    //Motor ID SPI selection
    cSPIN_Get_Param(cSPIN_SPEED);
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
    //Motor ID SPI selection 
    unsigned char direction = 0;                     //todo
    cSPIN_RegsStruct->MIN_SPEED = 0 ;               //cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct->MIN_SPEED);
    cSPIN_RegsStruct->MAX_SPEED = Speed ;           //cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct->MIN_SPEED);
    cSPIN_Move(direction, Step_N);  
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
    //Motor ID SPI selection
       unsigned char direction = 0;                   //todo
    cSPIN_RegsStruct->MIN_SPEED = 0 ;                //cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct->MIN_SPEED);
    cSPIN_RegsStruct->MAX_SPEED = Speed ;           //cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct->MIN_SPEED);
    cSPIN_Run(direction, Speed);
//    unsigned char level = getWaterLevel();          //Match fotocellula       
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
    //Motor ID SPI selection
    cSPIN_Hard_Stop();
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
    //Motor ID SPI selection
    cSPIN_RegsStruct->MIN_SPEED = 0 ;                //cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct->MIN_SPEED);
    cSPIN_RegsStruct->MAX_SPEED = Speed ;           //cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct->MIN_SPEED);
    cSPIN_Go_Home();
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
**                          3: Fotocellula Tavola
**                          4: Sensore Can Presence (Fotocellula o Ultrasuoni)
**                          5: Pannello Tavola
**                          6: Carrello Basi                                                                             
**                   'Filter': applicazione o meno del filtro in lettura Fotocellula (0 = NON applicato, 1 = applicato)
**
**      @retval stato della Fotocellula selezionata 'PhotoType' (0 = oscurata, 1 = NON oscurata)
**
*//*=====================================================================*//**
*/
unsigned char PhotocellStatus(unsigned short PhotoType, unsigned char Filter)
{
    switch (PhotoType)
    {
        case 0:
            return !FO_HOME;
        break;
        case 1:
            return !FO_ACC;
        break;
        case 2:
            return !FO_VALVE;
        break;
        case 3:
            return !FO_BRD;
        break;
        case 4:
            return !FO_CPR;
        break;
        case 5:
            return !INT_PAN;            
        break;
        case 6:
            return !INT_CAR;            
        break;        
        default:
        break;
    }    
    return 1; 
}
