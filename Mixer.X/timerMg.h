/* 
 * File:   timerMg.h
 * Author: michele.abelli
 * Description : Inclusioni relative al modulo TIMERMG.C
 * Created on 12 marzo 2020, 12.35
 */

#ifndef TIMERMG_H
#define	TIMERMG_H

/**
 * Timebase is 2 ms
 * e.g. 1s = 500 ticks
 */
#define  T_BASE 2

/**
 * Timers management
 */
#define NOT_RUNNING -2
#define START_TIMER 1
#define STOP_TIMER  0
#define T_CLEAR_IF_ELAPSED 1
#define T_START_IF_ELAPSED 2
#define T_READ 0
#define T_ELAPSED      -1
#define T_RUNNING       2
#define T_HALTED        0
#define T_STARTED       1
#define T_NOT_RUNNING  -2

/**
 * Conversion to seconds
 * 1 sec = 1000 ms, 1 count each 2 ms ->1000/2 = 500
 */
#define  CONV_SEC_COUNT  500L

/**
 * Timers
 */
 enum {
   /* 1 */ T_READ_IO,
   /* 2 */ T_DELAY_INTRA_FRAMES,
   /* 3 */ T_SLAVE_WAIT_TIMER,	
   /* 4 */ T_SLAVE_WAIT_LINK_TIMER,
   /* 5 */ T_HUM_CAP_OPEN_ON,
   /* 6 */ T_HUM_CAP_OPEN_PERIOD,	
   /* 7 */ T_HUM_CAP_CLOSED_ON,	
   /* 8 */ T_HUM_CAP_CLOSED_PERIOD,
   /* 9 */ T_DOS_PERIOD,
   /* 10 */T_HARD_RESET,
   /* 11 */T_LED_DURATION_ON,
   /* 12 */T_SPI_MEASUREMENT, 
   /* 13 */T_SPI_HARD_RESET,   
   /* 14 */T_ERROR_STATUS, 
   /* 15 */T_RESET, 
   /* 16 */T_DELAY_FAULT_1_ACTIVATION,
   /* 17 */T_DELAY_FAULT_1_DETECTION,
   /* 18 */T_DELAY_FAULT_1_ENABLING,
   /* 19 */T_COLLAUDO,
   /* 20 */T_START_STEPPER_MOTOR_MIXER,
   /* 21 */T_POLLING_STEPPER,
   /* 22 */T_MOTOR_WAITING_TIME,
   /* 23 */T_WAIT_NEB_ERROR,     
   /* 24 */T_MEASURING_TIME,
   /* 25 */T_WAIT_RELE_TIME,
   /* 26 */T_TIMEOUT_SPI3,
   /* 27 */T_WAIT_SPI3_COMMAND,
   /* 28 */T_WAIT_MIXER_TIME,                 
   /* 29 */T_START_STEPPER_MOTOR_DOOR,
   /* 30 */T_WAIT_DOOR_OPEN,           
   /* 31 */T_WAIT_DOOR_OPENING, 
   /* 32 */T_HEARTBEAT,
   /* 33 */T_WAIT_AIR_PUMP_TIME, 
   /* 34 */T_START_STEPPER_MOTOR_AUTOCAP,
   /* 35 */T_WAIT_AUTOCAP_OPENING,
   /* 36 */T_CAN_LIFTER_RESET,
   /* 37 */T_DELAY_INIT_DONE,
   /* 38 */T_CAN_LIFTER_OPERATION,
   N_TIMERS
 };

/* 1 */ //#define DELAY_READ_IO 10
/* 1 */ #define DELAY_READ_IO 2
/* 1 */ //#define DELAY_READ_IO 1 
/* 2 */ #define DELAY_INTRA_FRAMES 2
/* 3 */ #define DELAY_WAIT_SLAVE   3000 // 3sec
/* 4 */ #define DELAY_SLAVE_WAIT_LINK_TIMER  10000 // 10 sec
// Default Activation Duration with Autocap Open: 20"
/* 5 */ # define DELAY_HUM_CAP_OPEN_ON 10000	
// Base Timer Activation Period with Autocap Open: 1"
/* 6 */ # define DELAY_HUM_CAP_OPEN_PERIOD 500
// Default Activation Duration with Autocap Closed: 5"
/* 7 */ # define DELAY_HUM_CAP_CLOSED_ON 2500
// Base Timer Activation Period with Autocap Closed: 1"
/* 8 */ # define DELAY_HUM_CAP_CLOSED_PERIOD 500
// Base Timer Dosing Temperature Activation Period : 1"
/* 9 */	# define DELAY_DOS_PERIOD	500			
// Waiting Reset Time: 2 msec
/* 10 */# define DELAY_HARD_RESET 1  
// LED Duration ON 10"
/* 11 */# define DELAY_LED 3000
// Timeout on SPI TC72 Write Command: 200 msec
/* 12 */# define DELAY_SPI_MEASUREMENT 100	
// Waiting Temperature Sensor Reset Time: 200 msec
/* 13 *///# define DELAY_SPI_HARD_RESET 1	
/* 13 */# define DELAY_SPI_HARD_RESET 100	
// Waiting Time in Error Status: 100 msec
/* 14 */ # define DELAY_ERROR_STATUS 50	
// Reset Timeout: 30"
/* 15 */ # define DELAY_RESET 15000	
 // Delay Fault 1 Activation: 100msec
/* 16 */ # define DELAY_FAULT_1_ACTIVATION 50
// Delay Fault 1 Detection: 500msec
/* 17 */ # define DELAY_FAULT_1_DETECTION  250
// Delay Fault 1 Enabling: 100msec
/* 18 */ # define DELAY_FAULT_1_ENABLING 50 
// Timer Collaudo
/* 19 */ # define DELAY_COLLAUDO 100	
//Tempo base funzione movimentazione Stepper Mixer
/* 20 */ # define DELAY_DEFAULT_START_STEPPER_MIXER 500 //1 sec	
//Tempo base funzione movimentazione Stepper
/* 21 */ # define DELAY_POLLING_STEPPER 1000 //2 sec	
// Massimo tempo di attesa oscuramento Fotocellula HOME
/* 22 */ # define MOTOR_WAITING_TIME 2500 // 5000msec  
// Tempo di attesa alla partenza prima di gestire eventuali errori del Nebulizzatore (ossia della Resistenza riscaldatore in PTC)
/* 23 */# define WAIT_NEB_ERROR 10000 // 20000msec  
// Finestra temporale entro cui avviene la misura di durata delle funzioni implementate 
/* 24 */# define MEASURING_TIME 30000 // 60 sec   
// Tempo di attesa dopo attivazione/disattivazione Relè prima di controllarne il FAULT
/* 25 */# define WAIT_RELE_TIME 5000 // 100000msec  
// Timeout di Lettura / Scrittura SPI3 = Sensore di Temperatura
/* 26 */# define TIMEOUT_SPI3 250 // 500msec  
// Tempo di attesa lettura Byte dalla SPI3 
/* 27 */# define WAIT_SPI3_COMMAND 5000 // 10000msec 
/* 28 */ #define WAIT_MIXER_TIME 500 // Attesa di 1 sec
//Tempo base funzione movimentazione Stepper Door
/* 29 */ # define DELAY_DEFAULT_START_STEPPER_DOOR 500 //1 sec	
//Tempo massimo di Porta Aperta
/* 30 */ # define WAIT_DOOR_OPEN 5000 // 10sec
//Timeout Apertura Porta
/* 31 */ # define WAIT_DOOR_OPENING 6000 // 12sec
/* 32 */ # define DELAY_HEARTBEAT 50 // 100 ms 
/* 33 */ # define WAIT_AIR_PUMP_TIME 100 // 200msec 
//Tempo base funzione movimentazione Stepper Autocap
/* 34 */ # define DELAY_DEFAULT_START_STEPPER_MOTOR_AUTOCAP 500 //1sec 
//Timeout Apertura Autocap
/* 35 */ # define WAIT_AUTOCAP_OPENING 3500 // 7sec  
/* 36 */ # define DELAY_CAN_LIFTER_RESET 5000 // 10 sec 
/* 37 */ # define DELAY_INIT_DONE 100 // 200msec
/* 38 */ # define DELAY_CAN_LIFTER_OPERATION 500 // 1 sec

typedef struct {
  signed char Flg;
  unsigned short InitBase;
} timerstype;

extern unsigned short TimeBase;
extern timerstype TimStr[N_TIMERS];
extern unsigned long Durata[N_TIMERS];
extern void TimerMg (void);
extern unsigned short ReadTimer(unsigned char timer);
extern void StartTimer(unsigned char Timer);
extern void StopTimer(unsigned char Timer);
extern unsigned char NotRunningTimer(unsigned char Timer);
extern signed char StatusTimer(unsigned char Timer);
extern void InitTMR(void);
extern void T1_InterruptHandler(void);
extern void SetStartStepperTime(unsigned long time, unsigned short Motor_ID);
extern void readIn(void);

#endif	/* TIMERMG_H */

