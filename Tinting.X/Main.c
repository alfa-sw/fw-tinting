/* 
 * File:   main.c
 * Author: michele.abelli
 * Description  : Main
 * Created on 16 luglio 2018, 14.18
 */

/* ===== SETTING CONFIGURATION BITS================================================== */
/** CONFIGURATION **************************************************/
// PIC24FJ256GB110 Configuration Bit Settings
// 'C' source line config statements    
// CONFIG1
#pragma config WDTPS = PS128   // Watchdog Timer Postscaler bits->1:128
#pragma config FWPSA = PR128   // WDT Prescaler->Prescaler ratio of 1:128
#pragma config WINDIS = OFF    // Watchdog Timer Window->Standard Watchdog Timer enabled,(Windowed-mode is disabled)
#pragma config ICS    = PGx2   // Comm Channel Select->Emulator functions are shared with PGEC2/PGED2
#pragma config BKBUG  = ON     // Background Debug->Dev\ice resets into Debug mode
#pragma config GWRP   = OFF    // General Code Segment Write Protect->Writes to program memory are allowed
#pragma config GCP    = OFF    
// General Code Segment Code Protect->Code protection is disabled
#pragma config JTAGEN = OFF    // JTAG Port Enable->JTAG port is disabled
// CONFIG2
#pragma config POSCMOD  = HS   // Primary Oscillator Select->HS oscillator mode selected
#pragma config DISUVREG = OFF  // Internal USB 3.3V Regulator Disable bit->Regulator is disabled
#pragma config IOL1WAY  = OFF  // IOLOCK One-Way Set Enable bit->Unlimited Writes To RP Registers
#pragma config OSCIOFNC = OFF  // Primary Oscillator Output Function->OSCO functions as CLKO (FOSC/2)
#pragma config FCKSM    = CSDCMD // Clock Switching and Monitor->Both Clock Switching and Fail-safe Clock Monitor are disabled
#pragma config FNOSC    = PRIPLL // Oscillator Select->Primary oscillator with PLL module
#pragma config PLLDIV   = DIV2 // Oscillator input divided by 2
#pragma config IESO     = OFF  // Internal External Switch Over Mode->IESO mode (Two-speed start-up)disabled
// CONFIG3
#pragma config WPFP  = WPFP511 // Write Protection Flash Page Segment Boundary->Highest Page (same as page 170)
#pragma config WPDIS = WPDIS   // Segment Write Protection Disable bit->Segmented code protection disabled
#pragma config WPCFG = WPCFGDIS// Configuration Word Code Page Protection Select bit->Last page(at the top of program memory) and Flash configuration words are not protected
#pragma config WPEND = WPENDMEM// Segment Write Protection End Page Select bit->Write Protect from WPFP to the last page of memory

#if defined (WATCH_DOG_DISABLE)
#pragma config FWDTEN = OFF     // Watchdog Timer Enable->Watchdog Timer is disabled
#else
#pragma config FWDTEN = ON     // Watchdog Timer Enable->Watchdog Timer is enabled
#endif

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// include file definition
#include <xc.h>
#include "p24FJ256GB110.h"
#include "timerMg.h"
#include "gestio.h"
#include "tintingManager.h"
#include "tableManager.h"
#include "pumpManager.h"
#include "humidifierManager.h"
#include "mem.h"
#include "serialcom.h"
#include "serialcom_GUI.h"
#include "ram.h"
#include "define.h"
#include "typedef.h"
#include "stepper.h"
#include "i2c3.h"
#include "spi.h"
#include "gestIO.h"
#include "eepromManager.h"
#include "eeprom.h"
#include "spi3.h"
#include "errorManager.h"
#include "statusManager.h"
#include "autocapAct.h"
#include "colorAct.h"
#include "rollerAct.h"
#include "stdlib.h"

volatile const unsigned long *BootPtrTestResults = (unsigned long *) (__BL_SW_VERSION);

const unsigned long __attribute__ ((space(psv), address (__APPL_CODE_FW_VER)))
dummy0 = SW_VERSION;

// -----------------------------------------------------------------------------
void SPI1_InterruptHandler(void);
void SPI2_InterruptHandler(void);
void SPI3_InterruptHandler(void);

void Pippo(void);
// -----------------------------------------------------------------------------
/** T Y P E D E F S ******************************************************************* */

/*
**=============================================================================
**
**      Oggetto        : Funzione di servizio degli Interrupt NON usati dal 
**                       Programma Applicativo
**                                    
**      Parametri      : void
**
**      Ritorno        : void
**
**      Vers. - autore : 1.0 Michele Abelli
**
**=============================================================================
*/
void Pippo(void)
{
   unsigned int a;
   for (a = 0; a < 10; a++){}
}

static void ioRemapping(void)
/**/
/*===========================================================================*/
/**
**   @brief  Remapping PIN PIC
**
**   @param  void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  __builtin_write_OSCCONL(OSCCON & 0xbf); /*UnLock IO Pin Remapping*/     
    // UART2 - RS232
    RPOR15bits.RP30R = 0x0005;  //RF2->UART2:U2TX;
    RPINR19bits.U2RXR = 0x0011; //RF5->UART2:U2RX;
    // UART3 - RS485
    RPINR17bits.U3RXR = 0x002A; //RD12->UART3:U3RX;
    RPOR6bits.RP12R   = 0x001C; //RD11->UART3:U3TX;
    // SPI1 - MOTOR DRIVER    
 //   RPINR20bits.SCK1R = 0x0000; //RB0->SPI1:SCK1IN;
 //   RPOR6bits.RP13R = 0x0007;   //RB2->SPI1:SDO1;
 //   RPINR20bits.SDI1R = 0x0001; //RB1->SPI1:SDI1;
    // SPI2 - EEPPROM
    RPOR9bits.RP19R = 0x000A;   //RG8->SPI2:SDO2;
    RPOR10bits.RP21R = 0x000B;  //RG6->SPI2:SCK2OUT;
    RPINR22bits.SDI2R = 0x001A; //RG7->SPI2:SDI2;
    // SPI3 - TC72 TEMPERATURE SENSOR
    RPOR11bits.RP23R = 0x0021;  //RD2->SPI3:SCK3OUT;
    RPINR28bits.SDI3R = 0x0004; //RD9->SPI3:SDI3;
    RPOR5bits.RP11R = 0x0020;   //RD0->SPI3:SDO3;
    // I2C3 - SHT31 HUMIDITY SENSOR     
    // NO assignements    
  __builtin_write_OSCCONL(OSCCON | 0x40);   /*Lock IO Pin Remapping*/  
} /* ioRemapping() */

int main(void)
{
#ifndef NOLAB	
        unsigned short i, j, find_circ;
#endif            
        
        
//unsigned result, result1;
    // POSTSCALER Clock Division = 1 --> Clock Frequency = 32MHZ - 16MIPS
        
    CLKDIVbits.CPDIV0 = 0;     
    CLKDIVbits.CPDIV1 = 0;    
    
	// unlock OSCCON register: 'NOSC' = primary oscillator with PLL module - 
    // 'OSWEN' = 1 initiate an oscillator switch to the clock source specified by 'NOSC' 
	__builtin_write_OSCCONH(0x03);
	__builtin_write_OSCCONL(0x01);

	/* wait for clock to stabilize: Primary Oscillator with PLL module (XTPLL, HSPLL))*/
	while (OSCCONbits.COSC != 0b011)        
	  ;	
	/* wait for PLL to lock: PLL module is in lock, PLL start-up timer is satisfied */
	while (OSCCONbits.LOCK != 1)
	  ;
    // Auto generate initialization
// -----------------------------------------------------------------------------    
	ioRemapping();    
    InitTMR();
	initIO(); 
    INTERRUPT_Initialize();
    initHumidifierParam();
#ifndef CAR_REFINISHING_MACHINE    
	initSerialCom();
#endif    
    initSerialCom_GUI();
//    I2C3_Initialize(); 
    slave_id = TINTING;

    
#ifndef WATCH_DOG_DISABLE
    /* enable watchdog */
    ENABLE_WDT();
#else
#endif	
    // Prevent sw reset loop 
    inhibitReset = RCONbits.SWR;
    // Clear all RCON bits    
    RCON = 0;  
    
    StartTimer(T_ERROR_STATUS);
    __builtin_write_OSCCONL(OSCCON & 0xbf); /*UnLock IO Pin Remapping*/     
    spi_remapping(SPI_1);
    __builtin_write_OSCCONL(OSCCON | 0x40); /*Lock IO Pin Remapping*/  

    spi_init(SPI_1);  //SPI controllo motore    
	initStatusManager();      
    EEPROMInit();
    read_eeprom = checkEEprom();
	initTableStatusManager();
    initTableParam();
    initCleanParam();
	initPumpStatusManager();
    initPumpParam();
	initHumidifierStatusManager();    
    spi_remapping(SPI_2);
    spi_init(SPI_2); //SPI controllo EEprom
    spi_init(SPI_3); //SPI Sensore temperatura  
    StartTimer(T_MEASURING_TIME);
    MAX_Cycle_Duration = 0;
    Timer_New = 0;  
    jump_to_boot_done = 0xFF;     
    StartTimer(T_WAIT_READ_FW_VERSION);
    // Used ase time base by the visual indicator
    StartTimer(T_HEARTBEAT);  
#ifdef CAR_REFINISHING_MACHINE                                
    init_Roller();
#endif    
    while (1)
	{ 
#ifndef WATCH_DOG_DISABLE
        // Watchdog Clock source FRC 31KHz (internal RC oscillator), PRESCALER = 1/128, POSTSCALER = 1/128
        // Watchdog Period: (128*128/31000*1000)sec = 528msec
        // kicking the dog ;-) 
        ClrWdt();
#else
#endif          
        // Main loop measurement
        // ---------------------------------------------------------------------
        Timer_Old = Timer_New;
        Timer_New = ReadTimer(T_MEASURING_TIME);
        if ( (Timer_New > Timer_Old) && (Timer_Old != 0) ) 
            Cycle_Duration = Timer_New - Timer_Old;
        if (Cycle_Duration > MAX_Cycle_Duration)
            MAX_Cycle_Duration = Cycle_Duration; 
        // ---------------------------------------------------------------------        
		HumidifierManager();
        PumpManager();
        TableManager();
        TintingManager();
        StepperMovementsManager();
        // Manager del sensore di Temperatura 
        SPI3_Manager();
        // Manager del sensore di T/H
        //I2C_Manager();  //se non si connette il sensore questo sequencer blocca il main               
		TimerMg();
		gestioneIO();     
#ifndef CAR_REFINISHING_MACHINE        
        // RS485 Serial communication with actuators  
        serialCommManager_Act();
#endif        
        // RS232 Serial communication  
        serialCommManager_GUI();
        // ALARMs detection 
        monitorManager();
        // Process Manager
        statusManager();          
#ifdef AUTOCAP_MMT
        autocap_Manager();
#endif   
        
#ifdef CAR_REFINISHING_MACHINE
        roller_Manager();
#endif        
        // ---------------------------------------------------------------------
        // CanPresence photocell status
        TintingAct.CanPresence_photocell = PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER);           
        // Panel Table status
        TintingAct.PanelTable_state = PhotocellStatus(PANEL_TABLE, FILTER);
TintingAct.PanelTable_state = 0;
        
//------------------------------------------------------------------------------        
#ifndef CAR_REFINISHING_MACHINE     
        // Bases carriage State
        TintingAct.BasesCarriage_state = PhotocellStatus(BASES_CARRIAGE, FILTER); 
#else
        TintingAct.BasesCarriage_state = JarPhotocellStatus(MICRO_CAR, FILTER);
#endif        
//------------------------------------------------------------------------------        
#ifndef CAR_REFINISHING_MACHINE     
        // Water Level State
        if (TintingHumidifier.Humidifier_Enable == HUMIDIFIER_ENABLE)
            TintingAct.WaterLevel_state = !getWaterLevel();       
        else
            TintingAct.WaterLevel_state = FALSE;
#else
        TintingAct.WaterLevel_state = JarPhotocellStatus(MICRO_LEVEL, FILTER);
#endif 
//------------------------------------------------------------------------------                
        // bit0: Home photocell status                
        TintingAct.Home_photocell = PhotocellStatus(HOME_PHOTOCELL, FILTER);
        if (TintingAct.Home_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << HOME_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << HOME_PHOTOCELL);            
        // bit1: Coupling photocell status
        //TintingAct.Coupling_photocell = PhotocellStatus(COUPLING_PHOTOCELL, FILTER);
        TintingAct.Coupling_photocell = CouplingPhotocell_sts;
        if (TintingAct.Coupling_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << COUPLING_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << COUPLING_PHOTOCELL);            
        // bit2: Valve Home photocell status
        TintingAct.Valve_photocell = PhotocellStatus(VALVE_PHOTOCELL, FILTER);
        if (TintingAct.Valve_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << VALVE_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << VALVE_PHOTOCELL);            
        // bit3: Rotating Table photocell status
        TintingAct.Table_photocell = PhotocellStatus(TABLE_PHOTOCELL, FILTER);  
        if (TintingAct.Table_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << TABLE_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << TABLE_PHOTOCELL);                    
        // bit4 Valve Open Photocell
        TintingAct.ValveOpen_photocell = PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER);
        if (TintingAct.ValveOpen_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << VALVE_OPEN_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << VALVE_OPEN_PHOTOCELL);                            
#ifndef CAR_REFINISHING_MACHINE     
        // bit5: Autocap Closed Photocell status
        TintingAct.Autocap_Closed_photocell = PhotocellStatus(AUTOCAP_CLOSE_PHOTOCELL, FILTER);
        if (TintingAct.Autocap_Closed_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << AUTOCAP_CLOSE_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << AUTOCAP_CLOSE_PHOTOCELL);                                    
        // bit6: Autocap Opened Photocell status
        TintingAct.Autocap_Opened_photocell = PhotocellStatus(AUTOCAP_OPEN_PHOTOCELL, FILTER);
        if (TintingAct.Autocap_Opened_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << AUTOCAP_OPEN_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << AUTOCAP_OPEN_PHOTOCELL);                                            
#else
        // Autocap Close
        TintingAct.Photocells_state |= (1L << AUTOCAP_CLOSE_PHOTOCELL); 
        TintingAct.Photocells_state &= ~(1L << AUTOCAP_OPEN_PHOTOCELL);                                                    
#endif        
        // bit7: Brush Photocell status
        TintingAct.Brush_photocell = PhotocellStatus(BRUSH_PHOTOCELL, FILTER);
        if (TintingAct.Brush_photocell == TRUE)
            TintingAct.Photocells_state |= (1L << BRUSH_PHOTOCELL); 
        else
            TintingAct.Photocells_state &= ~(1L << BRUSH_PHOTOCELL);                                                            
        // ---------------------------------------------------------------------
#ifdef CAR_REFINISHING_MACHINE           
        // bit0: Jar Input Roller Photocell
        if (JarPhotocellStatus(JAR_INPUT_ROLLER_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << JAR_INPUT_ROLLER_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << JAR_INPUT_ROLLER_PHOTOCELL);

        // bit1: Jar Load Lifter Roller Photocell
        if (JarPhotocellStatus(JAR_LOAD_LIFTER_ROLLER_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << JAR_LOAD_LIFTER_ROLLER_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << JAR_LOAD_LIFTER_ROLLER_PHOTOCELL);

        // bit2: Jar Ouput Roller Photocell
        if (JarPhotocellStatus(JAR_OUTPUT_ROLLER_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << JAR_OUTPUT_ROLLER_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << JAR_OUTPUT_ROLLER_PHOTOCELL);

        // bit3: Load Lifter Down Photocell
        if (JarPhotocellStatus(LOAD_LIFTER_DOWN_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << LOAD_LIFTER_DOWN_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << LOAD_LIFTER_DOWN_PHOTOCELL);

        // bit4: Load Lifter Up Photocell
        if (JarPhotocellStatus(LOAD_LIFTER_UP_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << LOAD_LIFTER_UP_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << LOAD_LIFTER_UP_PHOTOCELL);

        // bit5: Unload Lifter Down Photocell
        if (JarPhotocellStatus(UNLOAD_LIFTER_DOWN_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << UNLOAD_LIFTER_DOWN_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << UNLOAD_LIFTER_DOWN_PHOTOCELL);

        // bit6: Unload Lifter Up Photocell
        if (JarPhotocellStatus(UNLOAD_LIFTER_UP_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << UNLOAD_LIFTER_UP_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << UNLOAD_LIFTER_UP_PHOTOCELL);

        // bit7: Unload Lifter Roller Photocell
        if (JarPhotocellStatus(JAR_UNLOAD_LIFTER_ROLLER_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << JAR_UNLOAD_LIFTER_ROLLER_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << JAR_UNLOAD_LIFTER_ROLLER_PHOTOCELL);       

        // bit8: Jar Dispensing Position Photocell
        if (JarPhotocellStatus(JAR_DISPENSING_POSITION_PHOTOCELL, FILTER) == TRUE)
            TintingAct.Jar_Photocells_state |= (1L << JAR_DISPENSING_POSITION_PHOTOCELL);
        else
            TintingAct.Jar_Photocells_state &= ~(1L << JAR_DISPENSING_POSITION_PHOTOCELL);       
#endif                
                
#if defined NOLAB	
        TintingAct.Circuit_Engaged = 1;
#else        
        // Check if a Circuit is Engaged            
        TintingAct.Steps_position = (signed long)GetStepperPosition(MOTOR_TABLE);        
        if (TintingAct.Steps_position < 0) {
            TintingAct.Steps_position = (-TintingAct.Steps_position) % TintingAct.Steps_Revolution;
            
            if (TintingAct.Steps_position != 0)
                TintingAct.Steps_position = TintingAct.Steps_Revolution - TintingAct.Steps_position;
        }   
        else
            TintingAct.Steps_position = TintingAct.Steps_position % TintingAct.Steps_Revolution;

        // Calculates 'Circuit_step_tmp[]'
        for (j = 0; j < MAX_COLORANT_NUMBER; j++) {
            find_circ = FALSE;
            
            for (i = 0; i < Total_circuit_n; i++) {

                if ( ((Circuit_step_original_pos[j] > TintingAct.Circuit_step_pos[i]) && ((Circuit_step_original_pos[j] - TintingAct.Circuit_step_pos[i]) <= TintingAct.Steps_Tolerance_Circuit) ) ||
                     ((Circuit_step_original_pos[j] <= TintingAct.Circuit_step_pos[i])&& ((TintingAct.Circuit_step_pos[i] - Circuit_step_original_pos[j]) <= TintingAct.Steps_Tolerance_Circuit) ) ) {                      
                    Circuit_step_tmp[j] = TintingAct.Circuit_step_pos[i];
                    find_circ = TRUE;
                    break;                        
                }
            }
            // Circuit 'i' is not present on the Table
            if (find_circ == FALSE)
                Circuit_step_tmp[j] = 0;
        } 
        
        // Calculates Circuit Engaged                
        if (Table_circuits_pos == ON) {
            find_circ = FALSE;
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if ( ((Circuit_step_tmp[i] != 0) && (TintingAct.Steps_position > Circuit_step_tmp[i]) && ( (TintingAct.Steps_position - Circuit_step_tmp[i]) < TintingAct.Steps_Tolerance_Circuit) ) ||
                     ((Circuit_step_tmp[i] != 0) && (TintingAct.Steps_position <= Circuit_step_tmp[i])&& ( (Circuit_step_tmp[i] - TintingAct.Steps_position) < TintingAct.Steps_Tolerance_Circuit) ) ) {
                    find_circ = TRUE;
                    TintingAct.Circuit_Engaged = i+1; 
                    break;
                }    
            }
            if (find_circ == FALSE)
                TintingAct.Circuit_Engaged = 0;                 
        }    
        else
            TintingAct.Circuit_Engaged = 0; 
#endif                    
    }   
}
// -----------------------------------------------------------------------------
// Default Interrupt
void __attribute__((__interrupt__,auto_psv)) _DefaultInterrupt(void)
{
    Nop();
    Nop();
    while(1);
}
void __attribute__((address(__APPL_T1)__interrupt__,auto_psv)) _T1Interrupt(void)
{
   T1_InterruptHandler();
}
void __attribute__((address(__APPL_U2RX1)__interrupt__, auto_psv)) _U2RXInterrupt(void)
{
   U2RX_InterruptHandler();
}
void __attribute__((address(__APPL_U2TX1)__interrupt__, auto_psv)) _U2TXInterrupt(void)
{
   U2TX_InterruptHandler();
}
/*
void __attribute__((address(__APPL_U3RX1)__interrupt__, auto_psv)) _U3RXInterrupt(void)
{
   U3RX_InterruptHandler();
}
void __attribute__((address(__APPL_U3TX1)__interrupt__, auto_psv)) _U3TXInterrupt(void)
{
   U3TX_InterruptHandler();
}
*/
void __attribute__((address(__APPL_SPI1)__interrupt__, auto_psv)) _SPI1Interrupt(void)
{
    SPI1_InterruptHandler();
}
void __attribute__((address(__APPL_SPI2)__interrupt__, auto_psv)) _SPI2Interrupt(void)
{
    SPI2_InterruptHandler();
}
void __attribute__((address(__APPL_SPI3)__interrupt__, auto_psv)) _SPI3Interrupt(void)
{
    SPI3_InterruptHandler();
}
/*
void __attribute__((address(__APPL_I2C3)__interrupt__, auto_psv)) _MI2C3Interrupt(void)
{
   MI2C3_InterruptHandler();
} 
*/
// -----------------------------------------------------------------------------
//#endif
//                      APPLICATION PROGRAM Service Routine NOT USED
// SPI1 GENERAL Interrupt handler 
void SPI1_InterruptHandler(void)
{
    Pippo();
}
// SPI2 GENERAL Interrupt handler 
void SPI2_InterruptHandler(void)
{
    Pippo();
}
// SPI3 GENERAL Interrupt handler 
void SPI3_InterruptHandler(void)
{
    Pippo();
}
// -----------------------------------------------------------------------------