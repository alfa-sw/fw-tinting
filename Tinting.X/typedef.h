/* 
 * File:   typedef.h
 * Author: michele.abelli
 *
 * Created on 16 luglio 2018, 15.01
 */

#ifndef TYPEDEF_H
#define	TYPEDEF_H

// -----------------------------------------------------------------------------
#define  N_CALIB_POINTS 10 // No. points in calibration curves (vol, steps) 
#define  N_CALIB_CURVE  5  // No. curves 
#define  N_SLAVES_COLOR_ACT 24
/*========================== DEFINIZIONI GENERALI DI TIPO ================== */
#define MAX_COLORANT_NUM 16
#define N_SLAVES_BYTES 6
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
    unsigned short  StatusType11      : 1;  //IO_GEN2
    unsigned short  StatusType12      : 1;  //BUTTON
    unsigned short  StatusType13      : 1;  //
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
	  unsigned char tinting_brush_open_load_error : 1;
	  unsigned char tinting_brush_overcurrent_thermal_error : 1;
	  unsigned char tinting_nebulizer_open_load_error : 1;
	  unsigned char tinting_nebulizer_overcurrent_thermal_error :1;
	  unsigned char tinting_pump_open_load_error : 1;
	  unsigned char tinting_pump_overcurrent_thermal_error : 1;
	  unsigned char tinting_rele_open_load_error :1;
	  unsigned char tinting_rele_overcurrent_thermal_error : 1;
      /* octet 4 */
      unsigned char tinting_table_self_recognition : 1;
      unsigned char tinting_table_positioning : 1;
      unsigned char tinting_table_test : 1;
      unsigned char tinting_table_setup_output_valve : 1;
      unsigned char tinting_table_steps_positioning : 1;
      unsigned char tinting_table_setup_output : 1;
	  unsigned char tinting_generic24v_open_load_error : 1;
	  unsigned char tinting_generic24v_overcurrent_thermal_error : 1;
   };
  } TintingFlags_1;
  
    union __attribute__ ((packed)) TintingFlags_2_t
  {
    unsigned long allFlags;
    unsigned char byteFlags[4];
    unsigned short wordFlags[2];
	
    struct {
      /* octet 1 */
      unsigned char tinting_pump_motor_thermal_shutdown_error  : 1;
      unsigned char tinting_valve_motor_thermal_shutdown_error : 1;
      unsigned char tinting_table_motor_thermal_shutdown_error : 1;
      unsigned char tinting_pump_motor_under_voltage_error     : 1;
      unsigned char tinting_valve_motor_under_voltage_error    : 1;
      unsigned char tinting_table_motor_under_voltage_error    : 1;
	  unsigned char tinting_eeprom_colorants_steps_position_crc_error : 1;
      unsigned char tinting_table_photo_read_light_error : 1;	
      /* octet 2 */
      unsigned char tinting_stirring_run : 1;	
      unsigned char tinting_valve_open_read_dark_error :1;
	  unsigned char tinting_valve_open_read_light_error :1;
	  unsigned char tinting_pump_photo_ingr_read_dark_error :1;
	  unsigned char tinting_panel_table_error :1;
	  unsigned char tinting_brush_read_light_error :1;
	  unsigned char tinting_bad_clean_param_error :1;
      unsigned char unused2 : 1;	  
      /* octet 3 */
      unsigned char unused3 : 8;	  

      /* octet 4 */
      unsigned char unuse4 : 8;	  
   };
  } TintingFlags_2;

  // Humidifier
  unsigned char Autocap_Status;
  
  unsigned char Read_Table_Position;

  unsigned char BasesCarriageOpen;
    
  unsigned long Temperature;
  unsigned long RH;  
  unsigned long Dosing_Temperature;
  unsigned char WaterLevel_state;
  unsigned char CriticalTemperature_state;
  
  // Peripherals State
  unsigned char RotatingTable_state;
  unsigned char WaterPump_state;
  unsigned char Nebulizer_Heater_state;
  unsigned char HeaterResistance_state;  
  unsigned char Brush_state;  
  unsigned char OpenValve_BigHole_state;  
  unsigned char OpenValve_SmallHole_state; 
  unsigned char Rotating_Valve_state;
  unsigned char Last_Cmd_Reset;
  
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
  // Autocap Closed Photocell status
  unsigned char Autocap_Closed_photocell;
  // Autocap Opened Photocell status
  unsigned char Autocap_Opened_photocell;
  // Brush Photocell status
  unsigned char Brush_photocell;
  // Can presence Photocell status  
  unsigned char CanPresence_photocell;
  // Table panel open status  
  unsigned char PanelTable_state;
  // Photocells status 
  unsigned char Photocells_state;
  // Circuit Steps Position with respect to Reference
  signed long Circuit_step_pos[MAX_COLORANT_NUM];
  // Circuit Steps Position with respect to Reference found in self learning procedure CW and CCW
  signed long Circuit_step_pos_cw[MAX_COLORANT_NUM];
  signed long Circuit_step_pos_ccw[MAX_COLORANT_NUM];
  // Theorical Circuit Steps Position with respect to Reference
  signed long Circuit_step_theorical_pos[MAX_COLORANT_NUM];
  // Color index. Range:  8 (= C1) ? 31 (= C24)
  unsigned char Color_Id;
  unsigned char Id_Punctual_Cleaning;
  unsigned char NextColor_Id;
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
  
  // cleaning status bitmask
  unsigned long Cleaning_status;
  // Cleaning Duration (sec))
  unsigned short Cleaning_duration;
  // Cleaning Pause (min))
  unsigned short  Cleaning_pause;  
  // Colorants Cleaning Enabling Mask 
  unsigned char Cleaning_Col_Mask[MAX_COLORANT_NUM];  
  unsigned char Clean_Mask[MAX_COLORANT_NUM];
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

  // Autotest
  unsigned short Autotest_Status;
  unsigned long Autotest_Total_Cycles_Number;
  unsigned long Autotest_Pause;
  unsigned long Autotest_Ricirculation_Time;
  unsigned long Autotest_Small_Volume;
  unsigned long Autotest_Medium_Volume;
  unsigned long Autotest_Big_Volume;
  unsigned long Autotest_Stirring_Time;
  unsigned short Autotest_Start[N_SLAVES_COLOR_ACT]; 
  unsigned short Autotest_Cleaning_Status;
  unsigned short Autotest_Heater_Status;
} TintingAct_t;

typedef struct
{
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
  
} TintingHumidifier_t;

typedef struct
{
  unsigned long Step_Accopp;
  unsigned long Step_Ingr;
  unsigned long Step_Recup;
  unsigned long Passi_Madrevite;
  unsigned long Passi_Appoggio_Soffietto;
  unsigned long V_Accopp;
  unsigned long V_Ingr;
  unsigned long V_Appoggio_Soffietto;
  unsigned long Delay_Before_Valve_Backstep;
  unsigned long Step_Valve_Backstep;
  unsigned char Speed_Valve;
  unsigned long N_steps_stroke;  
  unsigned long Free_param_1;
  unsigned long Free_param_2;
} TintingPump_t;

typedef struct
{
    unsigned long Steps_Revolution;
    unsigned long Steps_Tolerance_Revolution;
    unsigned long Steps_Reference;
    unsigned long Steps_Tolerance_Reference;
    unsigned long Steps_Circuit;
    unsigned long Steps_Tolerance_Circuit;
    unsigned long High_Speed_Rotating_Table;
    unsigned long Low_Speed_Rotating_Table;
    unsigned long Steps_Stirring;          
} TintingTable_t;

typedef struct
{
  unsigned short Cleaning_duration;
  unsigned short Cleaning_pause;
  unsigned char  Cleaning_Colorant_Mask[N_SLAVES_BYTES];
} TintingCleaning_t;
/**
 * GUI communication data structure
 ******************************************/
typedef struct
{
  unsigned char command;
  unsigned char typeMessage;
  /*** Status message contents
  ******************************************/
  // covers reserve bitmask
  unsigned char Cover_res;
  // covers availability bitmask 
  unsigned char Cover_avail;
  // coverse enabled bitmask
  unsigned char Cover_enabled;
  
  // containers reserve bitmask
  unsigned char Container_res;
  // containers availability bitmask
  unsigned char Container_avail;
  // containers enabled bitmask
  unsigned char Container_enabled;

  // color reserve bitmask
  unsigned long Color_res;

  // container presence bit (1 = PRESENT) 
  unsigned char Container_presence;
  // autocap status bit: 0 = CLOSED, 1 = OPEN
  unsigned char Autocap_status;
  // can lifter status bit: 0 = RETRACTED, 1 = EXTENDED
  unsigned char Canlift_status;  
  // doors status bit (1 = OPEN)
  unsigned char Doors_status;

  // slave activity status
  unsigned long Slave_status[2];

  // circuit engaged
  unsigned char Circuit_Engaged;
  // rotating table position with respect to Reference circuit
  signed long Steps_position;
  // autotest cycles completed
  unsigned long Autotest_Cycles_Number;
  
  // slave comm status (DEBUG only)
  unsigned long slave_comm[2];  
  unsigned char info_page; // 0 .. 3
  unsigned char reset_mode; // 0 = cold reset, 1 = warm reset   
  // Circuit Bases and Colorants used for Dispensing
  unsigned char used_colors_number; 
  // Circuits id (Bases and Colorants) used for Dispensing
  unsigned char used_colors_id[N_SLAVES_COLOR_ACT];
  // Discharge type (not used))
  unsigned char unloadType; 
  unsigned char dispenserType;
  unsigned char diag_motion_autocap;
  // Maximum simultaneous dispensing Colorants
  unsigned char simultaneous_colors_max;
  // Current simultaneous circuit number Dispensing
  unsigned char simultaneous_colors_nr;  
  // Simultaneous Dispensing cirucit mask
  unsigned long simultaneous_colors_status;  
  // Enable/Disable can presence during Dispening
  unsigned char check_can_dispensing;   
  // Dispensing Volume (Bases and Colorants)  
  unsigned long colors_vol[N_SLAVES_COLOR_ACT];  
  unsigned char id_color_circuit;
  // Calibration Curve ID
  unsigned char id_calib_curve; 
  // Circuits enable mask
  unsigned long diag_color_en; 
  // Type of pumps associated with every circuit (base or colorant)  
  unsigned long circuit_pump_types[N_SLAVES_COLOR_ACT]; 
  // Slave Enable mask
  unsigned char slaves_en[N_SLAVES_BYTES]; 
  // Number of circuits (Base or Colorants) used per Dispensing)
  unsigned char dispensation_colors_number;  
  // Circuits (Bases or Colorants) used for Dispensing   
  unsigned char dispensation_colors_id[N_SLAVES_COLOR_ACT];  
  // Ricirculation before Dispensing circuit mask
  unsigned long recirc_before_supply_status;  
  // recirculation status bitmask 
  unsigned long recirc_status;
  // stirring status bitmask
  unsigned long stirring_status;
} _procGUI_t;
/**
 * Colorant circuits parameters (0x1E bytes x entry)
 */
typedef struct _color_supply_par_t color_supply_par_t;
struct __attribute__ ((packed)) _color_supply_par_t {
  // Linear stroke extension [#steps] 
  unsigned short n_step_stroke;
  // Target point for continuous operation [#steps] */
  unsigned short n_step_continuous_end;
  // Resting position [#steps] 
  unsigned short n_step_home;
  // No-fly zone [#steps] 
  unsigned short n_step_backlash;
  // Waiting time after closing EV [ms] 
  unsigned short delay_EV_off;
  // Maximum dispensable volume per stroke [ml x 10^4] 
  unsigned long  vol_mu;
  // Minimum volume for continuous mode activation [ml x 10^4] 
  unsigned long  vol_mc;
  // Suction speed [rpm] 
  unsigned short speed_suction;
  // Recirculation speed [rpm] 
  unsigned short speed_recirc;
  // Recirculation duration [m] 
  unsigned char  recirc_duration;
  // Wait time between two recirculation cycles 
  unsigned char  recirc_pause;
  // Stirring duration [s] 
  unsigned short reshuffle_duration;
  // PWM % for stirring motor (unused) 
  unsigned char  reshuffle_pwm_pct;
  // Pre-dispensation recirculation cycles */
  unsigned char  recirc_before_dispensation_n_cycles;
  // Recirculation wait window time [m x 10] 
  unsigned char recirc_window;
  // Stirring wait window time [m x 10] 
  unsigned char reshuffle_window;
};
/**
 * Calibration curve parameters (0x60 bytes x entry)
 */
typedef struct _calib_curve_par_t calib_curve_par_t;
struct __attribute__ ((packed)) _calib_curve_par_t {
  // calibration curve speed
  unsigned short speed_value;
  // validity range lower bound [cc x 10^4] 
  unsigned long vol_min;
  // validity range upper bound [cc x 10^4] 
  unsigned long vol_max;
  unsigned char algorithm;
  // back-step configuration: enable, n_step, speed
  unsigned char en_back_step;
  unsigned short n_step_back_step;
  unsigned short speed_back_step;

  // Calibration point: < n_step, vol_cc > 
  unsigned long n_step[N_CALIB_POINTS];
  unsigned long vol_cc[N_CALIB_POINTS];
}; // 96 bytes wide

typedef struct {
  unsigned long n_step_cycle;
  unsigned short speed_cycle;
  unsigned short n_cycles;
} dispensationAct_t;

typedef struct
{
  unsigned char typeMessage;

  unsigned char algorithm;
# define COLOR_ACT_ALGORITHM_SINGLE_STROKE         (0)
# define COLOR_ACT_ALGORITHM_DOUBLE_STROKE         (1)
# define COLOR_ACT_ALGORITHM_SYMMETRIC_CONTINUOUS  (2)
# define COLOR_ACT_ALGORITHM_ASYMMETRIC_CONTINUOUS (3)
# define COLOR_ACT_ALGORITHM_HIGH_RES_STROKE	   (4)
# define COLOR_ACT_ALGORITHM_DOUBLE_GROUP			 (5)
# define COLOR_ACT_ALGORITHM_DOUBLE_GROUP_CONTINUOUS (6)

  unsigned long vol_t;

  unsigned short n_cycles;
  unsigned short n_cycles_supply;

  unsigned short delay_EV_off;

  unsigned long n_step_cycle;
  unsigned long n_step_cycle_supply;
  
  unsigned short speed_cycle;
  unsigned short speed_cycle_supply;

  unsigned char  recirc_pause; // sec
  unsigned char  delay_resh_after_supply;// sec

  unsigned short n_step_stroke;
  unsigned short n_step_continuous_end;

  unsigned short n_step_backlash;

  unsigned char  en_back_step;
  unsigned short n_step_back_step;
  unsigned short speed_back_step;

  unsigned short speed_suction;
  // Home Photocell status
  unsigned char  photoc_home; 
  // uswitch reserve status
  unsigned char  usw_reserve; 
  // uswitch reserve status without stability 60" filter
  unsigned char  usw_reserveDiag;	
  unsigned long  volume;
  unsigned long  totalVolume;
  // Parameters to send during continous dispensation
  unsigned short posStart;
  unsigned short posStop;
  unsigned short numCicliDosaggio;
  // Single Stroke Double Group
  unsigned short speed_cycle_channel_A;
  unsigned short speed_cycle_channel_B;
  unsigned short n_step_cycle_channel_A;
  unsigned short n_step_cycle_channel_B;
  unsigned short n_cycles_channel_A;
  unsigned short n_cycles_channel_B;
  // Continuous Double Group
  unsigned short speed_homing;	
  unsigned short posStart_A;	
  unsigned short posStart_B;	
  unsigned short posStop_A;	
  unsigned short posStop_B;	
  unsigned short speedContinous_A;	
  unsigned short speedContinous_B;	
  unsigned short numCicliDosaggio_A;	
  unsigned short numCicliDosaggio_B;	
  unsigned long  n_step_cycle_A;	
  unsigned long  n_step_cycle_B; 	
  unsigned short speed_cycle_A;	
  unsigned short speed_cycle_B;	
  unsigned short n_cycles_A;	
  unsigned short n_cycles_B;

  unsigned short N_step_back_step_Small_Hole;  
  unsigned short N_step_back_step_Big_Hole;
  unsigned short Speed_back_step_Small_Hole;  
  unsigned short Speed_back_step_Big_Hole; 
  unsigned char  SingleStrokeType;  

  // Color circuit command
  union __attribute__ ((packed))
  {
    unsigned char cmd;
    struct
    {
      unsigned char stop      : 1;
      unsigned char homing    : 1;
      unsigned char supply    : 1;
      unsigned char recirc    : 1;
      unsigned char reshuffle : 1;
      unsigned char set_ev    : 1;
      unsigned char unused    : 2;
    };
  } command;

  union __attribute__ ((packed)) colorFlags_t
  {
    unsigned long  allFlags;
    unsigned char  byteFlags[4];
    unsigned short wordFlags[2];
    struct
    {
      /* octect 1 */
      unsigned char stopped              : 1;
      unsigned char ready                : 1;
      unsigned char homing               : 1;
      unsigned char supply_run           : 1;
      unsigned char recirc_run           : 1;
      unsigned char reshuffle            : 1;
      unsigned char supply_end           : 1;
      unsigned char recirc_end           : 1;

      /* octect 2 */
      unsigned char homing_pos_error     : 1;
      unsigned char tout_error           : 1;
      unsigned char reset_error          : 1;
      unsigned char supply_calc_error    : 1;
      unsigned char software_error       : 1;
      unsigned char overcurrent_error    : 1;
      unsigned char homing_back_error    : 1;
      unsigned char pos0_read_light_error: 1;

      /* octet 3 */
      unsigned char end_stroke_read_dark_error : 1;
      unsigned char drv_over_curr_temp_error   : 1;
      unsigned char open_load_error            : 1;
	  unsigned char jump_to_boot               : 1;
      unsigned char unsigned_byte_3            : 4;

      /* octet 4 */
      unsigned char unused_byte_4        : 8;
    };
  } colorFlags;

  // used to debug 0 readings on color actuators 
  short photoc_zero_reading;
} colorAct_t;

/**
 * typedefs
 */
typedef struct
{
  unsigned char typeMessage;

  unsigned short n_step_home;
  unsigned short n_step_move;
  unsigned short speed_move;
  unsigned char photoc_home;

  unsigned char cl_direction; /* 0 = backward, 1 = forward */
  unsigned char cl_packing_pwm;
  unsigned char cl_operating_pwm;
  unsigned short cl_duration;
  unsigned char photoc_pack;
/**
* color circuit command
*/
  union __attribute__ ((packed))
  {
    unsigned char cmd;
    struct
    {
      unsigned char stop      : 1;
      unsigned char homing    : 1;
      unsigned char open      : 1;
      unsigned char close     : 1;

      unsigned char packing   : 1;
      unsigned char extend    : 1;
      unsigned char retract   : 1;
      unsigned char intr      : 1;
    };
  } command;

  union __attribute__ ((packed)) autocapFlags_t
  {
    unsigned long  allFlags;
    unsigned char  byteFlags[4];
    unsigned short wordFlags[2];

    struct
    {
      /* octet 1 */
      unsigned char ready                : 1;
      unsigned char homing               : 1;
      unsigned char running              : 1;
      unsigned char open                 : 1;

      unsigned char close                : 1;
      unsigned char home_pos_error       : 1;
      unsigned char tout_error           : 1;
      unsigned char homing_error         : 1;

      /* octet 2 */
      unsigned char reserved             : 1; /* just for alignment */
      unsigned char packing              : 1;
      unsigned char moving               : 1;
      unsigned char extend               : 1;

      unsigned char retract              : 1;
      unsigned char packing_error        : 1;
      unsigned char lifter_error         : 1;
      unsigned char software_error       : 1;

      /* octet 3 */
      unsigned char over_curr_temp_error : 1;
      unsigned char open_load_error      : 1;
	  unsigned char hum_10_par_rx		 : 1;
	  unsigned char hum_10_bad_par_error : 1;	  
	  unsigned char hum_10_too_low_water_level : 1;	  
      unsigned char jump_to_boot         : 1;
      unsigned char unused_byte_3        : 2;

      /* octet 4 */
      unsigned char unused_byte_4        : 8;
    };
  } autocapFlags;
} autocapAct_t;

typedef struct
{
  // Circuit Steps Position with respect to Reference
  signed long Circ_Pos[MAX_COLORANT_NUM];
} CircStepPosAct_t;

/** periodic processes status enum */
typedef enum {
  /* 0 */ PROC_IDLE,
  /* 1 */ PROC_READY,
  /* 2 */ PROC_RUNNING
} process_t;


#endif	/* TYPEDEF_H */

