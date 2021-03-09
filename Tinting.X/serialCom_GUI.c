/********************************************************************************
 **
 **      Filename     : serialCom_GUI.c
 **
 **      Description  : MABrd<->MGBrd comm module
 **
 **      Project      : Alfa Dispensers
 **
 **   ===========================================================================
 **
 */

#include "serialCom_GUI.h"
#include "serialCom.h"
#include "tintingmanager.h"
#include "p24FJ256GB110.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ram.h"
#include "typedef.h"
#include "define.h"
#include "gestIO.h"
#include "timerMg.h"
#include "mem.h"
#include "stepperParameters.h"
#include "eepromManager.h"
#include "statusManager.h"
#include "autocapAct.h"
#include "rollerAct.h"

/**
 * Module vars
 */
uartBuffer_t rxBuffer_GUI;
uartBuffer_t txBuffer_GUI;
serialGUI_t serialGUI;

static unsigned char deviceID_GUI;    
/**
 * Function definitions
 */
void clear_slave_comm(void)
/**/
/*============================================================================*/
/**
**   @brief clear GUI slave comm
**
**   @param index, the slave index
**
**   @return void
**/
/*============================================================================*/
/**/
{
  procGUI.slave_comm[0] = 0L;
  procGUI.slave_comm[1] = 0L;
}

void set_slave_status(unsigned short index, unsigned char value)
/**/
/*===========================================================================*/
/**
**   @brief set GUI slave status (1 = BUSY, 0 = IDLE)
**
**   @param index, the slave index
**   @param value, boolean value
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  unsigned short addr, ofs;

  addr = index / 32;
  ofs  = index % 32;

  if (! value) {
    procGUI.Slave_status[addr] &=
      ~ (1L << ofs);
  }
  else {
    procGUI.Slave_status[addr] |=
      (1L << ofs);
  }
  
  // 'index' is a MASTER BASE (0+1, 2+1, 4+1, 6+1)
  if (procGUI.circuit_pump_types[ofs] == PUMP_DOUBLE) {	  
	if (! value) {
		procGUI.Slave_status[addr] &=
			~ (1L << (ofs+1));  
	}
	else {
		procGUI.Slave_status[addr] |=
			(1L << (ofs+1));
	}
  }  
}

static void rebuildMessage_GUI(unsigned char receivedByte)
/**/
/*===========v================================================================*/
/**
**      @brief Called by  _U2RXInterrupt: update the rx buffer with
**             subsequent received bytes
**
**      @param receivedByte received bytes
**
**      @retval void
**/
/*============================================================================*/
/**/
{
  if (! IS_ERROR(rxBuffer_GUI)) {
    switch(rxBuffer_GUI.status) {
    case WAIT_STX:
        if (receivedByte == ASCII_STX) {
            STORE_BYTE( rxBuffer_GUI, receivedByte );
            rxBuffer_GUI.status = WAIT_ID;
        }
        break;
    case WAIT_ID:
        STORE_BYTE( rxBuffer_GUI, receivedByte );
        deviceID_GUI = REMOVE_OFFSET(receivedByte);
        if (! IS_VALID_GUI_ID(deviceID_GUI))
            SIGNAL_ERROR(rxBuffer_GUI);        
        else
            rxBuffer_GUI.status = WAIT_LENGTH;
        break;
    case WAIT_LENGTH:
        STORE_BYTE( rxBuffer_GUI, receivedByte );
        if ( receivedByte < ADD_OFFSET( MIN_FRAME_SIZE) || receivedByte > ADD_OFFSET( MAX_FRAME_SIZE))
            SIGNAL_ERROR(rxBuffer_GUI);
        else {
            /*
            The length embedded in the frame takes into account the
            entire frame length, for ease of implementation of tx/rx
            code. Here we discard the final 5 bytes (4 CRC + ETX). Later
            on, after the crc check, we'll be able to discard also the
            initial overhead [ STX, ID, LEN ]	
            */		 
            rxBuffer_GUI.length  = REMOVE_OFFSET(receivedByte);
            rxBuffer_GUI.length -= FRAME_END_OVERHEAD;
            rxBuffer_GUI.status  = WAIT_DATA;
        }
        break;
    case WAIT_DATA:
        // Check stuffying encoding 
        if (IS_ESCAPE(rxBuffer_GUI)) {
            // ESC ZERO --> ESC, ESC TWO --> STX, ESC THREE --> ETX 
            if (receivedByte != ASCII_ZERO && receivedByte != ASCII_TWO && receivedByte != ASCII_THREE) {
                // Illegal encoding detected
                SIGNAL_ERROR(rxBuffer_GUI);
            }
            CLEAR_ESCAPE(rxBuffer_GUI);
        }
        else {
            if (receivedByte == ASCII_ESC)
                SIGNAL_ESCAPE(rxBuffer_GUI);
        }
        STORE_BYTE( rxBuffer_GUI, receivedByte );
        if (rxBuffer_GUI.index == rxBuffer_GUI.length) {
            rxBuffer_GUI.status = WAIT_CRC;
        }
        break;
    case WAIT_CRC:
        // Received four CRC bytes?
        STORE_BYTE( rxBuffer_GUI, receivedByte );
        if (rxBuffer_GUI.index == FRAME_CRC_LENGTH + rxBuffer_GUI.length)
            rxBuffer_GUI.status = WAIT_ETX;
        break;	  
    case WAIT_ETX:
        // Waiting for ETX
        if (receivedByte != ASCII_ETX)
		  SIGNAL_ERROR(rxBuffer_GUI);
        else {
            rxBuffer_GUI.bufferFlags.rxCompleted = TRUE;
            IEC1bits.U2RXIE = 0;
        }
        break;
    default:
        SIGNAL_ERROR(rxBuffer_GUI);
        break;
    } // switch 
  } // if (!IS_ERROR()) 

  // Rx cleanup, in case of error. 
  if (IS_ERROR(rxBuffer_GUI)) {
    RESET_RECEIVER(rxBuffer_GUI);
  }
} // rebuildMessage_GUI() 

static void makeMessage_GUI()
/**/
/*===========================================================================*/
/**
**   @brief    Every T_GUI_TX_TIMER time, MMT send packet to Linux with
**             status information
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    unsigned char idx = 0;
    int i;
    long appoggioLong;
//    unsigned char Byte1, Byte2, Byte3, Byte4, Byte5; 
  
    if (StatusTimer(T_FIRST_LINK_GUI_TIMER) == T_HALTED)
        // Start first link 
        StartTimer(T_FIRST_LINK_GUI_TIMER);

    if (StatusTimer(T_GUI_TX_TIMER) == T_HALTED || StatusTimer(T_GUI_TX_TIMER) == T_ELAPSED)
        txBuffer_GUI.bufferFlags.startTx = TRUE;

    if (txBuffer_GUI.bufferFlags.startTx == TRUE) {
        unsigned short tmp;    
        initBuffer(&txBuffer_GUI);
        // Initialize tx frame, reserve extra byte for pktlen
        FRAME_BEGIN(&txBuffer_GUI, idx, MAB_DEVICE_ID);
        switch (procGUI.typeMessage)
        {
            case MACHINE_INFO:
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MACHINE_INFO);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.info_page);             
                // PAGE 0 (MMT + acts 1 .. 12)
                if (0 == procGUI.info_page) {
                    // MMT fw version number
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(SW_VERSION));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(SW_VERSION));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(SW_VERSION));
                    // Actuators fw version numbers
                    for (i = 0; i < 13; ++ i) {
                        unsigned long version = slaves_sw_versions[i];
                        STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(version));
                        STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(version));
                        STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(version));
                    }
                }
                // PAGE 1 (acts 13 .. 25)
                else if (1 == procGUI.info_page) {
                    // Actuators fw version numbers
                    for (i = 13; i < 25; ++ i) {
                      unsigned long version = slaves_sw_versions[i];
                      STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(version));
                      STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(version));
                      STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(version));
                    }
                }
                // PAGE 2 (acts 25 .. 37)
                else if (2 == procGUI.info_page) {
                    for (i = 25; i < 37; ++ i) {
                        unsigned long version = slaves_sw_versions[i];
                        STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(version));
                        STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(version));
                        STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(version));
                    }
                }
                // PAGE 3
                else {
                    for (i = 37; i < 48; ++ i) {
                      unsigned long version = slaves_sw_versions[i];
                      STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(version));
                      STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(version));
                      STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(version));
                    }
                }
            break; // case MACHINE_INFO
            
            case MACHINE_FW_VERSIONS:
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MACHINE_FW_VERSIONS);
                // MMT - LINUX Protocol version number
                STUFF_BYTE( txBuffer_GUI.buffer, idx, PROTOCOL_VERSION);
                // MMT fw version number
                STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(SW_VERSION));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(SW_VERSION));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(SW_VERSION));
                // Actuators fw version numbers 
                for (i = 0; i < 48; ++ i) {
                    unsigned long version = slaves_sw_versions[i];
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(version));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(version));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(version));
                }
                procGUI.typeMessage = CONTROLLO_PRESENZA_MACCHINA;      
//------------------------------------------------------------------------------ 
/*
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MACHINE_FW_VERSIONS);
                // MMT - LINUX Protocol version number
                STUFF_BYTE( txBuffer_GUI.buffer, idx, PROTOCOL_VERSION);
                                
                Byte1 = VERSION_PATCH(SW_VERSION) + ( (VERSION_MINOR(SW_VERSION) & 0x03) << 6); 
                Byte2 = ((VERSION_MINOR(SW_VERSION) & 0x3C) >> 2) + ( (VERSION_MAJOR(SW_VERSION) & 0x0F) << 4);
                Byte3 = ((VERSION_MAJOR(SW_VERSION) & 0x30) >> 4) + (VERSION_PATCH(BL_VERSION) << 2); 
                Byte4 = VERSION_MINOR(BL_VERSION) + ( (VERSION_MAJOR(BL_VERSION) & 0x03) << 6);
                Byte5 = (VERSION_MAJOR(BL_VERSION) & 0x3C) >> 2;                

//                Byte3 = ((VERSION_MAJOR(SW_VERSION) & 0x30) >> 4) + (VERSION_PATCH(BOOT_FW_VERSION()) << 2); 
//                Byte4 = VERSION_MINOR(BOOT_FW_VERSION()) + ( (VERSION_MAJOR(BOOT_FW_VERSION()) & 0x03) << 6);
//                Byte5 = (VERSION_MAJOR(BOOT_FW_VERSION()) & 0x15) >> 2;                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte1);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte2);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte3);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte4);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte5);

                for (i = 0; i < 4; ++ i) {
                    unsigned long FW_version   = slaves_sw_versions[i];
                    unsigned long Boot_version = slaves_boot_versions[i];
                    //Byte1 = VERSION_PATCH(FW_version) + ( (VERSION_MINOR(FW_version) & 0x03) << 6); 
                    Byte1 = 0x03; 

                    Byte2 = ((VERSION_MINOR(FW_version) & 0x3C) >> 2) + ( (VERSION_MAJOR(FW_version) & 0x0F) << 4);
                    Byte3 = ((VERSION_MAJOR(FW_version) & 0x30) 
                            >> 4) + (VERSION_PATCH(Boot_version) << 2); 
                    Byte4 = VERSION_MINOR(Boot_version) + ( (VERSION_MAJOR(Boot_version) & 0x03) << 6);
                    Byte5 = (VERSION_MAJOR(Boot_version) & 0x3C) >> 2;
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte1);
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte2);
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte3);
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte4);
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, Byte5);
                }                    
                //--------------------------------------------------------------                                        
                procGUI.typeMessage = CONTROLLO_PRESENZA_MACCHINA;
*/                               
            break; // case MACHINE_FW_VERSIONS
                
            case MACHINE_BOOT_VERSIONS:
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MACHINE_BOOT_VERSIONS);
                // MMT Boot version number
                STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(BOOT_FW_VERSION()));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(BOOT_FW_VERSION()));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(BOOT_FW_VERSION()));
                // Actuators Boot version numbers
                for (i = 0; i < 48; ++ i) {
                    unsigned long version = slaves_boot_versions[i];
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MAJOR(version));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_MINOR(version));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, VERSION_PATCH(version));
                }
                procGUI.typeMessage = CONTROLLO_PRESENZA_MACCHINA;

            break; // case MACHINE_BOOT_VERSIONS

            case GET_SLAVES_CONFIGURATION:
                STUFF_BYTE( txBuffer_GUI.buffer, idx, GET_SLAVES_CONFIGURATION);
                for (i = 0; i < N_SLAVES_BYTES; ++ i)
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.slaves_en[i]);
            break; // case GET_SLAVES_CONFIGURATION

            case DIAG_READ_LAST_DISPENSATION_PARAM:
                STUFF_BYTE( txBuffer_GUI.buffer, idx,DIAG_DATA_LAST_DISPENSATION_PARAM);
                STUFF_BYTE( txBuffer_GUI.buffer, idx,procGUI.id_color_circuit);
                // No. steps cycle (32 bits)
                STUFF_BYTE( txBuffer_GUI.buffer, idx,LSB_LSW(dispensationAct[procGUI.id_color_circuit].n_step_cycle));
                STUFF_BYTE( txBuffer_GUI.buffer, idx,MSB_LSW(dispensationAct[procGUI.id_color_circuit].n_step_cycle));
                STUFF_BYTE( txBuffer_GUI.buffer, idx,LSB_MSW(dispensationAct[procGUI.id_color_circuit].n_step_cycle));
                STUFF_BYTE( txBuffer_GUI.buffer, idx,MSB_MSW(dispensationAct[procGUI.id_color_circuit].n_step_cycle));
                // Cycle speed (16 bits)
                STUFF_BYTE( txBuffer_GUI.buffer, idx,LSB(dispensationAct[procGUI.id_color_circuit].speed_cycle));
                STUFF_BYTE( txBuffer_GUI.buffer, idx,MSB(dispensationAct[procGUI.id_color_circuit].speed_cycle));
                // No. cycles (16 bit)
                STUFF_BYTE( txBuffer_GUI.buffer, idx,LSB(dispensationAct[procGUI.id_color_circuit].n_cycles));
                STUFF_BYTE( txBuffer_GUI.buffer, idx,MSB(dispensationAct[procGUI.id_color_circuit].n_cycles));
            break; // case DIAG_READ_LAST_DISPENSATION_PARAM
        
            default: // STATUS message
                STUFF_BYTE( txBuffer_GUI.buffer, idx, STATO_MACCHINA);

                // status level, step (16 bits)
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MachineStatus.level);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MachineStatus.step);

                // alarm error code (16 bits)
                tmp = alarm();
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB(tmp));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB(tmp));
                
                // reserve, availability and enable bitmasks for each cover (24 bits)
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Cover_res);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Cover_avail);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Cover_enabled);

                // reserve and availability bit for each container (24 bits) */
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Container_res);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Container_avail);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Container_enabled);

                // color reserve bitmasks (32 bits) 
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(procGUI.Color_res));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(procGUI.Color_res));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_MSW(procGUI.Color_res));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_MSW(procGUI.Color_res));

                // container, autocap, canlifter and door status flags (32 bits) 
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Container_presence);

                if (!isAutocapActEnabled())
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, 1);
                else    
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Autocap_status);

                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Canlift_status);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.Doors_status);

                // Other machines simulate a clamp fixed in POS0 (consistency)
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);

                // recirculation status bitmasks (32 bits) 
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(procGUI.recirc_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(procGUI.recirc_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_MSW(procGUI.recirc_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_MSW(procGUI.recirc_status));

                // stirring status bitmasks (32 bits) 
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(procGUI.stirring_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(procGUI.stirring_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_MSW(procGUI.stirring_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_MSW(procGUI.stirring_status));

                // slave activity status bitmasks (48 bits) 
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(procGUI.Slave_status[0]));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(procGUI.Slave_status[0]));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_MSW(procGUI.Slave_status[0]));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_MSW(procGUI.Slave_status[0]));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(procGUI.Slave_status[1]));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(procGUI.Slave_status[1]));

                // Can Lifter Sensor
//                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                
                STUFF_BYTE( txBuffer_GUI.buffer, idx,Photo_Ingr_Direction);                
                
                // Can Lifter Current Height
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);

                // Can Lifter Range                
//                appoggioLong = DEVICE_DISABLED * 10000;                
                appoggioLong = 0;
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(appoggioLong));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(appoggioLong));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_MSW(appoggioLong));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_MSW(appoggioLong));                                
                if (TintingHumidifier.Humidifier_Enable == HUMIDIFIER_DISABLE) {
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(DEVICE_DISABLED));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(DEVICE_DISABLED));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(DEVICE_DISABLED));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(DEVICE_DISABLED));                
                }                    
                else { 
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(Photo_Ingr_Read_Dark_Counter_Error * 10));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(Photo_Ingr_Read_Dark_Counter_Error * 10));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(Photo_Ingr_Read_Light_Counter_Error * 10));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(Photo_Ingr_Read_Light_Counter_Error * 10));
/*                    
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(TintingAct.Temperature));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(TintingAct.Temperature));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(TintingAct.RH));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(TintingAct.RH));
 */
                }
#ifdef CAR_REFINISHING_MACHINE                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.WaterLevel_state);
#else
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.WaterLevel_state);
#endif                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.CriticalTemperature_state);
                if (TintingHumidifier.Temp_Enable == TEMP_DISABLE) {
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(DEVICE_DISABLED));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(DEVICE_DISABLED));                
                }    
                else {                    
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(TintingAct.Dosing_Temperature));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(TintingAct.Dosing_Temperature));
                }    
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.BasesCarriage_state);
                // Circuit Engaged
                if (TintingAct.Circuit_Engaged != 0)
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, (TintingAct.Circuit_Engaged + B8_BASE_IDX));                
                else
                    
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                

                if (!isTintingEnabled() ) {
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(DEVICE_DISABLED));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(DEVICE_DISABLED));                    
                }
                else { 
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW((TintingAct.Steps_position /(unsigned long)CORRECTION_TABLE_STEP_RES)));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW((TintingAct.Steps_position /(unsigned long)CORRECTION_TABLE_STEP_RES)));
                }
                if ( (MachineStatus.level == AUTOTEST_ST) || (OldMachineStatus == AUTOTEST_ST) ) {                    
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(procGUI.Autotest_Cycles_Number));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(procGUI.Autotest_Cycles_Number));
                }
                else {   
/*                    
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                
*/
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(Max_Retry_Photo_Ingr_Error));
                    STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(Max_Retry_Photo_Ingr_Error));
                }
                    
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(TintingAct.Cleaning_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(TintingAct.Cleaning_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_MSW(TintingAct.Cleaning_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_MSW(TintingAct.Cleaning_status));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.PanelTable_state);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Photocells_state); 
                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);   
                                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.slaves_en[0]);   
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.slaves_en[1]);   
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.slaves_en[2]);   
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.slaves_en[3]);   
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.slaves_en[4]);   
                STUFF_BYTE( txBuffer_GUI.buffer, idx, procGUI.slaves_en[5]);    

#ifdef CAR_REFINISHING_MACHINE
                STUFF_BYTE( txBuffer_GUI.buffer, idx, LSB_LSW(TintingAct.Jar_Photocells_state));
                STUFF_BYTE( txBuffer_GUI.buffer, idx, MSB_LSW(TintingAct.Jar_Photocells_state));
#else
                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);

#endif
                // Stop Formula    
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                
#ifdef CAR_REFINISHING_MACHINE                       
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Punctual_Cleaning_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Punctual_Recirculation_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Punctual_Stirring_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Refill_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Dispensing_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Cleaning_After_Dispensing_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Temporized_Recirculation_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Reset_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Temperature_Controlled_Dispensing_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Jar_Positioning_Process);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.EEprom_Erase_Write_Process);  
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Positioning_Process);  
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Self_Recognition_Process);
                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, TintingAct.Crx_Outputs_status);                                
#else                 
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                                
                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0); 

//                STUFF_BYTE( txBuffer_GUI.buffer, idx, 0);                 
#endif
                                                
                clear_slave_comm();
            break; // default (STATUS) 
        } // switch 

        // crc, pktlen taken care of here 
        FRAME_END(&txBuffer_GUI, idx);
        txBuffer_GUI.bufferFlags.txReady = TRUE;

        StartTimer(T_GUI_TX_TIMER);
    } //(txBuffer_GUI.bufferFlags.startTx == TRUE)
} // makeMessage_GUI() 

static void decodeMessage_GUI()
/**/
/*============================================================================*/
/**
**   @brief Decodes the received message, by calling the appropriate
**          decode function related to the slave: call to
**          serialSlave->decodeSerialMsg(&rxBuffer)
**
**
**   @param void
**
**   @return void
**/
/*============================================================================*/
/**/
{
    unsigned char idx = FRAME_PAYLOAD_START;
    unsigned char i, j, index;
    unsigned long timer_value;
    unsigned short timer_type;
    unionWord_t tmpWord;
    unionDWord_t tmpDWord;

    if (StatusTimer(T_FIRST_LINK_GUI_TIMER) == T_ELAPSED && StatusTimer(T_GUI_RX_TIMER) == T_HALTED) {
        StartTimer(T_GUI_RX_TIMER);
    }
    if (StatusTimer(T_GUI_RX_TIMER) == T_ELAPSED && serialGUI.numRetry >= MAX_GUI_RETRY) {
        // Declare serial timeout 
//        setAlarm(TIMEOUT_COM_MAB_MGB);
    }
    else {
        serialGUI.numRetry ++;
        StartTimer(T_GUI_RX_TIMER);
    }

    if (rxBuffer_GUI.bufferFlags.rxCompleted == TRUE) {
        // Avoid to manage message during the EEPROM deletion
        if ((CHECK_CRC16(&rxBuffer_GUI)) && (!clearEEPROMInCorso)) {

        // pre-process frame
        rxBuffer_GUI.length -= FRAME_PAYLOAD_START;
        unstuffMessage(&rxBuffer_GUI);

        // decode
        procGUI.typeMessage = rxBuffer_GUI.buffer[idx ++];
        switch (procGUI.typeMessage) {

        case DIAGNOSTICA_MACCHINA:
        case STANDBY_MACCHINA:
        case ALLARME_MACCHINA:
        case CONTROLLO_PRESENZA_MACCHINA:
        break;
        case ABORT:
        break;
        case BOOT_VERSIONS:
    	    procGUI.typeMessage = MACHINE_BOOT_VERSIONS;
        break;
        case READ_SLAVES_CONFIGURATION:
    	    procGUI.typeMessage = GET_SLAVES_CONFIGURATION;
        break;                
        case FW_VERSIONS:
            procGUI.typeMessage = MACHINE_FW_VERSIONS;
        break;
		
        case MACHINE_INFO:
            // 0 .. 3 
            procGUI.info_page = rxBuffer_GUI.buffer[idx ++];        
            // Waiting time at start up to permit,if BootLoader is present, Actuators Applications program to Starts after 9" 
            if (StatusTimer(T_WAIT_READ_FW_VERSION) == T_ELAPSED) {
                //StopTimer(T_WAIT_READ_FW_VERSION);
                procGUI.typeMessage = MACHINE_INFO;
            }	
            else
                procGUI.typeMessage = CONTROLLO_PRESENZA_MACCHINA;
        break;

        case RESET_MACCHINA:
            // Mode: 0 = COLD, 1 = WARM, 2 = HARD
            procGUI.reset_mode = rxBuffer_GUI.buffer[idx ++];
            if (procGUI.reset_mode == HARD_RESET)
                __asm__ volatile ("reset");                  
        break;

        case DISPENSAZIONE_COLORE_MACCHINA:
            // No. used colors
            procGUI.used_colors_number = rxBuffer_GUI.buffer[idx ++];
            for (i=0;i<N_SLAVES_COLOR_ACT;i++)
            {
                procGUI.used_colors_id[i] = 0;
            }
            // Qties of pigment foreach circuit [cc x 10^4], 32 bits each 
            j = 0;
            for (i = 0; i < (N_SLAVES_COLOR_ACT+8); i ++) {
                tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
                if (i < N_SLAVES_COLOR_ACT) {			  
                    procGUI.colors_vol[i] = tmpDWord.udword;
                    if (tmpDWord.udword) {
                        procGUI.used_colors_id[j] = i;
#ifdef CLEANING_AFTER_DISPENSING
                        if ( (i >= 8) && (TintingAct.Clean_Mask[i - COLORANT_ID_OFFSET -1] == TRUE) )
                            TintingAct.Cleaning_Col_Mask[i - COLORANT_ID_OFFSET -1] = TRUE;
#endif                
                        j ++;
                    }
                }  
                
            }
            // Ignoerd: Discharge type: 0 = NEGATIVE, 1 = POSITIVE 
            procGUI.unloadType = rxBuffer_GUI.buffer[idx ++];
            // Ignored 
            procGUI.dispenserType = rxBuffer_GUI.buffer[idx ++];
            // TINTING is a SEQUENTIAL 
            
            procGUI.simultaneous_colors_max = rxBuffer_GUI.buffer[idx ++];
            procGUI.simultaneous_colors_max = 1;
            // Check Can Presence at Dispening 		
            procGUI.check_can_dispensing = rxBuffer_GUI.buffer[idx ++];
            if ((procGUI.check_can_dispensing != 0) && (procGUI.check_can_dispensing != 1) )
                procGUI.check_can_dispensing = 0;
		break;
        
        case PAR_CURVA_CALIBRAZIONE_MACCHINA:
            // circuit ID 
            procGUI.id_color_circuit = rxBuffer_GUI.buffer[idx ++];
            // curve ID 
            index = procGUI.id_calib_curve = rxBuffer_GUI.buffer[idx ++];
            // Speed [rpm], 16 bit 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            calib_curve_par_writing.speed_value = calib_curve_par[index].speed_value = tmpWord.uword;
            // Algorithm, 0..3 
            calib_curve_par_writing.algorithm = calib_curve_par[index].algorithm = rxBuffer_GUI.buffer[idx ++];
            // Min vol [cc x 10^4], 32 bits
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            calib_curve_par_writing.vol_min = calib_curve_par[index].vol_min = tmpDWord.udword;		  
            // Max vol [cc x 10^4], 32 bits
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            calib_curve_par_writing.vol_max = calib_curve_par[index].vol_max = tmpDWord.udword;		  
            // Backstep data (enable, no. steps, speed), 40 bits
            calib_curve_par_writing.en_back_step = calib_curve_par[index].en_back_step = rxBuffer_GUI.buffer[idx ++];
/*
if (calib_curve_par_writing.algorithm == ALG_SINGLE_STROKE)   {             
    calib_curve_par_writing.en_back_step = 0;
    calib_curve_par[index].en_back_step = 0;
}
*/
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            calib_curve_par_writing.n_step_back_step = calib_curve_par[index].n_step_back_step = tmpWord.uword;
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            calib_curve_par_writing.speed_back_step = calib_curve_par[index].speed_back_step = tmpWord.uword;

            // Calibration points, 32 + 32 bits each 
            for (i = 0; i < N_CALIB_POINTS; i ++) {
                // Vol_i [cc x 10^4], 32 bits 
                tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
                calib_curve_par_writing.vol_cc[i] = calib_curve_par[index].vol_cc[i] = tmpDWord.udword;
                // Steps_i [cc x 10^4], 32 bits 
                tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
                tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
                calib_curve_par_writing.n_step[i] = calib_curve_par[index].n_step[i] = tmpDWord.udword;
            }
        break; // PAR_CURVA_CALIBRAZIONE_MACCHINA

        case PAR_CIRCUITO_COLORANTE_MACCHINA:
            // circuit ID 
            index = rxBuffer_GUI.buffer[idx ++];
            procGUI.id_color_circuit = index;
            // No step stroke, 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].n_step_stroke = tmpWord.uword;
            color_supply_par_writing.n_step_stroke = tmpWord.uword;
            // No step continuous end, 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].n_step_continuous_end = tmpWord.uword;
            color_supply_par_writing.n_step_continuous_end = tmpWord.uword;
            // No step home, 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].n_step_home = tmpWord.uword;
            color_supply_par_writing.n_step_home = tmpWord.uword;
            // No step backlash, 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].n_step_backlash = tmpWord.uword;
            color_supply_par_writing.n_step_backlash = tmpWord.uword;
            // Delay EV OFF[msecs], 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].delay_EV_off = tmpWord.uword;
            color_supply_par_writing.delay_EV_off = tmpWord.uword;
            // Vmu [cc x 10^4], 32 bits 
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].vol_mu = tmpDWord.udword;
            color_supply_par_writing.vol_mu = tmpDWord.udword;
            // Vmc [cc x 10^4], 32 bits 
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].vol_mc = tmpDWord.udword;
            color_supply_par_writing.vol_mc = tmpDWord.udword;
            // Suction speed [rpm], 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].speed_suction = tmpWord.uword;
            color_supply_par_writing.speed_suction = tmpWord.uword;
            // Recirculation speed [rpm], 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].speed_recirc = tmpWord.uword;
            color_supply_par_writing.speed_recirc = tmpWord.uword;
            // Recirculation duration [secs] 
            color_supply_par[index].recirc_duration = rxBuffer_GUI.buffer[idx ++];
            color_supply_par_writing.recirc_duration = color_supply_par[index].recirc_duration;
            // Recirculation pause [secs] 
            color_supply_par[index].recirc_pause = rxBuffer_GUI.buffer[idx ++];
            // Recirculation Pause has to be greater than 'MIN_RECIRC_PAUSE'
//            if (color_supply_par[index].recirc_pause < MIN_RECIRC_PAUSE)
//                color_supply_par[index].recirc_pause = MIN_RECIRC_PAUSE;
            color_supply_par_writing.recirc_pause = color_supply_par[index].recirc_pause;
            // Stirring duration [secs], 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            color_supply_par[index].reshuffle_duration = tmpWord.uword;
            color_supply_par_writing.reshuffle_duration = color_supply_par[index].reshuffle_duration;
            // Stirring PWM [pct] 
            color_supply_par[index].reshuffle_pwm_pct = rxBuffer_GUI.buffer[idx ++];
            color_supply_par_writing.reshuffle_pwm_pct = color_supply_par[index].reshuffle_pwm_pct;
            // Recirculation cycles before dispensing 
            color_supply_par[index].recirc_before_dispensation_n_cycles = rxBuffer_GUI.buffer[idx ++];
            color_supply_par_writing.recirc_before_dispensation_n_cycles = color_supply_par[index].recirc_before_dispensation_n_cycles;
            // Recirculation window [min x 10] 
            color_supply_par[index].recirc_window = rxBuffer_GUI.buffer[idx ++];
            color_supply_par_writing.recirc_window = color_supply_par[index].recirc_window;
            // Stirring window [min x 10]             
            color_supply_par[index].reshuffle_window = rxBuffer_GUI.buffer[idx ++];
            color_supply_par_writing.reshuffle_window = color_supply_par[index].reshuffle_window;
        break; // PAR_CIRCUITO_COLORANTE_MACCHINA 

        case PAR_SLAVES_CONFIGURATION:
            for (i = 0; i < N_SLAVES_BYTES; ++ i)
                en_slaves_writing[i] = rxBuffer_GUI.buffer[idx ++];
            
            pippo = 1;
        break;

        case DIAG_ATTIVA_AGITAZIONE_CIRCUITI:
        case DIAG_ATTIVA_RICIRCOLO_CIRCUITI:
        case DIAG_ATTIVA_AGITAZIONE_RICIRCOLO_CIRCUITI:
        case DIAG_ATTIVA_DISPENSAZIONE_CIRCUITI:
        case DIAG_ATTIVA_EV_DISPENSAZIONE:
            procGUI.command = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            procGUI.diag_color_en = tmpDWord.udword;
            for (i = 0; i < N_SLAVES_COLOR_ACT; ++ i) {
                if (procGUI.diag_color_en & (1L << i)) 
                    break;
            }
            i++;
            // 'indx' is a SLAVE of a DOUBLE GROUP --> Command 'DIAG_ATTIVA_RICIRCOLO_CIRCUITI' OR 'DIAG_ATTIVA_AGITAZIONE_RICIRCOLO_CIRCUITI'
            // has to be sent to MASTER cicuit '1L << procGUI.diag_color_en'
            if ( ( (procGUI.typeMessage == DIAG_ATTIVA_RICIRCOLO_CIRCUITI) || (procGUI.typeMessage == DIAG_ATTIVA_AGITAZIONE_RICIRCOLO_CIRCUITI) ||
                    (procGUI.typeMessage == DIAG_ATTIVA_AGITAZIONE_CIRCUITI) ) && 
                (procGUI.circuit_pump_types[i-1] == PUMP_DOUBLE) && (((i-1) % 2) != 0) )  {			
                procGUI.diag_color_en = procGUI.diag_color_en/2 ;
            }
            Run_Dispensing = 0;
        break;

        case DIAG_IMPOSTA_DISPENSAZIONE_CIRCUITI:
            // Circuit ID 
            index = rxBuffer_GUI.buffer[idx ++];
            /* No. step cycle for dispensation, 32 bits */
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].n_step_cycle_supply = tmpDWord.udword;
            // Dispensation cycle speed, 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].speed_cycle_supply = tmpWord.uword;
            // Algorithm, 0..4 
            colorAct[index].algorithm = rxBuffer_GUI.buffer[idx ++];
            if (colorAct[index].algorithm == ALG_DOUBLE_STROKE)
                colorAct[index].algorithm = ALG_HIGH_RES_STROKE; 
            // Dispensation cycles (ignored for CONTINUOUS) 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].n_cycles_supply = tmpWord.uword;
            // Backstep data: enable steps speed, 40 bits 
            colorAct[index].en_back_step = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].n_step_back_step = tmpWord.uword;
            // Backstep speed 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].speed_back_step = tmpWord.uword;
            // EV delay, 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].delay_EV_off = tmpWord.uword;
            // Suction speed, 16 bits
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].speed_suction = tmpWord.uword;
            // Stroke length, 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].n_step_stroke = tmpWord.uword;
            // Target point for CONTINUOUS mode (ignored in STROKE), 16 bits 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].n_step_continuous_end = tmpWord.uword;
        break;

        case DIAG_IMPOSTA_RICIRCOLO_CIRCUITI:
            // circuit ID 
            index = rxBuffer_GUI.buffer[idx ++];
            // 'indx' is a SLAVE of a DOUBLE GROUP --> Command and parameters are to be sent to MASTER cicuit 'indx-1'
            if ( (procGUI.circuit_pump_types[index] == PUMP_DOUBLE) && (index%2 != 0) )
                index--;
            // No. steps per cycle (16 bits) 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].n_step_cycle = tmpWord.uword;
            // Cycle speed (16 bits) 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].speed_cycle = tmpWord.uword;
            // No. cycles (16 bits) 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].n_cycles = tmpWord.uword;
            // Recirculation pause [s] (16 bits) 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            colorAct[index].recirc_pause = tmpWord.uword;
        break;

        case DIAG_MOVIMENTAZIONE_AUTOCAP:
            procGUI.diag_motion_autocap = rxBuffer_GUI.buffer[idx ++];
        break;
        
        case DIAG_DISPENSATION_VOL:
            procGUI.id_color_circuit = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            colorAct[procGUI.id_color_circuit].vol_t = tmpDWord.udword;
        break;

        case DIAG_READ_LAST_DISPENSATION_PARAM:
            procGUI.id_color_circuit = rxBuffer_GUI.buffer[idx ++];
        break;
        
        case DIAG_RESET_EEPROM:		
		break;
        		
        case DIAG_SETUP_TIMERS:
            timer_type   = rxBuffer_GUI.buffer[idx ++];;
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            timer_value = tmpDWord.udword * CONV_SEC_COUNT;
            if (timer_type == TIMER_OUT_SUPPLY) {
                Diag_Setup_Timer_Received = 1;
                Timer_Out_Supply_High = timer_value / 65536;
                Timer_Out_Supply_Low = timer_value % 65536;
            }
		break;

        case DIAG_JUMP_TO_BOOT:
		break;

        case DIAG_SETUP_PUMP_TYPE:
            for (i = 0; i < 32; ++ i)
                Pump_Types_Circuit_writing[i] = rxBuffer_GUI.buffer[idx ++];
		break;

	    case  ROTATING_TABLE_FIND_CIRCUITS_POSITION:
		break;

        case DIAG_SETUP_HUMIDIFIER_TEMPERATURE_PROCESSES:
            // Enable / Disable RH Sensor
            TintingHumidifierWrite.Humidifier_Enable = rxBuffer_GUI.buffer[idx ++];
            // Type of RH Sensor
            TintingHumidifierWrite.Humdifier_Type = rxBuffer_GUI.buffer[idx ++];
            // Humidifier Period
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingHumidifierWrite.Humidifier_Period = tmpWord.uword;
            // Humidifier Duration
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingHumidifierWrite.Humidifier_Multiplier = tmpWord.uword;
            // Autocap Open Nebulizer or Heater Duration
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingHumidifierWrite.AutocapOpen_Duration = tmpWord.uword;
            // Autocap Open Nebulizer or Heater Duration
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingHumidifierWrite.AutocapOpen_Period = tmpWord.uword;
            // Enable / Disable Temperature Sensor
            TintingHumidifierWrite.Temp_Enable = rxBuffer_GUI.buffer[idx ++];
            // Type of Temperature Sensor
            TintingHumidifierWrite.Temp_Type = rxBuffer_GUI.buffer[idx ++];
            // Temperature Period
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingHumidifierWrite.Temp_Period = tmpWord.uword;
            // Low Temperature threshold value
            TintingHumidifierWrite.Temp_T_LOW = rxBuffer_GUI.buffer[idx ++];
            // High Temperature threshold value		
            TintingHumidifierWrite.Temp_T_HIGH = rxBuffer_GUI.buffer[idx ++];
            // Heater Temperature threshold value		
            TintingHumidifierWrite.Heater_Temp = rxBuffer_GUI.buffer[idx ++];  
            // Heater Hysteresis value		
            TintingHumidifierWrite.Heater_Hysteresis = rxBuffer_GUI.buffer[idx ++];  
		break;
		
        case  UPDATE_TINTING_PUMP_SETTINGS:
            // Step_Accopp: Passi tra Fotocellule Madrevite e Ingranamento coperte
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Step_Accopp = tmpWord.uword;
            // Step_Ingr: Passi di Accoppiamento a Fotocellula Ingranamento Coperta
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Step_Ingr = tmpWord.uword;
            // Step_Recup: Passi di recupero giochi a Fotocellula ingranamento Coperta
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Step_Recup = tmpWord.uword;
            // Passi_Madrevite: Passi con Fotocellula Madrevite Coperta
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Passi_Madrevite = tmpWord.uword;
            // Passi_Appoggio_Soffietto: Passi per raggiungere la posizione di partenza nell’Erogazione in Alta Risoluzione
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Passi_Appoggio_Soffietto = tmpWord.uword;
            // V_Accopp: Velocità movimento tra Fotocellule Madrevite e Ingranamento coperte
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.V_Accopp = tmpWord.uword;
            // V_Ingr: Velocità a Fotocellula Ingranamento coperta
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.V_Ingr = tmpWord.uword;
            // V_Appoggio_Soffietto: Velocità per raggiungere la posizione di partenza nell’Erogazione in Alta risoluzione
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.V_Appoggio_Soffietto = tmpWord.uword;
            // Delay_Before_Valve_Backstep: 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Delay_Before_Valve_Backstep = tmpWord.uword;
            // Step_Valve_Backstep: Passi tra Valvola Chiusa e Aperta su Foro Piccolo
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Step_Valve_Backstep = tmpWord.uword;
            // Speed_valve: Velocità di movimentazione della Valvola
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Speed_Valve = tmpWord.uword;
            // N_steps_stroke: Steps on a full stroke , 16 bits
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.N_steps_stroke = tmpWord.uword;
            // Free_param_1 (Steps to Subract to 'Passi_Appoggio_Soffietto' in HighRes Mode with Big Hole)
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Free_param_1 = tmpWord.uword;
            // Free_param_2 (Type of Hole in Single Stroke Algorithm: Small/Big)
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingPumpWrite.Free_param_2 = tmpWord.uword;	
		break;

	    case  UPDATE_TINTING_TABLE_SETTINGS:
            // Steps_Revolution: Passi corrispondenti ad una rotazione completa della tavola
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Steps_Revolution = tmpWord.uword;
            // Steps_Tolerance_Revolution: Tolleranza sui passi corrispondenti ad una rotazione completa della tavola
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Steps_Tolerance_Revolution = tmpWord.uword;
            // Steps_Reference: Passi con Fotocellula presenza circuito riferimento oscurata
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Steps_Reference = tmpWord.uword;
            // Steps_Tolerance_Reference: Tolleranza sui passi con Fotocellula presenza circuito riferimento oscurata
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Steps_Tolerance_Reference = tmpWord.uword;
            // Steps_Circuit: Passi con Fotocellula presenza circuito oscurata
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Steps_Circuit = tmpWord.uword;
            // Steps_Tolerance_Circuit: Tolleranza sui passi con Fotocellula presenza circuito oscurata
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Steps_Tolerance_Circuit = tmpWord.uword;
            // High_Speed_Rotating_Table: Velocità massima rotazione della Tavola Rotante
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.High_Speed_Rotating_Table = tmpWord.uword;            
            // Low_Speed_Rotating_Table: Velocità minima rotazione della Tavola Rotante
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Low_Speed_Rotating_Table = tmpWord.uword;
            // Steps_Stirring: numero di giri della Tavola Rotante per Agitare
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingTableWrite.Steps_Stirring = tmpWord.uword;
		break;		

 	    case  UPDATE_TINTING_CLEANING_SETTINGS:
            // Cleaning Duration for all circuits (sec)
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingCleanWrite.Cleaning_duration = tmpWord.uword;
            // Cleaning Pause for all circuits (min)
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingCleanWrite.Cleaning_pause = tmpWord.uword;
            for (i = 0; i < N_SLAVES_BYTES; ++ i)
               // Id color circuit >= 8 AND <= 31
                TintingCleanWrite.Cleaning_Colorant_Mask[i] = rxBuffer_GUI.buffer[idx ++];                    
	  	break;
		
	    case  DIAG_ROTATING_TABLE_POSITIONING:
            // Id color circuit >= 8 AND <= 31
//            TintingAct.Color_Id = rxBuffer_GUI.buffer[idx ++] - COLORANT_ID_OFFSET;            
            TintingAct.NextColor_Id = rxBuffer_GUI.buffer[idx ++] - COLORANT_ID_OFFSET;            
            // Refilling Angle
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Refilling_Angle = tmpWord.uword;
            // Direction
            TintingAct.Direction = rxBuffer_GUI.buffer[idx ++];
	  	break;

	    case  DIAG_ROTATING_TABLE_SEARCH_POSITION_REFERENCE:
	  	break;

        case  DIAG_COLORANT_ACTIVATION_CLEANING:
            procGUI.command = rxBuffer_GUI.buffer[idx ++];
            // Id valid color circuit >= 8 AND <= 23            
            TintingAct.Id_Punctual_Cleaning = rxBuffer_GUI.buffer[idx ++]; 
            if ( (TintingAct.Id_Punctual_Cleaning >= 8) && (TintingAct.Id_Punctual_Cleaning <= 23) )
                TintingAct.Id_Punctual_Cleaning = TintingAct.Id_Punctual_Cleaning - COLORANT_ID_OFFSET; 
            else
                TintingAct.Id_Punctual_Cleaning = COLORANT_NOT_VALID;
            step_Clean = 0;
	  	break;

	    case  DIAG_SET_TINTING_PERIPHERALS:
            // Type of Tinting peripheral
            PeripheralAct.Peripheral_Types.bytePeripheral = rxBuffer_GUI.buffer[idx ++];
            // Peripheral Action (ON / OFF)
            TintingAct.Output_Act = rxBuffer_GUI.buffer[idx ++];  
	    break;

	    case  DIAG_ROTATING_TABLE_TEST:
	    break;
		
	    case  DIAG_ROTATING_TABLE_STEPS_POSITIONING:
            //  Type of Rotation Requested: ABSOLUTE or INCREMENTAL
            TintingAct.Rotation_Type = rxBuffer_GUI.buffer[idx ++];
            // Steps Number
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Steps_N = tmpWord.uword * (unsigned long)CORRECTION_TABLE_STEP_RES;;
            // Direction CW or CCW
            TintingAct.Direction = rxBuffer_GUI.buffer[idx ++];
	  	break;
	
         case DIAG_AUTOTEST_SETTINGS:
            // Start / Stop Autotest
            TintingAct.Autotest_Status = rxBuffer_GUI.buffer[idx ++];
            // Total Number of Cycles 
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Autotest_Total_Cycles_Number = tmpWord.uword;
            // Pause (sec) between 2 Cycles
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Autotest_Pause = tmpWord.uword;
            // 1sec = 500
            Durata[T_AUTOTEST_PAUSE] = TintingAct.Autotest_Pause * CONV_SEC_COUNT;	
            // Ricirculation Time for each circuit (sec)
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Autotest_Ricirculation_Time = tmpWord.uword;
            // 1sec = 500
            Durata[T_AUTOTEST_RICIRCULATION_TIME] = TintingAct.Autotest_Ricirculation_Time * CONV_SEC_COUNT;	
            // Small Volume Dosing (cc)
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Autotest_Small_Volume = tmpDWord.udword;
            // Medium Volume Dosing (cc)
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Autotest_Medium_Volume = tmpDWord.udword;
            // Big Volume Dosing (cc)
            tmpDWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[2] = rxBuffer_GUI.buffer[idx ++];
            tmpDWord.byte[3] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Autotest_Big_Volume = tmpDWord.udword;
            // Stirring Time (sec)
            tmpWord.byte[0] = rxBuffer_GUI.buffer[idx ++];
            tmpWord.byte[1] = rxBuffer_GUI.buffer[idx ++];
            TintingAct.Autotest_Stirring_Time = tmpWord.uword;
            // 1sec = 500
            Durata[T_AUTOTEST_STIRRING_TIME] = TintingAct.Autotest_Stirring_Time * CONV_SEC_COUNT;	
            // Start / Stop Cleaning
            TintingAct.Autotest_Cleaning_Status = rxBuffer_GUI.buffer[idx ++];
            // Start / Stop Heater
            TintingAct.Autotest_Heater_Status = rxBuffer_GUI.buffer[idx ++];            
         break;
 
#ifdef CAR_REFINISHING_MACHINE        
         case  CAN_MOVEMENT:
            Can_Transport.Dispensing_Roller = rxBuffer_GUI.buffer[idx ++];            
            Can_Transport.Lifter_Roller     = rxBuffer_GUI.buffer[idx ++];            
            Can_Transport.Input_Roller      = rxBuffer_GUI.buffer[idx ++];            
            Can_Transport.Lifter            = rxBuffer_GUI.buffer[idx ++]; 
            Can_Transport.Output_Roller     = rxBuffer_GUI.buffer[idx ++];            
            Can_Transport.typeCmd           = CAN_MOVEMENT_CMD;
	  	break;
        
         case CRX_OUTPUTS_MANAGEMENT:
            Can_Transport.Output_Number = rxBuffer_GUI.buffer[idx ++];
            Can_Transport.Output_Action = rxBuffer_GUI.buffer[idx ++];
            Can_Transport.typeCmd           = OUTPUTS_MANAGEMENT_CMD;            
         break;    
#endif        
        default:
        break;
      }
      setNewProcessingMsg();
    } // (CHECK_CRC16(&rxBuffer_GUI)) 

    initBuffer(&rxBuffer_GUI);
    IEC1bits.U2RXIE = 1;

    StartTimer(T_GUI_RX_TIMER);
  } // (rxBuffer_GUI.bufferFlags.rxCompleted == TRUE) 
} // decodeMessage_GUI() 

void initSerialCom_GUI(void)
/**/
/*===========================================================================*/
/**
**   @brief Set UART3 registers; reset rreceiver and transmission
**             buffers and flags. Start the FIRST_LINK timer window
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    // UARTEN = Disabled - USIDL = Continue module operation in Idle Mode - IREN = IrDa Encoder and Decoder disabled - RTSMD = UxRTS pin in Flow Control Mode
    // UEN1:UEN0 = UxTX and UxRX pins are enabled and used - WAKE = No Wake-up enabled - LPBACK = Loopback mode is disabled - ABAUD = Baud rate measurement disabled or completed
    // RXINV = UxRX Idle state is '1' - BRGH = High-Speed mode - PDSEL = 8-bit data, no parity - STSEL = One Stop bit 
    U2MODE = 0x08;  //High speed
    U2STA = 0;

	// BaudRate = 115200; Clock Frequency = 32MHz;   
    U2BRG = 33;

    // Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    // EN TX
    LATFbits.LATF2 = 1;
    TRISFbits.TRISF2 = OUTPUT;
    // EN RX
    LATFbits.LATF5 = 0;
    TRISFbits.TRISF5 = INPUT;
        
    // UART Enable
    U2MODEbits.UARTEN = 1;
    // Interrupt when last char is tranferred into TSR Register: so transmit buffer is empty
    U2STAbits.UTXISEL1 = 0;
    U2STAbits.UTXISEL0 = 1;
    // Transmit Enable
    U2STAbits.UTXEN = 1;
    // RESET Interrupt flags
    IFS1bits.U2RXIF = 0;
    IFS1bits.U2TXIF = 0;
    // Clear buffers
    initBuffer(&rxBuffer_GUI);
    initBuffer(&txBuffer_GUI);
    // Enable receiving
    IEC1bits.U2RXIE = 1;
}

static void sendMessage_GUI()
/**/
/*===========================================================================*/
/**
**   @brief Start the transmission, enabling the UART 3 transmission
**          flag and filling the UART3 tx buffer with the first byte
**          to be transmitted
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  if(txBuffer_GUI.bufferFlags.txReady == TRUE) {

    if ((txBuffer_GUI.bufferFlags.uartBusy != TRUE) &&
        (txBuffer_GUI.length <= BUFFER_SIZE)) {

      // Pulisco l'interrupt flag della trasmissione
      IFS1bits.U2TXIF = 0;

      // Abilito il flag UARTx En
      IEC1bits.U2TXIE = 1;

      // Scarico il primo byte nel buffer di trasmissione : Write data
      // byte to lower byte of UxTXREG word Take control of buffer
      txBuffer_GUI.bufferFlags.uartBusy = TRUE;

      if (U2STAbits.TRMT) {
        U2TXREG = txBuffer_GUI.buffer[txBuffer_GUI.index ++];
      }
      else {
        U2MODEbits.UARTEN = 0;
        initSerialCom_GUI();
      }
    }
  }
} // sendMessage_GUI() 

void serialCommManager_GUI()
/**/
/*===========================================================================*/
/**
**   @brief Sequencer of the module
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  decodeMessage_GUI();
  makeMessage_GUI();
  sendMessage_GUI();
} // serialCommManager_GUI()

/******************************************************************************/
/****************************** Interrupt Routine *****************************/
/******************************************************************************/

void  U2TX_InterruptHandler(void)
/**/
/*===========================================================================*/
/**
**   @brief Interrupt in TX della UART2
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
  if (_U2TXIE && _U2TXIF) {
    _U2TXIF = 0;

    if (txBuffer_GUI.index == txBuffer_GUI.length) {
      // Disabling UARTx En
      IEC1bits.U2TXIE = 0;

      txBuffer_GUI.bufferFlags.uartBusy = FALSE;
      txBuffer_GUI.bufferFlags.txReady = FALSE;
    }
    else {
      U2TXREG = txBuffer_GUI.buffer[txBuffer_GUI.index ++];
    }
  }
}

void  U2RX_InterruptHandler(void)
/**/
/*===========================================================================*/
/**
**   @brief Interrupt in RX della UART2
**
**   @param void
**
**   @return void
**/
/*===========================================================================*/
/**/
{
    register unsigned char flushUart;
    countBuffRx = 0;
    if (_U2RXIE && _U2RXIF) {
        _U2RXIF = 0;

        // Overrun Error 
        if (U2STAbits.OERR) {
          U2STAbits.OERR = 0;
          SIGNAL_ERROR(rxBuffer_GUI);
        }

        // Framing Error 
        if (U2STAbits.FERR) {
          flushUart = U2RXREG;
          SIGNAL_ERROR(rxBuffer_GUI);
        }
        while (U2STAbits.URXDA && countBuffRx < URXREG_NUM_BYTES) {
            flushUart = U2RXREG;
            rebuildMessage_GUI(flushUart);    
            countBuffRx++;
        }	 
    }
}
