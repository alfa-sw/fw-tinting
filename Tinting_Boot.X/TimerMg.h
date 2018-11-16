/**/
/*============================================================================*/
/**
 **      @file    TimerMg.h
 **
 **      @brief
 **/
/*============================================================================*/
/**/
#ifndef __TIMER_MG_H__
#define __TIMER_MG_H__

#define BASE_T_TIMERMG 20

#define START_TIMER 1
#define STOP_TIMER  0
#define T_CLEAR_IF_ELAPSED 1
#define T_START_IF_ELAPSED 2
#define T_READ 0
#define T_ELAPSED       -1
#define T_RUNNING       2
#define T_HALTED        0
#define T_STARTED       1
#define T_NOT_RUNNING   -2

#define TIMER_SOTTOFASE               BL_TimStr[0].Flg
#define TIMER_DELAY_CREDITI           BL_TimStr[1].Flg
#define TIMER_LAMP_CREDITI_LIMITATI   BL_TimStr[2].Flg

#define SPEED_CONTROL_RATE          2.0             // rate of speed controller in ms
#define MIPS                        40             // do not change
#define SYSTEM_CLOCK                (0.000001/MIPS) // system clock in ns
#define SPEED_CONTROL_RATE_TIMER    (int)(0.001*SPEED_CONTROL_RATE/SYSTEM_CLOCK/8)

/* Timer delays (time base is 2 ms) */
#define DELAY_FORCE_STAND_ALONE         4500  /*   9 s  */
#define DELAY_INTRA_FRAMES                 2  /*   4 ms */
#define DELAY_T_FIRST_WINDOW              50  /* 100 ms */

#define StopTimer(Timer)    (BL_TimStr[Timer].Flg = STOP_TIMER)
#define StartTimer(Timer)   (BL_TimStr[Timer].Flg = START_TIMER)
#define StatusTimer(Timer)  (BL_TimStr[Timer].Flg)


/*===== TIPI ================================================================*/
typedef struct {
  signed char Flg;
  unsigned short InitBase;
} timerstype;

enum {
  T_WAIT_FORCE_BL,            /* 0 */
  T_DELAY_INTRA_FRAMES,       /* 1 */
  T_FIRST_WINDOW,             /* 2 */
  N_TIMERS
};

extern timerstype BL_TimStr[N_TIMERS];
extern unsigned short BL_Durata[N_TIMERS];
extern void BL_TimerMg (void);
extern void BL_TimerInit (void);

#endif /* __TIMER_MG_H__ */
