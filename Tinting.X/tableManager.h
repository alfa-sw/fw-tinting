/* 
 * File:   tableManager.h
 * Author: michele.abelli
 * Description: Manage all Rotating Table activities
 * Created on 16 luglio 2018, 15.21
 */

#ifndef TABLEMANAGER_H
#define	TABLEMANAGER_H

extern void TableManager(void);
extern void initTableParam(void);
extern void initCleanParam(void);
extern void initTableStatusManager(void);
extern unsigned char AnalyzeTableParameters(void);
extern unsigned char TableHomingColorSupply(void);
extern unsigned char TableSelfRecognitionColorSupply(void);
extern unsigned char TablePositioningColorSupply(void);
extern unsigned char TableCleaningColorSupply(void);
extern unsigned char TableTestColorSupply(void);
extern unsigned char TableStepsPositioningColorSupply(void);
extern unsigned char TableGoToReference(void);
extern unsigned char TableStirring(void);
extern unsigned char ManageTableHomePosition(void);
extern unsigned char TableRun(void);
extern unsigned char AnalyzeCleanParameters(void);

#endif	/* TABLEMANAGER_H */

