/* 
 * File:   mixermanager.h
 * Author: michele.abelli
 * Description: Mixer Manager 
 * Created on 11 marzo 2020, 15.21
 */

#ifndef MIXERMANAGER_H
#define	MIXERMANAGER_H

extern void MixerManager(void);
extern unsigned char MixingColorSupply(void);
extern unsigned char MixerHomingColorSupply(void);
extern unsigned char MixerJarMotorSupply(void);
extern void initMixerParam(void);
extern unsigned char AnalyzeMixerParameters(void);
extern unsigned char MixerTesting(void);
extern void initMixerStatusManager(void);

#endif	/* MIXERMANAGER_H */

