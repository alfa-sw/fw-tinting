/* 
 * File:   timerMg.c
 * Author: michele.abelli
 * Description  : Timer management 
 * Created on 16 luglio 2018, 14.18
 */

/*===== INCLUSIONI ========================================================= */
#include "p24FJ256GB110.h"
#include "TimerMg.h"
#include "mem.h"
#include "typedef.h"
#include "define.h"
#include "humidifierManager.h"
#include "ram.h"
#include "gestIO.h"
#include "stepper.h"
#include "stepperParameters.h"
#include "spi.h"
#include "L6482H.h"


/*====== MACRO LOCALI ====================================================== */

/*====== TIPI LOCALI ======================================================== */

/*====== VARIABILI LOCALI =================================================== */
static unsigned long MonTimeBase;

/*====== VARIABILI GLOBALI =================================================== */
unsigned long TimeBase;

timerstype TimStr[N_TIMERS];

unsigned long Durata[N_TIMERS] = {
   /* 1 */ DELAY_READ_IO,
   /* 2 */ DELAY_INTRA_FRAMES,
   /* 3 */ DELAY_WAIT_SLAVE,	
   /* 4 */ DELAY_SLAVE_WAIT_LINK_TIMER,
   /* 5 */ DELAY_HUM_CAP_OPEN_ON,
   /* 6 */ DELAY_HUM_CAP_OPEN_PERIOD,	
   /* 7 */ DELAY_HUM_CAP_CLOSED_ON,	
   /* 8 */ DELAY_HUM_CAP_CLOSED_PERIOD,
   /* 9 */ DELAY_DOS_PERIOD,
   /* 10 */DELAY_HARD_RESET,
   /* 11 */DELAY_SHT31_MEASUREMENT,
   /* 12 */DELAY_SHT31_TIMEOUT,
   /* 13 */DELAY_WAIT_HEATER,
   /* 14 */DELAY_LED,  
   /* 15 */DELAY_SPI_MEASUREMENT, 
   /* 16 */DELAY_SPI_HARD_RESET,	
   /* 17 */DELAY_ERROR_STATUS,	
   /* 18 */DELAY_BEFORE_VALVE_CLOSE,
   /* 19 */DELAY_PAUSE_RECIRC,           
   /* 20 */DELAY_RESET,  
   /* 21 */DELAY_FAULT_1_ACTIVATION,
   /* 22 */DELAY_FAULT_1_DETECTION, 
   /* 23 */DELAY_FAULT_1_ENABLING,
   /* 24 */DELAY_COLLAUDO,
   /* 25 */DELAY_TEST_SERIALE,
   /* 26 */DELAY_DEFAULT_START_STEPPER_TABLE,
   /* 27 */DELAY_DEFAULT_START_STEPPER_PUMP,
   /* 28 */DELAY_DEFAULT_START_STEPPER_VALVE,
   /* 29 */DELAY_POLLING_STEPPER,
   /* 30 */DELAY_BEFORE_VALVE_BACKSTEP,  
   /* 31 */WAIT_HOLDING_CURRENT_TABLE_FINAL,                       
   /* 32 */DELAY_START_TABLE_MOTOR,
   /* 33 */MOTOR_WAITING_TIME,  
   /* 34 */TABLE_WAITING_TIME,
   /* 35 */TABLE_WAIT_BEETWEN_MOVEMENT,  
   /* 36 */WAIT_DISPENSING, 
   /* 37 */WAIT_NEB_ERROR,            
   /* 38 */VALVE_WAITING_TIME,
   /* 39 */MEASURING_TIME,
   /* 40 */VALVE_MOVING_TIME,
   /* 41 */WAIT_RELE_TIME,
   /* 42 */TIMEOUT_SPI3, 
   /* 43 */TIMEOUT_STIRRING,
   /* 44 */DELAY_FIRST_LINK_GUI_TIMER,  
   /* 45 */DELAY_GUI_TX_TIMER, 
   /* 46 */DELAY_GUI_RX_TIMER,  
   /* 47 */WAIT_READ_FW_VERSION, 
   /* 48 */AUTOTEST_PAUSE,
   /* 49 */AUTOTEST_RICIRCULATION_TIME,
   /* 50 */AUTOTEST_STIRRING_TIME, 
   /* 51 */DELAY_AUTOCAP_HOMING, 
   /* 52 */DELAY_STANDBY_TIMEBASE, 
   /* 53 */DELAY_STANDBY_RECIRC_STARTUP,
   /* 54 */DELAY_STANDBY_STIRRING_STARTUP,    
   /* 55 */DELAY_POWER_ON_TIMER,  
   /* 56 */DELAY_RESET_TIMEOUT, 
   /* 57 */DELAY_WAIT_START_RESET,
   /* 58 */WAIT_TABLE_POSITIONING,  
   /* 59 */SEND_PARAMETERS, 
   /* 60 */DELAY_LASER,
   /* 61 */DELAY_DIAGNOSTIC_TIMEBASE,
   /* 62 */DELAY_HEARTBEAT, 
   /* 63 */DELAY_WAIT_START_SUPPLY, 
   /* 64 */DELAY_TOUT_SUPPLY,  
   /* 65 */DELAY_WAIT_STOP,
   /* 66 */DELAY_ALARM_RECOVERY,
   /* 67 */DELAY_JUMP_TO_BOOT,
   /* 68 */DELAY_ALARM_AUTO_RESET,
   /* 69 */DELAY_BOOT_START,
   /* 70 */DELAY_FIRST_LINK_ACT_TIMER,
   /* 71 */DELAY_SLAVE_WINDOW_TIMER, 
   /* 72 */DELAY_MOTOR_AUTOCAP_ON,   
   /* 73 */TIMEOUT_AUTOCAP,    
   /* 74 */DELAY_INIT_DONE, 
   /* 75 */WAIT_GENERIC24V_TIME,     
   /* 76 */WAIT_BRUSH_ON,    
   /* 77 */WAIT_BRUSH_PAUSE,   
   /* 78 */WAIT_AUTOTEST_HEATER,   
   /* 79 */TEST_RELE,     
   /* 80 */WAIT_END_TABLE_POSITIONING,
   /* 81 */WAIT_AIR_PUMP_TIME, 
   /* 82 */WAIT_STIRRING_ON, 
   /* 83 */WAIT_SPI3_COMMAND,
   /* 84 */WAIT_BRUSH_ACTIVATION,
};

void InitTMR(void)

{
	unsigned char i;
	
	//Timer 1 controls position/speed controller sample time
	TMR1 = 0;  // Resetting TIMER
	// PR1 = SPEED_CONTROL_RATE_TIMER;
    // PR1 x PRESCALER (= 8) / FCY (=16MIPS) = 2msec
//	PR1 = 4000; 			// with 16MIPS interrupt every 2 ms
	PR1 = 400; 			// with 16MIPS interrupt every 0.2 ms

	T1CON = 0x0000;         // Reset timer configuration
	T1CONbits.TCKPS = 1;    // 1 = 1:8 prescaler

	IPC0bits.T1IP = 3;      // Set Timer 1 Interrupt Priority Level
	IFS0bits.T1IF = 0;      // Clear Timer1 Interrupt Flag
	IEC0bits.T1IE = 1;      // Enable Timer1 interrupt
	T1CONbits.TON = 1;      // Enable Timer1 with prescaler settings at 1:1 and
                             //clock source set to the internal instruction cycle

	for (i=0;i<N_TIMERS;i++)
	{
		TimStr[i].Flg=T_HALTED;
	}
}

void TimerMg(void)
/*
*//*=====================================================================*//**
**
**      @brief Sequencer of the  TIMERMG  module
**
**      @param void
**
**      @retval void
**
**
*//*=====================================================================*//**
*/
{
  unsigned char temp;

  MonTimeBase = TimeBase;

  for (temp = 0; temp < N_TIMERS; temp++)
  {
    if (TimStr[temp].Flg == T_RUNNING)
    {
      if ((MonTimeBase - TimStr[temp].InitBase) >= Durata[temp])
      {
        TimStr[temp].Flg = T_ELAPSED;
      }
    }

    if (TimStr[temp].Flg == T_STARTED)
    {
      TimStr[temp].InitBase = MonTimeBase;
      TimStr[temp].Flg = T_RUNNING;
    }

  }

}/*end TimerMg*/


unsigned short ReadTimer (unsigned char timer)
/*
*//*=====================================================================*//**
**
**      @brief Returns the time elapsed from start timer
**
**
**      @param timer timer identifier (from 0 to N_TIMERS-1)
**
**      @retval time total elapsed
**
**
*//*=====================================================================*//**
*/
{
  unsigned short TimeTot;
  TimeTot = (unsigned short)(TimeBase - TimStr[timer].InitBase);
  return (TimeTot);
} /* end ReadTimer */

void StartTimer(unsigned char Timer)
{
	if (Timer>=N_TIMERS)
	{
		return;
	}
	
    TimStr[Timer].Flg = START_TIMER;
}

void StopTimer(unsigned char Timer)
{
	if (Timer>=N_TIMERS)
	{
		return;
	}
	
    TimStr[Timer].Flg = STOP_TIMER;
}

void NotRunningTimer(unsigned char Timer)
{
	if (Timer>=N_TIMERS)
	{
		return;
	}

	TimStr[Timer].Flg = NOT_RUNNING;
}

signed char StatusTimer(unsigned char Timer)
{
    
	if (Timer>=N_TIMERS)
	{
		return 0;
	}
	
	return TimStr[Timer].Flg;
}

void T1_InterruptHandler(void)
{
    static unsigned char stirr_buffer_indx = 0;
    static unsigned char count_timer = 0;
    
    IFS0bits.T1IF = 0; // Clear Timer 1 Interrupt Flag
  	++ TimeBase ; 
    if (TintingHumidifier.Humdifier_Type == HUMIDIFIER_TYPE_2) {
        contaDuty++;
        if (contaDuty >= 50)
            contaDuty = 0;

        if (contaDuty < dutyPWM)
            NEBULIZER_ON();        
        else 
            NEBULIZER_OFF();        
    }
    // Read Stirring Fault "AIR_PUMP_F" and Cleaning Fault "OUT_24V_FAULT" Input every 0.2 * 4 msec = 0.8msec --> Filter duration 80msec
    count_timer++;
    if (count_timer == 4)
        count_timer = 0;    
    if ( (read_buffer_stirr == ON) && (count_timer == 0) ) {
        BufferStirring[stirr_buffer_indx] = AIR_PUMP_F;
        BufferCleaning[stirr_buffer_indx] = OUT_24V_FAULT;
        if (stirr_buffer_indx == STIRRING_BUFFER_DEPTH)
            stirr_buffer_indx = 0;
        else 
            stirr_buffer_indx++;
    }
/*    
    if (Start_High_Res == 1) {
        if (FO_ACC == DARK) {
            Start_High_Res = 0;

            StopStepper(MOTOR_PUMP);
            SetStepperHomePosition(MOTOR_PUMP);
        }         
    }
*/
}

void SetStartStepperTime(unsigned long time, unsigned short Motor_ID)
{	  	  
    switch (Motor_ID)
    {
        case MOTOR_TABLE:
             Durata[T_START_STEPPER_MOTOR_TABLE] =  time * T_BASE;
        break;
        case MOTOR_PUMP:
             Durata[T_START_STEPPER_MOTOR_PUMP] =  time * T_BASE;
        break;
        case MOTOR_VALVE:
             Durata[T_START_STEPPER_MOTOR_VALVE] =  time * T_BASE;
        break;
    }        
}
