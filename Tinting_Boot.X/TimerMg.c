/**/
/*============================================================================*/
/**
**      @file    timeMg.c
**
**      @brief   Timers Manager module
**/
/*============================================================================*/
/**/
#include "Compiler.h"
#include "TimerMg.h"
#include "macro.h"
#include "mem.h"
#include "p24FJ256GB110.h"

timerstype BL_TimStr[N_TIMERS];
unsigned short BL_Durata[N_TIMERS] = {
/*  0 */    DELAY_T_LED,
/*  1 */    DELAY_T_FILTER_KEY,
/*  2 */    DELAY_FORCE_STAND_ALONE,
/*  3 */    DELAY_INTRA_FRAMES,
/*  4 */    DELAY_RETRY_BROADCAST_MSG,
};

static volatile unsigned short BL_TimeBase;

// Timer Manager sequencer 
void BL_TimerMg(void)
{
    static unsigned short BL_MonTimeBase;
    unsigned char temp;
    // Mirror time base 
    _T2IE=0;
    BL_MonTimeBase = BL_TimeBase;
    _T2IE=1;

    for (temp = 0; temp < N_TIMERS; temp++) {
        if (BL_TimStr[temp].Flg == T_RUNNING) {
            if ((BL_MonTimeBase - BL_TimStr[temp].InitBase) >= BL_Durata[temp])
                BL_TimStr[temp].Flg = T_ELAPSED;
        }
        if (BL_TimStr[temp].Flg == T_STARTED) {
            BL_TimStr[temp].InitBase = BL_MonTimeBase;
            BL_TimStr[temp].Flg = T_RUNNING;
        }
    }
} // BL_TimerMg() 

// Timer1 initialization for TimerMg 
void BL_TimerInit (void)
{
    // Timer 2 controls position/speed controller sample time
	TMR2 = 0;  // Resetting TIMER
	// PR2 = SPEED_CONTROL_RATE_TIMER;
    // PR2 x PRESCALER (= 8) / FCY (=16MIPS) = 2msec
	PR2 = 4000; 			// with 16MIPS interrupt every 2 ms
	T2CON = 0x0000;         // Reset timer configuration
	T2CONbits.TCKPS = 1;    // 1 = 1:8 prescaler

//	IPC0bits.T1IP = 3;      // Set Timer 2 Interrupt Priority Level
	IFS0bits.T2IF = 0;      // Clear Timer1 Interrupt Flag
	IEC0bits.T2IE = 1;      // Enable Timer1 interrupt
	T2CONbits.TON = 1;      // Enable Timer1 with prescaler settings at 1:1 and
                            // clock source set to the internal instruction cycle    
}

// Timer 2 Interrupt handler 
void __attribute__((__interrupt__,auto_psv)) _AltT2Interrupt(void)
{
    IFS0bits.T2IF = 0;  // Clear Timer 2 Interrupt Flag
    ++ BL_TimeBase;
}