/* 
 * File:   stepper.h
 * Author: michele.abelli
 *
 * Created on 18 luglio 2018, 15.30
 */

#ifndef STEPPER_H
#define	STEPPER_H

enum
{
PHOTO_HOME, // 0: Fotocellula Home
PHOTO_ACC,  // 1: Fotocellula Accoppiamento
PHOTO_OPEN_EV, // 2: Fotocellula Valvola nella posizione di Home
PHOTO_EV, // 3: Fotocellula Tavola
PHOTO_VALVE_OPEN, // 4: Fotocellula Valvola Aperta
PHOTO_AUTOCAP_CLOSE, // 5: Fotocellula Autocap Chiuso
PHOTO_AUTOCAP_OPEN, // 6: Fotocellula Autocap Aperto
PHOTO_BRUSH, // 7: Fotocellula Spazzola  
PHOTO_CAN_PRESENCE, // 8: Sensore Can Presence (Fotocellula o Ultrasuoni)
PHOTO_TABLE, // 9: Pannello Tavola 
PHOTO_BASES_CARRIAGE, // 10: Carrello Basi
BUTTON_LPXC10, // 11: Pulsante LPXC10
};
#ifdef CAR_REFINISHING_MACHINE
    #define JAR_INPUT_ROLLER_PHOTOCELL          0
    #define JAR_LOAD_LIFTER_ROLLER_PHOTOCELL    1
    #define JAR_OUTPUT_ROLLER_PHOTOCELL         2
    #define LOAD_LIFTER_DOWN_PHOTOCELL          3 
    #define LOAD_LIFTER_UP_PHOTOCELL            4
    #define UNLOAD_LIFTER_DOWN_PHOTOCELL        5 
    #define UNLOAD_LIFTER_UP_PHOTOCELL          6
    #define JAR_UNLOAD_LIFTER_ROLLER_PHOTOCELL  7
    #define JAR_DISPENSING_POSITION_PHOTOCELL   8
    #define MICRO_CAR                           9
    #define MICRO_LEVEL                         10                                                                                                                                                           
#endif

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

#define FORWARD 	FWD
#define REVERSE 	REV




extern void ConfigStepper(unsigned short Motor_ID, unsigned short Resolution, unsigned short AccDecCurrent, unsigned short RunCurrent, 
                          unsigned short HoldingCurrent, unsigned long AccelerationRate, unsigned long DecelerationRate, unsigned char AlarmsEnabled);
extern void ReadStepperError(unsigned short Motor_ID, unsigned short *AlarmsError);
extern void SetStepperHomePosition(unsigned short Motor_ID);
extern signed long GetStepperPosition(unsigned short Motor_ID);
extern unsigned short GetStepperSpeed(unsigned short Motor_ID);
extern void MoveStepper(unsigned short Motor_ID, long Step_N, unsigned short Speed);
extern void StartStepper(unsigned short Motor_ID, unsigned short Speed, unsigned char Direction, unsigned char Transition_Type, unsigned short PhotoType, unsigned long Duration);
extern void StopStepper(unsigned short Motor_ID);
extern void MoveStepperToHome(unsigned short Motor_ID, unsigned short Speed);
extern void DCMotorManagement(unsigned short Motor_ID, unsigned char Mode);
extern unsigned char PhotocellStatus(unsigned short PhotoType, unsigned char Filter);
#ifdef CAR_REFINISHING_MACHINE
extern unsigned char JarPhotocellStatus(unsigned short PhotoType, unsigned char Filter);
#endif
extern unsigned short GetStatus (unsigned short Motor_ID);
extern void Read_All_Parameters(unsigned short Motor_ID);
extern void init_test_Stepper(unsigned short Motor_ID);
extern void test_Stepper(unsigned short Motor_ID);
extern void Run_Stepper(unsigned short Motor_ID, unsigned short Speed, unsigned char Direction);
extern void StepperMovementsManager(void);
extern void SoftStopStepper(unsigned short Motor_ID);
extern void SoftHiZ_Stepper(unsigned short Motor_ID);
extern void HardHiZ_Stepper(unsigned short Motor_ID);



#endif	/* STEPPER_H */

