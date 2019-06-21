/* 
 * File:   spi3.h
 * Author: michele.abelli
 * Description: SPI3 
 * Created on 15 ottobre 2018, 14.16
 */

#include "p24FJ256GB110.h"
#include "spi3.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "timerMg.h"
#include <xc.h>
#include <stdlib.h>

/*
*//*=====================================================================*//**
**      @brief Initialization SPI3
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void SPI3_Initialize(void)
{
    // MSTEN Master; DISSDO disabled; PPRE 64:1; SPRE 8:1; MODE16 disabled; SMP Middle; DISSCK disabled; CKP Idle:Low, Active:High; CKE Idle to Active; SSEN disabled; 
    // SPI Clock Frequency: 7.8125kHz
    // Communication is byte wide, MODE16 disabled
    SPI3CON1 = 0x20;
    // SPIFSD disabled; SPIBEN enabled; SPIFPOL disabled; SPIFE disabled; FRMEN disabled; 
    // Enhanced Buffer Master Mode selected, SPIBEN enabled
    SPI3CON2 = 0x01;
    // SPITBF disabled; SISEL SPI_INT_SPIRBF; SPIRBF disabled; SPIROV disabled; SPIEN enabled; SRXMPT disabled; SPISIDL disabled; 
    SPI3STAT = 0x800C;
    // Al momento NON usiamo gli Interrupt
    // Enable alt 3 SPI3 Interrupts
    //    SPITXI: SPI3TX - SPI3 Transfer Done
    //    Priority: 1
//    IPC2bits.SPI3TXIP = 1;
    //    SPII: SPI3 - SPI3 General
    //    Priority: 1
//    IPC2bits.SPI3IP = 1;
    //    SPIRXI: SPI3RX - SPI3 Receive Done
    //    Priority: 1
//    IPC14bits.SPI3RXIP = 1;
}

void Write_SPI3_Command(uint8_t *pTransmitData)
{
    // Send first Byte
    while( SPI3STATbits.SPITBF == TRUE) {}
    SPI3BUF = *((uint8_t*)pTransmitData);
    // Send second Byte
    while( SPI3STATbits.SPITBF == TRUE) {}
    SPI3BUF = *((uint8_t*)pTransmitData+1);
}  

void Read_SPI3_Command(uint8_t *pTransmitData, uint8_t *pReceiveData)
{
    // Send first Byte
    while( SPI3STATbits.SPITBF == TRUE) {}
    SPI3BUF = *((uint8_t*)pTransmitData);
    // Read first Byte (SRXMPT valid in Enhanced Bufer Mode)
    while ( SPI3STATbits.SRXMPT == TRUE) {}
    *((uint8_t*)pReceiveData) = SPI3BUF;    
}

/*
*//*=====================================================================*//**
**      @brief Updates Temperature TC72 Acquisition
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void SPI3_Manager (void)
{
    static uint8_t Status_SPI = SPI_IDLE;
    uint8_t Read_res[1];
    uint16_t TC72_Raw_Temperature;     
    uint8_t writeBuffer[2];
    static uint8_t Read_results[3];
    
    switch (Status_SPI)
    {
        case SPI_IDLE:
            if (Dos_Temperature_Enable == TRUE) {
                // Wait RESET ends
                if (TemperatureResetProcedure(ON) == TRUE)  {                   
                    // Continuous Mode selected 
                    writeBuffer[0] = 0x80; // MSB = 1           
                    writeBuffer[1] = 0x04; // OS = 0 - SHDN = 0             
                    // Write Multiple Byte Temperature Command to TC72
                    Write_SPI3_Command (writeBuffer);
                    Status_SPI = SPI_WRITE_MULTIPLE_BYTE_TRANSFER;                    
                }
            }    
        break;      
// -----------------------------------------------------------------------------
        case SPI_WRITE_MULTIPLE_BYTE_TRANSFER:
            if (Dos_Temperature_Enable == TRUE) { 
                if (Start_New_Temp_Measurement == TRUE) { 
                    StartTimer(T_SPI_MEASUREMENT);           
                    CE_TC72 = 0;
                    Status_SPI = SPI_WAIT_READ_RESULTS;                
                }
            }
            else 
                Status_SPI = SPI_IDLE;
        break;

        // WAIT 200 msec before to Read results (at least 150msec)
        case SPI_WAIT_READ_RESULTS:
            if (StatusTimer(T_SPI_MEASUREMENT) == T_ELAPSED)
            {
                StopTimer(T_SPI_MEASUREMENT);
                CE_TC72 = 1;
                Read_results[0] = 0x00;
                Read_results[1] = 0x00;
                Read_results[2] = 0x00 ;       
                Status_SPI = SPI_READ_RESULTS;                          
            }
        break;

        // Read Temperature 
        case SPI_READ_RESULTS:
            // Write Read Temperature Command to TC72
            writeBuffer[0] = 0x02; // MSB = 0, ADDRESS = 0x02        
            Read_SPI3_Command (writeBuffer, Read_res);

            // Clock output for other 3 bytes
            writeBuffer[0] = 0x00;        
            Read_SPI3_Command (writeBuffer, Read_res);
            Read_SPI3_Command (writeBuffer, Read_res);
            Read_SPI3_Command (writeBuffer, Read_res);            
            Read_results[0] = Read_res[0];
            Read_SPI3_Command (writeBuffer, Read_res);            
            Read_results[1] = Read_res[0]>>6;
            Read_SPI3_Command (writeBuffer, Read_res);            
            Read_results[2] = Read_res[0];
            if (Read_results[2] != 0x04) { 
                Sensor_Temp_Measurement_Error = TRUE;
                Status_SPI = SPI_HARD_RESET;          
            }
            else 
            {
                Sensor_Temp_Measurement_Error = FALSE;
                Status_SPI = SPI_CALCULATE_TEMPERATURE;          
            }
        break;

        // Calculate Absolute values from Raw data
        case SPI_CALCULATE_TEMPERATURE:
            // Temperature (°C) x 10            
            TC72_Raw_Temperature = Read_results[0] * 10 + (Read_results[1] * 25) / 10; 
            TC72_Temperature = TC72_Raw_Temperature; 
            Start_New_Temp_Measurement = FALSE;            
            Status_SPI = SPI_WRITE_MULTIPLE_BYTE_TRANSFER;            
        break;
        
        case SPI_HARD_RESET:
            Start_New_Temp_Measurement = FALSE;  
            Status_SPI = SPI_IDLE;
        break;

        default:
        break;    
    }        
}     

/*
*//*=====================================================================*//**
**      @brief SPI3 Hard reset Procedure 
**
**      @param unsigned chat 'type': ON  --> Hard Reset Activation
**                                   OFF --> StopHard Reset   
**
**      @retval unsigned int status: TRUE: Hard Reset Done or Stopped
**                                   WAIT: Hard Reset in Execution         
**
*//*=====================================================================*//**
*/
unsigned int TemperatureResetProcedure (unsigned char type)
{
    static unsigned int reset_procedure = RESET_ON;
    
    if (type == OFF)
    {
        StopTimer(T_SPI_HARD_RESET);
        reset_procedure = RESET_ON;
        return TRUE;
    }
    else 
    {
        switch (reset_procedure)
        {            
            case RESET_ON:
              
                StopTimer(T_SPI_HARD_RESET);
                StartTimer(T_SPI_HARD_RESET);
                // TC72 Disabled
                CE_TC72 = 0;
                reset_procedure = RESET_WAIT;
                return WAIT;
            break;
            case RESET_WAIT:
                if (StatusTimer(T_SPI_HARD_RESET) == T_ELAPSED)
                {            
                    // TC72 Enabled
                    CE_TC72 = 1;                
                    reset_procedure = RESET_ON;
                    return TRUE;
                }                            
                else
                    return WAIT;
            break;
            default:
                return WAIT;
            break;
        }                            
    }    
}
