/**/
/*===========================================================================*/
/**
 **      @file    main.c
 **
 **      @brief   acts bootloader main module
 **/
/*===========================================================================*/
/**/
#include <xc.h>
#include "p24FJ256GB110.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "serialCom.h"
#include "define.h"
#include "MACRO.h"
#include "mem.h"
#include "ram.h"
#include "const.h"
#include "main.h"
#include "BL_UART_ServerMg.h"
#include "Bootloader.h"
#include "TimerMg.h"
#include "mem.h"
#include "progMemFunctions.h"
#include "serialCom.h"

/* Macro per la definizione degli Array dei filtri */
#define FILTER_WINDOW           3
/* Larghezza della finestra del filtro */
#define FILTER_WINDOW_LENGTH    (FILTER_WINDOW-1)
#define FILTER_WINDOW_LOOP      (FILTER_WINDOW-2)
#define MAX_CHANGE              (FILTER_WINDOW/2)
#define MIN_COUNT               (FILTER_WINDOW*3/4)
#define LOW_FILTER              0
#define HIGH_FILTER             1
#define ERRORE_FILTRO           2
#define COUNT_RESET             0
#define INPUT_ARRAY             6

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

#if defined (DEBUG_SLAVE)
#pragma config FWDTEN = OFF     // Watchdog Timer Enable->Watchdog Timer is disabled
#else
#pragma config FWDTEN = ON     // Watchdog Timer Enable->Watchdog Timer is enabled
#endif

/* Bootloader struct lives @ 0x200 */
const BootloaderPointers_T __attribute__ ((space(prog), address (0x200))) Bootloader = {
  BL_CRCarea,
  BL_CRCareaFlash
};

const unsigned short __attribute__ ((space(psv), address (__BL_CODE_CRC)))
dummy1 = 0; /* this will be changed in the .hex file by the CRC helper */

static void BL_Init(void);
void BL_UserInit(void);
//static char CheckApplicationPresence(DWORD address);
char CheckApplicationPresence(DWORD address);
void jump_to_appl();
void BL_ServerMg(void);
static void BL_IORemapping(void);

// -----------------------------------------------------------------------------
//                      APPLICATION PROGRAM Service Routine
void APPLICATION_T1_InterruptHandler();
void APPLICATION_U3TX_InterruptHandler();
void APPLICATION_U3RX_InterruptHandler();
void APPLICATION_U2TX_InterruptHandler();
void APPLICATION_U2RX_InterruptHandler();
void APPLICATION_SPI1_InterruptHandler();
void APPLICATION_SPI2_InterruptHandler();
void APPLICATION_SPI3_InterruptHandler();
void APPLICATION_I2C3_InterruptHandler();

void Pippo(void);
// -----------------------------------------------------------------------------

/*
**=============================================================================
**
**      Oggetto        : Funzione di servizio degli Interrupt NON usati dal 
**                       BootLoader
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
 
// -----------------------------------------------------------------------------

/** T Y P E D E F S ******************************************************************* */

/** D A T A *************************************************************************** */

/** T E X T *************************************************************************** */
void BL_GestStandAlone(void)
/*
**=============================================================================
**
**      Oggetto        : Gestione accensione e spegnimento apparecchio
**                                       attraverso il tasto ON/OFF
**
**      Parametri      : void
**
**      Ritorno        : void
**
**      Vers. - autore : 1.0  nuovo  G.Comai
**
**=============================================================================
*/
{
  if (BLState.livello == INIT) {
    if ((StatusTimer(T_WAIT_FORCE_BL) == T_RUNNING) && isUART_Force_Slave_BL_Cmd()) {
      BL_ForceStandAlone();
      resetNewProcessingMsg();
    }
    else if (StatusTimer(T_WAIT_FORCE_BL) == T_ELAPSED &&
             BL_StandAlone == BL_WAIT_CHECK_STAND_ALONE)

    BL_StandAlone = CheckApplicationPresence(BL_STAND_ALONE_CHECK);

    if (StatusTimer(T_WAIT_FORCE_BL) == T_ELAPSED)
      StopTimer(T_WAIT_FORCE_BL);

  } /* (BLState.level == INIT) */
}

unsigned short BL_CRCarea(unsigned char *pointer, unsigned short n_char,unsigned short CRCinit)
/**=============================================================================
**
**      Oggetto        : Calcola CRC di una zona di byte specificata
**                       dai parametri di ingresso
**
**      Parametri      : pointer      Indirizzo iniziale dell'area
**                                    da controllare
**                       n_char       Numero dei bytes da includere nel calcolo
**                       CRCinit      Valore iniziale di CRC ( = 0 se n_char
**                                    copre l'intera zona da verificare,
**                                    = CRCarea della zona precedente se
**                                    si sta procedendo a blocchi
**
**      Ritorno        : CRCarea      Nuovo valore del CRC calcolato
**
**      Vers. - autore : 1.0  nuovo   G. Comai
**
**=============================================================================*/
{
  /* La routine proviene dalla dispensa "CRC Fundamentals", pagg. 196, 197. */

  /* Nota sull'algoritmo:
     dato un vettore, se ne calcoli il CRC_16: se si accodano i 2 bytes del
     CRC_16 a tale vettore (low byte first!!!), il CRC_16 calcolato
     sul vettore così ottenuto DEVE valere zero.
     Tale proprietà può essere sfruttata nelle comunicazione seriali
     per verificare che un messaggio ricevuto,
     contenente in coda i 2 bytes del CRC_16 (calcolati dal trasmettitore),
     sia stato inviato correttamente: il CRC_16, calcolato dal ricevente
     sul messaggio complessivo deve valere zero. */

  unsigned long i;
  unsigned short index;
  unsigned char psv_shadow;

  /* save the PSVPAG */
  psv_shadow = PSVPAG;

  /* set the PSVPAG for accessing CRC_TABLE[] */
  PSVPAG = __builtin_psvpage (BL_CRC_TABLE);

  for (i = 0; i < n_char; i++) {
    index = ( (CRCinit ^ ( (unsigned short) *pointer & 0x00FF) ) & 0x00FF);
    CRCinit = ( (CRCinit >> 8) & 0x00FF) ^ BL_CRC_TABLE[index];
    pointer = pointer + 1;

    /* Reset Watchdog*/
    ClrWdt();
  } /* end for */

  /* restore the PSVPAG for the compiler-managed PSVPAG */
  PSVPAG = psv_shadow;

  return CRCinit;
} /* end CRCarea */

unsigned short BL_CRCareaFlash(unsigned long address, unsigned long n_word,unsigned short CRCinit)
/**=============================================================================
**
**      Oggetto        : Calcola CRC di una zona di byte specificata
**                       dai parametri di ingresso
**
**      Parametri      : pointer      Indirizzo iniziale dell'area
**                                    da controllare
**                       n_char       Numero dei bytes da includere nel calcolo
**                       CRCinit      Valore iniziale di CRC ( = 0 se n_char
**                                    copre l'intera zona da verificare,
**                                    = CRCarea della zona precedente se
**                                    si sta procedendo a blocchi
**
**      Ritorno        : CRCarea      Nuovo valore del CRC calcolato
**
**      Vers. - autore : 1.0  nuovo   G. Comai
**
**=============================================================================*/
{
  DWORD_VAL dwvResult;
  /* La routine proviene dalla dispensa "CRC Fundamentals", pagg. 196, 197. */

  /* Nota sull'algoritmo:
     dato un vettore, se ne calcoli il CRC_16: se si accodano i 2 bytes del
     CRC_16 a tale vettore (low byte first!!!), il CRC_16 calcolato
     sul vettore così ottenuto DEVE valere zero.
     Tale proprietà può essere sfruttata nelle comunicazione seriali
     per verificare che un messaggio ricevuto,
     contenente in coda i 2 bytes del CRC_16 (calcolati dal trasmettitore),
     sia stato inviato correttamente: il CRC_16, calcolato dal ricevente
     sul messaggio complessivo deve valere zero. */

  unsigned long i;
  unsigned char j;
  unsigned short index;
  unsigned char psv_shadow;

  WORD wTBLPAGSave;

  /* save the PSVPAG */
  psv_shadow = PSVPAG;
  /* set the PSVPAG for accessing CRC_TABLE[] */
  PSVPAG = __builtin_psvpage (BL_CRC_TABLE);

  for (i = 0; i < n_word; i++) {

    /* Reset Watchdog*/
    ClrWdt();

    wTBLPAGSave = TBLPAG;
    TBLPAG = ((DWORD_VAL*)&address)->w[1];

    dwvResult.w[1] = __builtin_tblrdh((WORD)address);
    dwvResult.w[0] = __builtin_tblrdl((WORD)address);
    TBLPAG = wTBLPAGSave;
    for (j=0; j<4; j++) {
      index = ( (CRCinit ^ ( (unsigned short) dwvResult.v[j] & 0x00FF) ) & 0x00FF);
      CRCinit = ( (CRCinit >> 8) & 0x00FF) ^ BL_CRC_TABLE[index];
    }
    address+=2;
  } /* end for */

  /* restore the PSVPAG for the compiler-managed PSVPAG */
  PSVPAG = psv_shadow;

  return CRCinit;
} /* end CRCarea */

/********************************************************************
 * Function:        static void BL_Init(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        BL_Init is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.
 *
 * Note:            None
 *******************************************************************/
static void BL_Init(void)
{
  // Use Alternate Vector Table 
  INTCON2bits.ALTIVT = 1;
  BL_IORemapping();
  BL_TimerInit();
  BL_initSerialCom();
  BL_UserInit();
}

/********************************************************************
 * Function:        void BL_UserInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        BL_User_Init is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.
 *
 * Note:            None
 *******************************************************************/
void BL_UserInit(void)
{
  BLState.livello = INIT;
  BL_StandAlone = BL_WAIT_CHECK_STAND_ALONE;

  StartTimer(T_FIRST_WINDOW);
  StartTimer(T_WAIT_FORCE_BL);
}


static void BL_IORemapping(void)
/*
**=============================================================================
**
**      Oggetto        : Remapping PIN PIC
**
**
**      Parametri      : void
**
**      Ritorno        : void
**
**      Vers. - autore : 
**=============================================================================
*/
{
    __builtin_write_OSCCONL(OSCCON & 0xbf); /*UnLock IO Pin Remapping*/
     
    // UART2 - RS232
    RPOR15bits.RP30R = 0x0005;  //RF2->UART2:U2TX;
    RPINR19bits.U2RXR = 0x0011; //RF5->UART2:U2RX;
    // UART3 - RS485
    RPINR17bits.U3RXR = 0x002A; //RD12->UART3:U3RX;
    RPOR6bits.RP12R   = 0x001C; //RD11->UART3:U3TX;
    // SPI1 - MOTOR DRIVER
    RPINR20bits.SCK1R = 0x0000; //RB0->SPI1:SCK1IN;
    RPOR6bits.RP13R = 0x0007;   //RB2->SPI1:SDO1;
    RPINR20bits.SDI1R = 0x0001; //RB1->SPI1:SDI1;
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
    // -------------------------------------------------------------------------
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
//    LATA = 0x0000;
//    LATB = 0x0000;
//    LATC = 0x0000;

    /****************************************************************************
     * Setting the Weak Pull Up and Weak Pull Down SFR(s)
     ***************************************************************************/
/*
    IOCPDA = 0x0000;
    IOCPDB = 0x0000;
    IOCPDC = 0x0000;
    IOCPUA = 0x0000;
    IOCPUB = 0x0000;
    IOCPUC = 0x0000;
*/
    /****************************************************************************
     * Setting the Open Drain SFR(s)
     ***************************************************************************/
//    ODCA = 0x0000;
//    ODCB = 0x0000;
//    ODCC = 0x0000;

    /****************************************************************************
     * Setting the Analog/Digital Configuration SFR(s)
     ***************************************************************************/
    AD1PCFGH = 0x0000;
    // No Analog Input, all Digital 
    AD1PCFGL = 0xFFFF;    
    /****************************************************************************
     * Setting the GPIO Direction SFR(s)
     ***************************************************************************/
    TRISAbits.TRISA0  = INPUT;  // FO_ACC
    TRISAbits.TRISA1  = INPUT;  // BUSY_BRD
    TRISAbits.TRISA4  = INPUT;  // BUTTON
    TRISAbits.TRISA5  = OUTPUT; // CS_PMP
    TRISAbits.TRISA6  = OUTPUT; // NEB_IN
    TRISAbits.TRISA7  = OUTPUT; // NEB_F
    TRISAbits.TRISA9  = OUTPUT; // FO_GEN1
    TRISAbits.TRISA14 = INPUT;  // LEV_SENS
    // -------------------------------------------------------------------------
    TRISBbits.TRISB0   = OUTPUT; // SCK_DRIVER
    TRISBbits.TRISB1   = OUTPUT; // SDI_DRIVER
    TRISBbits.TRISB2   = INPUT;  // SDO_DRIVER
    TRISBbits.TRISB3   = INPUT;  // BRUSH_F1
    TRISBbits.TRISB4   = OUTPUT; // STCK_PMP
    TRISBbits.TRISB5   = OUTPUT; // STCK_EV
    TRISBbits.TRISB8   = INPUT;  // BUSY_EV
    TRISBbits.TRISB9   = OUTPUT; // CS_EV
    TRISBbits.TRISB10  = OUTPUT; // STBY_RST_PMP
    TRISBbits.TRISB11  = INPUT;  // FLAG_PMP
    TRISBbits.TRISB12  = INPUT;  // BUSY_PMP
    TRISBbits.TRISB13  = OUTPUT; // STBY_RST_BRD
    TRISBbits.TRISB14  = OUTPUT; // STCK_BRD
    TRISBbits.TRISB15  = INPUT;  // FLAG_BRD
    // -------------------------------------------------------------------------
    TRISCbits.TRISC2   = INPUT;  // FO_VALVE
    TRISCbits.TRISC3   = OUTPUT; // EEPROM_CS
    TRISCbits.TRISC4   = INPUT;  // FO_BRD
    TRISCbits.TRISC12  = INPUT;  // OSC1
    TRISCbits.TRISC13  = OUTPUT; // I3_BRUSH
    TRISCbits.TRISC14  = OUTPUT; // I4_BRUSH
    TRISCbits.TRISC15  = INPUT;  // OSC2
    // -------------------------------------------------------------------------
    TRISDbits.TRISD0   = OUTPUT; // SDO_TC72
    TRISDbits.TRISD1   = OUTPUT; // RELAY
    TRISDbits.TRISD2   = OUTPUT; // SCK_TC72
    TRISDbits.TRISD4   = INPUT;  // RELAY_F
    TRISDbits.TRISD5   = OUTPUT; // LASER_BHL
    TRISDbits.TRISD6   = OUTPUT; // IN1_BRUSH
    TRISDbits.TRISD7   = OUTPUT; // DECAY
    TRISDbits.TRISD8   = INPUT;  // FO_HOME
    TRISDbits.TRISD9   = INPUT;  // SDI_TC72
    TRISDbits.TRISD10  = OUTPUT; // I2_BRUSH
    TRISDbits.TRISD11  = OUTPUT; // RS485_TXD
    TRISDbits.TRISD12  = INPUT;  // RS485_RXD
    TRISDbits.TRISD13  = OUTPUT; // RS485_DE
    TRISDbits.TRISD14  = OUTPUT; // CS_BRD
    TRISDbits.TRISD15  = INPUT;  // INT_CAR
    // -------------------------------------------------------------------------
    TRISEbits.TRISE0   = INPUT;  // FLAG_EV
    TRISEbits.TRISE1   = INPUT;  // FO_CPR
    TRISEbits.TRISE2   = OUTPUT; // OUT_24V_IN
    TRISEbits.TRISE4   = OUTPUT; // LED_ON_OFF
    TRISEbits.TRISE5   = OUTPUT; // RST_BRUSH
    TRISEbits.TRISE6   = OUTPUT; // SCL_SHT31
    TRISEbits.TRISE7   = OUTPUT; // SDA_SHT31
    TRISEbits.TRISE9   = INPUT;  // BRUSH_F2
    // -------------------------------------------------------------------------
    TRISFbits.TRISF0   = OUTPUT; // RST_SHT31
    TRISFbits.TRISF1   = INPUT;  // ALERT_SHT31
    TRISFbits.TRISF2   = OUTPUT; // RS232_TXD
    TRISFbits.TRISF4   = INPUT;  // FO_GEN2
    TRISFbits.TRISF5   = INPUT;  // RS232_RXD
    TRISFbits.TRISF8   = INPUT;  // IO_GEN2
    TRISFbits.TRISF12  = INPUT;  // INT_PAN
    TRISFbits.TRISF13  = INPUT;  // IO_GEN1
    // -------------------------------------------------------------------------
    TRISGbits.TRISG1   = OUTPUT; // IN2_BRUSH
    TRISGbits.TRISG2   = OUTPUT; // USB_DP
    TRISGbits.TRISG3   = OUTPUT; // USB_DN
    TRISGbits.TRISG6   = OUTPUT; // SCK_EEPROM
    TRISGbits.TRISG7   = INPUT;  // SDI_EEPROM
    TRISGbits.TRISG8   = OUTPUT; // SDO_EEPROM
    TRISGbits.TRISG9   = OUTPUT; // STBY_RST_EV
    TRISGbits.TRISG12  = OUTPUT; // AIR_PUMP_IN
    TRISGbits.TRISG13  = INPUT;  // AIR_PUMP_F
    TRISGbits.TRISG14  = OUTPUT; // CE_TC72
    TRISGbits.TRISG15  = INPUT;  // OUT_24V_FAULT
    // -------------------------------------------------------------------------
}/* end IORemapping() */

/********************************************************************
 * Function:        char CheckApplPres(DWORD address)
 *
 * PreCondition:    None
 *
 * Input:           Long -> First Application Flash Address
 *
 * Output:          Char -> BootLoader Stand Alone = TRUE
 *
 * Side Effects:    None
 *
 * Overview:        CheckApplPres is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.
 *
 * Note:            None
 *******************************************************************/
char CheckApplicationPresence(DWORD address)
{
  return BL_NO_STAND_ALONE;    
/*
    if (ReadProgramMemory(address) == APPL_FLASH_MEMORY_ERASED_VALUE)
    return BL_STAND_ALONE;

  return BL_NO_STAND_ALONE;
*/
} // end CheckApplicationPresence() 

static void hw_init(void)
{
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

	// wait for clock to stabilize: Primary Oscillator with PLL module (XTPLL, HSPLL))
	while (OSCCONbits.COSC != 0b011)
	  ;	
	// wait for PLL to lock: PLL module is in lock, PLL start-up timer is satisfied 
	while (OSCCONbits.LOCK != 1)
	  ;        
    // Auto generate initialization
// -----------------------------------------------------------------------------    
}

void jump_to_appl()
{
  // Disabilito periferica relativa al Timer 1, prima del salto perchè gestito solo dalla AIVT 
  T2CON = 0x0000;
  PR2 = 0xFFFF;
  _T2IF = 0;
  _T2IE = 0;    
  // Write slave address into reserved data location before relinquishing control to the application. 
  slave_index = (unsigned short) BL_slave_id;

  boot_fw_version = (unsigned long) BL_SW_VERSION;  
  
  // Use standard vector table 
  INTCON2bits.ALTIVT = 0;

  // jump to app code, won't return 
  __asm__ volatile ("goto " __APPL_GOTO_ADDR);  
}

int main(void)
{
	// Hardware initialization 
	hw_init();
	// BootLoader initialization 
	BL_Init();
    // Slave address = 44
	BL_slave_id = TINTING_ID;
	ENABLE_WDT();
	do {        
		// Kick the dog 
		ClrWdt();        
		if(StatusTimer(T_FIRST_WINDOW) == T_ELAPSED) {
			BL_GestStandAlone();
			BL_serialCommManager();
			BL_UART_ServerMg();
		}
        BL_TimerMg();
	} while(BL_StandAlone != BL_NO_STAND_ALONE);
	Nop();
	Nop();
	jump_to_appl(); // goodbye 

  return 0; // unreachable, just get rid of compiler warning 
} // end main 

//------------------------------------------------------------------------------
// TIMER 1
void __attribute__((__interrupt__,auto_psv)) _T1Interrupt(void)
{
    APPLICATION_T1_InterruptHandler();
}
// UART3 TX
void __attribute__((__interrupt__,auto_psv)) _U3TXInterrupt(void)
{
    APPLICATION_U3TX_InterruptHandler();
}
// UART3 RX
void __attribute__((__interrupt__,auto_psv)) _U3RXInterrupt(void)
{
    APPLICATION_U3RX_InterruptHandler();
}
// UART2 TX
void __attribute__((__interrupt__,auto_psv)) _U2TXInterrupt(void)
{
    APPLICATION_U2TX_InterruptHandler();
}
// UART2 RX
void __attribute__((__interrupt__,auto_psv)) _U2RXInterrupt(void)
{
    APPLICATION_U2RX_InterruptHandler();
}
// SPI1
void __attribute__((__interrupt__,auto_psv)) _SPI1Interrupt(void)
{
    APPLICATION_SPI1_InterruptHandler();
}
// SPI2
void __attribute__((__interrupt__,auto_psv)) _SPI2Interrupt(void)
{
    APPLICATION_SPI2_InterruptHandler();
}
// SPI3
void __attribute__((__interrupt__,auto_psv)) _SPI3Interrupt(void)
{
    APPLICATION_SPI3_InterruptHandler();
}
// I2C3
void __attribute__((__interrupt__,auto_psv)) _MI2C3Interrupt(void) 
{
    APPLICATION_I2C3_InterruptHandler();
}
// -----------------------------------------------------------------------------
//                      APPLICATION PROGRAM Service Routine
// Timer 1 Interrupt handler 
void __attribute__((address(__APPL_T1))) APPLICATION_T1_InterruptHandler(void)
{
    Pippo();
}
// UART3 TX
void __attribute__((address(__APPL_U3TX1))) APPLICATION_U3TX_InterruptHandler(void)
{
    Pippo();
}
// UART3 RX
void __attribute__((address(__APPL_U3RX1))) APPLICATION_U3RX_InterruptHandler(void)
{
    Pippo();
}
// UART2 TX
void __attribute__((address(__APPL_U2TX1))) APPLICATION_U2TX_InterruptHandler(void)
{
    Pippo();
}
// UART2 RX
void __attribute__((address(__APPL_U2RX1))) APPLICATION_U2RX_InterruptHandler(void)
{
    Pippo();
}
// SPI1
void __attribute__((address(__APPL_SPI1))) APPLICATION_SPI1_InterruptHandler(void)
{
    Pippo();
}
// SPI2
void __attribute__((address(__APPL_SPI2))) APPLICATION_SPI2_InterruptHandler(void)
{
    Pippo();
}
// SPI3
void __attribute__((address(__APPL_SPI3))) APPLICATION_SPI3_InterruptHandler(void)
{
    Pippo();
}
// I2C3
void __attribute__((address(__APPL_I2C3))) APPLICATION_I2C3_InterruptHandler(void)
{
    Pippo();
}
// -----------------------------------------------------------------------------
