/* 
 * File:   define.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 14.44
 */

#ifndef DEFINE_H
#define	DEFINE_H


#ifdef DEBUG_MMT
#define DEBUG_MOTORS 1
#define DEBUG_BRUSH 0
#define DEBUG_POWER_OUT 0
#define DEBUG_OUT 0
#endif
// FAULT_1 on BRUSH DRV8842
//#define SKIP_FAULT_1
// FAULT on NEBULIZER TPS1H200-A
//#define SKIP_FAULT_NEB
// FAULT on PUMP TPS1H200-A
#define SKIP_FAULT_PUMP
// FAULT on RELE TPS1H200-A
//#define SKIP_FAULT_RELE
// FAULT on GENERIC24V TPS1H200-A
//#define SKIP_FAULT_GENERIC24V

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
  PUMP_IDLE,
  PUMP_PAR_RX,
  PUMP_START,
  PUMP_SETUP,
  PUMP_RUNNING,
  PUMP_RICIRCULATION,
  PUMP_HOMING,
  VALVE_HOMING,
  PUMP_SETUP_OUTPUT,
  PUMP_VALVE_OPEN_CLOSE,
  PUMP_END,
  PUMP_ERROR,
  PUMP_PAR_ERROR,    
  PUMP_CLOSE_VALVE,
  PUMP_GO_HOME, 
  PUMP_VALVE_ROTATING,
  VALVE_NEW_HOMING,
};

enum {
  TABLE_IDLE,
  TABLE_PAR_RX,
  TABLE_START,
  TABLE_SETUP,
  TABLE_RUNNING,
  TABLE_HOMING,
  TABLE_POSITIONING,
  TABLE_STEPS_POSITIONING,
  TABLE_CLEANING,
  TABLE_SELF_RECOGNITION,
  TABLE_TEST,
  TABLE_END,
  TABLE_ERROR,  
  TABLE_PAR_ERROR,
  TABLE_GO_REFERENCE,
  TABLE_STIRRING,
  TABLE_STOP_STIRRING,
  TABLE_SETUP_OUTPUT, 
  TABLE_RUN,
  TABLE_FIND_REFERENCE,
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
  HUMIDIFIER_PUMP_OVERCURRENT_THERMAL_ERROR,
  HUMIDIFIER_PUMP_OPEN_LOAD_ERROR,
  HUMIDIFIER_RELE_OVERCURRENT_THERMAL_ERROR,
  HUMIDIFIER_RELE_OPEN_LOAD_ERROR,     
};

enum {
  /*  0 */   COLORANT_SINGLE_BASE,
  /*  1 */   DOUBLE_GROUP_MASTER_NO_SLAVE,
  /*  2 */   DOUBLE_GROUP_MASTER_WITH_SLAVE,
  /*  3 */   DOUBLE_GROUP_SLAVE_NO_MASTER,
  /*  4 */   DOUBLE_GROUP_SLAVE_WITH_MASTER
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

#define URXREG_NUM_BYTES 4

#define REFERENCE_STEP_0    0
#define REFERENCE_STEP_1    1
#define REFERENCE_STEP_2    2
#define REFERENCE_STEP_ON   3

#define HUMIDIFIER_DISABLE	0
#define HUMIDIFIER_ENABLE	1

#define HUMIDIFIER_TYPE_0	0 // SENSIRION SHT31
#define HUMIDIFIER_TYPE_1	1 // NO SENSOR - Process Humidifier 1.0
#define HUMIDIFIER_TYPE_2   2 // NO SENSOR
        
#define HUMIDIFIER_MULTIPLIER 100
#define MAX_HUMIDIFIER_MULTIPLIER 1000

#define HUMIDIFIER_PWM      100 // WATER RESISTANCE for THOR process

#define HUMIDIFIER_PERIOD   1200 // 20 min
#define HUMIDIFIER_DURATION 2    // 2 sec

#define AUTOCAP_OPEN_DURATION	10 // 10"
#define MAX_HUMIDIFIER_DURATION_AUTOCAP_OPEN 120 // 120"
#define AUTOCAP_OPEN_PERIOD		30 // 30"

#define TEMP_DISABLE		0
#define TEMP_ENABLE			1

#define DOSING_TEMP_PROCESS_DISABLED 32768

#define TEMPERATURE_TYPE_0  0 // SENSIRION SHT31
#define TEMPERATURE_TYPE_1  1 // MICROCHIP TC72

#define TEMP_PERIOD			10 // 10 sec
//#define MIN_TEMP_PERIOD		10
#define MIN_TEMP_PERIOD		0

#define TEMP_T_LOW			10
#define TEMP_T_HIGH			15
#define HEATER_TEMP         40
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
#define AUTOCAP_OPEN		2

#define MEASUREMENT_OK      0
#define MEASUREMENT_ERROR   1

#define READ_OK      0
#define READ_ERROR   1

#define HUMIDIFIER_MAX_ERROR           5
#define DOSING_TEMPERATURE_MAX_ERROR   5

#define HUMIDIFIER_MAX_ERROR_DISABLE          20
#define DOSING_TEMPERATURE_MAX_ERROR_DISABLE  20

#define NEW_RICIRCULATION   36
// -----------------------------------------------------------------------------
// Default values for Pump
// Tolleranza sui passi madrevite in accoppiamento: 3.5mm
#define TOLL_ACCOPP 938 * CORRECTION_PUMP_STEP_RES
// Passi da fotocellula madrevite coperta a fotocellula ingranamento coperta: 6.5mm
#define STEP_ACCOPP 1740 * CORRECTION_PUMP_STEP_RES
// Passi a fotoellula ingranamento coperta per ingaggio circuito: 1.5mm
#define STEP_INGR   400 * CORRECTION_PUMP_STEP_RES
// Passi per recupero giochi: 1.075mm
#define STEP_RECUP  266 * CORRECTION_PUMP_STEP_RES
// (Prima versione di Pinna)Passi a fotocellula madrevite coperta per posizione di home: 7.40mm
//#define PASSI_MADREVITE 1968 * CORRECTION_PUMP_STEP_RES
// (Seconda versione di Pinna 8.1.2018) Passi a fotocellula madrevite coperta per posizione di home: 0.40mm
#define PASSI_MADREVITE 101 * CORRECTION_PUMP_STEP_RES
// Passi a fotocellula madrevite coperta per posizione di home: 7.40mm - 7.00 mm (per problemi di interferenza con la Tavola)
//#define PASSI_MADREVITE 100 * CORRECTION_PUMP_STEP_RES
// Velocità da fotocellula madrevite coperta a fotocellula ingranamento coperta (rpm)
#define V_ACCOPP    400
// Velocità a fotoellula ingranamento coperta per ingaggio circuito (rpm))
#define V_INGR      100
// Velocità per raggiungere la posizione di start ergoazione in alta risoluzione
#define V_APPOGGIO_SOFFIETTO   100

// Offset Valvola da posizione di zero a primo dente ingranato 
#define STEP_VALVE_OFFSET 36 * CORRECTION_VALVE_STEP_RES
// Offset Valvola da posizione di zero a primo dente ingranato + 5 % per Chiusura Valvola corretta
#define STEP_CLOSE_VALVE 46 * CORRECTION_VALVE_STEP_RES
// Passi da posizione di home/ricircolo sul fronte DARK/LIGHT della Fotocelluila (valvola chiusa) a posizone di valvola aperta su fori grande (3mm) e piccolo(0.8mm))
#define STEP_VALVE_OPEN 148 * CORRECTION_VALVE_STEP_RES // grande +148 (80°), piccolo -148 (-80°))
// Passi da posizione di home/ricircolo sul fronte DARK/LIGHT della Fotocellula a posizone di backstep (0.8mm)
#define STEP_VALVE_BACKSTEP 74 * CORRECTION_VALVE_STEP_RES // backstep -74 (-40°)
// Max step to do to search CW and CCW Home Valve Position
//#define MAX_STEP_VALVE_HOMING 166 * CORRECTION_VALVE_STEP_RES + STEP_VALVE_OFFSET //(+-90°)
#define MAX_STEP_VALVE_HOMING 266 * CORRECTION_VALVE_STEP_RES + STEP_VALVE_OFFSET //(+-90°)
#define MAX_STEP_VALVE_HOMING_STUPID 1100 * CORRECTION_VALVE_STEP_RES + STEP_VALVE_OFFSET //(+-90°)
// Passi da posizione di home/ricircolo al centro della Fotocelluila a transizione DARK/LIGHT verso foro di 3.0mm
//#define STEP_PHOTO_VALVE_BIG_HOLE   6 * CORRECTION_VALVE_STEP_RES
#define STEP_PHOTO_VALVE_BIG_HOLE 4 * CORRECTION_VALVE_STEP_RES
// Passi da posizione di home/ricircolo al centro della Fotocelluila a transizione DARK/LIGHT verso foro di 0.8mm
//#define STEP_PHOTO_VALVE_SMALL_HOLE 6 * CORRECTION_VALVE_STEP_RES
#define STEP_PHOTO_VALVE_SMALL_HOLE 4 * CORRECTION_VALVE_STEP_RES
// Velocità di apertura/chiusura valvola (rpm))
#define SPEED_VALVE  40
#define VALVE_STEPS_TOLERENCE 2 * CORRECTION_VALVE_STEP_RES
// N. steps in una corsa intera
#define N_STEPS_STROKE  ((unsigned long)3900 * (unsigned long)CORRECTION_PUMP_STEP_RES)
// Back step N. before to Open valve
#define PUMP_STEP_BACKSTEP  20 * CORRECTION_PUMP_STEP_RES
// Passi per raggiungere la posizione di start erogazione in alta risoluzione: 15.22mm
// Il valore teorico è 4267 passi
#define PASSI_APPOGGIO_SOFFIETTO ((unsigned long)4336 * (unsigned long)CORRECTION_PUMP_STEP_RES)
#define TOT_PASSI_APPOGGIO_SOFFIETTO PASSI_APPOGGIO_SOFFIETTO + STEP_RECUP
// Massimo N° passi in Appoggio Soffietto + Backstep consentiti 
#define MAX_PASSI_APPOGGIO_SOFFIETTO ((unsigned long)5000 * (unsigned long)CORRECTION_PUMP_STEP_RES) 

// Back Step Speed (rpm) before to Open Valve
#define PUMP_SPEED_BACKSTEP 100
// Massimo numero di passi durante l'Homing della Pompa in attesa della transizione DARK-LIGHT: 1.2mm --> 480half steps 
#define MAX_STEP_PUMP_HOMING_FORWARD 480 * CORRECTION_PUMP_STEP_RES
// Massimo numero di passi durante l'Homing della Pompa in attesa della transizione LIGHT-DARK 
#define MAX_STEP_PUMP_HOMING_BACKWARD 6742 * CORRECTION_PUMP_STEP_RES
// Passi che occorre fare dalla posizione di rotore sul fermo nella puleggia alla posizione di Home
#define STEP_VALVE_HOMING_OBSTACLE_CW 400 * CORRECTION_VALVE_STEP_RES
// Passi che occorre fare dalla posizione di rotore sul fermo nella puleggia alla posizione di Home
#define STEP_VALVE_HOMING_OBSTACLE_CCW 400 * CORRECTION_VALVE_STEP_RES
// Nell'Algoritmo Continuous alla fine dell'Aspirazione prima di Erogare occorre fare questi passi per attendere l'Apertura della check valve 
#define STEPS_TO_OPEN_CHECK_VALVE 250 * CORRECTION_PUMP_STEP_RES
// -----------------------------------------------------------------------------
// Default values for Rotating Table
// Passi corrispondenti ad un giro completa di 360° della tavola
//#define STEPS_REVOLUTION ((unsigned long)6342 * (unsigned long)CORRECTION_TABLE_STEP_RES) // 6343 corretto
// 15.1.2018 - Nuovo pignone con 16 denti invece di 22: + coppia 30%; - velocità 30%
#define STEPS_REVOLUTION ((unsigned long)8325 * (unsigned long)CORRECTION_TABLE_STEP_RES) // 8720 corretto
// Tolleranza in passi corrispondente ad una rotazione completa di 360° della tavola
#define STEPS_TOLERANCE_REVOLUTION 26 * CORRECTION_TABLE_STEP_RES
// Passi in cui la targetta di Riferiemnto rimane coperta (Misurato dal Firmware il 7.12.2018: 0.1875mm)
#define STEPS_REFERENCE 66 * CORRECTION_TABLE_STEP_RES 
// Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il Riferimento
#define STEPS_TOLERANCE_REFERENCE 26 * CORRECTION_TABLE_STEP_RES
// Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito (Misurato dal Firmware il 7.12.2018: 0.0825mm)
#define STEPS_CIRCUIT 29 * CORRECTION_TABLE_STEP_RES 
// Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito
#define STEPS_TOLERANCE_CIRCUIT 26 * CORRECTION_TABLE_STEP_RES
// Velocità massima di rotazione della tavola rotante (rpm))
#define HIGH_SPEED_ROTATING_TABLE 200
// Velocità minima di rotazione della tavola rotante (rpm)
#define LOW_SPEED_ROTATING_TABLE   20
// N° di giri della Tavola da compiere per effettuare lo Stirring (1 giro completo della Tavola)
#define STEPS_STIRRING  1
// N° di passi della Tavola rispetto al Riferimento per posizionarsi sulla Spazzola 
#define STEPS_CLEANING  2890 * CORRECTION_TABLE_STEP_RES
// Velocità massima ammessa della Tavola Rotante (rpm))
#define MAX_TABLE_SPEED  250
// Velocità minima ammessa della Tavola Rotante (rpm))
#define MIN_TABLE_SPEED  10
// Passi nella rotazione CW in cui la pinna del Circuito oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_CIRCUIT_CW 13 * CORRECTION_TABLE_STEP_RES
// Passi nella rotazione CCW in cui la pinna del Circuito oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_CIRCUIT_CCW 13 * CORRECTION_TABLE_STEP_RES
// Passi nella rotazione CW in cui la pinna del Riferimento oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_REFERENCE_CW 33 * CORRECTION_TABLE_STEP_RES
// Passi nella rotazione CCW in cui la pinna del Riferimento oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_REFERENCE_CCW 33 * CORRECTION_TABLE_STEP_RES
// Offset in Passi prima di andare nella posizione di Home
//#define STEP_PHOTO_TABLE_OFFSET 105 * CORRECTION_TABLE_STEP_RES
#define STEP_PHOTO_TABLE_OFFSET 138 * CORRECTION_TABLE_STEP_RES
// Maximum Rotating Angle (°)
#define MAX_ROTATING_ANGLE 180
// Steps between Reference position and Circuit N°1
//#define STEPS_REFERENCE_CIRC_1 260 * CORRECTION_TABLE_STEP_RES 
#define STEPS_REFERENCE_CIRC_1 130 * CORRECTION_TABLE_STEP_RES 
// Steps distance between 2 Circuits
#define STEPS_CIRCUITS (STEPS_REVOLUTION / 16) 
// Tolleranza sul posizionamento di 1 Circuito
#define STEPS_TOLERANCE_POSITIONING 26 * CORRECTION_TABLE_STEP_RES
// Tipologia di Foro di Dosaggio
#define BIG_HOLE    0
#define SMALL_HOLE  1    

// Tipologia di Stirring
#define BEFORE_EVERY_RICIRCULATION  0 // 2 giri completi prima di ogni Ricircolo
#define AFTER_LAST_RICIRCULATING_CIRCUIT  1 // Al termine del Ricircolo dell'ultimo circuito in configurazione
// -----------------------------------------------------------------------------
// Durata della Pulizia (sec))
#define CLEANING_DURATION 5
// Pausa della Pulizia (min))
#define CLEANING_PAUSE    30
// -----------------------------------------------------------------------------
// Types of Algorithm
#define ALG_SINGLE_STROKE          (0)
#define ALG_DOUBLE_STROKE          (1)
#define ALG_SYMMETRIC_CONTINUOUS   (2)
#define ALG_ASYMMETRIC_CONTINUOUS  (3)
#define HIGH_RES_STROKE  		   (4)
#define ALG_HIGH_RES_STROKE	   	   (4)
#define ALG_DOUBLE_GROUP 		   (5)
#define ALG_DOUBLE_GROUP_CONTINUOUS (6)
// -----------------------------------------------------------------------------
#define SINGLE_STROKE_FULL_ROOM    (0)
#define SINGLE_STROKE_EMPTY_ROOM   (1)
#define SINGLE_STROKE_CLEVER       (2)
// -----------------------------------------------------------------------------
// Maximum Table Moving Error admitted before to give Error
#define MAX_TABLE_ERROR 1
// -----------------------------------------------------------------------------
// Photocell Sensor
#define HOME_PHOTOCELL          0
// Coupling Photocell
#define COUPLING_PHOTOCELL      1 
// Valve Homing Photocell
#define VALVE_PHOTOCELL         2          
// Table Photocell
#define TABLE_PHOTOCELL         3             
// Valve Open Photocell
#define VALVE_OPEN_PHOTOCELL    4          
// Autocap CLOSE Photocell
#define AUTOCAP_CLOSE_PHOTOCELL 5
// Autocap OPEN Photocell
#define AUTOCAP_OPEN_PHOTOCELL  6
// BRUSH Photocell
#define BRUSH_PHOTOCELL         7
// Can Presence Photocell
#define CAN_PRESENCE_PHOTOCELL  8 
// Panel Table
#define PANEL_TABLE             9
// Bases Carriage
#define BASES_CARRIAGE          10
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
#define PUSHED     1
#define NOT_PUSHED 0
// -----------------------------------------------------------------------------

#define TINTING_COLORANT_OFFSET 8
// -----------------------------------------------------------------------------
#define DIR_EROG      0 
#define DIR_SUCTION   1
//#define DIR_EROG      1 
//#define DIR_SUCTION   0
// -----------------------------------------------------------------------------
//#define DARK_LIGHT    0
//#define LIGHT_DARK    1
#define DARK_LIGHT    1
#define LIGHT_DARK    0

#define TRANSACTION_DISABLED    0xFF
// -----------------------------------------------------------------------------
#define CLOSE       0
#define OPEN        1
// -----------------------------------------------------------------------------
#define ABSOLUTE    0
#define INCREMENTAL 1
// Maximum numebr of Colorante in Tinting Machine
#define MAX_COLORANT_NUMBER 16
// -----------------------------------------------------------------------------
#define MAX_COLORANT_NUM   16
#define N_SLAVES_BASE_ACT   5
#define N_SLAVES_COLOR_ACT 24
// 6 bytes are required to represent 48 bits
#define N_SLAVES_BYTES 6
// -----------------------------------------------------------------------------
// PUMP TYPES
#define NO_PUMP 0
#define PUMP_005 1
#define PUMP_020 2
#define PUMP_050 3
#define PUMP_300 4
#define PUMP_DOUBLE 5
// -----------------------------------------------------------------------------
// Dispensation algorithma 
# define ALG_STROKE     (0)
# define ALG_CONTINUOUS (1)
# define ALG_HIGH_RES_STROKE (4)
// -----------------------------------------------------------------------------
#define STATIC      0
#define DYNAMIC     1
// -----------------------------------------------------------------------------
#define COLORANT_ID_OFFSET  7
// -----------------------------------------------------------------------------
#define NUM_MAX_ERROR_TIMEOUT 20
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
# define ABS(x) ((x) >= (0) ? (x) : (-x))  
// -----------------------------------------------------------------------------
// Version helpers
#define VERSION_MAJOR(x) (LSB_MSW(x))
#define VERSION_MINOR(x) (MSB_LSW(x))
#define VERSION_PATCH(x) (LSB_LSW(x))
// -----------------------------------------------------------------------------
// colorAct
#define FILLING_SEQUENCE_200		1
#define FILLING_SEQUENCE_20_100_80	2
#define FILLING_SEQUENCE_20_180		3
#define FILLING_SEQUENCE_50_100_50	4
#define FILLING_SEQUENCE_50_150	    5

#define COLOR_ACT_STROKE_OPERATING_MODE     (0)
#define COLOR_ACT_CONTINUOUS_OPERATING_MODE (1)
#define COLOR_ACT_HIGH_RES_STROKE_OPERATING_MODE (2)

#define CONV_MIN_SEC 60
#define CONV_TIME_UNIT_MIN 10 //valore espresso in 10'

#define NO_TABLE_AVAILABLE 255
#define DELAY_RESHUFFLE_AFTER_SUPPLY_SEC  5 //sec

#define B1_BASE_IDX (B1_BASE_ID - 1)
#define B8_BASE_IDX (B8_BASE_ID - 1)
#define C1_COLOR_IDX (C1_COLOR_ID - 1)

#define N_MAX_SIMULTANEOUS_ACTS (4)
// -----------------------------------------------------------------------------
#define	PANEL_CLOSE		0
#define PANEL_OPEN		1

#define CARRIAGE_CLOSE	0
#define CARRIAGE_OPEN	1

#define	NO_TRANSITION	0
#define HIGH_LOW		1
#define LOW_HIGH		2
// -----------------------------------------------------------------------------
#define HALT() while(1)
// -----------------------------------------------------------------------------
#define	AUTOTEST_SMALL_VOLUME	1
#define AUTOTEST_MEDIUM_VOLUME	2
#define AUTOTEST_BIG_VOLUME     3
// -----------------------------------------------------------------------------

#ifndef SKIP_FAULT_1
enum {
  /* 0 */ FAULT_1_IDLE,
  /* 1 */ FAULT_1_WAIT_ENABLING,
  /* 2 */ FAULT_1_WAIT_ACTIVATION,
  /* 3 */ FAULT_1_ERROR,
  /* 4 */ FAULT_NEB_ERROR,
  /* 5 */ FAULT_PUMP_ERROR,
  /* 6 */ FAULT_RELE_ERROR,               
  /* 7 */ FAULT_GENERIC24V_ERROR,               
};
#endif

#define IS_IN1_BRUSH_OFF() (IN1_BRUSH == 0)
#define IS_IN2_BRUSH_OFF() (IN2_BRUSH == 0)
#define isFault_1_Conditions() (IS_IN1_BRUSH_OFF() && IS_IN2_BRUSH_OFF())
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
# define OUT24V_OFF()        \
do {                         \
	OUT_24V_IN = OFF;        \
} while (0)

# define OUT24V_ON()         \
do {                         \
	OUT_24V_IN = ON;         \
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
# define SPAZZOLA_OFF()     \
do {                        \
	OUT_24V_IN = OFF;       \
} while (0)

# define SPAZZOLA_ON()      \
do {                        \
	OUT_24V_IN = ON;        \
} while (0)
// ----------------------------
# define STEPPER_TABLE_OFF()  \
do {                          \
     HardHiZ_Stepper(MOTOR_TABLE); \
} while (0)

# define STEPPER_TABLE_ON()   \
do {                          \
    Run_Stepper(MOTOR_TABLE, TintingAct.High_Speed_Rotating_Table, CW); \
} while (0)
// ----------------------------
# define STEPPER_VALVE_OFF()  \
do {                          \
	StopStepper(MOTOR_VALVE); \
} while (0)

# define STEPPER_VALVE_ON()   \
do {                          \
	StartStepper(MOTOR_VALVE, TintingAct.Speed_Valve, CCW, LIGHT_DARK, VALVE_PHOTOCELL, 0);\
} while (0)
// ----------------------------
# define STEPPER_PUMP_OFF()   \
do {                          \
	StopStepper(MOTOR_PUMP);  \
} while (0)

# define STEPPER_PUMP_ON()   \
do {                         \
    StartStepper(MOTOR_PUMP, TintingAct.V_Accopp, DIR_SUCTION, LIGHT_DARK, HOME_PHOTOCELL, 0); \
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
# define RS_485_OFF()        \
do {                         \
	RS485_DE = OFF;          \
} while (0)

# define RS_485_ON()         \
do {                         \
	RS485_DE = ON;           \
} while (0)
//------------------------------------------------------------------------------
#ifdef DEBUG_MMT
// -----------------------------
# define PUMP_MOT_OFF()     \
do {                        \
	CS_PMP = OFF;         \
} while (0)

# define PUMP_MOT_ON()      \
do {                        \
	CS_PMP = ON;          \
} while (0)
// ----------------------------
# define TABLE_OFF()        \
do {                        \
	CS_BRD = OFF;           \
} while (0)

# define TABLE_ON()         \
do {                        \
	CS_BRD = ON;            \
} while (0)
// ----------------------------
# define VALVE_OFF()        \
do {                        \
	CS_EV = OFF;            \
} while (0)

# define VALVE_ON()         \
do {                        \
	CS_EV = ON;             \
} while (0)
// ----------------------------
# define PUMP_OFF()         \
do {                        \
	AIR_PUMP_IN = OFF;      \
} while (0)

# define PUMP_ON()          \
do {                        \
	AIR_PUMP_IN = ON;       \
} while (0)
// ----------------------------
# define RELAY_OFF()         \
do {                         \
	RELAY = OFF;             \
} while (0)

# define RELAY_ON()          \
do {                         \
	RELAY = ON;              \
} while (0)
// ----------------------------
# define BRUSH_1N1_OFF()     \
do {                         \
	IN1_BRUSH = OFF;         \
} while (0)

# define BRUSH_1N1_ON()      \
do {                         \
	IN1_BRUSH = ON;          \
} while (0)
// ----------------------------
# define BRUSH_1N2_OFF()     \
do {                         \
	IN2_BRUSH = OFF;         \
} while (0)

# define BRUSH_1N2_ON()      \
do {                         \
	IN2_BRUSH = ON;          \
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

# define NEBULIZER_OFF()     \
do {                         \
	NEB_IN = OFF;            \
} while (0)

# define NEBULIZER_ON()      \
do {                         \
	NEB_IN = ON;             \
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
# define BHL_OFF()           \
do {                         \
	LASER_BHL = OFF;         \
} while (0)

# define BHL_ON()            \
do {                         \
	LASER_BHL = ON;          \
} while (0)
// ----------------------------
# define RST_SHT31_OFF()     \
do {                         \
	RST_SHT31 = OFF;         \
} while (0)

# define RST_SHT31_ON()      \
do {                         \
	RST_SHT31 = ON;          \
} while (0)
// ----------------------------
   # define CE_TC72_OFF()    \
do {                         \
	CE_TC72 = OFF;           \
} while (0)

# define CE_TC72_ON()        \
do {                         \
	CE_TC72 = ON;            \
} while (0)
// ----------------------------
#else
#endif


// ----------------------------
# define isColorCmdStop()  		  (TintingAct.command.tinting_stop)
# define isColorCmdHome()  		  (TintingAct.command.tinting_home)
# define isColorCmdSupply()       (TintingAct.command.tinting_supply)
# define isColorCmdRecirc()       (TintingAct.command.tinting_recirc)
# define isColorCmdSetupParam()   (TintingAct.command.tinting_setup_param)
# define isColorCmdSetupOutput()  (TintingAct.command.tinting_setup_output)
# define isColorCmdSetupProcess() (TintingAct.command.tinting_setup_process)
# define isColorCmdIntr()         (TintingAct.command.tinting_intr)
# define isColorCmdStirring()     (TintingAct.command.tinting_stirring)
# define isColorCmdStopProcess()  (TintingAct.command.tinting_stop_process)
# define isColorCmdIdle()         (TintingAct.command.cmd == 0)
// -----------------------------------------------------------------------------
# define isHumidifierError()                                                   \
    ( (Status.level == TINTING_RH_ERROR_ST)             ||                     \
      (Status.level == TINTING_TEMPERATURE_ERROR_ST)    ||                     \
      (Status.level == TINTING_BAD_PAR_HUMIDIFIER_ERROR_ST)             ||     \
      (Status.level == TINTING_NEBULIZER_OVERCURRENT_THERMAL_ERROR_ST)  ||     \
      (Status.level == TINTING_NEBULIZER_OPEN_LOAD_ERROR_ST)            ||     \
      (Status.level == TINTING_PUMP_OVERCURRENT_THERMAL_ERROR_ST)       ||     \
      (Status.level == TINTING_PUMP_OPEN_LOAD_ERROR_ST) ||                     \
      (Status.level == TINTING_RELE_OVERCURRENT_THERMAL_ERROR_ST)       ||     \
      (Status.level == TINTING_RELE_OPEN_LOAD_ERROR_ST) )
// -----------------------------------------------------------------------------
#define isColorantActEnabled(x)                 \
  (isSlaveCircuitEn(x))

// I2C1
#ifndef I2C1_CONFIG_TR_QUEUE_LENGTH
        #define I2C1_CONFIG_TR_QUEUE_LENGTH 1
#endif

#define I2C1_TRANSMIT_REG                       I2C1TRN                 // Defines the transmit register used to send data.
#define I2C1_RECEIVE_REG                        I2C1RCV                 // Defines the receive register used to receive data.

// The following control bits are used in the I2C state machine to manage
// the I2C module and determine next states.
#define I2C1_WRITE_COLLISION_STATUS_BIT         I2C1STATbits.IWCOL      // Defines the write collision status bit.
#define I2C1_ACKNOWLEDGE_STATUS_BIT             I2C1STATbits.ACKSTAT    // I2C ACK status bit.

#define I2C1_START_CONDITION_ENABLE_BIT         I2C1CONLbits.SEN         // I2C START control bit.
#define I2C1_REPEAT_START_CONDITION_ENABLE_BIT  I2C1CONLbits.RSEN        // I2C Repeated START control bit.
#define I2C1_RECEIVE_ENABLE_BIT                 I2C1CONLbits.RCEN        // I2C Receive enable control bit.
#define I2C1_STOP_CONDITION_ENABLE_BIT          I2C1CONLbits.PEN         // I2C STOP control bit.
#define I2C1_ACKNOWLEDGE_ENABLE_BIT             I2C1CONLbits.ACKEN       // I2C ACK start control bit.
#define I2C1_ACKNOWLEDGE_DATA_BIT               I2C1CONLbits.ACKDT       // I2C ACK data control bit.

#define RESET_ON    0
#define RESET_WAIT  1
        
#define WAIT 0   

// Max Write Command Retry Number with Sensor
#define SLAVE_I2C_GENERIC_RETRY_MAX 5

#endif	/* DEFINE_H */

