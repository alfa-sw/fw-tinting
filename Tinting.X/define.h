/* 
 * File:   define.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 14.44
 */

#ifndef DEFINE_H
#define	DEFINE_H

#define TRUE 1
#define FALSE 0

#define DISABLE   0
#define ENABLE    1

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

#define HUMIDIFIER_DISABLE	0
#define HUMIDIFIER_ENABLE	1

#define HUMIDIFIER_TYPE_0	0 // SENSIRION SHT31
#define HUMIDIFIER_TYPE_1	1 // NO SENSOR - Process Humidifier 1.0

#define HUMIDIFIER_PERIOD   1200 // 20 min
#define HUMIDIFIER_DURATION 2    // 2 sec

#define AUTOCAP_OPEN_DURATION	10 // 10"
#define MAX_HUMIDIFIER_DURATION_AUTOCAP_OPEN 120 // 120"
#define AUTOCAP_OPEN_PERIOD		30 // 30"

#define TEMP_DISABLE		0
#define TEMP_ENABLE			1

#define TEMPERATURE_TYPE_0  0 // SENSIRION SHT31
#define TEMPERATURE_TYPE_1  1 // MICROCHIP TC72

#define TEMP_PERIOD			300 // 5 min
#define MIN_TEMP_PERIOD		10

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
// Default values for Pump
// Tolleranza sui passi madrevite in accoppiamento: 3.5mm
#define TOLL_ACCOPP 938
// Passi da fotocellula madrevite coperta a fotocellula ingranamento coperta: 6.5mm
#define STEP_ACCOPP 1740
// Passi a fotoellula ingranamento coperta per ingaggio circuito
#define STEP_INGR   402
// Passi per recupero giochi: 1.0mm
#define STEP_RECUP  268
// Passi a fotocellula madrevite coperta per posizione di home
#define PASSI_MADREVITE 3282
// Passi per raggiungere la posizione di start ergoazione in alta risoluzione
#define PASSI_APPOGGIO_SOFFIETTO 3200
// Velocità da fotocellula madrevite coperta a fotocellula ingranamento coperta (rpm)
#define V_ACCOPP    600
// Velocità a fotoellula ingranamento coperta per ingaggio circuito (rpm))
#define V_INGR      50
// Velocità per raggiungere la posizione di start ergoazione in alta risoluzione
#define V_APPOGGIO_SOFFIETTO   300
// Passi da posizione di home/ricircolo al centro della fotocelluila (valvola chiusa) a posizone di valvola aperta su fori grande (3mm) e piccolo(0.8mm))
#define STEP_VALVE_OPEN 148 // grande +148 (80°), piccolo -148 (-80°))
// Passi da posizione di home/ricircolo al centro della fotocelluila (valvola chiusa) a posizone di backstep (0.8mm)
#define STEP_VALVE_BACKSTEP 74 // backstep -74 (-40°)
// Max step to do to search CW and CCW Home Valve Position
#define MAX_STEP_VALVE_HOMING 166 //(+-90°) 
// Passi da posizione di home/ricircolo al centro della fotocelluila a transizione DARK/LIGHT verso foro di 3.0mm
#define STEP_PHOTO_VALVE_BIG_HOLE   6
// Passi da posizione di home/ricircolo al centro della fotocelluila a transizione DARK/LIGHT verso foro di 0.8mm
#define STEP_PHOTO_VALVE_SMALL_HOLE 6
// Velocità di apertura/chiusura valvola (rpm))
#define SPEED_VALVE  10
// N. steps in una corsa intera
#define N_STEPS_STROKE  1600
// Back step N. before to Open valve
#define PUMP_STEP_BACKSTEP  50
// Back Step Speed (rpm) before to Open Valve
#define PUMP_SPEED_BACKSTEP 200
// Max step to do to search in both directions Home Position
#define MAX_STEP_PUMP_HOMING (STEP_ACCOPP + STEP_INGR + STEP_RECUP + PASSI_APPOGGIO_SOFFIETTO + 400)   
// -----------------------------------------------------------------------------
// Default values for Rotating Table
// Passi corrispondenti ad un giro completa di 360° della tavola
#define STEPS_REVOLUTION 6342 // 6343 corretto
// Tolleranza in passi corrispondente ad una rotazione completa di 360° della tavola
#define STEPS_TOLERANCE_REVOLUTION 20
// Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento (12mm))
#define STEPS_REFERENCE 42 // 21.3 corretto
// Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
#define STEPS_TOLERANCE_REFERENCE   2
// Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito (6mm)
#define STEPS_CIRCUIT 18 // 19.2 corretto  
// Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito
#define STEPS_TOLERANCE_CIRCUIT 2
// Velocità massima di rotazione della tavola rotante (rpm))
#define HIGH_SPEED_ROTATING_TABLE 200
// Velocità minima di rotazione della tavola rotante (rpm)
#define LOW_SPEED_ROTATING_TABLE   20
// Distanza in passi tra il circuito di riferimento e la spazzola
#define STEPS_CLEANING  1000
// Velocità massima ammessa della Tavola Rotante (rpm))
#define MAX_TABLE_SPEED  300
// Velocità minima ammessa della Tavola Rotante (rpm))
#define MIN_TABLE_SPEED  10
// Passi nella rotazione CW in cui la pinna del Circuito oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_CIRCUIT_CW 12
// Passi nella rotazione CCW in cui la pinna del Circuito oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_CIRCUIT_CCW 12
// Passi nella rotazione CW in cui la pinna del Riferimento oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_REFERENCE_CW 20
// Passi nella rotazione CCW in cui la pinna del Riferimento oscura la Fotocellula fino al suo centro ottico   
#define STEP_PHOTO_TABLE_REFERENCE_CCW 20
// Offset in Passi prima di andare nella posizione di Home
#define STEP_PHOTO_TABLE_OFFSET 40
// Maximum Rotating Angle (°))
#define MAX_ROTATING_ANGLE 180
// Steps between Reference position and Circuit N°1
#define STEPS_REFERENCE_CIRC_1 198  
// Steps distance between 2 Circuits
#define STEPS_CIRCUITS STEPS_REVOLUTION / 16 

// -----------------------------------------------------------------------------
// Durata della Pulizia (sec))
#define CLEANING_DURATION 5
// Pausa della Pulizia (min))
#define CLEANING_PAUSE    30
// -----------------------------------------------------------------------------
// Motor Parameters
// Motor Types
#define MOTOR_TABLE 0
#define MOTOR_PUMP  1
#define MOTOR_VALVE 2
// Motor Resolution (1/1: 0, ½: 1, ¼: 2, 1/8: 3, 1/16: 4, 1/32: 5, 1/64: 6, 1/128: 7, 1/256: 8)
#define RESOLUTION_TABLE    1 // 1/2 steps  
#define RESOLUTION_PUMP     1 // 1/2 steps
#define RESOLUTION_VALVE    1 // 1/2 steps
// Phase Current (RMS) during ramp movement (= A x 10) 
#define RAMP_PHASE_CURRENT_TABLE    45 // 4.5 A  
#define RAMP_PHASE_CURRENT_PUMP     45 // 4.5 A
#define RAMP_PHASE_CURRENT_VALVE    45 // 4.5 A
// Phase Current (RMS) during constans speed movement (= A x 10) 
#define PHASE_CURRENT_TABLE 45 // 4.5 A  
#define PHASE_CURRENT_PUMP  45 // 4.5 A
#define PHASE_CURRENT_VALVE 45 // 4.5 A
// Holding Current (RMS) (= A x 10) 
#define HOLDING_CURRENT_TABLE 5 // 0.5 A  
#define HOLDING_CURRENT_PUMP  5 // 0.5 A
#define HOLDING_CURRENT_VALVE 5 // 0.5 A
// Acceleration (step/sec^2) during acceleration ramp
#define ACC_RATE_TABLE      10 // 10 step /sec^2  
#define ACC_RATE_PUMP       10 // 10 step /sec^2
#define ACC_RATE_VALVE      10 // 10 step /sec^2
// Deceleration (step/sec^2) during deceleration ramp
#define DEC_RATE_TABLE      10 // 10 step /sec^2  
#define DEC_RATE_PUMP       10 // 10 step /sec^2
#define DEC_RATE_VALVE      10 // 10 step /sec^2
// Maximum Pump Acceleration and Deceleration (step/sec^2)
#define MAX_ACC_RATE_PUMP   50 // 50 step /sec^2
#define MAX_DEC_RATE_PUMP   50 // 50 step /sec^2
// Alarms Enable mask (0 = Disable, 1 = Enabled)
#define ALARMS_TABLE        1  // Enabled  
#define ALARMS_PUMP         1  // Enabled
#define ALARMS_VALVE        1  // Enabled
// Alarms Bit Mask types
#define OVER_CURRENT_DETECTION  0x01 // bit0
#define THERMAL_SHUTDOWN        0x02 // bit1
#define THERMAL_WARNING         0x04 // bit2
#define UNDER_VOLTAGE_LOCK_OUT  0x08 // bit3
#define STALL_DETECTION         0x10 // bit4
// -----------------------------------------------------------------------------
// Types of Algorithm
#define ALG_SINGLE_STROKE          (0)
#define ALG_DOUBLE_STROKE          (1)
#define ALG_SYMMETRIC_CONTINUOUS   (2)
#define ALG_ASYMMETRIC_CONTINUOUS  (3)
#define HIGH_RES_STROKE  		   (4)
// -----------------------------------------------------------------------------
// MIN and MAX limit
#define MAX_SPEED   1200 // (rpm))
// -----------------------------------------------------------------------------
// Photocell Sensor
// Pump Homing Photocell
#define HOME_PHOTOCELL          0
// Coupling Photocell
#define COUPLING_PHOTOCELL      1 
// Valve Homing Photocell
#define VALVE_PHOTOCELL         2          
// Table Photocell
#define TABLE_PHOTOCELL         3             
// Can Presence Photocell
#define CAN_PRESENCE_PHOTOCELL  4 
// Panel Table
#define PANEL_TABLE             5 
// bases Carriage
#define BASES_CARRIAGE          6
// -----------------------------------------------------------------------------
#define FILTER      0
#define NO_FILTER   1
// -----------------------------------------------------------------------------
#define DARK    0
#define LIGHT   1
// -----------------------------------------------------------------------------
#define CW    0 
#define CCW   1
// -----------------------------------------------------------------------------
#define DIR_EROG      0 
#define DIR_SUCTION   1
// -----------------------------------------------------------------------------
#define DARK_LIGHT    0
#define LIGHT_DARK    1
// -----------------------------------------------------------------------------
#define CLOSE       0
#define OPEN        1
// -----------------------------------------------------------------------------
#define ABSOLUTE    0
#define INCREMENTAL 1
// Maximum numebr of Colorante in Tinting Machine
#define MAX_COLORANT_NUMBER 16
// -----------------------------------------------------------------------------
#define STATIC      0
#define DYNAMIC     1
// -----------------------------------------------------------------------------
#define COLORANT_ID_OFFSET  7
// -----------------------------------------------------------------------------

# define ABS(x) ((x) >= (0) ? (x) : (-x))  

# define NEBULIZER_OFF()	\
do {                        \
	NEB = OFF;              \
} while (0)

# define NEBULIZER_ON()     \
do {                        \
	NEB = ON;               \
} while (0)
// -----------------------------
# define RISCALDATORE_OFF() \
do {                        \
	RISCALD = OFF;          \
} while (0)

# define RISCALDATORE_ON()  \
do {                        \
	RISCALD = ON;           \
} while (0)
// -----------------------------
# define PUMP_OFF()         \
do {                        \
	PUMP = OFF;             \
} while (0)

# define PUMP_ON()          \
do {                        \
	PUMP = ON;              \
} while (0)
// ----------------------------
# define BRUSH_OFF()        \
do {                        \
	BRUSH = OFF;            \
} while (0)

# define BRUSH_ON()         \
do {                        \
	BRUSH = ON;             \
} while (0)
// ----------------------------
# define TABLE_OFF()        \
do {                        \
	TABLE = OFF;            \
} while (0)

# define TABLE_ON()         \
do {                        \
	TABLE = ON;             \
} while (0)
// ----------------------------
# define VALVE_OFF()        \
do {                        \
	VALVE = OFF;            \
} while (0)

# define VALVE_ON()         \
do {                        \
	VALVE = ON;             \
} while (0)
// ----------------------------
# define PUMP_OFF()         \
do {                        \
	PUMP = OFF;             \
} while (0)

# define PUMP_ON()          \
do {                        \
	PUMP = ON;              \
} while (0)
// ----------------------------

# define isColorCmdStop()  		  (TintingAct.command.tinting_stop)
# define isColorCmdHome()  		  (TintingAct.command.tinting_home)
# define isColorCmdSupply()       (TintingAct.command.tinting_supply)
# define isColorCmdRecirc()       (TintingAct.command.tinting_recirc)
# define isColorCmdSetupParam()   (TintingAct.command.tinting_setup_param)
# define isColorCmdSetupOutput()  (TintingAct.command.tinting_setup_output)
# define isColorCmdSetupProcess() (TintingAct.command.tinting_setup_process)
# define isColorCmdIntr()         (TintingAct.command.tinting_intr)
# define isColorCmdSetupClean()   (TintingAct.command.tinting_setup_clean)
// -----------------------------------------------------------------------------
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

