
/**
  I2C3 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    i2c3.h

  @Summary
    This is the generated header file for the I2C3 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for I2C3.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.75
        Device            :  PIC24FJ256GB110

    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.35
        MPLAB 	          :  MPLAB X v5.05
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#ifndef _I2C3_H
#define _I2C3_H
/**
  Section: Included Files
*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <xc.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
 Section: Data Type Definitions
*/

/**
  I2C Driver Message Status Type Enumeration

  @Summary
    Defines the different message status when processing
    TRBs.

  @Description
    This type enumeration specifies the different types of status
    that an i2c request will have. For every submission of an i2c
    transaction, the status of that transaction is available.
    Based on the status, new transactions can be requested to the
    driver or a recovery can be performed to resend the transaction.

 */

typedef enum
{
    I2C3_MESSAGE_FAIL,
    I2C3_MESSAGE_PENDING,
    I2C3_MESSAGE_COMPLETE,
    I2C3_STUCK_START,
    I2C3_MESSAGE_ADDRESS_NO_ACK,
    I2C3_DATA_NO_ACK,
    I2C3_LOST_STATE
} I2C3_MESSAGE_STATUS;

typedef enum
{
    I2C_IDLE,
    I2C_WRITE_SINGLE_SHOT_DATA_ACQUISITION,
    I2C_WAIT_READ_RESULTS,
    I2C_READ_RESULTS,
    I2C_CALCULATE_HUMIDITY_TEMPERATURE,
    I2C_WRITE_STATUS_COMMAND,
    I2C_WAIT_READ_STATUS,
    I2C_READ_STATUS,
    I2C_HARD_RESET,
    I2C_WRITE_HEATER_ON,
    I2C_WRITE_HEATER_OFF,
    I2C_WAIT_HEATER_ON,        
    I2C_WRITE_HEATER_STATUS_COMMAND,
    I2C_READ_ON_STATUS,
    I2C_WAIT_READ_STATUS_OFF,
    I2C_WRITE_HEATER_OFF_STATUS_COMMAND,
    I2C_WAIT_READ_OFF_STATUS,
    I2C_READ_OFF_STATUS,
    I2C_WAIT_READ_ON_STATUS,
} I2C3_ACQUISITION_STATUS;

typedef struct
{
    uint16_t  address;          // Bits <10:1> are the 10 bit address.
                                // Bits <7:1> are the 7 bit address
                                // Bit 0 is R/W (1 for read)
    uint8_t   length;           // the # of bytes in the buffer
    uint8_t   *pbuffer;         // a pointer to a buffer of length bytes
} I2C3_TRANSACTION_REQUEST_BLOCK;
        
extern void I2C3_Initialize(void);

extern void I2C3_MasterWrite(
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address,
                                I2C3_MESSAGE_STATUS *pstatus);

extern void I2C3_MasterRead(
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address,
                                I2C3_MESSAGE_STATUS *pstatus);
                                

extern void I2C3_MasterTRBInsert(
                                uint8_t count,
                                I2C3_TRANSACTION_REQUEST_BLOCK *ptrb_list,
                                I2C3_MESSAGE_STATUS *pflag);
                                

extern void I2C3_MasterReadTRBBuild(
                                I2C3_TRANSACTION_REQUEST_BLOCK *ptrb,
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address);                               
                                

extern void I2C3_MasterWriteTRBBuild(
                                I2C3_TRANSACTION_REQUEST_BLOCK *ptrb,
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address);                           
                                
extern bool I2C3_MasterQueueIsEmpty(void);                              
extern bool I2C3_MasterQueueIsFull(void);             

extern unsigned int ResetProcedure (unsigned char type);
extern void I2C_Manager (void);
extern uint8_t Write_I2C_Command (uint8_t writeCommand[2]);
extern uint8_t Read_I2C_Command (uint8_t *res, uint8_t bytes_n);
extern void MI2C3_InterruptHandler(void);

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif //_I2C3_H
    
/**
 End of File
*/
