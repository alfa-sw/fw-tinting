/* 
 * File:   define.h
 * Author: michele.abelli
 *
 * Created on 12 marzo 2020, 10.35
 */

#ifndef DEFINE_H
#define	DEFINE_H

// FAULT_1 on BRUSH DRV8842
#define SKIP_FAULT_1
// FAULT on NEBULIZER TPS1H200-A
#define SKIP_FAULT_NEB
// FAULT on AIR PUMP TPS1H200-A
#define SKIP_FAULT_PUMP
// FAULT on RELE 
#define SKIP_FAULT_RELE
// FAULT on GENERIC24V TPS1H200-A
#define SKIP_FAULT_GENERIC24V

#define TRUE 1
#define FALSE 0

#define DISABLE   0
#define ENABLE    1

/******************************************************************************/
/*********************************** HW_IO_Remapping **************************/
/******************************************************************************/
//PPS Outputs
#define NULL_IO     0
#define C1OUT_IO    1
#define C2OUT_IO    2
#define U1TX_IO     3
#define U1RTS_IO    4
#define U2TX_IO     5
#define U2RTS_IO    6
#define SDO1_IO     7
#define SCK1_IO     8
#define SS1OUT_IO   9
#define SDO2_IO    10
#define SCK2_IO    11
#define SS2OUT_IO  12
#define SDI1_IO    13
#define OC1_IO     18
#define OC2_IO     19
#define OC3_IO     20
#define OC4_IO     21
#define OC5_IO     22
#define OC6_IO     23
#define OC7_IO     24
#define OC8_IO     25
#define SDI2_IO    26
#define U3TX_IO    28
#define U3RTS_IO   29
#define U4TX_IO    30
#define U4RTS_IO   31
#define SDO3_IO    32
#define SCK3_IO    33
#define SS3OUT_IO  34
#define OC9_IO     35

enum {
  PROC_RUN,
  PROC_OK,
  PROC_FAIL,
};

enum {
  MIXER_IDLE,
  MIXER_PAR_RX,
  MIXER_START,
  MIXER_SETUP,
  MIXER_RUNNING,
  MIXER_HOMING,
  MIXER_SETUP_OUTPUT,
  MIXER_END,
  MIXER_ERROR,
  MIXER_PAR_ERROR, 
  JAR_MOTOR_RUNNING,
  SET_MIXER_MOTOR_HIGH_CURRENT,
};

enum {
  HUMIDIFIER_IDLE,
  HUMIDIFIER_START,
  HUMIDIFIER_RUNNING,
  HUMIDIFIER_PAR_RX,
  HUMIDIFIER_SETUP_OUTPUT,
  HUMIDIFIER_TOO_LOW_WATER_LEVEL,
  HUMIDIFIER_RH_ERROR,
  HUMIDIFIER_TEMPERATURE_ERROR,
  HUMIDIFIER_PAR_ERROR,
  HUMIDIFIER_NEBULIZER_OVERCURRENT_THERMAL_ERROR,
  HUMIDIFIER_NEBULIZER_OPEN_LOAD_ERROR,
  HUMIDIFIER_RELE_OVERCURRENT_THERMAL_ERROR,
  HUMIDIFIER_RELE_OPEN_LOAD_ERROR,
  HUMIDIFIER_AIR_PUMP_OVERCURRENT_THERMAL_ERROR,
  HUMIDIFIER_AIR_PUMP_OPEN_LOAD_ERROR,  
};

enum {
    AUTOCAP_INIT_ST,
    AUTOCAP_READY_ST,
    AUTOCAP_SEARCH_PACKING_ST,
    AUTOCAP_PACKED_ST,
    AUTOCAP_SEARCH_HOMING_ST,
    AUTOCAP_CLOSE_ST,
    AUTOCAP_OPEN_RUN_ST,
    AUTOCAP_OPEN_ST,
    AUTOCAP_CLOSE_RUN_ST,
    AUTOCAP_SEARCH_PACKING_CLOSED_ST,
    AUTOCAP_PACKED_CLOSED_ST,
    AUTOCAP_EXTEND_RUN_ST,
    AUTOCAP_EXTEND_ST,
    AUTOCAP_RETRACT_RUN_ST,
    AUTOCAP_ERROR_ST,
};

enum {
  LIGHT_OFF,
  LIGHT_STEADY,
  LIGHT_PULSE_SLOW,
  LIGHT_PULSE_FAST,
};

// Watchdog control
#define ENABLE_WDT()                            \
  do {                                          \
    _SWDTEN = 1;                                \
  } while (0)

#define DISABLE_WDT()                           \
  do {                                          \
    _SWDTEN = 0;                                \
  } while (0)

// Default values for Humidifier 2.0
// -----------------------------------------------------------------------------
#define OFF 0
#define ON 1
#define DONE 2

#define REFERENCE_STEP_0    0
#define REFERENCE_STEP_1    1
#define REFERENCE_STEP_2    2
#define REFERENCE_STEP_ON   3

#define HUMIDIFIER_DISABLE	0
#define HUMIDIFIER_ENABLE	1

#define HUMIDIFIER_TYPE_0	0 // SENSIRION SHT31
#define HUMIDIFIER_TYPE_1	1 // NO SENSOR - Process Humidifier 1.0
#define HUMIDIFIER_TYPE_2   2 // NO SENSOR
        
#define HUMIDIFIER_MULTIPLIER 5
#define MAX_HUMIDIFIER_MULTIPLIER 1000

#define HUMIDIFIER_PWM      100 // WATER RESISTANCE for THOR process
//#define HUMIDIFIER_PWM      30 // WATER RESISTANCE for THOR process

//#define HUMIDIFIER_PERIOD   1200 // 20 min
//#define HUMIDIFIER_DURATION 2    // 2 sec
#define HUMIDIFIER_PERIOD   30 // 30sec
#define HUMIDIFIER_DURATION 10    // 10 sec

#define AUTOCAP_OPEN_DURATION	5 // 5"
#define MAX_HUMIDIFIER_DURATION_AUTOCAP_OPEN 120 // 120"
#define AUTOCAP_OPEN_PERIOD		30 // 30"

#define TEMP_DISABLE		0
#define TEMP_ENABLE			1

#define TEMPERATURE_TYPE_0  0 // SENSIRION SHT31
#define TEMPERATURE_TYPE_1  1 // MICROCHIP TC72

#define TEMP_PERIOD			300 // 5 min
#define MIN_TEMP_PERIOD		5

#define TEMP_T_LOW			10
#define TEMP_T_HIGH			20
#define HEATER_TEMP         10
#define HEATER_HYSTERESIS   1 

#define NEBULIZER			1
#define POMPA    			2
#define TURN_LED            4
#define RISCALDATORE        8
        
#define OUTPUT_OFF			0
#define OUTPUT_ON			1

#define ROTATING_CW			0
#define ROTATING_CCW	    1

#define AUTOCAP_CLOSED		0
#define AUTOCAP_OPEN		1
#define AUTOCAP_ERROR		2

#define MEASUREMENT_OK      0
#define MEASUREMENT_ERROR   1

#define READ_OK      0
#define READ_ERROR   1

#define HUMIDIFIER_MAX_ERROR           5
#define DOSING_TEMPERATURE_MAX_ERROR   5

#define HUMIDIFIER_MAX_ERROR_DISABLE          20
#define DOSING_TEMPERATURE_MAX_ERROR_DISABLE  20
// -----------------------------------------------------------------------------
// Photocell Sensor
// Mixer Homing Photocell
#define HOME_PHOTOCELL          0
// Jar Photocell
#define JAR_PHOTOCELL           1 
// Door Open Photocell
#define DOOR_OPEN_PHOTOCELL     2
// Door Closed
#define DOOR_MICROSWITCH        3
// Autocap Open Photocell
#define AUTOCAP_OPEN_PHOTOCELL  4
// Autocap Lifter Down Photocell
#define AUTOCAP_LIFTER_PHOTOCELL  5
// -----------------------------------------------------------------------------
#define FILTER      1
#define NO_FILTER   0
// -----------------------------------------------------------------------------
#define DARK    1
#define LIGHT   0
// -----------------------------------------------------------------------------
#define CW    0 
#define CCW   1
// -----------------------------------------------------------------------------
//#define DARK_LIGHT    0
//#define LIGHT_DARK    1
#define DARK_LIGHT    1
#define LIGHT_DARK    0

#define TRANSACTION_DISABLED    0xFF
// -----------------------------------------------------------------------------
#define CLOSE       1
#define OPEN        0
// -----------------------------------------------------------------------------
#define ABSOLUTE    0
#define INCREMENTAL 1
// -----------------------------------------------------------------------------
#define STATIC      0
#define DYNAMIC     1
// -----------------------------------------------------------------------------
#define COLORANT_ID_OFFSET  7
// -----------------------------------------------------------------------------
#define DETERMINED      0 
#define UNDETERMINED    1
// -----------------------------------------------------------------------------
#define CLOSING_STEP0 0
#define CLOSING_STEP1 1
#define CLOSING_STEP2 2
#define CLOSING_STEP3 3
#define CLOSING_STEP4 4
// -----------------------------------------------------------------------------
#define MIXER_N_PROFILE  10
#define MIXER_MAX_DOOR_OPEN 60 // sec
#define MIXER_MAX_N_CYCLE 50 
#define MIXER_MAX_SPEED 700 // (RPM))
#define MIXER_MAX_DURATION 60 // Duration for a single sempiphase (sec)

#define MIXER_TIME_DOOR_OPEN 10 // 10 sec
#define MIXER_DEFAULT_N_CYCLE 2
#define MIXER_DEFAULT_SPEED 700 // (RPM))
#define MIXER_DEFAULT_DURATION 5 // Duration for a single sempiphase (sec)
#define MIXER_DEFAULT_HOMING_SPEED 20 // (RPM)
#define MIXER_LOW_HOMING_SPEED 20 // (RPM)
#define MIXER_HOME_PHOTO_STEPS 4 * CORRECTION_MIXER_STEP_RES // Larghezza pinna Mixer 8mm
#define MIXER_DOOR_DEFAULT_HOMING_SPEED 10 // (RPM) (attenzione se mettiamo 100rpm non vede la fotocellula!!!))
#define MIXER_DOOR_HOME_PHOTO_STEPS 9 * CORRECTION_DOOR_STEP_RES // Larghezza pinna Sportellino 4mm

# define AUTOCAP_SPEED_SEARCH_PHOTOC   200  // RPM 
# define AUTOCAP_SPEED_HOMING          300  // RPM 

// Motor steps in 1 full round (360°))
#define MIXER_MOTOR_ONE_ROUND 400 * CORRECTION_MIXER_STEP_RES
// Offset Steps to do before reaching Mixer Home position
#define STEP_PHOTO_MIXER_OFFSET 100 * CORRECTION_MIXER_STEP_RES
// Maximum Step with Mixer Motor ON to wait HOME Photocell become DARK
#define MAX_STEP_MIXER_MOTOR_HOME_PHOTOCELL 500 * CORRECTION_MIXER_STEP_RES
// Passi da posizione di Porta Aperta (Fotocellula coperta e centrata)  a Porta Chiusa (Microswitch premuto)
//#define STEP_DOOR_MOTOR_OPEN 733 * CORRECTION_DOOR_STEP_RES   
#define STEP_DOOR_MOTOR_OPEN 350 * CORRECTION_DOOR_STEP_RES   
// Maximum Step with Door Motor ON to wait Door Open Photocell become DARK
//#define MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL 900 * CORRECTION_DOOR_STEP_RES
#define MAX_STEP_DOOR_MOTOR_OPEN_PHOTOCELL 400 * CORRECTION_DOOR_STEP_RES
// Steps to understand if Micrcoswitch is not working
#define STEP_DOOR_MOTOR_CHECK_MICRO 50 * CORRECTION_DOOR_STEP_RES

// Maximum Step with Autocap Motor ON to wait Autocap Open Photocell become DARK (= 4sec at 200RPM)
#define MAX_STEP_AUTOCAP_OPEN 8000 * CORRECTION_AUTOCAP_STEP_RES
        
# define ABS(x) ((x) >= (0) ? (x) : (-x))  

enum {
  /* 0 */ FAULT_1_IDLE,
  /* 1 */ FAULT_1_WAIT_ENABLING,
  /* 2 */ FAULT_1_WAIT_ACTIVATION,
  /* 3 */ FAULT_1_ERROR,
  /* 4 */ FAULT_NEB_ERROR,
  /* 5 */ FAULT_MIXER_ERROR,
  /* 6 */ FAULT_RELE_ERROR,               
  /* 7 */ FAULT_GENERIC24V_ERROR,               
};

#define IS_IN1_BRUSH_OFF() (IN1_BRUSH == 0)
#define IS_IN2_BRUSH_OFF() (IN2_BRUSH == 0)
#define isFault_1_Conditions() (IS_IN1_BRUSH_OFF() && IS_IN1_BRUSH_OFF())
#define isFault_1_Detection()  (BRUSH_F2 == 0)

#define isFault_Neb_Detection() (NEB_F == 0)
#define isFault_Pump_Detection()(AIR_PUMP_F == 0)
#define isFault_Rele_Detection()(RELAY_F == 0)
#define isFault_Generic24V_Detection() (OUT_24V_FAULT == 0)

#define DRV8842_RESET()     \
  do {                      \
    RST_BRUSH = 0;          \
  } while (0)

#define DRV8842_STOP_RESET()\
  do {                      \
    RST_BRUSH = 1;          \
  } while (0)

# define NEBULIZER_OFF()	\
do {                        \
	NEB_IN = OFF;           \
} while (0)

# define NEBULIZER_ON()     \
do {                        \
	NEB_IN = ON;            \
} while (0)
// -----------------------------
# define WATER_PUMP_OFF()   \
do {                        \
	AIR_PUMP_IN = OFF;      \
} while (0)

# define WATER_PUMP_ON()    \
do {                        \
	AIR_PUMP_IN = ON;       \
} while (0)
// ----------------------------

# define LED_OFF()           \
do {                         \
	LED_ON_OFF = OFF;        \
} while (0)

# define LED_ON()            \
do {                         \
	LED_ON_OFF = ON;         \
} while (0)
// ----------------------------
# define RISCALDATORE_OFF() \
do {                        \
	RELAY = OFF;         \
} while (0)

# define RISCALDATORE_ON()  \
do {                        \
	RELAY = ON;          \
} while (0)
// -----------------------------
# define WATER_PUMP_OFF()   \
do {                        \
	AIR_PUMP_IN = OFF;      \
} while (0)

# define WATER_PUMP_ON()    \
do {                        \
	AIR_PUMP_IN = ON;       \
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
# define BRUSH_OFF()        \
do {                        \
	IN1_BRUSH = OFF;        \
} while (0)

# define BRUSH_ON()         \
do {                        \
	IN1_BRUSH = ON;         \
} while (0)
// ----------------------------
# define STEPPER_MIXER_OFF()   \
do {                          \
	HardHiZ_Stepper(MOTOR_MIXER);  \
} while (0)

# define STEPPER_MIXER_ON()   \
do {                         \
    StartStepper(MOTOR_MIXER, (unsigned short)TintingAct.Mixer_Homimg_Speed, CW, LIGHT_DARK, HOME_PHOTOCELL, 0); \
} while (0)
// ----------------------------
# define STEPPER_DOOR_OFF()   \
do {                          \
	HardHiZ_Stepper(MOTOR_DOOR);  \
} while (0)

# define STEPPER_DOOR_ON()   \
do {                         \
    StartStepper(MOTOR_DOOR, (unsigned short)TintingAct.Mixer_Door_Homimg_Speed, CW, LIGHT_DARK, DOOR_OPEN_PHOTOCELL, 0); \
} while (0)
// ----------------------------
# define STEPPER_AUTOCAP_OFF()   \
do {                          \
	HardHiZ_Stepper(MOTOR_AUTOCAP);  \
} while (0)

# define STEPPER_AUTOCAP_ON()   \
do {                         \
    StartStepper(MOTOR_AUTOCAP, (unsigned short)TintingAct.Autocap_Homimg_Speed, CW, LIGHT_DARK, AUTOCAP_OPEN_PHOTOCELL, 0); \
} while (0)
// ----------------------------
# define CAN_LIFTER_FWD()                               \
  do {                                                  \
    IN1_BRUSH = ON;          \
    IN2_BRUSH = OFF;         \
  } while (0)

# define CAN_LIFTER_BWD()                               \
  do {                                                  \
    IN1_BRUSH = OFF;          \
    IN2_BRUSH = ON;           \
  } while (0)

# define CAN_LIFTER_IDLE()                              \
  do {                                                  \
    IN1_BRUSH = OFF;          \
    IN2_BRUSH = OFF;          \
  } while (0)
// ----------------------------  
# define RST_BRD_OFF()       \
do {                         \
	STBY_RST_BRD = OFF;      \
} while (0)

# define RST_BRD_ON()        \
do {                         \
	STBY_RST_BRD = ON;       \
} while (0)
// ----------------------------  
# define RST_PMP_OFF()       \
do {                         \
	STBY_RST_PMP = OFF;      \
} while (0)

# define RST_PMP_ON()        \
do {                         \
	STBY_RST_PMP = ON;       \
} while (0)
// ---------------------------- 
# define RST_EV_OFF()        \
do {                         \
	STBY_RST_EV = OFF;       \
} while (0)

# define RST_EV_ON()         \
do {                         \
	STBY_RST_EV = ON;        \
} while (0)
// ---------------------------- 
# define isColorCmdStop()  		  (TintingAct.command.tinting_stop)
# define isColorCmdHome()  		  (TintingAct.command.tinting_home)
# define isColorCmdIntr()         (TintingAct.command.tinting_intr)
# define isColorCmdIdle()         (TintingAct.command.cmd == 0)
# define isColorCmdPacking()      (TintingAct.command.packing)
# define isColorCmdOpen()         (TintingAct.command.open)
# define isColorCmdClose()        (TintingAct.command.close)
# define isColorCmdExtend()       (TintingAct.command.extend)
# define isColorCmdRetract()      (TintingAct.command.retract)


// -----------------------------------------------------------------------------
# define isHumidifierError()                                                   \
    ( (Status.level == TINTING_RH_ERROR_ST)             ||                     \
      (Status.level == TINTING_TEMPERATURE_ERROR_ST)    ||                     \
      (Status.level == TINTING_BAD_PAR_HUMIDIFIER_ERROR_ST)             ||     \
      (Status.level == TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST)  ||     \
      (Status.level == TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST)            ||     \
      (Status.level == TINTING_AIR_PUMP_OVERCURRENT_THERMAL_ERROR_ST)   ||     \
      (Status.level == TINTING_AIR_PUMP_OPEN_LOAD_ERROR_ST)             ||     \
      (Status.level == TINTING_MIXER_OVERCURRENT_THERMAL_ERROR_ST)      ||     \
      (Status.level == TINTING_MIXER_OPEN_LOAD_ERROR_ST) ||                     \
      (Status.level == TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST)       ||     \
      (Status.level == TINTING_RELE_OPEN_LOAD_ERROR_ST) )
// -----------------------------------------------------------------------------
#define RESET_ON    0
#define RESET_WAIT  1
        
#define WAIT 0   

#endif	/* DEFINE_H */

