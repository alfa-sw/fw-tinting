/* 
 * File:   typedef.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 15.01
 */

#ifndef TYPEDEF_H
#define	TYPEDEF_H

#define MAX_COLORANT_NUM 16

/*========================== DEFINIZIONI GENERALI DI TIPO ================== */
typedef union {
  unsigned char bytes[2];
  unsigned short word;

  struct {
    /* User-operated inputs */
    unsigned short  StatusType0       : 1;  //LEV_SENS
    unsigned short  StatusType1       : 1;  //FO_CPR
    unsigned short  StatusType2       : 1;  //FO_VALV 
    unsigned short  StatusType3       : 1;  //FO_ACC
    unsigned short  StatusType4       : 1;  //FO_BRD
    unsigned short  StatusType5       : 1;  //FO_HOME
    unsigned short  StatusType6       : 1;  //FO_GEN1
    unsigned short  StatusType7       : 1;  //FO_GEN2
    unsigned short  StatusType8       : 1;  //INT_CAR
    unsigned short  StatusType9       : 1;  //INT_PAN
    unsigned short  StatusType10      : 1;  //IO_GEN1
    unsigned short  StatusType11      : 1;  //FO_GEN2
    unsigned short  StatusType12      : 1;  //BUTTON
    unsigned short  StatusType13      : 1;
    unsigned short  StatusType14      : 1; 
    unsigned short  StatusType15      : 1; 
} Bit;

  struct {
    unsigned char low;
    unsigned char high;
  } byte;
} DigInStatusType;

/*!  Union   */
typedef union __attribute__ ((packed))
{
  unsigned short allflags;
  struct
  {
    unsigned short unused:16;
  } bit;
} statusFlags_t;


typedef union __attribute__ ((packed))
{
  unsigned short uword;
  signed short   sword;
  unsigned char  byte[2];
} unionWord_t;


typedef union __attribute__ ((packed))
{
  unsigned long  udword;
  signed long    sdword;
  unsigned short word[2];
  unsigned char  byte[4];
} unionDWord_t;

typedef struct
{
  unsigned char level;
  unsigned char phase;
  unsigned char step;
  unsigned char errorCode;
} status_t;

typedef union {
  unsigned char bytes[2];
  unsigned short word;

  struct {
    /* User-operated inputs */
    unsigned short  HiZ       : 1;      
    unsigned short  BUSY      : 1;  
    unsigned short  SW_F      : 1;  
    unsigned short  SW_EVN    : 1;  
    unsigned short  DIR       : 1;  
    unsigned short  MOT_STATUS: 2;  
    unsigned short  CMD_ERROR : 1;      
    unsigned short  STCK_MOD  : 1;  
    unsigned short  UVLO      : 1;  
    unsigned short  UVLO_ADC  : 1;
    unsigned short  TH_STATUS : 2; 
    unsigned short  OCD       : 1;     
    unsigned short  unused    : 2; 
  } Bit;

} Stepper_Status;

typedef struct
{
  unsigned char Action;
  union __attribute__ ((packed)) PeripheralTypes_t
  {
	unsigned char bytePeripheral;
	
	struct {
      unsigned char RotatingTable    : 1;
      unsigned char Cleaner			 : 1;
      unsigned char WaterPump		 : 1;
      unsigned char Nebulizer_Heater : 1;
      unsigned char HeaterResistance : 1;
      unsigned char OpenValve_BigHole   : 1;
      unsigned char OpenValve_SmallHole : 1;
      unsigned char Rotating_Valve   : 1;      
	};	  
  } Peripheral_Types;
} PeripheralAct_t;

typedef struct
{
  unsigned char typeMessage;

  // tinting command 
  union __attribute__ ((packed))
  {
    unsigned int cmd;
    struct
    {
      unsigned char tinting_stop      	: 1;
      unsigned char tinting_home	    : 1;
      unsigned char tinting_supply      : 1;
      unsigned char tinting_recirc      : 1;
      unsigned char tinting_setup_param : 1;
      unsigned char tinting_setup_output  : 1;
      unsigned char tinting_stop_process : 1;
      unsigned char tinting_intr : 1;
      unsigned char unused : 8;
    };
  } command;

  union __attribute__ ((packed)) TintingFlags_t
  {
    unsigned long allFlags;
    unsigned char byteFlags[4];
    unsigned short wordFlags[2];
	
    struct {
      /* octet 1 */
      unsigned char tinting_stopped		: 1;
      unsigned char tinting_ready       : 1;
      unsigned char tinting_homing      : 1;
      unsigned char tinting_supply_run  : 1;
      unsigned char tinting_recirc_run  : 1;
      unsigned char tinting_cleaning    : 1;
      unsigned char tinting_supply_end  : 1;
      unsigned char tinting_recirc_end  : 1;

      /* octet 2 */
	  unsigned char tinting_bad_humidifier_par_error  : 1;	  
	  unsigned char tinting_tout_error     : 1;	  
      unsigned char tinting_RH_error       : 1;
      unsigned char tinting_Temperature_error : 1;
      unsigned char tinting_table_software_error    : 1;
      unsigned char tinting_pump_homing_pos_error	: 1;	  
	  unsigned char tinting_valve_homing_pos_error  : 1;
	  unsigned char tinting_table_homing_pos_error  : 1;

      /* octet 3 */
	  unsigned char tinting_pump_reset_error	: 1;
	  unsigned char tinting_valve_reset_error	: 1;
	  unsigned char tinting_table_reset_error	: 1;
	  unsigned char tinting_pump_supply_calc_error	: 1;
	  unsigned char tinting_pos0_read_light_error	: 1;
	  unsigned char tinting_end_stroke_read_dark_error	: 1;
	  unsigned char tinting_pump_motor_open_load_error  : 1;
	  unsigned char tinting_valve_motor_open_load_error	: 1;

      /* octet 4 */
	  unsigned char tinting_table_motor_open_load_error	: 1;
	  unsigned char tinting_pump_motor_overcurrent_error	: 1;
	  unsigned char tinting_valve_motor_overcurrent_error	: 1;
	  unsigned char tinting_table_motor_overcurrent_error	: 1;
	  unsigned char tinting_pump_motor_home_back_error	: 1;
	  unsigned char tinting_valve_motor_home_back_error	: 1;

	  unsigned char tinting_jump_to_boot   : 1;	  
	  unsigned char tinting_hum_par_rx	   : 1;
   };
  } TintingFlags;
  
  union __attribute__ ((packed)) TintingFlags_1_t
  {
    unsigned long allFlags;
    unsigned char byteFlags[4];
    unsigned short wordFlags[2];
	
    struct {
      /* octet 1 */
      unsigned char tinting_timeout_table_move_error : 1;
      unsigned char tinting_table_search_position_reference_error : 1;
      unsigned char tinting_lack_circuits_position_error : 1;
      unsigned char tinting_timeout_self_learning_procedure_error : 1;
      unsigned char tinting_self_learning_procedure_error: 1;
      unsigned char tinting_table_move_error : 1;
      unsigned char tinting_table_mismatch_position_error : 1;
      unsigned char tinting_bad_pump_param_error : 1;
      /* octet 2 */
      unsigned char tinting_bad_table_param_error : 1;	  
      unsigned char tinting_peripheral_on : 1;
	  unsigned char tinting_pump_photo_ingr_read_light_error : 1;
	  unsigned char tinting_pump_photo_home_read_dark_error : 1;
	  unsigned char tinting_valve_pos0_read_light_error : 1;
	  unsigned char tinting_pump_software_error : 1;
	  unsigned char tinting_table_test_error : 1;
      unsigned char tinting_bad_peripheral_param_error : 1;
      /* octet 3 */
      unsigned char CRCCircuitStepsPosFailed : 1;
	  unsigned char unused_byte_3 : 7;
      /* octet 4 */
      unsigned char tinting_table_self_recognition : 1;
      unsigned char tinting_table_positioning : 1;
      unsigned char tinting_table_test : 1;
      unsigned char tinting_table_setup_output_valve : 1;
      unsigned char tinting_table_steps_positioning : 1;
      unsigned char tinting_table_setup_output : 1;
	  unsigned char unused_byte_4 : 2;
   };
  } TintingFlags_1;
  
  // Humidifier
  unsigned char Autocap_Status;
  
  unsigned char Read_Table_Position;

  unsigned char BasesCarriageOpen;
  
  // 0 --> Humidifier Disabled
  // 1 --> Humidifier Enabled
  unsigned char Humidifier_Enable;
  // Type of R/H Sensor: 
  // 0 --> Sensirion SHT30
  // 1 --> ..................
  unsigned char Humdifier_Type;
  // PWM value on HUMIDIFIER_TYPE_2
  unsigned char Humidifier_PWM;
  // Process Period (sec))
  unsigned long Humidifier_Period;
  // Multiplier Coefficient
  unsigned long Humidifier_Multiplier;
  // Nebulizer Duration with Autocap Open (sec)
  // 0 --> Nebulizer always OFF
  unsigned long AutocapOpen_Duration;
  // Nebulizer Period with Autocap Open (sec)
  // 0 --> Nebulizer always ON
  unsigned long AutocapOpen_Period;
  // 0 --> Humidifier Disabled
  // 1 --> Humidifier Enabled
  unsigned char Temp_Enable;
  // Type of R/H Sensor: 
  // 0 --> Sensirion SHT30
  // 1 --> ..................
  unsigned char Temp_Type;
  // Dosing Temperature Period (sec)  
  unsigned long Temp_Period;
  // Low Temperature Threshold (°C) 
  unsigned char Temp_T_LOW;
  // High Temperature Threshold (°C) 
  unsigned char Temp_T_HIGH; 
  // Heater Temperature Threshold (°C) 
  unsigned char Heater_Temp;
  // Hystersis Interval (°C)
  unsigned char Heater_Hysteresis;
  
  unsigned long Temperature;
  unsigned long RH;  
  unsigned long Dosing_Temperature;
  unsigned char WaterLevel_state;
  unsigned char CriticalTemperature_state;
  
  // Peripherals State
  unsigned char RotatingTable_state;
  unsigned char Cleaner_state;
  unsigned char WaterPump_state;
  unsigned char Nebulizer_Heater_state;
  unsigned char HeaterResistance_state;  
  unsigned char OpenValve_BigHole_state;  
  unsigned char OpenValve_SmallHole_state; 
  unsigned char Rotating_Valve_state;
  
  // Microswitch status    
  unsigned char BasesCarriage_state;
  // Circuit Engaged
  unsigned char Circuit_Engaged;
  // Rotating Table position with respect to Reference circuit
  signed long Steps_position;
  // Home Photocell status
  unsigned char Home_photocell;
  // Coupling Photocell status
  unsigned char Coupling_photocell;
  // Valve Photocell status
  unsigned char Valve_photocell;
  // Valve Open Photocell status  
  unsigned char ValveOpen_photocell;
  // Rotating Table Photocell status
  unsigned char Table_photocell;
  // Can presence Photocell status  
  unsigned char CanPresence_photocell;
  // Table panel open status  
  unsigned char PanelTable_state;
  // Circuit Steps Position with respect to Reference
  signed long Circuit_step_pos[MAX_COLORANT_NUM];
  // Circuit Steps Position with respect to Reference found in self learning procedure CW and CCW
  signed long Circuit_step_pos_cw[MAX_COLORANT_NUM];
  signed long Circuit_step_pos_ccw[MAX_COLORANT_NUM];
  // Theorical Circuit Steps Position with respect to Reference
  signed long Circuit_step_theorical_pos[MAX_COLORANT_NUM];
  // Color index. Range:  8 (= C1) ? 31 (= C24)
  unsigned char Color_Id;
  // Max step N. in one Full Stroke (also Continuous)
  unsigned long N_step_full_stroke;
  // Step N. in one Dosing (also COntinuous) or Ricirculation stroke
  unsigned long N_step_stroke;
  // Dosing (also Continuous) or Ricirculation Speed (rpm))
  unsigned long Speed_cycle;
  // N. Dosing (also Continuous) or Ricirculation strokes
  unsigned long N_cycles;
  // Dosing Algorithm
  unsigned char Algorithm;
  // Back Step Enable (also Continuous)
  unsigned char En_back_step;
  // Back step N.  (also Continuous)
  unsigned long N_step_back_step_2;
  // Back Step Speed (rpm) (also Continuous)
  unsigned long Speed_back_step_2;
  // Minimum stroke before Valve Open (also Continuous)
  unsigned long N_step_backlash;
  // Waiting Time with motor stopped before Valve Close (also Continuous)
  unsigned long Delay_EV_off;
  // Suction Speed (also Continuous)
  unsigned long Speed_suction;
  // Stirring Duration after Dispensing (also Continuous)
  unsigned char Delay_resh_after_supply;
  // Waiting Time between 2 different strokes in Ricirculation (sec)
  unsigned char Recirc_pause;  
  
  // Continuous Start Step Position
  unsigned long PosStart;
  // Continuous  Stop Step Position 
  unsigned long PosStop;
  // Continuous Dosing Speed (rpm))
  unsigned long Speed_cycle_supply;
  // Continuous Dosing Cycles
  unsigned long N_CicliDosaggio;
  
  // Passi da fotocellula madrevite coperta a fotocellula ingranamento coperta
  unsigned long Step_Accopp;
  // Passi a fotoellula ingranamento coperta per ingaggio circuito
  unsigned long Step_Ingr;
  // Passi per recupero giochi
  unsigned long Step_Recup;
  // Passi a fotocellula madrevite coperta per posizione di home
  unsigned long Passi_Madrevite;
  // Passi per raggiungere la posizione di start ergoazione in alta risoluzione
  unsigned long Passi_Appoggio_Soffietto;
  // Velocità da fotocellula madrevite coperta a fotocellula ingranamento coperta
  unsigned long V_Accopp;
  // Velocità a fotoellula ingranamento coperta per ingaggio circuito
  unsigned long V_Ingr;
  // Velocità per raggiungere la posizione di start ergoazione in alta risoluzione
  unsigned long V_Appoggio_Soffietto;
  // Passi da posizione di home/ricircolo (valvola chiusa) a posizone di valvola aperta su fori grande (3mm) e piccolo(0.8mm))
  unsigned long Step_Valve_Open;
  // Passi da posizione di home/ricircolo (valvola chiusa) a posizone di backstep (0.8mm))
  unsigned long Step_Valve_Backstep;
  // Velocità di apertura/chiusura valvola
  unsigned char Speed_Valve;
  // N. steps in una corsa intera
  unsigned long N_steps_stroke;  
  // Back step N. before to Open valve in Small and Big Hole
  unsigned long N_step_back_step_Small_Hole;
  unsigned long N_step_back_step_Big_Hole;  
  // Back Step Speed (rpm) before to Open Valve in Small and Big Hole
  unsigned long Speed_back_step_Small_Hole;
  unsigned long Speed_back_step_Big_Hole;
  // Waiting Time 
  unsigned long Delay_Before_Valve_Backstep;
  // Parametri attualmente non utilizzati
  unsigned long Free_param_1;
  unsigned long Free_param_2;
  // Type of Single Stroke: 0 --> Camera piena - 1 --> Camera Vuota 
  unsigned char SingleStrokeType;  
  unsigned short StrokeType;    
  // Coloranti presenti sulla Tavola rotante
  unsigned char Colorant_1;
  unsigned char Colorant_2;
  unsigned char Colorant_3;
  // Tabella dei Coloranti abilitati sulla Tavola
  unsigned char Table_Colorant_En[MAX_COLORANT_NUM];      
  
  // Passi corrispondenti ad un giro completa di 360° della tavola
  unsigned long Steps_Revolution;
  // Tolleranza in passi corrispondente ad una rotazione completa di 360° della tavola
  unsigned long Steps_Tolerance_Revolution;
  // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
  unsigned long Steps_Reference;
  // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato il riferimento
  unsigned long Steps_Tolerance_Reference;
  // Passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito 
  unsigned long Steps_Circuit;
  // Tolleranza sui passi in cui la fotocellula presenza circuito rimane coperta quando è ingaggiato un generico circuito
  unsigned long Steps_Tolerance_Circuit;
  // Velocità massima di rotazione della tavola rotante
  unsigned long High_Speed_Rotating_Table;
  // Velocità minima di rotazione della tavola rotante
  unsigned long Low_Speed_Rotating_Table;
  // N°. di giri della Tavola per effettuare lo Stirring
  unsigned long Steps_Stirring;
  // Type of Table Circuit Positions compilation method: STATIC or DYNAMIC
  unsigned char Table_Step_position;
  
  // Cleaning Duration (sec))
  unsigned long Cleaning_duration;
  // Cleaning Pause (min))
  unsigned long  Cleaning_pause;  
  // Angolo di rotazione della Tavola Rotante rispetto alla posizone di ingaggio (°))
  unsigned long Refilling_Angle;
  // Direzione rotazione (CW o CCW))
  unsigned char Direction; 
  // Soglia di Passi di effettuare in 1 movimentazione tra 'High_Speed_Rotating_Table' e 'Low_Speed_Rotating_Table'
  signed long Steps_Threshold;  
    
  // Tipologia di movimentazione richiesta: assoluta o incrementale
  unsigned char Rotation_Type;
  // Numero di passi di cui la Tavola deve ruotare
  unsigned long Steps_N;
  
  // Tipo di Uscita
  unsigned char Output_Type;
  // Enable/Disable Output
  unsigned char Output_Act;  

} TintingAct_t;

typedef struct
{
  // Circuit Steps Position with respect to Reference
  signed long Circ_Pos[MAX_COLORANT_NUM];
} CircStepPosAct_t;


#endif	/* TYPEDEF_H */

