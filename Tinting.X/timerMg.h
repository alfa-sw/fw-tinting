/* 
 * File:   timerMg.h
 * Author: michele.abelli
 * Description : Inclusioni relative al modulo TIMERMG.C
 * Created on 16 luglio 2018, 14.48
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
   N_TIMERS
 };

/* 1 */ #define DELAY_READ_IO 10
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
// Waiting SHT31 Measurement: 8 msec
/* 11 */# define DELAY_SHT31_MEASUREMENT 4
// Timeout on SHT31 Write Command: 4 msec
/* 12 */# define DELAY_SHT31_TIMEOUT 2
// Wait 10" with HEATER ON
 /* 13*/# define DELAY_WAIT_HEATER 5000
// LED Duration ON 10"
/* 14 */# define DELAY_LED 3000
// Timeout on SPI TC72 Write Command: 200 msec
/* 15 */# define DELAY_SPI_MEASUREMENT 100	
// Waiting Temperature Sensor Reset Time: 200 msec
/* 16 *///# define DELAY_SPI_HARD_RESET 1	
/* 16 */# define DELAY_SPI_HARD_RESET 100	
// Waiting Time in Error Status: 100 msec
/* 17 */ # define DELAY_ERROR_STATUS 50	
// Waiting Time Before to Close valve: 4"
/* 18 */ # define DELAY_BEFORE_VALVE_CLOSE 2000	
// Ricirculation Pause. Waiting time betwen 2 stroke in opposite direction: 1"
/* 19 */ # define DELAY_PAUSE_RECIRC 500	
// Reset Timeout: 60"
/* 20 */ //# define DELAY_RESET 30000	
/* 20 */ # define DELAY_RESET 5000	
 // Delay Fault 1 Activation: 100msec
/* 21 */ # define DELAY_FAULT_1_ACTIVATION 50
// Delay Fault 1 Detection: 500msec
/* 22 */ # define DELAY_FAULT_1_DETECTION  250
// Delay Fault 1 Enabling: 100msec
/* 23 */ # define DELAY_FAULT_1_ENABLING 50 
// Timer Collaudo
/* 24 */ # define DELAY_COLLAUDO 100	

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

#endif	/* TIMERMG_H */

