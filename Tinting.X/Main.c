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
#pragma config BKBUG  = ON     // Background Debug->Device resets into Debug mode
#pragma config GWRP   = OFF    // General Code Segment Write Protect->Writes to program memory are allowed
#pragma config GCP    = OFF    // General Code Segment Code Protect->Code protection is disabled
#pragma config JTAGEN = OFF    // JTAG Port Enable->JTAG port is disabled
// CONFIG2
#pragma config POSCMOD  = HS   // Primary Oscillator Select->HS oscillator mode selected
#pragma config DISUVREG = OFF  // Internal USB 3.3V Regulator Disable bit->Regulator is disabled
#pragma config IOL1WAY  = OFF  // IOLOCK One-Way Set Enable bit->Unlimited Writes To RP Registers
#pragma config OSCIOFNC = OFF  // Primary Oscillator Output Function->OSCO functions as CLKO (FOSC/2)
#pragma config FCKSM    = CSDCMD // Clock Switching and Monitor->Both Clock Switching and Fail-safe Clock Monitor are disabled
#pragma config FNOSC    = PRIPLL // Oscillator Select->Primary oscillator with PLL module
#pragma config PLL_96MHZ = ON  // 96MHz PLL Disable->Enabled
#pragma config PLLDIV   = DIV2 // USB 96 MHz PLL Prescaler Select bits->Oscillator input divided by 5 (20MHz input)
#pragma config IESO     = OFF  // Internal External Switch Over Mode->IESO mode (Two-speed start-up)disabled
// CONFIG3
#pragma config WPFP  = WPFP511 // Write Protection Flash Page Segment Boundary->Highest Page (same as page 170)
#pragma config WPDIS = WPDIS   // Segment Write Protection Disable bit->Segmented code protection disabled
#pragma config WPCFG = WPCFGDIS// Configuration Word Code Page Protection Select bit->Last page(at the top of program memory) and Flash configuration words are not protected
#pragma config WPEND = WPENDMEM// Segment Write Protection End Page Select bit->Write Protect from WPFP to the last page of memory

#if defined (DEBUG_SLAVE)
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
#include "statusManager.h"
#include "tableManager.h"
#include "pumpManager.h"
#include "humidifierManager.h"
#include "mem.h"
#include "serialcom.h"
#include "ram.h"
#include "define.h"
#include "typedef.h"
#include "stepper.h"

volatile const unsigned short *PtrTestResults = (unsigned short *) (__BL_TEST_RESULTS_ADDR);
volatile const unsigned long *BootPtrTestResults = (unsigned long *) (__BL_SW_VERSION);
// -----------------------------------------------------------------------------
//                      APPLICATION PROGRAM Service Routine
void APPLICATION_T1_InterruptHandler(void);
void APPLICATION_U1TX_InterruptHandler(void);
void APPLICATION_U1RX_InterruptHandler(void);
void APPLICATION_MI2C1_InterruptHandler(void);
void APPLICATION_SPI1_InterruptHandler(void);
void APPLICATION_SPI1TX_InterruptHandler(void);
void APPLICATION_SPI1RX_InterruptHandler(void);

void SPI1_InterruptHandler(void);
//void SPI1TX_InterruptHandler(void);
//void SPI1RX_InterruptHandler(void);
void MI2C1_InterruptHandler(void);

void Pippo(void);
// -----------------------------------------------------------------------------
/** T Y P E D E F S ******************************************************************* */
#if defined NO_BOOTLOADER

typedef union {
  unsigned char byte;
  struct {
    unsigned char  StatusType0: 1;
    unsigned char  StatusType1: 1;
    unsigned char  StatusType2: 1;
    unsigned char  StatusType3: 1;
    unsigned char  StatusType4: 1;
    unsigned char  StatusType5: 1;
    unsigned char  StatusType6: 1;
    unsigned char  StatusType7: 1;
  } Bit;
} DigInMicroSwitch;

static DigInMicroSwitch DigInMSwitch;

#else
#endif

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

int main(void)
{
    unsigned short i, find_circ;
    // Manually generated
// -----------------------------------------------------------------------------        
    // We can use 96MHZ PLL OR PLLX4: both are correct
	// 1. 96MHz PLL
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
	InitTMR();
	initIO();
    INTERRUPT_Initialize();
	initStatusManager();
	initTableStatusManager();
	initPumpStatusManager();
	initHumidifierStatusManager();
	initSerialCom();
    
#if defined NO_BOOTLOADER
  // if NON HARDCODED address is defined and BootLoader is NOT present, 
  // Slave Addres is read directly from dip switches)
  if (slave_id == UNIVERSAL_ID) {  
	/* Read 485 address bits from dip-switch on S1 */
	DigInMSwitch.Bit.StatusType0 = ~SW3;
	DigInMSwitch.Bit.StatusType1 = ~SW2;
	DigInMSwitch.Bit.StatusType2 = ~SW1;
	DigInMSwitch.Bit.StatusType3 = ~SW4;
	DigInMSwitch.Bit.StatusType4 = ~SW5;
	DigInMSwitch.Bit.StatusType5 = ~SW6;
	DigInMSwitch.Bit.StatusType6 = 0;
	DigInMSwitch.Bit.StatusType7 = 0;

  	slave_id = 0x00FF & DigInMSwitch.byte;	
	//slave_id = 44;  
	}
#else
  // if NON HARDCODED address is defined and BootLoader is present, Slave Addres is read from BootLoader (= from dip switches)
 if (slave_id == UNIVERSAL_ID)
	slave_id = SLAVE_ADDR();
#endif

#ifndef DEBUG_SLAVE
    /* enable 16msec watchdog */
    ENABLE_WDT();
#else
#endif	

    StartTimer(T_ERROR_STATUS);
    while (1)
	{
#ifndef DEBUG_SLAVE
        /* kicking the dog ;-) */
        ClrWdt();
#else
#endif			
        // main loop
		HumidifierManager();
        PumpManager();
        TableManager();
        StatusManager();
		TimerMg();
		gestioneIO();
		serialCommManager();
        // ---------------------------------------------------------------------
        // Home photocell status
        TintingAct.Home_photocell = PhotocellStatus(HOME_PHOTOCELL, FILTER);
        // Coupling photocell status
        TintingAct.Coupling_photocell = PhotocellStatus(COUPLING_PHOTOCELL, FILTER);
        // Valve photocell status
        TintingAct.Valve_photocell = PhotocellStatus(VALVE_PHOTOCELL, FILTER);
        // Rotating Table photocell status
        TintingAct.Table_photocell = PhotocellStatus(TABLE_PHOTOCELL, FILTER);
        // CanPresence photocell status
        TintingAct.CanPresence_photocell = PhotocellStatus(CAN_PRESENCE_PHOTOCELL, FILTER);           
        // Panel Table status
        TintingAct.PanelTable_state = PhotocellStatus(PANEL_TABLE, FILTER);    
        // Bases carriage State
        TintingAct.BasesCarriage_state = PhotocellStatus(BASES_CARRIAGE, FILTER);
        // ---------------------------------------------------------------------
        // Rotating Table position with respect to Reference
        TintingAct.Steps_position = GetStepperPosition(MOTOR_TABLE);
        if (Table_circuits_pos == ON) {
            find_circ = 0;
            for (i = 0; i < MAX_COLORANT_NUMBER; i++) {
                if (ABS(TintingAct.Steps_position - TintingAct.Circuit_step_pos[i]) < TintingAct.Steps_Tolerance_Circuit) {
                    find_circ = 1;
                    TintingAct.Circuit_Engaged = i+1; 
                    break;
                }    
            }
            if (find_circ == 0)
                TintingAct.Circuit_Engaged = 0;                 
        }    
        else
            TintingAct.Circuit_Engaged = 0;            
    }   
}

// -----------------------------------------------------------------------------
//                      APPLICATION PROGRAM Service Routine
// ISR used when BOOT and APPLICATION PROGRAMS are both present
#ifndef NO_BOOTLOADER

// Timer 1 Interrupt handler 
void __attribute__((address(__APPL_T1))) APPLICATION_T1_InterruptHandler(void)
{
    T1_InterruptHandler();
}
// UART1 RX Interrupt handler 
void __attribute__((address(__APPL_U1RX1))) APPLICATION_U1RX_InterruptHandler(void)
{
    U1RX_InterruptHandler();
}
// UART1 TX Interrupt handler 
void __attribute__((address(__APPL_U1TX1))) APPLICATION_U1TX_InterruptHandler(void)
{
    U1TX_InterruptHandler();
}
// I2C1 Interrupt handler 
void __attribute__((address(__APPL_MI2C1))) APPLICATION_MI2C1_InterruptHandler(void)
{
    MI2C1_InterruptHandler();
}
// SPI1 Interrupt handler 
void __attribute__((address(__APPL_SPI1))) APPLICATION_SPI1_InterruptHandler(void)
{
    SPI1_InterruptHandler();
}
// SPI1TX Interrupt handler 
void __attribute__((address(__APPL_SPI1TX))) APPLICATION_SPI1TX_InterruptHandler(void)
{
    SPI1TX_InterruptHandler();
}
// SPI1RX Interrupt handler 
void __attribute__((address(__APPL_SPI1RX))) APPLICATION_SPI1RX_InterruptHandler(void)
{
   SPI1RX_InterruptHandler();
}
// -----------------------------------------------------------------------------
// ISR used when only Application Program runs
#else
// Default Interrupt
void __attribute__((__interrupt__,auto_psv)) _DefaultInterrupt(void)
//void _DefaultInterrupt(void)
{
    Nop();
    Nop();
    while(1);
}
void __attribute__((__interrupt__,auto_psv)) _T1Interrupt(void)
{
   T1_InterruptHandler();
   
}
void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void)
{
   U1RX_InterruptHandler();
}
void __attribute__((__interrupt__, no_auto_psv)) _U1TXInterrupt(void)
{
   U1TX_InterruptHandler();
}
void __attribute__ ( ( interrupt, no_auto_psv ) ) _MI2C1Interrupt ( void )
{
   MI2C1_InterruptHandler();
} 
void __attribute__ ( ( interrupt, no_auto_psv ) ) _SPI1Interrupt ( void )
{
    SPI1_InterruptHandler();
}
/*
void __attribute__ ( ( interrupt, no_auto_psv ) ) _SPI1TXInterrupt ( void )
{
    SPI1TX_InterruptHandler();
}
void __attribute__ ( ( interrupt, no_auto_psv ) ) _SPI1RXInterrupt ( void )
{
   SPI1RX_InterruptHandler();
}
*/
// -----------------------------------------------------------------------------
#endif
//                      APPLICATION PROGRAM Service Routine NOT USED
// SPI1 GENERAL Interrupt handler 
void SPI1_InterruptHandler(void)
{
    Pippo();
}
/**/
// SPI1TX Interrupt handler 
void SPI1TX_InterruptHandler(void)
{
    Pippo();
}
/*
// SPI1RX Interrupt handler 
void SPI1RX_InterruptHandler(void)
{
    Pippo();
}
*/
// MI2C1 Interrupt handler 
void MI2C1_InterruptHandler(void)
{
    Pippo();
}
// -----------------------------------------------------------------------------