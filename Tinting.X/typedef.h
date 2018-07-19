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
      unsigned char unused : 1;      
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
      unsigned char tinting_setup_process : 1;
      unsigned char tinting_setup_intr  : 1;
      unsigned char tinting_setup_clean : 1;
      unsigned char unused              : 7;
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
      unsigned char tinting_software_error    : 1;
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
	  unsigned char unused_byte_2 : 6;

      /* octet 3 */
	  unsigned char unused_byte_3 : 8;

      /* octet 4 */
	  unsigned char unused_byte_4 : 8;
   };
  } TintingFlags_1;
  
  // Humidifier
  unsigned char Autocap_Status;
  
  // 0 --> Humidifier Disabled
  // 1 --> Humidifier Enabled
  unsigned char Humidifier_Enable;
  // Type of R/H Sensor: 
  // 0 --> Sensirion SHT30
  // 1 --> ..................
  unsigned char Humdifier_Type;
  // Process Period (sec))
  unsigned long Humidifier_Period;
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

  // Resistance  
  unsigned long Temperature;
  unsigned long RH;  
  unsigned long Dosing_Temperature;
  unsigned char Nebulizer_state;
  unsigned char Resistance_state;  
  unsigned char Riscaldatore_state;
  unsigned char WaterLevel_state;
  unsigned char CriticalTemperature_state;

  // Microswitch status    
  unsigned char BasesCarriage_state;
  // Circuit Engaged
  unsigned char Circuit_Engaged;
  // Rotating Table position with respect to Reference circuit
  unsigned long Steps_position;
  // Home Photocell status
  unsigned char Home_photocell;
  // Coupling Photocell status
  unsigned char Coupling_photocell;
  // Valve Photocell status
  unsigned char Valve_photocell;
  // Rotating Table Photocell status
  unsigned char Table_photocell;
  // Can presence Photocell status  
  unsigned char CanPresence_photocell;
  // Table panel open status  
  unsigned char PanelTable_state;
  // Circuit Steps Position with respec to Reference
  unsigned long Circuit_step_pos[MAX_COLORANT_NUM];
  
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
  unsigned long N_step_back_step;
  // Back Step Speed (rpm)  (also Continuous)
  unsigned long Speed_back_step;
  // Minimum stroke before Valve Open  (also Continuous)
  unsigned long N_step_backlash;
  // Waiting Time with motor stopped after Valve Close  (also Continuous)
  unsigned long Delay_EV_off;
  // Suction Speed (also Continuous)
  unsigned long Speed_suction;
  // Stirring Duration after Dispensing  (also Continuous)
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
  // Passi da posizione di home (valvola chiusa) a posizone di valvola aperta su foro grande
  unsigned long Step_Valve_Open_Big;
  // Passi da posizione di home (valvola chiusa) a posizone di valvola aperta su foro piccolo
  unsigned long Step_Valve_Open_Small;
  // Velocità di apertura/chiusura valvola
  unsigned char Speed_Valve;
  // N. steps in una corsa intera
  unsigned long N_steps_stroke; 
  
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
  // Distanza in passi tra il circuito di riferimento e la spazzola
  unsigned long Steps_Cleaning;
  
  // Cleaning Duration (sec))
  unsigned long Cleaning_duration;
  // Cleaning Pause (min))
  unsigned long  Cleaning_pause;  
  // Angolo di rotazione della tavola rotante rispetto alla posizone di ingaggio (°))
  unsigned long Refilling_Angle;
  // Direzione rotazione (CW o CCW))
  unsigned char Direction; 
  
  // Tipo di Uscita
  unsigned char Output_Type;
  // Enable/Disable Output
  unsigned char Output_Act;  

} TintingAct_t;

#endif	/* TYPEDEF_H */

