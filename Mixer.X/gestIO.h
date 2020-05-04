/* 
 * File:   gestIO.h
 * Author: michele.abelli
 *
 * Created on 12 mazo 2020, 11.48
 */

#ifndef GESTIO_H
#define	GESTIO_H

#define INPUT 1
#define OUTPUT 0

// Mapping PIN REV_00 
// -----------------------------------------------------------------------------
#define FO_ACC      _RA0 // Input Coupling Photocell 
#define BUSY_BRD    _RA1 // Input L6482H Table Motor Stepper Controller Busy line (Signal Low during a command) 
#define BUTTON      _RA4 // Input OFF-LINE LPXC10 Button
#define CS_PMP      _RA5 // Output L6482H Pump Motor Stepper Controller Chip Select 
#define NEB_IN      _RA6 // Output Nebulizer TPS1H200-A Power ON/OFF
#define NEB_F       _RA7 // Input Nebulizer TPS1H200-A Fault
#define FO_GEN1     _RA9 // Input Generic Photocell 1
#define LEV_SENS    _RA14 // Input Water Level Sensor
// -----------------------------------------------------------------------------
#define SCK_DRIVER  _RB0 // Output L6482H Valve,Pump Motor Stepper Controller Serial Clock Input
#define SDI_DRIVER  _RB1 // Output L6482H Valve,Pump Motor Stepper Controller Serial Data Input
#define SDO_DRIVER  _RB2 // Input L6482H Valve, Pump Motor Stepper Controller Serial Data Output
#define BRUSH_F1    _RB3 // Input ISEN on Brush DC Motor Driver DRV8842 
#define STCK_PMP    _RB4 // Output L6482H Pump Motor Stepper Controller Step Clock Input  
#define STCK_EV     _RB5 // Output L6482H Valve Motor Stepper Controller Step Clock Input
#define BUSY_EV     _RB8 // Input L6482H Valve Motor Stepper Controller Busy line (Signal Low during a command) 
#define CS_EV       _RB9 // Output L6482H Valve Motor Stepper Controller Chip Select 
#define STBY_RST_PMP _RB10 // Output L6482H Pump Motor Stepper Controller Reset  
#define FLAG_PMP    _RB11 // Input L6482H Pump Motor Stepper Controller Status Flag (Signal High when an alarm occurs) 
#define BUSY_PMP    _RB12 // Input L6482H Pump Motor Stepper Controller Busy line (Signal Low during a command)
#define STBY_RST_BRD _RB13 // Output L6482H Table Motor Stepper Controller Reset 
#define STCK_BRD     _RB14 // Output L6482H Table Motor Stepper Controller Step Clock Input 
#define FLAG_BRD     _RB15 // Input L6482H Table Motor Stepper Controller Status Flag (Signal High when an alarm occurs)  
// -----------------------------------------------------------------------------
#define FO_VALV  _RC2  // Input Valve Photocell 
#define EEPROM_CS _RC3  // Output EEprom Chip Select
#define FO_BRD    _RC4  // Input Table Photocell
#define OSC1      _RC12 // Input OSC1
#define I3_BRUSH  _RC13 // Output Brush I3 Winding Current
#define I4_BRUSH  _RC14 // Output Brush I4 Winding Current 
#define OSC2      _RC15 // Input OSC2
// -----------------------------------------------------------------------------
#define SDO_TC72  _RD0  // Output Serial Temperature Sensor TC72  
#define RELAY     _RD1  // Output Relè Activation (for HEATER) 
#define SCK_TC72  _RD2  // Output Clock Temperature Sensor 
#define RELAY_F   _RD4  // Input Fault on Rele Acrtivation 
#define LASER_BHL _RD5  // Output Laser Bung Hole Locator
#define IN1_BRUSH _RD6  // Output Power IN1 Brush DC Motor Driver DRV8842 
#define DECAY     _RD7  // Output Decay Mode Brush DC Motor Driver DRV8842
#define FO_HOME   _RD8  // Input Home Photocell
#define SDI_TC72  _RD9  // Input Serial Temperature Sensor TC72 
#define I2_BRUSH  _RD10 // Output Brush I2 Winding Current
#define RS485_TXD _RD11 // Output RX485 TX
#define RS485_RXD _RD12 // Input RS485 RX 
#define RS485_DE  _RD13 // Output Enable RS485 Transreceiver 
#define CS_BRD    _RD14 // Output L6482H Table Motor Stepper Controller Chip Select 
#define INT_CAR   _RD15 // Input Carriage Microswitch 
// -----------------------------------------------------------------------------
#define FLAG_EV    _RE1 // Input L6482H Valve Motor Stepper Controller Status Flag (Signal High when an alarm occurs) 
#define FO_CPR     _RE2 // Input Can presence Photocell 
#define OUT_24V_IN _RE3 // Output generic 24V Power ON/OFF  
#define LED_ON_OFF _RE4 // Output LED LPXLPB8 Power ON/OFF  
#define RST_BRUSH  _RE5 // Output Reset on Brush DC Motor Driver DRV8842 
#define SCL_SHT31  _RE6 // Output Serial Clock SHT31 Sensor
#define SDA_SHT31  _RE7 // Output Serial Data SHT31 Sensor
#define BRUSH_F2   _RE9 // Input Fault on Brush DC Motor Driver DRV8842
// -----------------------------------------------------------------------------
#define RST_SHT31   _RF0  // Output Reset Sensor SHT31
#define ALERT_SHT31 _RF1  // Input Alert Sensor SHT31 
#define RS232_TXD   _RF2  // Output Serial 232 TX data
#define FO_GEN2     _RF4  // Input Generic Photocell 2
#define RS232_RXD   _RF5  // Input Serial 232 RX Data
#define IO_GEN2     _RF8  // Input Generic GPIO 2  
#define INT_PAN     _RF12 // Input Panel Microswitch
#define IO_GEN1     _RF13 // Input Generic GPIO 1
// -----------------------------------------------------------------------------
#define IN2_BRUSH   _RG1 // Output Power IN2 Brush DC Motor Driver DRV8842   
#define USB_DP      _RG2 // Output USB DP
#define USB_DN      _RG3 // Output USB DN
#define SCK_EEPROM  _RG6 // Output EEprom Clock 
#define SDI_EEPROM  _RG7 // Input EEprom Data Input
#define SDO_EEPROM  _RG8 // Output EEprom Data Ouput
#define STBY_RST_EV _RG9 // Output L6482H Valve Motor Stepper Controller Reset
#define AIR_PUMP_IN _RG12 // Output Air Pump TPS1H200-A Input
#define AIR_PUMP_F  _RG13 // Input  Air Pump TPS1H200-A Fault
#define CE_TC72     _RG14 // Ouput Temperature Sensor TC72 Enable
#define OUT_24V_FAULT _RG15 // Input Fault on Generic 24V Exit
// -----------------------------------------------------------------------------


enum
{
    MOTOR_MIXER,
    MOTOR_DOOR,
    MOTOR_AUTOCAP,
    ALL_DRIVERS
};

extern void initIO(void);
extern void gestioneIO(void);
extern unsigned char getWaterLevel(void);
extern void INTERRUPT_Initialize(void);
extern void Enable_Driver(unsigned short Motor_ID);
extern void Disable_Driver(unsigned short Motor_ID);
extern void SPI_Set_Slave(unsigned short Motor_ID);

/*============ GPIO settings macros========================================== */    
/*---------------------------------------------------------------------------*/
/*
**                                      DIRECTION
*/
/*---------------------------------------------------------------------------*/
//----------------PORT A------------------------------------------------------//
#define SET_RA0_DIRECTION(x) (TRISAbits.TRISA0 = (x))
#define SET_RA1_DIRECTION(x) (TRISAbits.TRISA1 = (x))
#define SET_RA2_DIRECTION(x) (TRISAbits.TRISA2 = (x))
#define SET_RA3_DIRECTION(x) (TRISAbits.TRISA3 = (x))
#define SET_RA4_DIRECTION(x) (TRISAbits.TRISA4 = (x)) 
#define SET_RA5_DIRECTION(x) (TRISAbits.TRISA5 = (x))
#define SET_RA6_DIRECTION(x) (TRISAbits.TRISA6 = (x))
#define SET_RA7_DIRECTION(x) (TRISAbits.TRISA7 = (x))
#define SET_RA9_DIRECTION(x) (TRISAbits.TRISA9 = (x))
#define SET_RA10_DIRECTION(x) (TRISAbits.TRISA10 = (x))
#define SET_RA14_DIRECTION(x) (TRISAbits.TRISA14 = (x))
#define SET_RA15_DIRECTION(x) (TRISAbits.TRISA15 = (x))

//----------------PORT B------------------------------------------------------//
#define SET_RB0_DIRECTION(x) (TRISBbits.TRISB0 = (x))
#define SET_RB1_DIRECTION(x) (TRISBbits.TRISB1 = (x))
#define SET_RB2_DIRECTION(x) (TRISBbits.TRISB2 = (x))
#define SET_RB3_DIRECTION(x) (TRISBbits.TRISB3 = (x))
#define SET_RB4_DIRECTION(x) (TRISBbits.TRISB4 = (x))
#define SET_RB5_DIRECTION(x) (TRISBbits.TRISB5 = (x))
#define SET_RB6_DIRECTION(x) (TRISBbits.TRISB6 = (x))
#define SET_RB7_DIRECTION(x) (TRISBbits.TRISB7 = (x))
#define SET_RB8_DIRECTION(x) (TRISBbits.TRISB8 = (x))
#define SET_RB9_DIRECTION(x) (TRISBbits.TRISB9 = (x))
#define SET_RB10_DIRECTION(x) (TRISBbits.TRISB10 = (x))
#define SET_RB11_DIRECTION(x) (TRISBbits.TRISB11 = (x))
#define SET_RB12_DIRECTION(x) (TRISBbits.TRISB12 = (x))
#define SET_RB13_DIRECTION(x) (TRISBbits.TRISB13 = (x))
#define SET_RB14_DIRECTION(x) (TRISBbits.TRISB14 = (x))
#define SET_RB15_DIRECTION(x) (TRISBbits.TRISB15 = (x))

//----------------PORT C------------------------------------------------------//
#define SET_RC1_DIRECTION(x) (TRISCbits.TRISC1 = (x))
#define SET_RC2_DIRECTION(x) (TRISCbits.TRISC2 = (x))
#define SET_RC3_DIRECTION(x) (TRISCbits.TRISC3 = (x))
#define SET_RC4_DIRECTION(x) (TRISCbits.TRISC4 = (x))
#define SET_RC12_DIRECTION(x) (TRISCbits.TRISC12 = (x))
#define SET_RC13_DIRECTION(x) (TRISCbits.TRISC13 = (x))
#define SET_RC14_DIRECTION(x) (TRISCbits.TRISC14 = (x))
#define SET_RC15_DIRECTION(x) (TRISCbits.TRISC15 = (x))

//----------------PORT D------------------------------------------------------//
#define SET_RD0_DIRECTION(x) (TRISDbits.TRISD0 = (x))
#define SET_RD1_DIRECTION(x) (TRISDbits.TRISD1 = (x))
#define SET_RD2_DIRECTION(x) (TRISDbits.TRISD2 = (x))
#define SET_RD3_DIRECTION(x) (TRISDbits.TRISD3 = (x))
#define SET_RD4_DIRECTION(x) (TRISDbits.TRISD4 = (x))
#define SET_RD5_DIRECTION(x) (TRISDbits.TRISD5 = (x))
#define SET_RD6_DIRECTION(x) (TRISDbits.TRISD6 = (x))
#define SET_RD7_DIRECTION(x) (TRISDbits.TRISD7 = (x))
#define SET_RD8_DIRECTION(x) (TRISDbits.TRISD8 = (x))
#define SET_RD9_DIRECTION(x) (TRISDbits.TRISD9 = (x))
#define SET_RD10_DIRECTION(x) (TRISDbits.TRISD10 = (x))
#define SET_RD11_DIRECTION(x) (TRISDbits.TRISD11 = (x))
#define SET_RD12_DIRECTION(x) (TRISDbits.TRISD12 = (x))
#define SET_RD13_DIRECTION(x) (TRISDbits.TRISD13 = (x))
#define SET_RD14_DIRECTION(x) (TRISDbits.TRISD14 = (x))
#define SET_RD15_DIRECTION(x) (TRISDbits.TRISD15 = (x))
/*---------------------------------------------------------------------------*/
/*
**                                     OUTPUT
*/
/*---------------------------------------------------------------------------*/
//----------------PORT A------------------------------------------------------//
#define SET_RA0(x) ( LATAbits.LATA0 = (x) )
#define SET_RA1(x) ( LATAbits.LATA1 = (x) )
#define SET_RA2(x) ( LATAbits.LATA2 = (x) )
#define SET_RA3(x) ( LATAbits.LATA3 = (x) )
#define SET_RA5(x) ( LATAbits.LATA5 = (x) )
#define SET_RA6(x) ( LATAbits.LATA6 = (x) )
#define SET_RA7(x) ( LATAbits.LATA7 = (x) )
#define SET_RA9(x) ( LATAbits.LATA9 = (x) )
#define SET_RA10(x) ( LATAbits.LATA10 = (x) )
#define SET_RA14(x) ( LATAbits.LATA14 = (x) )
#define SET_RA15(x) ( LATAbits.LATA15 = (x) )

//----------------PORT B------------------------------------------------------//
#define SET_RB0(x) ( LATBbits.LATB0 = (x) )
#define SET_RB1(x) ( LATBbits.LATB1 = (x) )
#define SET_RB2(x) ( LATBbits.LATB2 = (x) )
#define SET_RB3(x) ( LATBbits.LATB3 = (x) )
#define SET_RB4(x) ( LATBbits.LATB4 = (x) )
#define SET_RB5(x) ( LATBbits.LATB5 = (x) )
#define SET_RB6(x) ( LATBbits.LATB6 = (x) )
#define SET_RB7(x) ( LATBbits.LATB7 = (x) )
#define SET_RB8(x) ( LATBbits.LATB8 = (x) )
#define SET_RB9(x) ( LATBbits.LATB9 = (x) )
#define SET_RB10(x) ( LATBbits.LATB10 = (x) )
#define SET_RB11(x) ( LATBbits.LATB11 = (x) )
#define SET_RB12(x) ( LATBbits.LATB12 = (x) )
#define SET_RB13(x) ( LATBbits.LATB13 = (x) )
#define SET_RB14(x) ( LATBbits.LATB14 = (x) )
#define SET_RB15(x) ( LATBbits.LATB15 = (x) )

#define TOGGLE_RB6() ( LATBbits.LATB6 ^= TRUE )
#define TOGGLE_RB7() ( LATBbits.LATB7 ^= TRUE )

#define TOGGLE_RB50() ( LATBbits.LATB5 ^= TRUE )

//----------------PORT C------------------------------------------------------//
#define SET_RC1(x) ( LATCbits.LATC1 = (x) )
#define SET_RC2(x) ( LATCbits.LATC2 = (x) )
#define SET_RC3(x) ( LATCbits.LATC3 = (x) )
#define SET_RC4(x) ( LATCbits.LATC4 = (x) )
#define SET_RC12(x) ( LATCbits.LATC12 = (x) )
#define SET_RC13(x) ( LATCbits.LATC13 = (x) )
#define SET_RC14(x) ( LATCbits.LATC14 = (x) )
#define SET_RC15(x) ( LATCbits.LATC15 = (x) )

#define TOGGLE_RC1() ( LATCbits.LATC1 ^= TRUE )
#define TOGGLE_RC2() ( LATCbits.LATC2 ^= TRUE )
#define TOGGLE_RC3() ( LATCbits.LATC3 ^= TRUE )
#define TOGGLE_RC4() ( LATCbits.LATC4 ^= TRUE )

//----------------PORT D------------------------------------------------------//
#define SET_RD0(x)  ( LATDbits.LATD0 = (x) )
#define SET_RD1(x)  ( LATDbits.LATD1 = (x) )
#define SET_RD2(x)  ( LATDbits.LATD2 = (x) )
#define SET_RD3(x)  ( LATDbits.LATD3 = (x) )
#define SET_RD4(x)  ( LATDbits.LATD4 = (x) )
#define SET_RD5(x)  ( LATDbits.LATD5 = (x) )
#define SET_RD6(x)  ( LATDbits.LATD6 = (x) )
#define SET_RD7(x)  ( LATDbits.LATD7 = (x) )
#define SET_RD8(x)  ( LATDbits.LATD8 = (x) )
#define SET_RD9(x)  ( LATDbits.LATD9 = (x) )
#define SET_RD10(x)  ( LATDbits.LATD10 = (x) )
#define SET_RD11(x)  ( LATDbits.LATD11 = (x) )
#define SET_RD12(x)  ( LATDbits.LATD12 = (x) )
#define SET_RD13(x)  ( LATDbits.LATD13 = (x) )
#define SET_RD14(x)  ( LATDbits.LATD14 = (x) )
#define SET_RD15(x)  ( LATDbits.LATD15 = (x) )

//----------------PORT E------------------------------------------------------//
#define SET_RE0(x)  ( LATEbits.LATE0 = (x) )
#define SET_RE1(x)  ( LATEbits.LATE1 = (x) )
#define SET_RE2(x)  ( LATEbits.LATE2 = (x) )
#define SET_RE3(x)  ( LATEbits.LATE3 = (x) )
#define SET_RE4(x)  ( LATEbits.LATE4 = (x) )
#define SET_RE5(x)  ( LATEbits.LATE5 = (x) )
#define SET_RE6(x)  ( LATEbits.LATE6 = (x) )
#define SET_RE7(x)  ( LATEbits.LATE7 = (x) )
#define SET_RE8(x)  ( LATEbits.LATE8 = (x) )
#define SET_RE9(x)  ( LATEbits.LATE9 = (x) )

//----------------PORT F------------------------------------------------------//
#define SET_RF0(x)  ( LATFbits.LATF0 = (x) )
#define SET_RF1(x)  ( LATFbits.LATF1 = (x) )
#define SET_RF2(x)  ( LATFbits.LATF2 = (x) )
#define SET_RF3(x)  ( LATFbits.LATF3 = (x) )
#define SET_RF4(x)  ( LATFbits.LATF4 = (x) )
#define SET_RF5(x)  ( LATFbits.LATF5 = (x) )
#define SET_RF8(x)  ( LATFbits.LATF8 = (x) )
#define SET_RF12(x)  ( LATFbits.LATF12 = (x) )
#define SET_RF13(x)  ( LATFbits.LATF13 = (x) )

//----------------PORT F------------------------------------------------------//
#define SET_RG0(x)  ( LATGbits.LATG0 = (x) )
#define SET_RG1(x)  ( LATGbits.LATG1 = (x) )
#define SET_RG2(x)  ( LATGbits.LATG2 = (x) )
#define SET_RG3(x)  ( LATGbits.LATG3 = (x) )
#define SET_RG6(x)  ( LATGbits.LATG6 = (x) )
#define SET_RG7(x)  ( LATGbits.LATG7 = (x) )
#define SET_RG8(x)  ( LATGbits.LATG8 = (x) )
#define SET_RG9(x)  ( LATGbits.LATG9 = (x) )
#define SET_RG12(x)  ( LATGbits.LATG12 = (x) )
#define SET_RG13(x)  ( LATGbits.LATG13 = (x) )
#define SET_RG14(x)  ( LATGbits.LATG14 = (x) )
#define SET_RG15(x)  ( LATGbits.LATG15 = (x) )
/*---------------------------------------------------------------------------*/
/*
**                                      INPUT
*/
/*---------------------------------------------------------------------------*/

//----------------PORT A------------------------------------------------------//
#define READ_RA0     ( PORTAbits.PORTA0 )
#define READ_RA1     ( PORTAbits.PORTA1 )
#define READ_RA2     ( PORTAbits.PORTA2 )
#define READ_RA3     ( PORTAbits.PORTA3 )
#define READ_RA4     ( PORTAbits.PORTA4 )
#define READ_RA5     ( PORTAbits.PORTA5 )
#define READ_RA6     ( PORTAbits.PORTA6 )
#define READ_RA7     ( PORTAbits.PORTA7 )
#define READ_RA9     ( PORTAbits.PORTA9 )
#define READ_RA10     ( PORTAbits.PORTA10 )
#define READ_RA14     ( PORTAbits.PORTA14 )
#define READ_RA15     ( PORTAbits.PORTA15 )

//----------------PORT B------------------------------------------------------//
#define READ_RB0     ( PORTBbits.PORTB0 )
#define READ_RB1     ( PORTBbits.PORTB1 )
#define READ_RB2     ( PORTBbits.PORTB2 )
#define READ_RB3     ( PORTBbits.PORTB3 )
#define READ_RB4     ( PORTBbits.PORTB4 ) 
#define READ_RB5     ( PORTBbits.PORTB5 )
#define READ_RB6     ( PORTBbits.PORTB6 )
#define READ_RB7     ( PORTBbits.PORTB7 )
#define READ_RB8     ( PORTBbits.PORTB8 )
#define READ_RB9     ( PORTBbits.PORTB9 )
#define READ_RB10     ( PORTBbits.PORTB10 )
#define READ_RB11     ( PORTBbits.PORTB11 )
#define READ_RB12     ( PORTBbits.PORTB12 ) 
#define READ_RB13     ( PORTBbits.PORTB13 )
#define READ_RB14     ( PORTBbits.PORTB14 )
#define READ_RB15     ( PORTBbits.PORTB15 )

//----------------PORT C------------------------------------------------------//
#define READ_RC1     ( PORTCbits.PORTC1 )
#define READ_RC2     ( PORTCbits.PORTC2 )
#define READ_RC3     ( PORTCbits.PORTC3 )
#define READ_RC4     ( PORTCbits.PORTC4 ) 
#define READ_RC12     ( PORTCbits.PORTC12 )
#define READ_RC13     ( PORTCbits.PORTC13 )
#define READ_RC14     ( PORTCbits.PORTC14 )
#define READ_RC15     ( PORTCbits.PORTC15 )

//----------------PORT D------------------------------------------------------//
#define READ_RD0     ( PORTDbits.PORTD0 )
#define READ_RD1     ( PORTDbits.PORTD1 )
#define READ_RD2     ( PORTDbits.PORTD2 )
#define READ_RD3     ( PORTDbits.PORTD3 )
#define READ_RD4     ( PORTDbits.PORTD4 )
#define READ_RD5     ( PORTDbits.PORTD5 )
#define READ_RD6     ( PORTDbits.PORTD6 )
#define READ_RD7     ( PORTDbits.PORTD7 )
#define READ_RD8     ( PORTDbits.PORTD8 )
#define READ_RD9     ( PORTDbits.PORTD9 )
#define READ_RD10     ( PORTDbits.PORTD10 )
#define READ_RD11     ( PORTDbits.PORTD11 )
#define READ_RD12     ( PORTDbits.PORTD12 )
#define READ_RD13     ( PORTDbits.PORTD13 )
#define READ_RD14     ( PORTDbits.PORTD14 )
#define READ_RD15     ( PORTDbits.PORTD15 )

//----------------PORT E------------------------------------------------------//
#define READ_RE0     ( PORTEbits.PORTE0 )
#define READ_RE1     ( PORTEbits.PORTE1 )
#define READ_RE2     ( PORTEbits.PORTE2 )
#define READ_RE3     ( PORTEbits.PORTE3 )
#define READ_RE4     ( PORTEbits.PORTE4 )
#define READ_RE5     ( PORTEbits.PORTE5 )
#define READ_RE6     ( PORTEbits.PORTE6 )
#define READ_RE7     ( PORTEbits.PORTE7 )
#define READ_RE8     ( PORTEbits.PORTE8 )
#define READ_RE9     ( PORTEbits.PORTE9 )

//----------------PORT F------------------------------------------------------//
#define READ_RF0     ( PORTFbits.PORTF0 )
#define READ_RF1     ( PORTFbits.PORTF1 )
#define READ_RF2     ( PORTFbits.PORTF2 )
#define READ_RF3     ( PORTFbits.PORTF3 )
#define READ_RF4     ( PORTFbits.PORTF4 )
#define READ_RF5     ( PORTFbits.PORTF5 )
#define READ_RF8     ( PORTFbits.PORTF8 )
#define READ_RF12     ( PORTFbits.PORTF12 )
#define READ_RF13     ( PORTFbits.PORTF13 )

//----------------PORT G------------------------------------------------------//
#define READ_RG0     ( PORTGbits.PORTG0 )
#define READ_RG1     ( PORTGbits.PORTG1 )
#define READ_RG2     ( PORTGbits.PORTG2 )
#define READ_RG3     ( PORTGbits.PORTG3 )
#define READ_RG6     ( PORTGbits.PORTG6 )
#define READ_RG7     ( PORTGbits.PORTG7 )
#define READ_RG8     ( PORTGbits.PORTG8 )
#define READ_RG9     ( PORTGbits.PORTG9 )
#define READ_RG12     ( PORTGbits.PORTG12 )
#define READ_RG13     ( PORTGbits.PORTG13 )
#define READ_RG14     ( PORTGbits.PORTG14 )
#define READ_RG15     ( PORTGbits.PORTG15 )


#endif	/* GESTIO_H */

