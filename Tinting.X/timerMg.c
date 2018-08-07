/* 
 * File:   timerMg.c
 * Author: michele.abelli
 * Description  : Timer management
 * Created on 16 luglio 2018, 14.18
 */

/*===== INCLUSIONI ========================================================= */
#include "p24FJ256GB106.h"
#include "TimerMg.h"
#include "mem.h"
#include "typedef.h"

/*====== MACRO LOCALI ====================================================== */

/*====== TIPI LOCALI ======================================================== */

/*====== VARIABILI LOCALI =================================================== */
static unsigned short MonTimeBase;

/*====== VARIABILI GLOBALI =================================================== */
unsigned short TimeBase;

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
   /* 19 */DELAY_RESET,           
};

void InitTMR(void)
{
	unsigned char i;
	
	//Timer 1 controls position/speed controller sample time
	TMR1 = 0;  // Resetting TIMER
	// PR1 = SPEED_CONTROL_RATE_TIMER;
    // PR1 x PRESCALER (= 8) / FCY (=16MIPS) = 2msec
	PR1 = 4000; 			// with 16MIPS interrupt every 2 ms
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

unsigned char NotRunningTimer(unsigned char Timer)
{
	if (Timer>=N_TIMERS)
	{
		return 0;
	}

	return (TimStr[Timer].Flg == STOP_TIMER);
}

signed char StatusTimer(unsigned char Timer)
{
	if (Timer>=N_TIMERS)
	{
		return 0;
	}
	
	return TimStr[Timer].Flg;
}

//void __attribute__((__interrupt__,auto_psv)) _T1Interrupt(void)
void T1_InterruptHandler(void)
{
	IFS0bits.T1IF = 0;                          //Clear Timer 1 Interrupt Flag

  	++ TimeBase ;
}



