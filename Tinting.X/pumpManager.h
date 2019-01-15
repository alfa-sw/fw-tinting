/* 
 * File:   pumpManager.h
 * Author: michele.abelli
 * Description: EEprom 
 * Created on 16 luglio 2018, 15.19
 */

#ifndef PUMPMANAGER_H
#define	PUMPMANAGER_H

extern void PumpManager(void);
extern void initPumpParam(void);
extern void initPumpStatusManager(void);
extern unsigned char SingleStrokeColorSupply(void);
extern unsigned char HighResColorSupply(void);
extern unsigned char OldContinuousColorSupply(void);
extern unsigned char ContinuousColorSupply(void);
extern unsigned char AnalyzeFormula(void);
extern unsigned char AnalyzeContinuousFormula(void);
extern unsigned char AnalyzePumpParameters(void);
extern unsigned char AnalyzeRicirculationCommand(void);
extern unsigned char RicirculationColorSupply(void);
extern unsigned char PumpHomingColorSupply(void);
extern unsigned char ValveHomingColorSupply(void);
extern unsigned char ValveOpenClose(void);
#endif	/* PUMPMANAGER_H */

