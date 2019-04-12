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
#include "i2c3.h"
#include "spi.h"
#include "gestIO.h"
#include "eepromManager.h"
#include "eeprom.h"
#include "spi3.h"

volatile const unsigned short *PtrTestResults = (unsigned short *) (__BL_TEST_RESULTS_ADDR);
volatile const unsigned long *BootPtrTestResults = (unsigned long *) (__BL_SW_VERSION);
// -----------------------------------------------------------------------------
//                      APPLICATION PROGRAM Service Routine
void APPLICATION_T1_InterruptHandler(void);
void APPLICATION_U2TX_InterruptHandler(void);
void APPLICATION_U2RX_InterruptHandler(void);
void APPLICATION_U3TX_InterruptHandler(void);
void APPLICATION_U3RX_InterruptHandler(void);
void APPLICATION_SPI1_InterruptHandler(void);
void APPLICATION_SPI2_InterruptHandler(void);
void APPLICATION_SPI3_InterruptHandler(void);
void APPLICATION_MI2C3_InterruptHandler(void);

void  U2RX_InterruptHandler(void);
void  U2TX_InterruptHandler(void);
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
	initTableStatusManager();
    initTableParam();
	initPumpStatusManager();
    initPumpParam();
	initHumidifierStatusManager();
    initHumidifierParam();
	initSerialCom();
    I2C3_Initialize(); 
    Check_Presence = FALSE; 
#if defined NO_BOOTLOADER
  // if BootLoader is NOT present, Slave address is HARDCODED without dip switch 
  // slave_id = 44;
  slave_id = TINTING;  
#else  
  // if BootLoader is present, Slave Addres is read from BootLoader
  slave_id = SLAVE_ADDR();
  //slave_id = TINTING;    
  
#endif

#ifndef WATCH_DOG_DISABLE
    /* enable watchdog */
    ENABLE_WDT();
#else
#endif	
    StartTimer(T_ERROR_STATUS);
//    StartTimer(T_RESET);
    __builtin_write_OSCCONL(OSCCON & 0xbf); /*UnLock IO Pin Remapping*/     
    spi_remapping(SPI_1);
    __builtin_write_OSCCONL(OSCCON | 0x40);   /*Lock IO Pin Remapping*/  

    spi_init(SPI_1);  //SPI controllo motore    
	initStatusManager();      
    EEPROMInit();
    spi_remapping(SPI_2);
    spi_init(SPI_2); //SPI controllo EEprom
    spi_init(SPI_3); //SPI Sensore temperatura  
    StartTimer(T_MEASURING_TIME);
    MAX_Cycle_Duration = 0;
    Timer_New = 0;
#ifdef DEBUG_MMT
//    Enable_Driver(MOTOR_TABLE); //CN14   //PORTBbits.RB13  
    Enable_Driver(MOTOR_PUMP);  //CN15   //PORTBbits.RB10
//    Enable_Driver(MOTOR_VALVE);  //CN17 //PORTGbits.RG9
    init_test_Stepper(MOTOR_TABLE);
//    init_test_Stepper(MOTOR_PUMP); 
//    init_test_Stepper(MOTOR_VALVE);
    StartTimer(T_POLLING_STEPPER); 
#endif
    
    while (1)
	{
#ifdef DEBUG_MMT
        TimerMg();
        //Collaudo_Output();
		gestioneIO();              
        if (StatusTimer(T_POLLING_STEPPER)==T_ELAPSED)
        {
            test_Stepper(MOTOR_TABLE);
//            test_Stepper(MOTOR_PUMP);
//            test_Stepper(MOTOR_VALVE);  
            StartTimer(T_POLLING_STEPPER);
        }
        StepperMovementsManager();
#else        

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
        StatusManager();
        StepperMovementsManager();
        // Manager del sensore di Temperatura 
        //SPI3_Manager();
        // Manager del sensore di T/H
        //I2C_Manager();  //se non si connette il sensore questo sequencer blocca il main        
/*        
if (StatusTimer(T_RESET) == T_ELAPSED){
    StopTimer(T_RESET);
    StartTimer(T_RESET);
    if (pippo == 0) {
        WATER_PUMP_ON();
        pippo = 1;
    }
    else if (pippo == 1) {
        WATER_PUMP_OFF();
        pippo = 0;
    }   
}
*/
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
        // Valve Open Photocell
        TintingAct.ValveOpen_photocell = PhotocellStatus(VALVE_OPEN_PHOTOCELL, FILTER);
        // Water Level State
        TintingAct.WaterLevel_state = !getWaterLevel();        
// ---------------------------------------------------------------------
#if defined NOLAB	
        TintingAct.Circuit_Engaged = 1;
#else        
// Check if a Circuit is Engaged    
        // Read Position
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
#endif        
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
// UART3 TX
void __attribute__((address(__APPL_U3TX1))) APPLICATION_U3TX_InterruptHandler(void)
{
    U3TX_InterruptHandler();
}
// UART3 RX
void __attribute__((address(__APPL_U3RX1))) APPLICATION_U3RX_InterruptHandler(void)
{
    U3RX_InterruptHandler();
}
// UART2 TX
void __attribute__((address(__APPL_U2TX1))) APPLICATION_U2TX_InterruptHandler(void)
{
    U2TX_InterruptHandler();
}
// UART2 RX
void __attribute__((address(__APPL_U2RX1))) APPLICATION_U2RX_InterruptHandler(void)
{
    U2RX_InterruptHandler();
}
// SPI1
void __attribute__((address(__APPL_SPI1))) APPLICATION_SPI1_InterruptHandler(void)
{
    SPI1_InterruptHandler();
}
// SPI2
void __attribute__((address(__APPL_SPI2))) APPLICATION_SPI2_InterruptHandler(void)
{
    SPI2_InterruptHandler();
}
// SPI3
void __attribute__((address(__APPL_SPI3))) APPLICATION_SPI3_InterruptHandler(void)
{
    SPI3_InterruptHandler();
}
// I2C3
void __attribute__((address(__APPL_I2C3))) APPLICATION_I2C3_InterruptHandler(void)
{
    MI2C3_InterruptHandler();
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
void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt(void)
{
   U2RX_InterruptHandler();
}
void __attribute__((__interrupt__, no_auto_psv)) _U2TXInterrupt(void)
{
   U2TX_InterruptHandler();
}
void __attribute__((__interrupt__, no_auto_psv)) _U3RXInterrupt(void)
{
   U3RX_InterruptHandler();
}
void __attribute__((__interrupt__, no_auto_psv)) _U3TXInterrupt(void)
{
   U3TX_InterruptHandler();
}
void __attribute__ ( ( interrupt, no_auto_psv ) ) _SPI1Interrupt ( void )
{
    SPI1_InterruptHandler();
}
void __attribute__ ( ( interrupt, no_auto_psv ) ) _SPI2Interrupt ( void )
{
    SPI2_InterruptHandler();
}
void __attribute__ ( ( interrupt, no_auto_psv ) ) _SPI3Interrupt ( void )
{
    SPI3_InterruptHandler();
}
void __attribute__ ( ( interrupt, no_auto_psv ) ) _MI2C3Interrupt ( void )
{
   MI2C3_InterruptHandler();
} 
// -----------------------------------------------------------------------------
#endif
//                      APPLICATION PROGRAM Service Routine NOT USED
void  U2RX_InterruptHandler(void)
{
    Pippo();
}
void  U2TX_InterruptHandler(void)
{
    Pippo();
}
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