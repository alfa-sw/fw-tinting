/* 
 * File:  gestIO.c
 * Author: michele.abelli
 * Description: Digital Input Sampling 
 * Created on 13 marzo 2020, 17.30
 */
#include "typedef.h"
#include "gestIO.h"
#include "p24FJ256GB110.h"
#include "timerMg.h"
#include "define.h"
#include "ram.h"
#include "humidifierManager.h"


#define FILTER_WINDOW           5
//#define INPUT_ARRAY				4
#define INPUT_ARRAY				16
#define FILTER_WINDOW_LENGTH    (FILTER_WINDOW-1)
#define FILTER_WINDOW_LOOP      (FILTER_WINDOW-2)
#define MAX_CHANGE              (FILTER_WINDOW/2)
#define MIN_COUNT               (FILTER_WINDOW*3/4)
#define ERRORE_FILTRO 	        2
#define LOW_FILTER              0
#define HIGH_FILTER             1
#define COUNT_RESET             0
#define MASK_FILTER_OFF         0x0000

const unsigned char MASK_BIT_8[]={0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
const unsigned short MASK_BIT_16[]={0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,0x100,0x200,0x400,0x800,0x1000,0x2000,0x4000,0x8000};

DigInStatusType DigInStatus, DigInNotFiltered;
DigInStatusType DigInStatus, DigInNotFilteredExtended;

static unsigned char  n_filter;
static unsigned char zero_counter, one_counter, ChangeStatus, Out_Status;
static signed char index_0, index_1;
static unsigned char DummyOutput_low, DummyOutput_high, shift;
static unsigned char  FILTRAGGIO_LOW[FILTER_WINDOW];
static unsigned char  FILTRAGGIO_HIGH[FILTER_WINDOW];
//static DigInStatusType OutputFilter;
static unsigned short FilterSensorInput(DigInStatusType InputFilter);
void readIn(void);

void initIO(void)
{
    /****************************************************************************
     * Setting the Output Latch SFR(s)
     ***************************************************************************/
//    LATA = 0x0000;
    LATB = 0x0000;
    LATC = 0x0000;

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
    ODCB = 0x0000;
    ODCC = 0x0000;

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
    TRISAbits.TRISA7  = INPUT;  // NEB_F
    TRISAbits.TRISA9  = INPUT; // FO_GEN1
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
    TRISCbits.TRISC2   = INPUT;  // FO_VALV
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
    // Set PPS - SPI1
    RPOR5bits.RP10R   = 0x0009; //RB10->SPI1:SS1OUT;
    RPOR5bits.RP11R   = 0x0008; //RB11->SPI1:SCK1OUT;
    RPINR20bits.SDI1R = 0x000D; //RB13->SPI1:SDI1;
    RPOR6bits.RP12R   = 0x0007; //RB12->SPI1:SDO1;	
    
    
}

void INTERRUPT_Initialize (void)
{
    // Enable nested interrupts
    INTCON1bits.NSTDIS = 0;    
    //    UERI: U2E - UART2 Error
    //    Priority: 1
        IPC16bits.U2ERIP = 1;
    //    UTXI: U2TX - UART2 Transmitter
    //    Priority: 1
        IPC7bits.U2TXIP = 1;
    //    URXI: U2RX - UART2 Receiver
    //    Priority: 1
        IPC7bits.U2RXIP = 1;
    //    UERI: U3E - UART3 Error
    //    Priority: 1
        IPC20bits.U3ERIP = 1;
    //    UTXI: U3TX - UART3 Transmitter
    //    Priority: 1
        IPC20bits.U3TXIP = 1;
    //    URXI: U3RX - UART3 Receiver
    //    Priority: 1
        IPC20bits.U3RXIP = 1;
    //    MICI: MI2C3 - I2C3 Master Events
    //    Priority: 1
        IPC21bits.MI2C3P = 1;
    //    TI: T1 - Timer1
    //    Priority: 1
        IPC0bits.T1IP = 1;       
}

void gestioneIO(void)
{
	// Check the IO value
	if (StatusTimer(T_READ_IO)==T_HALTED)
	{
		StartTimer(T_READ_IO);

	}

	if (StatusTimer(T_READ_IO)==T_ELAPSED)
	{
		StartTimer(T_READ_IO);
		readIn();
		DigInStatus.word=FilterSensorInput(DigInNotFiltered);
	}

}

static unsigned short  FilterSensorInput(DigInStatusType InputFilter)
	
/*=====================================================================*//**
**
**      @brief Filter of the keys state
**
**      @param InputFilter input
**
**      @retval ouput filtered
**
**
**
*//*=====================================================================*//**
*/
{
  unsigned char temp_uc;
  signed char temp_sc;

  /* n_filter = Finestra campioni del filtro */
  if (n_filter < FILTER_WINDOW_LENGTH)
  {
    n_filter++;
  }
  else
  {
    n_filter = 0;
  }

  FILTRAGGIO_LOW[n_filter] = InputFilter.byte.low;
  FILTRAGGIO_HIGH[n_filter] = InputFilter.byte.high;

  for(temp_uc = 0 ; temp_uc < (INPUT_ARRAY / 2); temp_uc++) // INPUT_ARRAY = N° di ingressi da filtrare
  {
    shift = 0x1 << temp_uc;

    //ByteLow
    for(temp_sc = FILTER_WINDOW_LOOP; temp_sc >= 0; temp_sc--)
    {
      //Indice 0
      index_0 = n_filter - temp_sc;
      if (index_0 < 0)
      {
        index_0 += FILTER_WINDOW;
      }
      //Indice 1
      index_1 = n_filter - temp_sc - 1;
      if (index_1 < 0)
      {
        index_1 += FILTER_WINDOW;
      }

      if ( (FILTRAGGIO_LOW[index_0] ^ FILTRAGGIO_LOW[index_1]) & shift)
      {
        ChangeStatus++;
      }

      if ( FILTRAGGIO_LOW[index_0] & shift)
      {
        one_counter++;
      }
      else
      {
        zero_counter++;
      }

      if (temp_sc == 0)
      {
        if (FILTRAGGIO_LOW[index_1] & shift)
        {
          one_counter++;
        }
        else
        {
          zero_counter++;
        }
      }
    }

    if (ChangeStatus > MAX_CHANGE)
    {
      if (zero_counter >= MIN_COUNT)
      {
        Out_Status = LOW_FILTER;
      }
      else if (one_counter >= MIN_COUNT)
      {
        Out_Status = HIGH_FILTER;
      }
      else
      {
        Out_Status = ERRORE_FILTRO;
      }
    }
    else
    {
      if (zero_counter > one_counter)
      {
        Out_Status = LOW_FILTER;
      }
      else
      {
        Out_Status = HIGH_FILTER;
      }
    }

    zero_counter = COUNT_RESET;
    one_counter = COUNT_RESET;
    ChangeStatus = COUNT_RESET;

    // Segnale d'ingresso filtrato
    if (Out_Status != ERRORE_FILTRO)
    {
      if (!temp_uc)
      {
        DummyOutput_low = Out_Status;
      }
      else
      {
        DummyOutput_low |= (Out_Status << temp_uc);
      }
    }

    /*Byte High*/
    for(temp_sc = FILTER_WINDOW_LOOP; temp_sc >= 0 ; temp_sc--)
    {
      /*Indice 0*/
      index_0 = n_filter - temp_sc;
      if (index_0 < 0)
      {
        index_0 += FILTER_WINDOW;
      }
      /*Indice 1*/
      index_1 = n_filter - temp_sc - 1;
      if (index_1 < 0)
      {
        index_1 += FILTER_WINDOW;
      }

      if ( (FILTRAGGIO_HIGH[index_0] ^ FILTRAGGIO_HIGH[index_1]) & shift)
      {
        ChangeStatus++;
      }

      if (FILTRAGGIO_HIGH[index_0] & shift)
      {
        one_counter++;
      }
      else
      {
        zero_counter++;
      }

      if (temp_sc == 0)
      {
        if (FILTRAGGIO_HIGH[index_1] & shift)
        {
          one_counter++;
        }
        else
        {
          zero_counter++;
        }
      }
    }

    if (ChangeStatus > MAX_CHANGE)
    {
      if (zero_counter >= MIN_COUNT)
      {
        Out_Status = LOW_FILTER;
      }
      else if (one_counter >= MIN_COUNT)
      {
        Out_Status = HIGH_FILTER;
      }
      else
      {
        Out_Status = ERRORE_FILTRO;
      }
    }
    else
    {
	if (zero_counter > one_counter)
      {
        Out_Status = LOW_FILTER;
      }
      else
      {
        Out_Status = HIGH_FILTER;
      }
    }

    zero_counter = COUNT_RESET;
    one_counter = COUNT_RESET;
    ChangeStatus = COUNT_RESET;

    /* Segnale d'ingresso filtrato */
    if (Out_Status != ERRORE_FILTRO)
    {
      if (!temp_uc)
      {
        DummyOutput_high = Out_Status;
      }
      else
      {
        DummyOutput_high |= (Out_Status << temp_uc);
      }
    }
  }

  OutputFilter.byte.low = DummyOutput_low;
  OutputFilter.byte.high = DummyOutput_high;

  return (OutputFilter.word);

} /*end FilterSensorInput*/

unsigned char getWaterLevel(void)
{
    return DigInStatus.Bit.StatusType0;
}

void readIn(void)
{		
	DigInNotFiltered.Bit.StatusType0 = LEV_SENS;
	DigInNotFiltered.Bit.StatusType1 = FO_CPR; // Jar Presence
	DigInNotFiltered.Bit.StatusType2 = FO_VALV; // Autocap Open
	DigInNotFiltered.Bit.StatusType3 = FO_ACC;
	DigInNotFiltered.Bit.StatusType4 = FO_BRD; // Door Open
	DigInNotFiltered.Bit.StatusType5 = FO_HOME; // Mixer Home
    DigInNotFiltered.Bit.StatusType6 = FO_GEN1;
	DigInNotFiltered.Bit.StatusType7 = FO_GEN2;
    DigInNotFiltered.Bit.StatusType8 = INT_CAR; // Door Closed Microswitch
    DigInNotFiltered.Bit.StatusType9 = INT_PAN;
    DigInNotFiltered.Bit.StatusType10 = IO_GEN1;
    DigInNotFiltered.Bit.StatusType11 = IO_GEN2;
    DigInNotFiltered.Bit.StatusType12 = BUTTON;
    DigInNotFiltered.Bit.StatusType13 = BUSY_BRD;
    DigInNotFiltered.Bit.StatusType14 = BUSY_PMP;
    DigInNotFiltered.Bit.StatusType15 = BUSY_EV;        
        
    //Altri Ingressi
    DigInNotFilteredExtended.Bit.StatusType0 = RELAY_F ;
    DigInNotFilteredExtended.Bit.StatusType1 = AIR_PUMP_F;
    DigInNotFilteredExtended.Bit.StatusType2 = NEB_F;
    
        if (!DigInNotFilteredExtended.Bit.StatusType0)
    {
        Nop();
    }
    
        if (!DigInNotFilteredExtended.Bit.StatusType1)
    {
        Nop();
    }
    
        if (!DigInNotFilteredExtended.Bit.StatusType2)
    {
        Nop();
    }
    
    
    
}

/*
*//*=======================================================================*//**
**      @brief Selezione chip select motore     
**
**      @param input 'Motor_ID': tipo di Stepper
**                      0: Mixer Motor
**                      1: Door Motor
**                      2: -
**
**      @retval nessuno
**
*//*=======================================================================*//**
*/
void SPI_Set_Slave(unsigned short Motor_ID)
{

    switch (Motor_ID)
    {
        case MOTOR_MIXER:
             SET_RD14(0); 
             SET_RA5(1);
             SET_RB9(1);
        break;
        case MOTOR_DOOR:
             SET_RD14(1); 
             SET_RA5(0);
             SET_RB9(1);
        break;
        case MOTOR_AUTOCAP:
             SET_RD14(1); 
             SET_RA5(1);
             SET_RB9(0);       
        break;    
        case ALL_DRIVERS:
             SET_RD14(1); 
             SET_RA5(1);
             SET_RB9(1);
        break;
    }
}

void Enable_Driver(unsigned short Motor_ID)
{      
    switch (Motor_ID)
    {
        case MOTOR_MIXER:
            SET_RB13(TRUE);
        break;
        case MOTOR_DOOR:
            SET_RB10(TRUE);           
        break;  
        case MOTOR_AUTOCAP:
            SET_RG9(TRUE);
        break;    
        case ALL_DRIVERS:
            SET_RB13(TRUE);
            SET_RB10(TRUE);
            SET_RG9(TRUE);
        break;                      
    }
}

void Disable_Driver(unsigned short Motor_ID)
{
    switch (Motor_ID)
    {
        case MOTOR_MIXER:
            RST_BRD_OFF();
        break;
        case MOTOR_DOOR:
            RST_PMP_OFF();
        break;  
        case MOTOR_AUTOCAP:
            RST_EV_OFF();
        break;    
        case ALL_DRIVERS:
            RST_BRD_OFF();
            RST_PMP_OFF(); 
            RST_EV_OFF(); 
        break;                      
    }    
}