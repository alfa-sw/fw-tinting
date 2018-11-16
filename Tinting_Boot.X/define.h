/**/
/*============================================================================*/
/**
 **      @file    define.h
 **
 **      @brief   File di definizioni
  **/
/*============================================================================*/
/**/
#ifndef __DEFINE_H
#define __DEFINE_H

#include "Macro.h"

/* Parametri Flash */
#define FLASH_PAGE_WORD_LENGTH 1024
#define FLASH_PAGE_BYTE_LENGTH 1536

#define START_APPL_ADDRESS 0x00002C00L
#define END_APPL_ADDRESS   0x0002A800L

#define EXPECTED_FIRST_PAGE_APPL (11) /* starts @ 0x2C00 */
#define FIRST_PG_APPL (START_APPL_ADDRESS / FLASH_PAGE_WORD_LENGTH)
#if FIRST_PG_APPL != EXPECTED_FIRST_PAGE_APPL
#  warning First application page does not not match expected value. Check define.h
#endif

#define EXPECTED_LAST_PAGE_APPL (169) /* ends @ 0x2A800 */
#define LAST_PG_APPL  (END_APPL_ADDRESS / FLASH_PAGE_WORD_LENGTH - 1)
#if LAST_PG_APPL != EXPECTED_LAST_PAGE_APPL
#  warning Last application page does not not match expected value. Check define.h
#endif

/******************************************************************************************/
/***************************************** Main *******************************************/
/******************************************************************************************/
#define BL_STAND_ALONE_CHECK            (START_APPL_ADDRESS + 4)
#define APPL_FLASH_MEMORY_ERASED_VALUE  0x00FFFFFFL

#define TINTING_ID 44;

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
#define FO_VALVE  _RC2  // Input Valve Photocell 
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
typedef enum {
  PROC_OK,             /* Procedura corretta      */
  PROC_ERROR,          /* Valore scritto e letto non coincidono  */
  PROC_PROTECTED,      /* La flash è protetta in scrittura       */
  PROC_ERROR_PROTECT,  /* La flash si trova nello stato protetto */
  PROC_ERROR_12V,
  PROC_ADDRESS_ERROR,  /* Indirizzo errato */
  PROC_MAX_FAILS,
  PROC_CHK_ERROR
} PROC_RES;

typedef struct __attribute__ ((packed)) {
  unsigned char livello;
  unsigned char fase;
  unsigned char step;
  PROC_RES ProcRes;
} Stato;

/* Enum per BLState.livello */
enum {
  POWER_OFF = 0,
  INIT,
  USB_CONNECT_EXECUTION,  /* supported if BOOTLOADER_USB macro is uncommented */
  UART_FW_UPLOAD,         /* firmware upload */
  UART_FW_UPLOAD_FAILED,  /* firmware upload failed */
};

/* Enum per BLState.step per BLState.fase=SAT_FW_UPLOAD --- Utilizzati
   sia per USB che per comunicazione seriale */
enum {
  ERASE_DEVICE = 0,
  WAIT_DATA_PACKET,
  PROGRAM_DEVICE,
  PROGRAM_END,
  GET_DATA,
  RESET_SYSTEM,
  DO_RESET
};

#endif /* __DEFINE_H */
