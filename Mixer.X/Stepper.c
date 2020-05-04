/* 
 * File:   Stepper.c
 * Author: michele.abelli
 * Description: Stepper Motor management
 * Created on 12 marzo 2020, 16.16
 */

/*===== INCLUSIONI ========================================================= */
#include "timerMg.h"
#include "serialcom.h"
#include "gestio.h"
#include "typedef.h"
#include "L6482H.h"
#include "stepperParameters.h"
#include "p24FJ256GB110.h"
#include "stepper.h"
#include "ram.h"
#include "define.h"
#include "stepperTable.h"
 
/*====== TIPI LOCALI ======================================================== */
typedef union {
  unsigned char bytes[2];
  unsigned short word;

  struct {
    /* User-operated inputs */
    unsigned short  HiZ       : 1;      
    unsigned short  BUSY      : 1;  
    unsigned short  SW_F      : 1;  
    unsigned short  SW_EVN    : 1;  
    unsigned short  DIR       : 1;  
    unsigned short  MOT_STATUS: 2;  
    unsigned short  CMD_ERROR : 1;      
    unsigned short  STCK_MOD  : 1;  
    unsigned short  UVLO      : 1;  
    unsigned short  UVLO_ADC  : 1;
    unsigned short  TH_STATUS : 2; 
    unsigned short  OCD       : 1;     
    unsigned short  unused    : 2; 
  } Bit;

} Stepper_Status_Type;

/*====== MACRO LOCALI ====================================================== */
//enumerativo status per la macchina di movimentazione
enum
{
    STATUS_MOVEMENT_WAIT_PHOTO_HOME,
    STATUS_MOVEMENT_WAIT_PHOTO_JAR,
    STATUS_MOVEMENT_WAIT_DOOR_MICRO,
    STATUS_MOVEMENT_WAIT_PHOTO_DOOR_OPEN,    
    STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_OPEN, 
    STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_LIFTER,
    /*INSERIRE QUI ALTRE TRANSAZIONI DI TIPO FOTOCELULLA**/
    STATUS_MOVEMENT_WAIT_TIME,
    STATUS_MOVEMENT_DEINIT,
};

typedef struct
{
  unsigned char status;
  unsigned char transaction;  //3 possibili valori
} Stepper_Movements_Status_Type;


/*====== VARIABILI LOCALI =================================================== */
static Stepper_Movements_Status_Type stepperMovementStatus[ALL_DRIVERS]={{STATUS_MOVEMENT_DEINIT,TRANSACTION_DISABLED}};
static cSPIN_RegsStruct_TypeDef  cSPIN_RegsStruct = {0};  //to set

/*=====================================================================*//**
**      @brief Configurazione di un Motore Stepper 
**
**      @param input 'Motor_ID': tipo di Stepper
**                   'Resolution': risoluzione in passi  nella movimentazione del motore stepper
**                                 (1/1: 0, ½: 1, ¼: 2, 1/8: 3, 1/16: 4, 1/32: 5, 1/64: 6, 1/128: 7, 1/256: 8)  
**                   'AccDecCurrent':  corrente dA (picco) alle fasi del motore stepper durante la movimentazione in rampa 
**                   'RunCurrent': corrente dA (picco) alle fasi del motore stepper durante la movimentazione a velocità costante 
**                   'HoldingCurrent': corrente dA (picco) alle fasi a motore fermo                                                                               
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
                   unsigned short HoldingCurrent, unsigned long AccelerationRate, unsigned long DecelerationRate, unsigned char AlarmsEnabled)
{       
    //Motor ID SPI selection 
    unsigned char currentReg = 0;
    unsigned short accentReg = 0;    
    
   //Risoluzione Motore NOTA BENE:VALORE REGISTRO al RESET 0             
   cSPIN_RegsStruct.STEP_MODE = cSPIN_Get_Param(cSPIN_STEP_MODE, Motor_ID);
    
   switch (Resolution)
    {
        case 0: cSPIN_RegsStruct.STEP_MODE = 0; //  1/1
        break;
        case 1: cSPIN_RegsStruct.STEP_MODE = 1; // 1/2
        break;
        case 2: cSPIN_RegsStruct.STEP_MODE = 2; // 1/4
        break;
        case 3: cSPIN_RegsStruct.STEP_MODE = 3; // 1/8
        break;
        case 4: cSPIN_RegsStruct.STEP_MODE = 4; // 1/16
        break;
    }   
   
    //Risoluzione
    cSPIN_RegsStruct.STEP_MODE |=  0x08;  //1. When the register is written Bit 3 must be set to 1.
    cSPIN_Set_Param(cSPIN_STEP_MODE, cSPIN_RegsStruct.STEP_MODE, Motor_ID);

    //AccDecCurrent  (picco))  ->  Valore assoluto che indica i mV   
    currentReg = AccDecCurrent * 100 /156;
    cSPIN_RegsStruct.TVAL_ACC = currentReg ;            
    cSPIN_Set_Param(cSPIN_TVAL_ACC, cSPIN_RegsStruct.TVAL_ACC, Motor_ID);       
    cSPIN_RegsStruct.TVAL_DEC = currentReg;            
    cSPIN_Set_Param(cSPIN_TVAL_DEC, cSPIN_RegsStruct.TVAL_DEC, Motor_ID);
    
    currentReg = RunCurrent * 100 /156;
    cSPIN_RegsStruct.TVAL_RUN = currentReg ;               
    cSPIN_Set_Param(cSPIN_TVAL_RUN, cSPIN_RegsStruct.TVAL_RUN, Motor_ID);
    
    currentReg = HoldingCurrent * 100 /156;
    cSPIN_RegsStruct.TVAL_HOLD = currentReg;          
    cSPIN_Set_Param(cSPIN_TVAL_HOLD, cSPIN_RegsStruct.TVAL_HOLD, Motor_ID);
    
    //Accelerazioni
    if (AccelerationRate > 59000)
        {
        AccelerationRate = 59000;
        }
    if (AccelerationRate < 1000)
        {
        AccelerationRate = 1000;
        }
    accentReg = TABLE_ACC_DEC[AccelerationRate/1000]; //uso tabella
    cSPIN_RegsStruct.ACC = accentReg;          
    cSPIN_Set_Param(cSPIN_ACC, cSPIN_RegsStruct.ACC, Motor_ID);
    
    if (DecelerationRate > 59000)
        {
        DecelerationRate = 59000;
        }
    if (DecelerationRate < 1000)
        {
        DecelerationRate = 1000;
        }
    accentReg = TABLE_ACC_DEC[DecelerationRate/1000]; //uso tabella
    cSPIN_RegsStruct.DEC = accentReg;          
    cSPIN_Set_Param(cSPIN_DEC, cSPIN_RegsStruct.DEC, Motor_ID);
        
    //ALTRI PARAMETRI     
    cSPIN_RegsStruct.MIN_SPEED = 0;         
    cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct.MIN_SPEED, Motor_ID);
    
    //Abilitazione Allarmi
    cSPIN_RegsStruct.ALARM_EN = AlarmsEnabled ;
    cSPIN_Set_Param(cSPIN_ALARM_EN, cSPIN_RegsStruct.ALARM_EN, Motor_ID);     
     
     cSPIN_RegsStruct.T_FAST =  0x35;
     cSPIN_Set_Param(cSPIN_T_FAST, cSPIN_RegsStruct.T_FAST, Motor_ID); 
     
     cSPIN_RegsStruct.TON_MIN =  0x05;
     cSPIN_Set_Param(cSPIN_TON_MIN, cSPIN_RegsStruct.TON_MIN, Motor_ID); 
     
     cSPIN_RegsStruct.TOFF_MIN=  0x29;
     cSPIN_Set_Param(cSPIN_TOFF_MIN, cSPIN_RegsStruct.TOFF_MIN, Motor_ID); 
          
//     cSPIN_RegsStruct.OCD_TH =  0x08;
//     cSPIN_RegsStruct.OCD_TH =  0x0A;
//     cSPIN_RegsStruct.OCD_TH =  0x16;
     cSPIN_RegsStruct.OCD_TH =  0x1A;     
     cSPIN_Set_Param(cSPIN_OCD_TH, cSPIN_RegsStruct.OCD_TH, Motor_ID); 
     
      cSPIN_RegsStruct.FS_SPD =  0x3FF;
     cSPIN_Set_Param(cSPIN_FS_SPD , cSPIN_RegsStruct.FS_SPD, Motor_ID); 

     cSPIN_RegsStruct.GATECFG1 =  0x00C4;
     cSPIN_Set_Param(cSPIN_GATECFG1 , cSPIN_RegsStruct.GATECFG1, Motor_ID); 

     cSPIN_RegsStruct.GATECFG2 =  0x61;
     cSPIN_Set_Param(cSPIN_GATECFG2 , cSPIN_RegsStruct.GATECFG2, Motor_ID);
        
     cSPIN_RegsStruct.CONFIG =  0xA280;
     cSPIN_Set_Param(cSPIN_CONFIG , cSPIN_RegsStruct.CONFIG, Motor_ID); 
}

/*
*//*=====================================================================*//**
**      @brief Lettura maschera errori del motore stepper 'Motor_ID' 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:               
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
    Stepper_Status_Type Status_reg;
    
    //Motor ID SPI selection 
    
    Status_reg.word = cSPIN_Get_Param(cSPIN_STATUS, Motor_ID);

    *AlarmsError = (unsigned short)Status_reg.word;
}

/*
*//*=====================================================================*//**
**      @brief Impostazione della posizione di Home (Zero) del motore stepper 'Motor_ID'
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:               
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void SetStepperHomePosition(unsigned short Motor_ID)
{
    //Motor ID SPI selection
    cSPIN_Reset_Pos(Motor_ID);
}

/*
*//*=====================================================================*//**
**      @brief Lettura della posizione del motore stepper 'Motor_ID' in passi intesi in base alla risoluzione impostata
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:               
**
**      @retval posizione del motore stepper 'Motor_ID' rispetto alla posizione di Home espressa in passi intesi in base alla risoluzione impostata
**
*//*=====================================================================*//**
*/
signed long GetStepperPosition(unsigned short Motor_ID)
{
    signed long pos_32; 
    // Motor_ID SPI selection
    pos_32 = cSPIN_Get_Param(cSPIN_ABS_POS, Motor_ID);
	pos_32 = pos_32 & 0x3FFFFF;
    // Numero positivo, nessuna necessità di conversione
    if ((pos_32 & 0x200000) == 0)
        return pos_32;
    // Numero negativo, necessità di conversione da complemento a 2 a 22 bit a complemento a 2 a 32 bit
    else {
        // Conversione da complemento a 2 a 22 bit a decimale
        pos_32 = ~(pos_32);
        pos_32 = pos_32 & 0x3FFFFF;
        pos_32++;
        // Conversione da decimale a complemento a 2 a 32 bit
        pos_32 = ~(pos_32);
        pos_32++;
        return pos_32;        
    }         
//    return cSPIN_Get_Param(cSPIN_ABS_POS, Motor_ID);
}

/*
*//*=====================================================================*//**
**      @brief Restituisce la velocità  in step/tick di movimentazione del motore stepper 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:               
**
**      @retval velocità di movimentazione del motore stepper 'Motor_ID' 
**
*//*=====================================================================*//**
*/
unsigned short GetStepperSpeed(unsigned short Motor_ID)
{
    //Motor ID SPI selection    
    uint32_t speed_reg = 0;
   speed_reg =  cSPIN_Get_Param(cSPIN_SPEED, Motor_ID);
    
   return 0;
   
}

/*
*//*=====================================================================*//**
**      @brief movimentazione del motore stepper 'Motor_ID', di 'Step_N' mezzi 
**             passi interi rispetto alla posizione corrente, alla velocità 'Speed' espressa in RPM 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:              
**                   'Step_N': numero di passi (mezzi passi interi). Puo essere un numero negativo
**                   'Speed': velocità di movimentazione(rpm)
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void MoveStepper(unsigned short Motor_ID, long Step_N, unsigned short Speed_RPM)
{
    //Motor ID SPI selection 
    uint16_t regSpeed = 0;
    unsigned char direction = FWD;           
    unsigned long absolute_step_number = 0;        
    
    Step_N = -Step_N;
    
    //Velocità
    regSpeed = 0;
    cSPIN_RegsStruct.MIN_SPEED = regSpeed;         
    cSPIN_Set_Param(cSPIN_MIN_SPEED, cSPIN_RegsStruct.MIN_SPEED, Motor_ID);
    
    regSpeed = TABLE_MAX_SPEED[Speed_RPM/10];
    cSPIN_RegsStruct.MAX_SPEED = regSpeed;   
    cSPIN_Set_Param(cSPIN_MAX_SPEED, cSPIN_RegsStruct.MAX_SPEED, Motor_ID);    
    
    if (Step_N > 0)
    {
        direction = FWD;
        absolute_step_number = Step_N;
    }
    else
    {
        direction = REV;
        absolute_step_number = - Step_N;
    }
                            
    cSPIN_Move(direction, absolute_step_number, Motor_ID);  
}

/*
*//*=====================================================================*//**
**      @brief movimentazione del motore stepper 'Motor_ID'  alla velocità 'Speed' espressa in RPM 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:                
**                   'Speed': velocità di movimentazione(rpm)
**                   'Direction': Direzione di movimentazione(rpm)
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void Run_Stepper(unsigned short Motor_ID, unsigned short Speed_RPM, unsigned char Direction)
{
    uint32_t regSpeed = 0;
    uint16_t maxSpeed_RPM = Speed_RPM ;
    
    maxSpeed_RPM = TABLE_MAX_SPEED[maxSpeed_RPM/10];
    cSPIN_RegsStruct.MAX_SPEED = maxSpeed_RPM;       
    cSPIN_Set_Param(cSPIN_MAX_SPEED, cSPIN_RegsStruct.MAX_SPEED, Motor_ID);   
    
    //Conversione velocità da RPM a step/tick
    regSpeed = TABLE_SPEED_RUN_COMMAND[Speed_RPM/10];
        
    cSPIN_Run(Direction, regSpeed, Motor_ID);
    
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
**                      0: Mixer
**                      1: 
**                      2:                  
**                   'Speed': velocità di movimentazione(rpm)
**                   'Direction: direzione della movimentazione (CW, o CCW) (DIR_EROG, o DIR_SUCTION)
**                   'Transition_Type': tipo di transizione da intercettare (0 = LOW_HIGH, 1 = HIGH_LOW) 
**                                      con filtro attivato se 'Duration' è = 0                 
**                   'Photo_Type': tipo di Fotocellula
**                      0: Mixer Homing Photocell
**                      1: Jar Photocell
**                      2: Door Closed Microswitch
**                      3: Door Open Photocell
**                      4: 
**                      5: 
**                      6: 
**                   'Duration':  durata di attivazione del Motore Stepper (sec). Essa ha la priorità su Transition_Type
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void StartStepper(unsigned short Motor_ID, unsigned short Speed_RPM, 
                  unsigned char Direction, unsigned char Transition_Type, unsigned short PhotoType, unsigned long Duration)
{     
    uint32_t regSpeed = 0;
    uint16_t maxSpeed_RPM = Speed_RPM ;
    
    maxSpeed_RPM = TABLE_MAX_SPEED[maxSpeed_RPM/10];
    cSPIN_RegsStruct.MAX_SPEED = maxSpeed_RPM;       
    cSPIN_Set_Param(cSPIN_MAX_SPEED, cSPIN_RegsStruct.MAX_SPEED, Motor_ID);   
    
    //Conversione velocità da RPM a step/tick
    regSpeed = TABLE_SPEED_RUN_COMMAND[Speed_RPM/10];
    
    //Impostazine macchina a stati
    if (Duration)
    {
        switch (Motor_ID)
        {   
            //Uso Timer 
            SetStartStepperTime(Duration,Motor_ID);   
            case MOTOR_MIXER:                
            {
                StartTimer(T_START_STEPPER_MOTOR_MIXER);   
            }
            break;
            case MOTOR_DOOR:
            {
                StartTimer(T_START_STEPPER_MOTOR_DOOR);   
            }
            case MOTOR_AUTOCAP:
            {
                StartTimer(T_START_STEPPER_MOTOR_AUTOCAP);   
            }                
        }         
        stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_TIME; 
        stepperMovementStatus[Motor_ID].transaction =  TRANSACTION_DISABLED;
        //Faccio partire la movimentazione
        cSPIN_Run(Direction,regSpeed, Motor_ID);
    }
    else  //uso transazione
    {                
        switch(PhotoType)
        {
            case HOME_PHOTOCELL: // Mixer Homing Photocell
            {                
                if((Transition_Type == LIGHT_DARK) && (FO_HOME == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_HOME;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_HOME == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_HOME;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }                                  
            }
            break;
            case JAR_PHOTOCELL: // Jar Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_CPR == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_JAR;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_CPR == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_JAR;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
            }
            break;
            case DOOR_MICROSWITCH: // Door Closed Microswitch
            {
                if((Transition_Type == LIGHT_DARK) && ((!INT_CAR) == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_DOOR_MICRO;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && ((!INT_CAR) == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_DOOR_MICRO;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
            }
            break; 
            case DOOR_OPEN_PHOTOCELL: // Door Open Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_BRD == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_DOOR_OPEN;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_BRD == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_DOOR_OPEN;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
            }                
            break;
            case AUTOCAP_OPEN_PHOTOCELL: // Autocap Open Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_VALV == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_OPEN;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_VALV == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_OPEN;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
            }
            break;  
            case AUTOCAP_LIFTER_PHOTOCELL: // Autocap Lifter Down Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_ACC == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_LIFTER;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_ACC == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_LIFTER;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
            }
            break;  
            
            // Photocell Sensor 
            default: break;
        
        }                        
        
    }  
    
}

/*
*//*=====================================================================*//**
**      @brief Arresto immediato del motore stepper 'Motor_ID'  senza effettuare la rampa di decelerazione 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:                
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void StopStepper(unsigned short Motor_ID)
{
    //Motor ID SPI selection
    cSPIN_Hard_Stop(Motor_ID);
}

void SoftStopStepper(unsigned short Motor_ID)
/*
*//*=====================================================================*//**
**      @brief Il comando SoftStopStepper fa decelerare il motore con una decelerazione 
*              programmata valore fino al raggiungimento del valore MIN_SPEED e quindi arresta
*              il motore mantenendo il rotore in posizione (viene applicata una coppia
*              di stazionamento).
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:                 
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
{
    cSPIN_Soft_Stop(Motor_ID);
}


void SoftHiZ_Stepper(unsigned short Motor_ID)
/*
*//*=====================================================================*//**
**      @brief Il comando SoftHiZ_Stepper fa decelerare il motore con una decelerazione 
*              programmata valore fino al raggiungimento del valore MIN_SPEED e
*             quindi forza i ponti in stato  di alta impedenza (non è presente la coppia di stazionamento).
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:               
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
{
    cSPIN_Soft_HiZ(Motor_ID);
}

void HardHiZ_Stepper(unsigned short Motor_ID)
/*
*//*=====================================================================*//**
**      @brief Il comando HardHiZ forza istantaneamente i ponti in uno stato di
*              alta impedenza (nessuna coppia è presente).
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:               
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
{
    stepperMovementStatus[Motor_ID].status  = STATUS_MOVEMENT_DEINIT;
    stepperMovementStatus[Motor_ID].transaction = TRANSACTION_DISABLED;
    cSPIN_Hard_HiZ(Motor_ID);
}
/*
*//*=====================================================================*//**
**      @brief Movimentazione del motore stepper 'Motor_ID', nella posizione di Home, alla velocità 'Speed' espressa in RPM 
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:                 
**                   'Speed': velocità di movimentazione(rpm)
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
void MoveStepperToHome(unsigned short Motor_ID, unsigned short Speed_RPM)
{
    uint16_t regSpeed = 0;
    //Motor ID SPI selection
    //Velocità    
    regSpeed = TABLE_MAX_SPEED[Speed_RPM/10];
    cSPIN_RegsStruct.MAX_SPEED = regSpeed;   
    cSPIN_Set_Param(cSPIN_MAX_SPEED, cSPIN_RegsStruct.MAX_SPEED, Motor_ID);    
    
    cSPIN_Go_Home(Motor_ID);
}

/*
*//*=====================================================================*//**
**      @brief Attivazione/Disattivazione 'Mode' del Motore DC selezionato 'Motor_ID'     
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2:    
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
**                          0: Fotocellula Mixer Home
**                          1: Fotocellula Presena Barattolo
**                          2: Microswitch Porta Chiusa
**                          3: Fotocellula Porta Aperta 
**                   'Filter': applicazione o meno del filtro in lettura Fotocellula (0 = NON applicato, 1 = applicato)
**
**      @retval stato della Fotocellula selezionata 'PhotoType' (0 = oscurata, 1 = NON oscurata)
**
*//*=====================================================================*//**
*/
unsigned char PhotocellStatus(unsigned short PhotoType, unsigned char Filter)
{
unsigned char ret = FALSE;

/*		
	DigInNotFiltered.Bit.StatusType0 = LEVEL;
	DigInNotFiltered.Bit.StatusType1 = FO_CPR; = Jar Presence
	DigInNotFiltered.Bit.StatusType2 = FO_VALV; = Autocap Open 
	DigInNotFiltered.Bit.StatusType3 = FO_ACC;  = Autocap Lifter Down
	DigInNotFiltered.Bit.StatusType4 = FO_BRD;  = Door Open
	DigInNotFiltered.Bit.StatusType5 = FO_HOME; = Mixer Home
    DigInNotFiltered.Bit.StatusType6 = FO_GEN1;
	DigInNotFiltered.Bit.StatusType7 = FO_GEN2;
    DigInNotFiltered.Bit.StatusType8 = !INT_CAR; = Door Closed
    DigInNotFiltered.Bit.StatusType9 = INT_PAN;
    DigInNotFiltered.Bit.StatusType10 = IO_GEN1;
    DigInNotFiltered.Bit.StatusType11 = IO_GEN2;
    DigInNotFiltered.Bit.StatusType12 = BUTTON;
*/
    switch (PhotoType)
    {
        case PHOTO_HOME: // 0: Fotocellula Mixer Home
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType5 ? TRUE:FALSE;
            }
            else
            {
                ret = FO_HOME;
            }
        }
        break;
        case PHOTO_JAR:  // 1: Fotocellula presenza Barattolo
        {
            if (Filter)
            {
//                ret =  OutputFilter.Bit.StatusType1 ? TRUE:FALSE;
                ret =  OutputFilter.Bit.StatusType2 ? TRUE:FALSE;                
            }
            else
            {
                ret = FO_CPR;
//                ret = FO_VALV;
            }
        }
        break;
        case MICROSWITCH_DOOR:  // 2: Microswitch Porta Aperta
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType8 ? TRUE:FALSE;
            }
            else
            {
                ret = !INT_CAR;
            }
        }
        break;   
        case PHOTO_DOOR_OPEN:  // 3: Fotocellula Porta Aperta
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType4 ? TRUE:FALSE;
            }
            else
            {
                ret = FO_BRD;
            }
        }
        break;
        case PHOTO_AUTOCAP_OPEN:  // 4: Fotocellula Autocap Aperto
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType2 ? TRUE:FALSE;
            }
            else
            {
                ret = FO_VALV;
            }
        }
        break;           
        case PHOTO_AUTOCAP_LIFTER:  // 5: Fotocellula Autocap Lifter Down
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType3 ? TRUE:FALSE;
            }
            else
            {
                ret = FO_ACC;
            }
        }
        break;                   
        default:
        {              
        }
        break;    
    }
    
    return ret;
}

unsigned short GetStatus(unsigned short Motor_ID)
/*
*//*=====================================================================*//**
**      @brief Read  STATUS REGISTER
**
**      @param nessuno
**
**      @retval STATUS REGISTER
**
*//*=====================================================================*//**
*/
{
return cSPIN_Get_Status(Motor_ID);
}

void StepperMovementsManager(void)
/*
*//*=====================================================================*//**
**      @brief Sequencer per gestioni comandi
**
**      @param nessuno
**
**      @retval nessumno
**
*//*=====================================================================*//**
*/
{
    unsigned char motor =0;

    for (motor=0; motor< ALL_DRIVERS; motor++)
    {
        switch (stepperMovementStatus[motor].status)
            {
            case STATUS_MOVEMENT_DEINIT:
            {
                //do nothing
            }
            break;
            case STATUS_MOVEMENT_WAIT_TIME:
            {                       
                switch (motor)
                {                                          
                    case MOTOR_MIXER:                        
                    {
                        if (StatusTimer(T_START_STEPPER_MOTOR_MIXER)==T_ELAPSED)
                            {
                            StopTimer(T_START_STEPPER_MOTOR_MIXER);
                            StopStepper(MOTOR_MIXER);
                            }    
                    }
                    break;
                    case MOTOR_DOOR:                        
                    {
                        if (StatusTimer(T_START_STEPPER_MOTOR_DOOR)==T_ELAPSED)
                            {
                            StopTimer(T_START_STEPPER_MOTOR_DOOR);
                            StopStepper(MOTOR_DOOR);
                            }    
                    }
                    break;                    
                    case MOTOR_AUTOCAP:                        
                    {
                        if (StatusTimer(T_START_STEPPER_MOTOR_AUTOCAP)==T_ELAPSED)
                            {
                            StopTimer(T_START_STEPPER_MOTOR_AUTOCAP);
                            StopStepper(MOTOR_AUTOCAP);
                            }    
                    }
                    break;                    
                }
            }
            break;
            case STATUS_MOVEMENT_WAIT_PHOTO_HOME:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_HOME == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_HOME == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }
            }
            break;
            case STATUS_MOVEMENT_WAIT_PHOTO_JAR:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_CPR == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_CPR == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }       
            }
            break;
            case STATUS_MOVEMENT_WAIT_DOOR_MICRO:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if((!INT_CAR) == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if((!INT_CAR) == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }       
            }
            break;  
            case STATUS_MOVEMENT_WAIT_PHOTO_DOOR_OPEN:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_BRD == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_BRD == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }       
            }
            break;
            case STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_OPEN:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_VALV == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_VALV == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }       
            }
            break;
            case STATUS_MOVEMENT_WAIT_PHOTO_AUTOCAP_LIFTER:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_ACC == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_ACC == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }       
            }
            break;
            
        };  //end switc
    }  //end for
}