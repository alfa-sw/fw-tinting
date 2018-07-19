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
// Passi da fotocellula madrevite coperta a fotocellula ingranamento coperta
#define STEP_ACCOPP 2176
// Passi a fotoellula ingranamento coperta per ingaggio circuito
#define STEP_INGR   482
// Passi per recupero giochi
#define STEP_RECUP  248
// Passi a fotocellula madrevite coperta per posizione di home
#define PASSI_MADREVITE 3110
// Passi per raggiungere la posizione di start ergoazione in alta risoluzione
#define PASSI_APPOGGIO_SOFFIETTO 3200
// Velocità da fotocellula madrevite coperta a fotocellula ingranamento coperta (rpm)
#define V_ACCOPP    100
// Velocità a fotoellula ingranamento coperta per ingaggio circuito (rpm))
#define V_INGR      50
// Velocità per raggiungere la posizione di start ergoazione in alta risoluzione
#define V_APPOGGIO_SOFFIETTO   300
// Passi da posizione di home (valvola chiusa) a posizone di valvola aperta su foro grande
#define STEP_VALVE_OPEN_BIG    34
// Passi da posizione di home (valvola chiusa) a posizone di valvola aperta su foro piccolo
#define STEP_VALVE_OPEN_SMALL  98
// Velocità di apertura/chiusura valvola (rpm))
#define SPEED_VALVE  10
// N. steps in una corsa intera
#define N_STEPS_STROKE  1600
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
// -----------------------------------------------------------------------------
// Durata della Pulzia (sec))
#define CLEANING_DURATION 5
// Pausa della Pulizia (min))
#define CLEANING_PAUSE    30
// -----------------------------------------------------------------------------

# define NEBULIZER_OFF()	\
do {                        \
	NEB = OFF;              \
} while (0)

# define NEBULIZER_ON()     \
do {                        \
	NEB = ON;               \
} while (0)

# define AIR_PUMP_OFF()     \
do {                        \
	PUMP = OFF;             \
} while (0)

# define AIR_PUMP_ON()      \
do {                        \
	PUMP = ON;              \
} while (0)

# define RISCALDATORE_OFF() \
do {                        \
	RISCALD = OFF;          \
} while (0)

# define RISCALDATORE_ON()  \
do {                        \
	RISCALD = ON;           \
} while (0)

# define isColorCmdStop()  		  (TintingAct.command.tinting_stop)
# define isColorCmdHome()  		  (TintingAct.command.tinting_home)
# define isColorCmdSupply()       (TintingAct.command.tinting_supply)
# define isColorCmdRecirc()       (TintingAct.command.tinting_recirc)
# define isColorCmdSetupParam()   (TintingAct.command.tinting_setup_param)
# define isColorCmdSetupOutput()  (TintingAct.command.tinting_setup_output)
# define isColorCmdSetupProcess() (TintingAct.command.tinting_setup_process)
# define isColorCmdSetupIntr()    (TintingAct.command.tinting_setup_intr)
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

