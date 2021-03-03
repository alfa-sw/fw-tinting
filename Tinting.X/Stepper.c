/* 
 * File:   Stepper.h
 * Author: michele.abelli
 * Description: Stepper Motor management
 * Created on 16 luglio 2018, 14.16
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
    STATUS_MOVEMENT_WAIT_PHOTO_COUPLING,
    STATUS_MOVEMENT_WAIT_PHOTO_TABLE,
    STATUS_MOVEMENT_WAIT_PHOTO_VALVE,           
    STATUS_MOVEMENT_WAIT_PHOTO_CAN_PRESENCE,
    STATUS_MOVEMENT_WAIT_PHOTO_PANEL_TABLE,
    STATUS_MOVEMENT_WAIT_PHOTO_BASES_CARRIAGE,
    /*INSERIRE QUI ALTRE TRASAZIONI DI TIPO FOTOCELULLA**/
    STATUS_MOVEMENT_WAIT_TIME,
    STATUS_MOVEMENT_DEINIT,
    STATUS_MOVEMENT_WAIT_PHOTO_OPEN_VALVE
};

typedef struct
{
  unsigned char status;
  unsigned char transaction;  //3 possibili valori
} Stepper_Movements_Status_Type;


/*====== VARIABILI LOCALI =================================================== */
//static Stepper_Status_Type StatusBoardDriver;
//static Stepper_Status_Type StatusPumpDriver;
//static Stepper_Status_Type StatusValveDriver;
static Stepper_Movements_Status_Type stepperMovementStatus[ALL_DRIVERS]={{STATUS_MOVEMENT_DEINIT,TRANSACTION_DISABLED}};
static cSPIN_RegsStruct_TypeDef  cSPIN_RegsStruct = {0};  //to set
//static cSPIN_RegsStruct_TypeDef  cSPIN_RegsStructRead = {0}; //to read

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
//     cSPIN_RegsStruct.OCD_TH =  0x1A;     
     cSPIN_RegsStruct.OCD_TH =  0x1F;          
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
    cSPIN_Reset_Pos(Motor_ID);
}

/*
*//*=====================================================================*//**
**      @brief Lettura della posizione del motore stepper 'Motor_ID' in passi intesi in base alla risoluzione impostata
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
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
    if ((pos_32 & 0x200000) == 0)  {
       return pos_32;
    }
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
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
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
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
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
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**                   'Speed': velocità di movimentazione(rpm)
**                   'Direction: direzione della movimentazione (CW, o CCW) (DIR_EROG, o DIR_SUCTION)
**                   'Transition_Type': tipo di transizione da intercettare (0 = LOW_HIGH, 1 = HIGH_LOW) 
**                                      con filtro attivato se 'Duration' è = 0                 
**                   'Photo_Type': tipo di Fotocellula
**                      0: Pump Homing Photocell
**                      1: Coupling Photocell
**                      2: Valve Homing Photocell
**                      3: Table Photocell
**                      4: Can Presence Photocell 
**                      5: Panel Table
**                      6: Bases Carriage
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
            case MOTOR_TABLE:
            {
                StartTimer(T_START_STEPPER_MOTOR_TABLE);   
            }
            break;
            case MOTOR_PUMP:
                
            {
                StartTimer(T_START_STEPPER_MOTOR_PUMP);   
            }
            break;
            case MOTOR_VALVE:
            {
                StartTimer(T_START_STEPPER_MOTOR_VALVE);  
            }
            break;        
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
            case HOME_PHOTOCELL: // Pump Homing Photocell
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
            case COUPLING_PHOTOCELL: // Coupling Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_ACC == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_COUPLING;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_ACC == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_COUPLING;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
                
            }
            break;
            case VALVE_PHOTOCELL: // Valve Homing Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_VALV == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_VALVE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_VALV == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_VALVE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }    
            }
            break;
            case VALVE_OPEN_PHOTOCELL: // Valve Open Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_GEN1 == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_OPEN_VALVE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_GEN1 == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_OPEN_VALVE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }    
            }
            break;                        
            case TABLE_PHOTOCELL: // Table Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_BRD == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_TABLE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_BRD == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_TABLE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
            }
            break;
            case CAN_PRESENCE_PHOTOCELL: // Can Presence Photocell
            {
                if((Transition_Type == LIGHT_DARK) && (FO_CPR == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_CAN_PRESENCE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (FO_CPR == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_CAN_PRESENCE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }   
            }
            break;
            case PANEL_TABLE: // Panel Table
            {
                if((Transition_Type == LIGHT_DARK) && (INT_PAN == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_PANEL_TABLE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (INT_PAN == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_PANEL_TABLE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;                    
                }    
            }
            break;
            case BASES_CARRIAGE: // Bases Carriage
            {
                if((Transition_Type == LIGHT_DARK) && (INT_CAR == LIGHT))
                {                    
                    cSPIN_Run(Direction,regSpeed, Motor_ID);    
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_BASES_CARRIAGE;
                    stepperMovementStatus[Motor_ID].transaction =  Transition_Type;
                }
                else if((Transition_Type == DARK_LIGHT) && (INT_CAR == DARK))
                {
                    cSPIN_Run(Direction,regSpeed, Motor_ID);
                    stepperMovementStatus[Motor_ID].status = STATUS_MOVEMENT_WAIT_PHOTO_BASES_CARRIAGE;
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
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
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
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
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
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola                 
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
{
    stepperMovementStatus[MOTOR_TABLE].status = STATUS_MOVEMENT_DEINIT;
    stepperMovementStatus[MOTOR_PUMP].status  = STATUS_MOVEMENT_DEINIT;
    stepperMovementStatus[MOTOR_VALVE].status = STATUS_MOVEMENT_DEINIT;
    stepperMovementStatus[MOTOR_TABLE].transaction = TRANSACTION_DISABLED;
    stepperMovementStatus[MOTOR_PUMP].transaction = TRANSACTION_DISABLED;
    stepperMovementStatus[MOTOR_VALVE].transaction = TRANSACTION_DISABLED;        
    cSPIN_Hard_HiZ(Motor_ID);
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
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola    
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
                            6: Fotocellula Valvola Aperta
**                   'Filter': applicazione o meno del filtro in lettura Fotocellula (0 = NON applicato, 1 = applicato)
**
**      @retval stato della Fotocellula selezionata 'PhotoType' (0 = oscurata, 1 = NON oscurata)
**
*//*=====================================================================*//**
*/
unsigned char PhotocellStatus(unsigned short PhotoType, unsigned char Filter)
{
unsigned char ret = FALSE;
    switch (PhotoType)
    {
        case PHOTO_HOME: // 0: Fotocellula Home
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
        case PHOTO_ACC:  // 1: Fotocellula Accoppiamento
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
        case PHOTO_OPEN_EV: // 2: Fotocellula Valvola Home
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
        case PHOTO_EV: // 3: Fotocellula Tavola
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
        case PHOTO_VALVE_OPEN: // 4: Fotocellula Valvola Aperta
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType6 ? TRUE:FALSE;
            }
            else
            {
                ret = FO_GEN1;
            }
        }
        break; 
        case PHOTO_AUTOCAP_CLOSE: // 5: Fotocellula Autocap Chiuso
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType7 ? TRUE:FALSE;
            }
            else
            {
                ret = FO_GEN2;
            }
        }
        break;         
        case PHOTO_AUTOCAP_OPEN: // 6: Fotocellula Autocap Aperto
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType10 ? TRUE:FALSE;
            }
            else
            {
                ret = IO_GEN1;
            }
        }
        break;  
        case PHOTO_BRUSH: // 7: Fotocellula Spazzola
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType11 ? TRUE:FALSE;
            }
            else
            {
                ret = IO_GEN2;
            }
        }
        break;  
        case PHOTO_CAN_PRESENCE: // 8: Sensore Can Presence (Fotocellula o Ultrasuoni) 
        {
            if (Filter)
            {             
                ret =  OutputFilter.Bit.StatusType1 ? FALSE:TRUE;
            }
            else
            {
                ret = ~FO_CPR;
            }
        }
        break;
        case PHOTO_TABLE: // 9: Fotocellula Pannello Tavola   
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType9 ? TRUE:FALSE;
            }
            else
            {
                ret = INT_PAN;
            }
        }
        break;                
        case PHOTO_BASES_CARRIAGE: // 10: Carrello Basi
        {
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType8 ? TRUE:FALSE;
            }
            else
            {
                ret = INT_CAR;
            }
        }
        break;         
        case BUTTON_LPXC10: // 11: Pulsante LPXC10
            if (Filter)
            {
                ret =  OutputFilter.Bit.StatusType12 ? TRUE:FALSE;
            }
            else
            {
                ret = BUTTON;
            }            
        break;    
        default:
        {              
        }
        break;    
    }
    
    return ret;
}

void Read_All_Parameters(unsigned short Motor_ID)
/*
*//*=====================================================================*//**
**      @brief Read  All Parameters of input driver 
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
{
/*
    unsigned char i;    
    uint32_t spin_parameters[cSPIN_STATUS] = {0};
    
    //LEGGO TUTTI
    Nop();
    for ( i = 1; i <= cSPIN_STATUS ;i++)
    {
        if( (i != 0x0D) && (i != 0x11) && (i != 0x14) )  //salto i registri RESERVED
        {
            spin_parameters[i-1] = cSPIN_Get_Param(i, Motor_ID); 
        }
    }
    
    //valorizzo struttura         
      cSPIN_RegsStructRead.ABS_POS =  spin_parameters[cSPIN_ABS_POS-1];// cSPIN_Get_Param((uint8_t)cSPIN_ABS_POS);    
      cSPIN_RegsStructRead.EL_POS =  spin_parameters[cSPIN_EL_POS-1];//cSPIN_Get_Param((uint8_t)cSPIN_EL_POS);   
      cSPIN_RegsStructRead.MARK =  spin_parameters[cSPIN_MARK-1];//cSPIN_Get_Param((uint8_t)cSPIN_MARK); 
      cSPIN_RegsStructRead.SPEED =  spin_parameters[cSPIN_SPEED-1];//cSPIN_Get_Param((uint8_t)cSPIN_SPEED); 
      cSPIN_RegsStructRead.ACC = spin_parameters[cSPIN_ACC-1];//cSPIN_Get_Param((uint8_t)cSPIN_ACC);
      cSPIN_RegsStructRead.DEC = spin_parameters[cSPIN_DEC-1];//cSPIN_Get_Param((uint8_t)cSPIN_DEC);      
      cSPIN_RegsStructRead.MAX_SPEED = spin_parameters[cSPIN_MAX_SPEED-1];//cSPIN_Get_Param((uint8_t)cSPIN_MAX_SPEED); 
      cSPIN_RegsStructRead.MIN_SPEED = spin_parameters[cSPIN_MIN_SPEED-1];//cSPIN_Get_Param((uint8_t)cSPIN_MIN_SPEED); 
      cSPIN_RegsStructRead.FS_SPD = spin_parameters[cSPIN_FS_SPD-1];//cSPIN_Get_Param((uint8_t)cSPIN_FS_SPD); 
      cSPIN_RegsStructRead.TVAL_HOLD = spin_parameters[cSPIN_TVAL_HOLD-1];//cSPIN_Get_Param((uint8_t)cSPIN_TVAL_HOLD); 
      cSPIN_RegsStructRead.TVAL_RUN = spin_parameters[cSPIN_TVAL_RUN-1];//cSPIN_Get_Param((uint8_t)cSPIN_TVAL_RUN); 
      cSPIN_RegsStructRead.TVAL_ACC = spin_parameters[cSPIN_TVAL_ACC-1];//cSPIN_Get_Param((uint8_t)cSPIN_TVAL_ACC); 
      cSPIN_RegsStructRead.TVAL_DEC = spin_parameters[cSPIN_TVAL_DEC-1];//cSPIN_Get_Param((uint8_t)cSPIN_TVAL_DEC); 
      //cSPIN_RegsStruct.RESERVED_3 = cSPIN_Get_Param(cSPIN_RESERVED_3); 
      cSPIN_RegsStructRead.T_FAST = spin_parameters[cSPIN_T_FAST-1];//cSPIN_Get_Param((uint8_t)cSPIN_T_FAST); 
      cSPIN_RegsStructRead.TON_MIN = spin_parameters[cSPIN_TON_MIN-1];//cSPIN_Get_Param((uint8_t)cSPIN_TON_MIN); 
      cSPIN_RegsStructRead.TOFF_MIN = spin_parameters[cSPIN_TOFF_MIN-1];//cSPIN_Get_Param((uint8_t)cSPIN_TOFF_MIN); 
      //cSPIN_RegsStruct.RESERVED_2 = cSPIN_Get_Param(cSPIN_RESERVED_2); 
      cSPIN_RegsStructRead.ADC_OUT = spin_parameters[cSPIN_ADC_OUT-1];//cSPIN_Get_Param((uint8_t)cSPIN_ADC_OUT); 
      cSPIN_RegsStructRead.OCD_TH = spin_parameters[cSPIN_OCD_TH-1];//cSPIN_Get_Param((uint8_t)cSPIN_OCD_TH); 
      //cSPIN_RegsStruct.RESERVED_1 = spin_parameters[i-1];//cSPIN_Get_Param(cSPIN_RESERVED_1); 
      cSPIN_RegsStructRead.STEP_MODE = spin_parameters[cSPIN_STEP_MODE-1];//cSPIN_Get_Param((uint8_t)cSPIN_STEP_MODE); 
      cSPIN_RegsStructRead.ALARM_EN = spin_parameters[cSPIN_ALARM_EN-1];//cSPIN_Get_Param((uint8_t)cSPIN_ALARM_EN); 
      cSPIN_RegsStructRead.GATECFG1 = spin_parameters[cSPIN_GATECFG1-1];//cSPIN_Get_Param((uint8_t)cSPIN_GATECFG1); 
       cSPIN_RegsStructRead.GATECFG2 = spin_parameters[cSPIN_GATECFG2-1];//cSPIN_Get_Param((uint8_t)cSPIN_GATECFG2); 
      cSPIN_RegsStructRead.CONFIG = spin_parameters[cSPIN_CONFIG-1];//cSPIN_Get_Param((uint8_t)cSPIN_CONFIG); 
      cSPIN_RegsStructRead.STATUS = spin_parameters[cSPIN_STATUS-1];//cSPIN_Get_Param((uint8_t)cSPIN_STATUS);     
      Nop();
*/
}

void init_test_Stepper(unsigned short Motor_ID)
/*
*//*=====================================================================*//**
**      @brief Read  All Parameters
**
**      @param  input 'Motor_ID': tipo di Stepper
**                      0: Pompa
**                      1: Tavola Rotante
**                      2: Valvola    
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
{
/*    
    switch (Motor_ID)
    {
        case MOTOR_TABLE:
        {
            StatusBoardDriver.word = GetStatus(MOTOR_TABLE);
            Read_All_Parameters(Motor_ID);
            Nop();
            ConfigStepper(MOTOR_TABLE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);    
            StatusBoardDriver.word = GetStatus(MOTOR_TABLE);
            Read_All_Parameters(Motor_ID);             
            Nop();
            //SetStepperHomePosition(MOTOR_TABLE);
            //MoveStepper(MOTOR_TABLE,400,120);//N step = 400,speed = 120 rpm -> 1 giro intero  in 0.5 sec            
            //MoveStepper(MOTOR_TABLE,100,120);//N step = 100,speed = 120 rpm -> 1/4 giro intero  in 0.5 sec            
            //Run_Stepper(MOTOR_TABLE,60,FORWARD);
            
            //Transazione DARK -> LIGHT HOME_PHOTOCELL
             StartStepper(MOTOR_TABLE, 120, CW, DARK_LIGHT, HOME_PHOTOCELL, 0); 
            
            //Transazione LIGHT-> DARK HOME_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, HOME_PHOTOCELL, 0); 
            
            //Transazione DARK -> LIGHT  COUPLING_PHOTOCELL
             //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,COUPLING_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK COUPLING_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, COUPLING_PHOTOCELL, 0); 
            
             //Transazione DARK -> LIGHT  VALVE_PHOTOCELL
             //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,VALVE_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK VALVE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);             
            
            //Transazione DARK -> LIGHT  TABLE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,TABLE_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK TABLE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0); 
                                    
            //Durata 1000
            //StartStepper(MOTOR_TABLE,480,CW,0,0,1000);           //1 sec
        }
        break;
        case MOTOR_PUMP:
        {
            StatusBoardDriver.word = GetStatus(MOTOR_PUMP);
            Read_All_Parameters(Motor_ID);
            Nop();
            ConfigStepper(MOTOR_PUMP, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);    
            StatusBoardDriver.word = GetStatus(MOTOR_PUMP);
            Read_All_Parameters(Motor_ID);             
            Nop();
            //SetStepperHomePosition(MOTOR_PUMP);
            //MoveStepper(MOTOR_PUMP,400,120);//N step = 400,speed = 120 rpm -> 1 giro intero  in 0.5 sec            
            MoveStepper(MOTOR_PUMP,100,120);//N step = 100,speed = 120 rpm -> 1/4 giro intero  in 0.5 sec
            
            //Transazione DARK -> LIGHT HOME_PHOTOCELL
            // StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT, HOME_PHOTOCELL, 0); 
            
            //Transazione LIGHT-> DARK HOME_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, HOME_PHOTOCELL, 0); 
            
            //Transazione DARK -> LIGHT  COUPLING_PHOTOCELL
             //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,COUPLING_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK COUPLING_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, COUPLING_PHOTOCELL, 0); 
            
             //Transazione DARK -> LIGHT  VALVE_PHOTOCELL
             //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,VALVE_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK VALVE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);             
            
            //Transazione DARK -> LIGHT  TABLE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,TABLE_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK TABLE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0); 
                                    
            //Durata 1000
            //StartStepper(MOTOR_PUMP,480,CW,0,0,1000);           //1 sec
        }
   
        break;
        case MOTOR_VALVE:
        {
            StatusBoardDriver.word = GetStatus(MOTOR_VALVE);
            Read_All_Parameters(MOTOR_VALVE);
            Nop();
            ConfigStepper(MOTOR_VALVE, RESOLUTION_TABLE, RAMP_PHASE_CURRENT_TABLE, PHASE_CURRENT_TABLE, HOLDING_CURRENT_TABLE, ACC_RATE_TABLE, DEC_RATE_TABLE, ALARMS_TABLE);    
            StatusBoardDriver.word = GetStatus(MOTOR_VALVE);
            Read_All_Parameters(MOTOR_VALVE);             
            Nop();
            //SetStepperHomePosition(MOTOR_VALVE);
            //MoveStepper(MOTOR_VALVE,400,120);//N step = 400,speed = 120 rpm -> 1 giro intero  in 0.5 sec            
            MoveStepper(MOTOR_VALVE,100,120);//N step = 100,speed = 120 rpm -> 1/4 giro intero  in 0.5 sec 
            
            //Transazione DARK -> LIGHT HOME_PHOTOCELL
            // StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT, HOME_PHOTOCELL, 0); 
            
            //Transazione LIGHT-> DARK HOME_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, HOME_PHOTOCELL, 0); 
            
            //Transazione DARK -> LIGHT  COUPLING_PHOTOCELL
             //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,COUPLING_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK COUPLING_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, COUPLING_PHOTOCELL, 0); 
            
             //Transazione DARK -> LIGHT  VALVE_PHOTOCELL
             //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,VALVE_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK VALVE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, VALVE_PHOTOCELL, 0);             
            
            //Transazione DARK -> LIGHT  TABLE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, DARK_LIGHT,TABLE_PHOTOCELL , 0); 
            
            //Transazione LIGHT-> DARK TABLE_PHOTOCELL
            //StartStepper(MOTOR_TABLE, 480, CW, LIGHT_DARK, TABLE_PHOTOCELL, 0); 
                                                
            //StartStepper(MOTOR_VALVE,480,CW,0,0,1000);           //1 sec
        }
        break;
    }
*/
}

void test_Stepper(unsigned short Motor_ID)
/*
*//*=====================================================================*//**
**      @brief Read  All Parameters
**
**      @param nessuno
**
**      @retval nessuno
**
*//*=====================================================================*//**
*/
{
/*
    long ABS_POS = 0;
    unsigned short currentSpeed = 0;
    static unsigned char lastDirectionBoard = REV;
    static unsigned char lastDirectionPump = REV;
    static unsigned char lastDirectionValve = REV;
    
    switch (Motor_ID)
    {
        case MOTOR_TABLE:
        {
            Nop();
            StatusBoardDriver.word = GetStatus(MOTOR_TABLE);
            Read_All_Parameters(MOTOR_TABLE);
            ABS_POS = GetStepperPosition(MOTOR_TABLE);
            currentSpeed = GetStepperSpeed (MOTOR_TABLE);
            Nop();                                         
                          
             if (StatusBoardDriver.Bit.MOT_STATUS == 0x00)  //se ferma
                {
                if (lastDirectionBoard == FWD)
                    {
                    StartStepper(MOTOR_TABLE,480,REV,0,0,50);           //0.5 sec
                    lastDirectionBoard = REV;
                    }
                else
                    {
                    StartStepper(MOTOR_TABLE,480,FWD,0,0,50);           //0.5 sec
                    lastDirectionBoard = FWD;
                    }
                Nop();
                }
                
            //Test funzione Run_Stepper 
            //Run_Stepper(unsigned short Motor_ID, unsigned short Speed_RPM, unsigned char Direction)
            //Run_Stepper(MOTOR_TABLE,480,FORWARD);  //speed = 120 rpm -> 1 giro intero  in 0.25 sec   
 
            //MoveStepperToHome(MOTOR_TABLE,480);                                           
        }
        break;
        case MOTOR_PUMP:
        {
            Nop();
            StatusPumpDriver.word = GetStatus(MOTOR_PUMP);
            Read_All_Parameters(MOTOR_PUMP);
            ABS_POS = GetStepperPosition(MOTOR_PUMP);
            currentSpeed = GetStepperSpeed (MOTOR_PUMP);
            if (StatusPumpDriver.Bit.MOT_STATUS == 0x00)  //se ferma
                {
                if (lastDirectionPump == FWD)
                    {
                    StartStepper(MOTOR_PUMP,480,REV,0,0,50);           //0.5 sec
                    lastDirectionPump = REV;
                    }
                else
                    {
                    StartStepper(MOTOR_PUMP,480,FWD,0,0,50);           //0.5 sec
                    lastDirectionPump = FWD;
                    }
       
                }
            Nop();
        }
        break;
        case MOTOR_VALVE:
            Nop();
            StatusValveDriver.word = GetStatus(MOTOR_VALVE);
            Read_All_Parameters(MOTOR_VALVE);
            ABS_POS = GetStepperPosition(MOTOR_VALVE);
            currentSpeed = GetStepperSpeed (MOTOR_VALVE);
             if (StatusValveDriver.Bit.MOT_STATUS == 0x00)  //se ferma
                {
                if (lastDirectionValve == FWD)
                    {
                    StartStepper(MOTOR_VALVE,480,REV,0,0,50);           //0.5 sec
                    lastDirectionValve = REV;
                    }
                else
                    {
                    StartStepper(MOTOR_VALVE,480,FWD,0,0,50);           //0.5 sec
                    lastDirectionValve = FWD;
                    }
       
                }
            Nop();
        break;
    }
*/
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
                    case MOTOR_TABLE:
                    {
                    if (StatusTimer(T_START_STEPPER_MOTOR_TABLE)==T_ELAPSED)
                        {
                        StopTimer(T_START_STEPPER_MOTOR_TABLE);
                        StopStepper(MOTOR_TABLE);
                        }
                    }
                    break;
                    case MOTOR_PUMP:                        
                    {
                    if (StatusTimer(T_START_STEPPER_MOTOR_PUMP)==T_ELAPSED)
                        {
                        StopTimer(T_START_STEPPER_MOTOR_PUMP);
                        StopStepper(MOTOR_PUMP);
                        }    
                    }
                    break;
                    case MOTOR_VALVE:
                    {
                    if (StatusTimer(T_START_STEPPER_MOTOR_VALVE)==T_ELAPSED)
                        {
                        StopTimer(T_START_STEPPER_MOTOR_VALVE);
                        StopStepper(MOTOR_VALVE);
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
            case STATUS_MOVEMENT_WAIT_PHOTO_COUPLING:
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
            case STATUS_MOVEMENT_WAIT_PHOTO_TABLE:
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
            case STATUS_MOVEMENT_WAIT_PHOTO_VALVE:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_VALV  == LIGHT) //if cerco luce
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
            case STATUS_MOVEMENT_WAIT_PHOTO_CAN_PRESENCE:
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
            case STATUS_MOVEMENT_WAIT_PHOTO_PANEL_TABLE:
            {
                                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_GEN1 == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_GEN1 == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }
                
            }
            break;
            case STATUS_MOVEMENT_WAIT_PHOTO_BASES_CARRIAGE:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_GEN2 == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_GEN2 == DARK)  //if cerco buio
                    {
                       Nop();
                       StopStepper(motor);
                       stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                       stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }
                }
                
            }
            break;
            case STATUS_MOVEMENT_WAIT_PHOTO_OPEN_VALVE:
            {
                //DARK_LIGHT
                if (stepperMovementStatus[motor].transaction == DARK_LIGHT)
                {                    
                    if(FO_GEN1  == LIGHT) //if cerco luce
                    {
                        Nop();
                        StopStepper(motor);
                        stepperMovementStatus[motor].transaction = TRANSACTION_DISABLED;
                        stepperMovementStatus[motor].status = STATUS_MOVEMENT_DEINIT;
                    }                      
                }
                else if (stepperMovementStatus[motor].transaction == LIGHT_DARK)
                {                  
                    if(FO_GEN1 == DARK)  //if cerco buio
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