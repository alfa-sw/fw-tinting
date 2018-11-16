/* 
 * File:   stepper.h
 * Author: michele.abelli
 *
 * Created on 18 luglio 2018, 15.30
 */

#ifndef STEPPER_H
#define	STEPPER_H


extern void ConfigStepper(unsigned short Motor_ID, unsigned short Resolution, unsigned short AccDecCurrent, unsigned short RunCurrent, 
                          unsigned short HoldingCurrent, unsigned long AccelerationRate, unsigned long DecelerationRate, unsigned short AlarmsEnabled);
extern void ReadStepperError(unsigned short Motor_ID, unsigned short *AlarmsError);
extern void SetStepperHomePosition(unsigned short Motor_ID);
extern long GetStepperPosition(unsigned short Motor_ID);
extern unsigned short GetStepperSpeed(unsigned short Motor_ID);
extern void MoveStepper(unsigned short Motor_ID, long Step_N, unsigned short Speed);
extern void StartStepper(unsigned short Motor_ID, unsigned short Speed, unsigned char Direction, unsigned char Transition_Type, unsigned short PhotoType, unsigned long Duration);
extern void StopStepper(unsigned short Motor_ID);
extern void MoveStepperToHome(unsigned short Motor_ID, unsigned short Speed);
extern void DCMotorManagement(unsigned short Motor_ID, unsigned char Mode);
extern unsigned char PhotocellStatus(unsigned short PhotoType, unsigned char Filter);


#endif	/* STEPPER_H */

