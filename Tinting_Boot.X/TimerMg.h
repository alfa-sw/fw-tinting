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
#define DELAY_T_LED                     500    /* 1 s */
#define DELAY_T_FILTER_KEY              10     /* 20 ms */
#define DELAY_FORCE_STAND_ALONE         1000   /* 2 s */
#define DELAY_INTRA_FRAMES              10     /* 2 ms */
#define DELAY_RETRY_BROADCAST_MSG       50     /* 100 ms */
#define DELAY_SEND_RESET_MSG            50     /* 100 ms */


#define StopTimer(Timer)    (BL_TimStr[Timer].Flg = STOP_TIMER)
#define StartTimer(Timer)   (BL_TimStr[Timer].Flg = START_TIMER)
#define StatusTimer(Timer)  (BL_TimStr[Timer].Flg)


/*===== TIPI ================================================================*/
typedef struct {
  signed char Flg;
  unsigned short InitBase;
} timerstype;

enum {
     T_LED,                      /*  0 */
     T_DEL_FILTER_KEY,           /*  1 */
     T_WAIT_FORCE_BL,            /*  2 */
     T_DELAY_INTRA_FRAMES,       /*  3 */
     T_RETRY_BROADCAST_MSG,      /*  4 */
     T_SEND_RESET_MSG,           /*  5 */
     N_TIMERS
};

extern timerstype BL_TimStr[N_TIMERS];
extern unsigned short BL_Durata[N_TIMERS];
extern void BL_TimerMg (void);
extern void BL_TimerInit (void);

#endif /* __TIMER_MG_H__ */
