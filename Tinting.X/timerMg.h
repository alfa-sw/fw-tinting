/* 
 * File:   timerMg.h
 * Author: michele.abelli
 * Description : Inclusioni relative al modulo TIMERMG.C
 * Created on 16 luglio 2018, 14.48
 */

#ifndef TIMERMG_H
#define	TIMERMG_H

/**
 * Timebase is 0.2 ms
 * e.g. 1s = 5000 ticks
 */
//#define  T_BASE 2
#define  T_BASE 5

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
 * 1 sec = 1000 ms, 1 count each 0.2 ms ->1000/0.2 = 5000
 */
#define  CONV_SEC_COUNT  5000L

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
   /* 11 */T_SHT31_MEASUREMENT,
   /* 12 */T_SHT31_WRITE_TIMEOUT,
   /* 13 */T_SHT31_HEATER,
   /* 14 */T_LED_DURATION_ON,
   /* 15 */T_SPI_MEASUREMENT, 
   /* 16 */T_SPI_HARD_RESET,   
   /* 17 */T_ERROR_STATUS, 
   /* 18 */T_DELAY_BEFORE_VALVE_CLOSE, 
   /* 19 */T_PAUSE_RECIRC,   
   /* 20 */T_RESET, 
   /* 21 */T_DELAY_FAULT_1_ACTIVATION,
   /* 22 */T_DELAY_FAULT_1_DETECTION,
   /* 23 */T_DELAY_FAULT_1_ENABLING,
   /* 24 */T_COLLAUDO,
   /* 25 */T_TEST_SERIALE,
   /* 26 */T_START_STEPPER_MOTOR_TABLE,
   /* 27 */T_START_STEPPER_MOTOR_PUMP,
   /* 28 */T_START_STEPPER_MOTOR_VALVE,
   /* 29 */T_POLLING_STEPPER,
   /* 30 */T_DELAY_BEFORE_VALVE_BACKSTEP,
   /* 31 */T_WAIT_HOLDING_CURRENT_TABLE_FINAL,  
   /* 32 */T_DELAY_START_TABLE_MOTOR,
   /* 33 */T_MOTOR_WAITING_TIME,
   /* 34 */T_TABLE_WAITING_TIME,
   /* 35 */T_TABLE_WAIT_BEETWEN_MOVEMENT, 
   /* 36 */T_WAIT_DISPENSING,        
   /* 37 */T_WAIT_NEB_ERROR,     
   /* 38 */T_VALVE_WAITING_TIME,
   /* 39 */T_MEASURING_TIME,
   /* 40 */T_VALVE_MOVING_TIME,           
   /* 41 */T_WAIT_RELE_TIME,
   /* 42 */T_TIMEOUT_SPI3,
   /* 43 */T_TIMEOUT_STIRRING,
   /* 44 */T_FIRST_LINK_GUI_TIMER,
   /* 45 */T_GUI_TX_TIMER,
   /* 46 */T_GUI_RX_TIMER,    
   /* 47 */T_WAIT_READ_FW_VERSION,
   /* 48 */T_AUTOTEST_PAUSE,
   /* 49 */T_AUTOTEST_RICIRCULATION_TIME,
   /* 50 */T_AUTOTEST_STIRRING_TIME,
   /* 51 */T_AUTOCAP_HOMING,
   /* 52 */T_STANDBY_TIMEBASE,  
   /* 53 */T_STANDBY_RECIRC_STARTUP,
   /* 54 */T_STANDBY_STIRRING_STARTUP,  
   /* 55 */T_POWER_ON_TIMER,       
   /* 56 */T_RESET_TIMEOUT,
   /* 57 */T_WAIT_START_RESET,        
   /* 58 */T_WAIT_TABLE_POSITIONING, 
   /* 59 */T_SEND_PARAMETERS,
   /* 60 */T_DELAY_LASER,    
   /* 61 */T_DIAGNOSTIC_TIMEBASE,  
   /* 62 */T_HEARTBEAT, 
   /* 63 */T_WAIT_START_SUPPLY,          
   /* 64 */T_OUT_SUPPLY,        
   /* 65 */T_DELAY_WAIT_STOP,   
   /* 66 */T_ALARM_RECOVERY,
   /* 67 */T_DELAY_JUMP_TO_BOOT,    
   /* 68 */T_ALARM_AUTO_RESET,
   /* 69 */T_DELAY_BOOT_START, 
   /* 70 */T_FIRST_LINK_ACT_TIMER,
   /* 71 */T_SLAVE_WINDOW_TIMER,
   /* 72 */T_MOTOR_AUTOCAP_ON,
   /* 73 */T_TIMEOUT_AUTOCAP,     
   /* 74 */T_DELAY_INIT_DONE,  
   /* 75 */T_WAIT_GENERIC24V_TIME,  
   /* 76 */T_WAIT_BRUSH_ON,
   /* 77 */T_WAIT_BRUSH_PAUSE,     
   /* 78 */T_WAIT_AUTOTEST_HEATER, 
   /* 79 */T_TEST_RELE,   
   /* 80 */T_WAIT_END_TABLE_POSITIONING,
   /* 81 */T_WAIT_AIR_PUMP_TIME,        
   /* 82 */T_WAIT_STIRRING_ON,                   
   N_TIMERS
 };

/* 1 */ //#define DELAY_READ_IO 10
/* 1 */ #define DELAY_READ_IO 20
/* 1 */ //#define DELAY_READ_IO 10 
/* 2 */ #define DELAY_INTRA_FRAMES 50 // 10 msec
/* 3 */ #define DELAY_WAIT_SLAVE   30000 // 3sec
/* 4 */ #define DELAY_SLAVE_WAIT_LINK_TIMER  100000 // 10 sec
// Default Activation Duration with Autocap Open: 20"
/* 5 */ # define DELAY_HUM_CAP_OPEN_ON 100000	
// Base Timer Activation Period with Autocap Open: 1"
/* 6 */ # define DELAY_HUM_CAP_OPEN_PERIOD 5000
// Default Activation Duration with Autocap Closed: 5"
/* 7 */ # define DELAY_HUM_CAP_CLOSED_ON 25000
// Base Timer Activation Period with Autocap Closed: 1"
/* 8 */ # define DELAY_HUM_CAP_CLOSED_PERIOD 5000
// Base Timer Dosing Temperature Activation Period : 1"
/* 9 */	# define DELAY_DOS_PERIOD	5000			
// Waiting Reset Time: 2 msec
/* 10 */# define DELAY_HARD_RESET 10  
// Waiting SHT31 Measurement: 8 msec
/* 11 */# define DELAY_SHT31_MEASUREMENT 40
// Timeout on SHT31 Write Command: 4 msec
/* 12 */# define DELAY_SHT31_TIMEOUT 20
// Wait 10" with HEATER ON
 /* 13*/# define DELAY_WAIT_HEATER 50000
// LED Duration ON 10"
/* 14 */# define DELAY_LED 30000
// Timeout on SPI TC72 Write Command: 200 msec
/* 15 */# define DELAY_SPI_MEASUREMENT 1000	
// Waiting Temperature Sensor Reset Time: 200 msec
/* 16 *///# define DELAY_SPI_HARD_RESET 1	
/* 16 */# define DELAY_SPI_HARD_RESET 1000	
// Waiting Time in Error Status: 1000 msec
/* 17 */ # define DELAY_ERROR_STATUS 5000	
// Waiting Time Before to Close valve: 4"
/* 18 */ # define DELAY_BEFORE_VALVE_CLOSE 20000	
// Ricirculation Pause. Waiting time betwen 2 stroke in opposite direction: 1"
/* 19 */ # define DELAY_PAUSE_RECIRC 5000	
// Reset Timeout: 120"
/* 20 */ # define DELAY_RESET 600000	
 // Delay Fault 1 Activation: 100msec
/* 21 */ # define DELAY_FAULT_1_ACTIVATION 500
// Delay Fault 1 Detection: 500msec
/* 22 */ # define DELAY_FAULT_1_DETECTION  2500
// Delay Fault 1 Enabling: 100msec
/* 23 */ # define DELAY_FAULT_1_ENABLING 500 
// Timer Collaudo
/* 24 */ # define DELAY_COLLAUDO 1000	
// Timer Test Seriale
/* 25 */ # define DELAY_TEST_SERIALE 10000	
//Tempo base funzione movimentazione Stepper
/* 26 */ # define DELAY_DEFAULT_START_STEPPER_TABLE 5000 //1 sec	
//Tempo base funzione movimentazione Stepper
/* 27 */ # define DELAY_DEFAULT_START_STEPPER_PUMP 5000 //1 sec	
//Tempo base funzione movimentazione Stepper
/* 28 */ # define DELAY_DEFAULT_START_STEPPER_VALVE 5000 //1 sec	
//Tempo base funzione movimentazione Stepper
/* 29 */ # define DELAY_POLLING_STEPPER 10000 //2 sec	
// Attesa prima di aprire la valvola sul foro di Backstep nel dosaggio in Alta Risoluzione
/* 30 */ # define DELAY_BEFORE_VALVE_BACKSTEP 25000 // 5sec           
// Attesa con Holding Current massima nel motore della Tavola
/* 31 */ //# define WAIT_HOLDING_CURRENT_TABLE_FINAL 3000000 // 10min           
/* 31 */ # define WAIT_HOLDING_CURRENT_TABLE_FINAL 100000 // 20sec           
// Attesa prima della movimentazione del motore della Tavola
/* 32 */ # define DELAY_START_TABLE_MOTOR 500 // 100msec  
// Massimo tempo di attesa movimentazione motore Pompa
/* 33 */ # define MOTOR_WAITING_TIME 50000 // 10000msec  
// Massimo tempo di attesa movimentazione motore Tavola
/* 34 */ # define TABLE_WAITING_TIME 150000 // 30000msec  
// Tempo di Attesa dopo la fine di uma movimentazione della Tavola ad Alta Velocità (deve essere > 500msec sennò problemi))
/* 35 */# define TABLE_WAIT_BEETWEN_MOVEMENT 1000 // 200msec           
/* 36 */# define WAIT_DISPENSING 5000 // 1000msec           
// Tempo di attesa alla partenza prima di gestire eventuali errori del Nebulizzatore (ossia della Resistenza riscaldatore in PTC)
/* 37 */# define WAIT_NEB_ERROR 100000 // 20000msec  
// Massimo tempo di attesa movimentazione motore Valvola
/* 38 */# define VALVE_WAITING_TIME 25000 // 5000msec  
// Finestra temporale entro cui avviene la misura di durata delle funzioni implementate 
/* 39 */# define MEASURING_TIME 300000 // 60 sec   
// Massimo tempo di attesa movimentazione motore Valvola nella movimentazione con Fotocellule Scoperte
/* 40 *///# define VALVE_MOVING_TIME 7500 // 1500msec  
/* 40 */# define VALVE_MOVING_TIME 25000 // 5000msec  
// Tempo di attesa dopo attivazione/disattivazione Relè prima di controllarne il FAULT
/* 41 */# define WAIT_RELE_TIME 50000 // 10000msec  
// Timeout di Lettura / Scrittura SPI3 = Sensore di Temperatura
/* 42 */# define TIMEOUT_SPI3 2500 // 500msec  
// Timeout Stirring: se attivo oltre a questo intervallo viene spento
/* 43 */# define TIMEOUT_STIRRING 1500000 // 5min
/* 44 */# define DELAY_FIRST_LINK_GUI_TIMER  50000 // 10 sec
/* 45 */# define DELAY_GUI_TX_TIMER 1000 // 200 msec 
/* 46 */# define DELAY_GUI_RX_TIMER 5000 // 1000 msec 
/* 47 */# define WAIT_READ_FW_VERSION 75000 // 15 sec 
/* 48 */# define AUTOTEST_PAUSE 600000 // 120 sec
/* 49 */# define AUTOTEST_RICIRCULATION_TIME 300000 // 60 sec
/* 50 */# define AUTOTEST_STIRRING_TIME 300000 // 60 sec
/* 51 */# define DELAY_AUTOCAP_HOMING 150000 // 30 sec
/* 52 */# define DELAY_STANDBY_TIMEBASE 5000 // 1 sec     
/* 53 */# define DELAY_STANDBY_RECIRC_STARTUP 10000 // 2 sec
/* 54 */# define DELAY_STANDBY_STIRRING_STARTUP 10000 // 2 sec    
/* 55 */# define DELAY_POWER_ON_TIMER 7500  // 1.5 sec
/* 56 */# define DELAY_RESET_TIMEOUT 1500000 // 5 min 
/* 57 */# define DELAY_WAIT_START_RESET 10000 // 2 sec
/* 58 */# define WAIT_TABLE_POSITIONING 450000 // 90 sec 
/* 59 */# define SEND_PARAMETERS 10000 // 2 sec
/* 60 */# define DELAY_LASER 100000 // 20 sec 
/* 61 */# define DELAY_DIAGNOSTIC_TIMEBASE 5000 // 1 sec 
/* 62 */# define DELAY_HEARTBEAT 500 // 100 ms 
/* 63 */# define DELAY_WAIT_START_SUPPLY 300000 // 60 sec 
/* 64 */# define DELAY_TOUT_SUPPLY 1500000 // 5 min  
/* 65 */# define DELAY_WAIT_STOP 25000 // 5 sec 
/* 66 */# define DELAY_ALARM_RECOVERY 50000 // 10 sec
/* 67 */# define DELAY_JUMP_TO_BOOT 1000 // 200msec
/* 68 */# define DELAY_ALARM_AUTO_RESET 25000 // 5 sec 
/* 69 */# define DELAY_BOOT_START 5000 // 1 sec
/* 70 */# define DELAY_FIRST_LINK_ACT_TIMER 25000  // 5 sec 
/* 71 */# define DELAY_SLAVE_WINDOW_TIMER 500 // 100msec
/* 72 */# define DELAY_MOTOR_AUTOCAP_ON 500 // 100msec
/* 73 */# define TIMEOUT_AUTOCAP 20000 // 4 sec 
/* 74 */# define DELAY_INIT_DONE 1000 // 200msec
// Tempo di attesa dopo attivazione/disattivazione Uscita Generica 24V prima di controllarne il FAULT 
/* 75 */# define WAIT_GENERIC24V_TIME 50000 // 10000msec 
// Durata del ciclo di pulizia per ogni circuito
/* 76 */# define WAIT_BRUSH_ON 25000 // 5000msec 
// Tempo di base per il conteggio della Pausa del ciclo di Pulizia
#ifndef CLEANING_AFTER_DISPENSING    
/* 77 */# define WAIT_BRUSH_PAUSE 300000 // 60 sec 
#else
/* 77 */# define WAIT_BRUSH_PAUSE 25000  // 5 sec  
#endif
// Durata attivazione Processo Heater con Riscaldatore PTC acceso in AUTOTEST
/* 78 */# define WAIT_AUTOTEST_HEATER 1500000 // 5 min  
// Durata attivazione Test Rele al Reset
/* 79 */# define TEST_RELE 25000 // 5sec  
// Attesa stabilizzazione Tavola alla fine del Posizionamento 
/* 80 */# define WAIT_END_TABLE_POSITIONING 1000 // 200 msec 
// Tempo di attesa dopo attivazione/disattivazione Pompa Aria (= Stirring) prima di controllarne il FAULT
/* 81 *///# define WAIT_AIR_PUMP_TIME 50000 // 100000msec 
/* 81 */# define WAIT_AIR_PUMP_TIME 25000 // 5000msec 
// Tempo di attesa dall'accensione dello Stirring con PWM basso prima di impostarlo al 100% 
/* 82 */# define WAIT_STIRRING_ON 2500 // 500msec
 /**
 * Timers Type
 */
enum {
  /*  0 */ TIMER_OUT_SUPPLY = 0,
};
 
typedef struct {
  signed char Flg;
  unsigned long InitBase;
} timerstype;

extern unsigned long TimeBase;
extern timerstype TimStr[N_TIMERS];
extern unsigned long Durata[N_TIMERS];
extern void TimerMg (void);
extern unsigned short ReadTimer(unsigned char timer);
extern void StartTimer(unsigned char Timer);
extern void StopTimer(unsigned char Timer);
extern void NotRunningTimer(unsigned char Timer);
extern signed char StatusTimer(unsigned char Timer);
extern void InitTMR(void);
extern void T1_InterruptHandler(void);
extern void SetStartStepperTime(unsigned long time, unsigned short Motor_ID);
extern void readIn(void);

#endif	/* TIMERMG_H */

