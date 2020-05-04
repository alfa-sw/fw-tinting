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
PHOTO_HOME, // 0: Fotocellula Mixr Home
PHOTO_JAR,  // 1: Fotocellula presenza Barattolo
PHOTO_DOOR_OPEN,  // 2: Fotocellula Porta Aperta        
MICROSWITCH_DOOR, // 3: Microswitch Porta Ciusa
PHOTO_AUTOCAP_OPEN, // 4: Fotocellula Autocap Aperto
PHOTO_AUTOCAP_LIFTER, // 5: Fotocellula Autocap Lifter Down
};

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
extern unsigned short GetStatus (unsigned short Motor_ID);
extern void Run_Stepper(unsigned short Motor_ID, unsigned short Speed, unsigned char Direction);
extern void StepperMovementsManager(void);
extern void SoftStopStepper(unsigned short Motor_ID);
extern void SoftHiZ_Stepper(unsigned short Motor_ID);
extern void HardHiZ_Stepper(unsigned short Motor_ID);



#endif	/* STEPPER_H */

