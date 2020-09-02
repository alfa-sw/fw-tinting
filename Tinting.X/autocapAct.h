/* 
 * File:   autocapAct.h
 * Author: michele.abelli
 *
 * Created on 20 maggio 2019, 12.54
 */

#ifndef _AUTOCAPACT_H_
#define _AUTOCAPACT_H_

#include "gestIO.h"
/**
 * commands
 */
#define CMD_IDLE    0x00
#define CMD_STOP    0x01
#define CMD_HOME    0x02
#define CMD_OPEN    0x04
#define CMD_CLOSE   0x08
#define CMD_PACK    0x10
#define CMD_EXTEND  0x20
#define CMD_RETRACT 0x40
#define CMD_INTR    0x80

// Autocap Status
#define TINTING_AUTOCAP_CLOSED	0
#define TINTING_AUTOCAP_OPEN	1
#define TINTING_AUTOCAP_ERROR	2


/**
 * Actuator FSM
 */
enum {
  /* 0  */ AUTOCAP_INIT_ST,
  /* 1  */ AUTOCAP_READY_ST,
  /* 2  */ AUTOCAP_SEARCH_PACKING_ST,
  /* 3  */ AUTOCAP_PACKED_ST,
  /* 4  */ AUTOCAP_SEARCH_HOMING_ST,
  /* 5  */ AUTOCAP_CLOSE_ST,
  /* 6  */ AUTOCAP_OPEN_RUN_ST,
  /* 7  */ AUTOCAP_OPEN_ST,
  /* 8  */ AUTOCAP_CLOSE_RUN_ST,
  /* 9  */ AUTOCAP_SEARCH_PACKING_CLOSED_ST,
  /* 10 */ AUTOCAP_PACKED_CLOSED_ST,
  /* 11 */ AUTOCAP_EXTEND_RUN_ST,
  /* 12 */ AUTOCAP_EXTEND_ST,
  /* 13 */ AUTOCAP_RETRACT_RUN_ST,

  /** Errors */
  /* 14 */ AUTOCAP_TOUT_ERROR_ST,
  /* 15 */ AUTOCAP_SOFTWARE_ERROR_ST,
  /* 16 */ AUTOCAP_HOME_POS_ERROR_ST,
  /* 17 */ AUTOCAP_HOMING_ERROR_ST,
  /* 18 */ AUTOCAP_PACKING_ERROR_ST,
  /* 19 */ AUTOCAP_LIFTER_ERROR_ST,
  /* 20 */ AUTOCAP_OVERCURRENT_ERROR_ST,
  /* 21 */ AUTOCAP_DRV_OVER_CURR_TEMP_ERROR_ST,
  /* 22 */ AUTOCAP_OPEN_LOAD_ERROR_ST,
  /* 23 */ AUTOCAP_JUMP_TO_BOOT,  
  /* 24 */   
  /* 25 */     
  /* 26 */ HUMIDIFIER_10_VALID_PARAM_RECEIVED_ST = 26,
  /* 27 */ HUMIDIFIER_10_PARAM_ERROR_ST = 27,
  };
  
/**
 * constants
 */
#define AUTOCAP_SPEED_RPM   10
#define AUTOCAP_STEP_OPEN   100
#define AUTOCAP_STEP_HOME   100 

/**
 * Function-like macros (statements)
 */   
#define isAutocapCmd(y)                         \
  (autocapAct.command.cmd == y)

#ifndef AUTOCAP_MMT  
    #define isAutocapActEnabled()               \
      (isSlaveCircuitEn(AUTOCAP_ID-1))
#else
    #define isAutocapActEnabled()               \
      (autocap_enabled)
#endif

#define posHomingAutocapAct()                                   \
  do {                                                          \
    setAutocapActMessage(POS_HOMING);                           \
    autocapAct.command.cmd = CMD_HOME;                          \
    autocapAct.n_step_home = AUTOCAP_STEP_HOME;                 \
  } while (0)

#define intrAutocapAct()                                        \
  do {                                                          \
    setAutocapActMessage(CONTROLLO_PRESENZA);                   \
    autocapAct.command.cmd = CMD_INTR;                          \
  } while (0)

#define stopAutocapAct()                                        \
  do {                                                          \
    setAutocapActMessage(CONTROLLO_PRESENZA);                   \
    autocapAct.command.cmd = CMD_STOP;                          \
  } while (0)

#ifndef AUTOCAP_MMT
    #define openAutocapAct()                                        \
      do {                                                          \
        setAutocapActMessage(POSIZIONA_AUTOCAP);                    \
        autocapAct.command.cmd = CMD_OPEN;                          \
                                                                    \
        autocapAct.n_step_move = AUTOCAP_STEP_OPEN;                 \
        autocapAct.speed_move = AUTOCAP_SPEED_RPM;                  \
      } while (0)
#else          
    #define openAutocapAct()                                        \
      do {                                                          \
        autocapAct.command.open = TRUE;                             \
      } while (0)                                                  
#endif                                                         

#ifndef AUTOCAP_MMT
#define closeAutocapAct()                                       \
  do {                                                          \
    setAutocapActMessage(POSIZIONA_AUTOCAP);                    \
    autocapAct.command.cmd = CMD_CLOSE;                         \
                                                                \
    autocapAct.n_step_move = AUTOCAP_STEP_HOME;                 \
    autocapAct.speed_move = AUTOCAP_SPEED_RPM;                  \
  } while (0)
#else          
    #define closeAutocapAct()                                   \
      do {                                                      \
        autocapAct.command.close = TRUE;                        \
      } while (0)                                                  
#endif                                                         

#define JumpToBoot_AutocapAct()	 	            		            \
	do {                                                            \
		setAutocapActMessage(JUMP_TO_BOOT); 	        	     	\
		autocapAct.command.cmd = CMD_IDLE;              		    \
	} while (0)

#define posHomingAutocapMMT()                                   \
  do {                                                          \
    autocapAct.command.cmd = CMD_HOME;                          \
    autocapAct.autocapFlags.homing = TRUE;                      \
  } while (0)
    
#define isAutocapAct_jumptoboot()               \
	(autocapAct.autocapFlags.jump_to_boot)	
	  
#define isAutocapActReady()                     \
  (autocapAct.autocapFlags.ready)

#define isAutocapActRunning()                   \
  (autocapAct.autocapFlags.running)

/* opening if run is TRUE, completed otherwise */
#define isAutocapActOpen()                      \
  (autocapAct.autocapFlags.open)

/* closing if run is TRUE, completed otherwise */
#define isAutocapActClose()                     \
  (autocapAct.autocapFlags.close)

/* homing in progress if run is TRUE, completed otherwise */
#define isAutocapActHoming()                    \
  (autocapAct.autocapFlags.homing)
 
/* error conditions */
#define isAutocapActError()                                 \
  (autocapAct.autocapFlags.tout_error           ||          \
   autocapAct.autocapFlags.homing_error         ||          \
   autocapAct.autocapFlags.packing_error        ||          \
   autocapAct.autocapFlags.lifter_error         ||          \
   autocapAct.autocapFlags.software_error       ||          \
   autocapAct.autocapFlags.over_curr_temp_error ||          \
   autocapAct.autocapFlags.open_load_error)

# define isAutocapCmdHoming()  (autocapAct.command.homing)
# define isAutocapCmdOpen()    (autocapAct.command.open)
# define isAutocapCmdClose()   (autocapAct.command.close)

// ----------------------------
# define AUTOCAP_MMT_OPENING()  \
do {                         \
    IN1_BRUSH = ON;          \
    IN2_BRUSH = OFF;         \
} while (0)

// ----------------------------
# define AUTOCAP_MMT_CLOSING()  \
do {                          \
    IN1_BRUSH = OFF;          \
    IN2_BRUSH = ON;           \
} while (0)

// ----------------------------
# define AUTOCAP_MMT_STOPPED()  \
do {                          \
    IN1_BRUSH = OFF;          \
    IN2_BRUSH = OFF;          \
} while (0)

// ----------------------------
# define I2_BRUSH_OFF()      \
do {                         \
	I2_BRUSH = OFF;          \
} while (0)

# define I2_BRUSH_ON()       \
do {                         \
	I2_BRUSH = ON;           \
} while (0)
// ----------------------------
# define I3_BRUSH_OFF()      \
do {                         \
	I3_BRUSH = OFF;          \
} while (0)

# define I3_BRUSH_ON()       \
do {                         \
	I3_BRUSH = ON;           \
} while (0)
// ----------------------------
# define I4_BRUSH_OFF()      \
do {                         \
	I4_BRUSH = OFF;          \
} while (0)

# define I4_BRUSH_ON()       \
do {                         \
	I4_BRUSH = ON;           \
} while (0)
// ----------------------------

#ifndef CAR_REFINISHING_MACHINE  
/**
 * Function prototypes
 */
#ifndef AUTOCAP_MMT
extern void makeAutocapActMessage(uartBuffer_t *txBuffer, unsigned char slave_id);
extern void decodeAutocapActMessage(uartBuffer_t *rxBuffer, unsigned char slave_id);
#else
extern void autocap_Manager(void);
extern void init_Autocap(void);
#endif
extern void setAutocapActMessage(unsigned char packet_type);

/**
 * initialization helpers
 */
extern void initAutocapActHoming(void);
extern int isAutocapActHomingCompleted(void);

#endif
        
#endif /* _AUTOCAPACT_H_ */
