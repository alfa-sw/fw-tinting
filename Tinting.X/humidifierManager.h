/* 
 * File:   humidifierManager.h
 * Author: michele.abelli
 *
 * Created on 18 luglio 2018, 12.53
 */

#ifndef HUMIDIFIERMANAGER_H
#define	HUMIDIFIERMANAGER_H

extern void HumidifierManager(void);
extern void humidifierStatusManager(void);
extern void initHumidifierParam(void);
extern void initHumidifierStatusManager(void);
extern void StopHumidifier(void);
extern int AnalyzeHumidifierParam(void);
extern int AnalyzeSetupOutputs(void);
extern int AcquireTemperature(unsigned char Temp_Type, unsigned long *Temp);
extern int AcquireHumidityTemperature(unsigned char Hum_Type, unsigned long *Temp, unsigned long *Humidity);
extern void HumidifierProcessCalculation(unsigned long RH, unsigned long Temperature, unsigned long *Period, unsigned long *Neb_Duration);

#endif	/* HUMIDIFIERMANAGER_H */

