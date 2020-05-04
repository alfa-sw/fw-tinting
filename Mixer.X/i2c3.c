
/**
  I2C3 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    i2c3.c

  @Summary
    This is the generated header file for the i2c3 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for i2c3.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.75
        Device            :  PIC24FJ256GB110

    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.35
        MPLAB             :  MPLAB X v5.05
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
#include "p24FJ256GB110.h"
#include "ram.h"
#include "gestio.h"
#include "define.h"
#include "timerMg.h"
#include "i2c3.h"
#include <xc.h>
#include <stdlib.h>

const uint16_t POLYNOMIAL = 0x131; //P(x)=x^8+x^5+x^4+1 = 100110001

/**
 Section: Data Types
*/

/**
  I2C Driver Queue Status Type

  @Summary
    Defines the type used for the transaction queue status.

  @Description
    This defines type used to keep track of the queue status.
 */

typedef union
{
    struct
    {
            uint8_t full:1;
            uint8_t empty:1;
            uint8_t reserved:6;
    }s;
    uint8_t status;
} I2C_TR_QUEUE_STATUS;

/**
  I2C Driver Queue Entry Type

  @Summary
    Defines the object used for an entry in the i2c queue items.

  @Description
    This defines the object in the i2c queue. Each entry is a composed
    of a list of TRBs, the number of the TRBs and the status of the
    currently processed TRB.
 */
typedef struct
{
    uint8_t                         count;          // a count of trb's in the trb list
    I2C3_TRANSACTION_REQUEST_BLOCK  *ptrb_list;     // pointer to the trb list
    I2C3_MESSAGE_STATUS             *pTrFlag;       // set with the error of the last trb sent.
                                                    // if all trb's are sent successfully,
                                                    // then this is I2C3_MESSAGE_COMPLETE
} I2C_TR_QUEUE_ENTRY;

/**
  I2C Master Driver Object Type

  @Summary
    Defines the object that manages the i2c master.

  @Description
    This defines the object that manages the sending and receiving of
    i2c master transactions.
  */

typedef struct
{
    /* Read/Write Queue */
    I2C_TR_QUEUE_ENTRY          *pTrTail;       // tail of the queue
    I2C_TR_QUEUE_ENTRY          *pTrHead;       // head of the queue
    I2C_TR_QUEUE_STATUS         trStatus;       // status of the last transaction
    uint8_t                     i2cDoneFlag;    // flag to indicate the current
                                                // transaction is done
    uint8_t                     i2cErrors;      // keeps track of errors


} I2C_OBJECT ;

/**
  I2C Master Driver State Enumeration

  @Summary
    Defines the different states of the i2c master.

  @Description
    This defines the different states that the i2c master
    used to process transactions on the i2c bus.
*/

typedef enum
{
    S_MASTER_IDLE,
    S_MASTER_RESTART,
    S_MASTER_SEND_ADDR,
    S_MASTER_SEND_DATA,
    S_MASTER_SEND_STOP,
    S_MASTER_ACK_ADDR,
    S_MASTER_RCV_DATA,
    S_MASTER_RCV_STOP,
    S_MASTER_ACK_RCV_DATA,
    S_MASTER_NOACK_STOP,
    S_MASTER_SEND_ADDR_10BIT_LSB,
    S_MASTER_10BIT_RESTART,
    
} I2C_MASTER_STATES;

/**
 Section: Macro Definitions
*/

/* defined for I2C3 */


#ifndef I2C3_CONFIG_TR_QUEUE_LENGTH
        #define I2C3_CONFIG_TR_QUEUE_LENGTH 1
#endif

#define I2C3_TRANSMIT_REG                       I2C3TRN			// Defines the transmit register used to send data.
#define I2C3_RECEIVE_REG                        I2C3RCV	// Defines the receive register used to receive data.

// The following control bits are used in the I2C state machine to manage
// the I2C module and determine next states.
#define I2C3_WRITE_COLLISION_STATUS_BIT         I2C3STATbits.IWCOL	// Defines the write collision status bit.
#define I2C3_ACKNOWLEDGE_STATUS_BIT             I2C3STATbits.ACKSTAT	// I2C ACK status bit.

#define I2C3_START_CONDITION_ENABLE_BIT         I2C3CONbits.SEN		// I2C START control bit.
#define I2C3_REPEAT_START_CONDITION_ENABLE_BIT  I2C3CONbits.RSEN	// I2C Repeated START control bit.
#define I2C3_RECEIVE_ENABLE_BIT                 I2C3CONbits.RCEN	// I2C Receive enable control bit.
#define I2C3_STOP_CONDITION_ENABLE_BIT          I2C3CONbits.PEN		// I2C STOP control bit.
#define I2C3_ACKNOWLEDGE_ENABLE_BIT             I2C3CONbits.ACKEN 	// I2C ACK start control bit.
#define I2C3_ACKNOWLEDGE_DATA_BIT               I2C3CONbits.ACKDT	// I2C ACK data control bit.

/**
 Section: Local Functions
*/

static void I2C3_FunctionComplete(void);
static void I2C3_Stop(I2C3_MESSAGE_STATUS completion_code);

/**
 Section: Local Variables
*/

static I2C_TR_QUEUE_ENTRY            i2c3_tr_queue[I2C3_CONFIG_TR_QUEUE_LENGTH];
static I2C_OBJECT                    i2c3_object;
static I2C_MASTER_STATES             i2c3_state = S_MASTER_IDLE;
static uint8_t                       i2c3_trb_count;

static I2C3_TRANSACTION_REQUEST_BLOCK *p_i2c3_trb_current;
static I2C_TR_QUEUE_ENTRY            *p_i2c3_current = NULL;


/**
  Section: Driver Interface
*/

void I2C3_Initialize(void)
{
    
    i2c3_object.pTrHead = i2c3_tr_queue;
    i2c3_object.pTrTail = i2c3_tr_queue;
    i2c3_object.trStatus.s.empty = true;
    i2c3_object.trStatus.s.full = false;

    i2c3_object.i2cErrors = 0;
    
    // Initialize the hardware
    // Baud Rate Generator Value: I2CBRG 18 --> Clock Frequency 400KHz   
//    I2C1BRG = 0x0012;
    I2C3BRG = 0x090; // 50KHz

    // ACKEN disabled; STREN disabled; GCEN disabled; SMEN disabled; DISSLW enabled; I2CSIDL disabled; ACKDT Sends ACK; SCLREL Holds; RSEN disabled; IPMIEN disabled; A10M 7 Bit; PEN disabled; RCEN disabled; SEN disabled; I2CEN enabled; 
    I2C3CON = 0x8000;
    // P disabled; S disabled; I2COV disabled; IWCOL disabled; 
    I2C3STAT = 0x00;

    /* MI2C3 - I2C3 Master Events */
    // clear the master interrupt flag
    IFS5bits.MI2C3IF = 0;
    // enable the master interrupt
    IEC5bits.MI2C3IE = 1;
    
    // Sensor Hard Reset OFF
    RST_SHT31 = 1;
    
    // Sensirion SHT31 fixed Address
    SHT31_DeviceAddress = 0x44;
}


uint8_t I2C3_ErrorCountGet(void)
{
    uint8_t ret;

    ret = i2c3_object.i2cErrors;
    return ret;
}

void MI2C3_InterruptHandler(void)
{
  
    static uint8_t  *pi2c_buf_ptr;
    static uint16_t i2c_address;
    static uint8_t  i2c_bytes_left;
    static uint8_t  i2c_10bit_address_restart = 0;

    IFS5bits.MI2C3IF = 0;
            
    // Check first if there was a collision.
    // If we have a Write Collision, reset and go to idle state */
    if(I2C3_WRITE_COLLISION_STATUS_BIT)
    {
        // clear the Write colision
        I2C3_WRITE_COLLISION_STATUS_BIT = 0;
        i2c3_state = S_MASTER_IDLE;
        *(p_i2c3_current->pTrFlag) = I2C3_MESSAGE_FAIL;

        // reset the buffer pointer
        p_i2c3_current = NULL;

        return;
    }

    /* Handle the correct i2c state */
    switch(i2c3_state)
    {
        case S_MASTER_IDLE:    /* In reset state, waiting for data to send */

            if(i2c3_object.trStatus.s.empty != true)
            {
                // grab the item pointed by the head
                p_i2c3_current     = i2c3_object.pTrHead;
                i2c3_trb_count     = i2c3_object.pTrHead->count;
                p_i2c3_trb_current = i2c3_object.pTrHead->ptrb_list;

                i2c3_object.pTrHead++;

                // check if the end of the array is reached
                if(i2c3_object.pTrHead == (i2c3_tr_queue + I2C3_CONFIG_TR_QUEUE_LENGTH))
                {
                    // adjust to restart at the beginning of the array
                    i2c3_object.pTrHead = i2c3_tr_queue;
                }

                // since we moved one item to be processed, we know
                // it is not full, so set the full status to false
                i2c3_object.trStatus.s.full = false;

                // check if the queue is empty
                if(i2c3_object.pTrHead == i2c3_object.pTrTail)
                {
                    // it is empty so set the empty status to true
                    i2c3_object.trStatus.s.empty = true;
                }

                // send the start condition
                I2C3_START_CONDITION_ENABLE_BIT = 1;

                // start the i2c request
                i2c3_state = S_MASTER_SEND_ADDR;
            }

            break;

        case S_MASTER_RESTART:

            /* check for pending i2c Request */

            // ... trigger a REPEATED START
            I2C3_REPEAT_START_CONDITION_ENABLE_BIT = 1;

            // start the i2c request
            i2c3_state = S_MASTER_SEND_ADDR;

            break;

        case S_MASTER_SEND_ADDR_10BIT_LSB:

            if(I2C3_ACKNOWLEDGE_STATUS_BIT)
            {
                i2c3_object.i2cErrors++;
                I2C3_Stop(I2C3_MESSAGE_ADDRESS_NO_ACK);
            }
            else
            {
                // Remove bit 0 as R/W is never sent here
                I2C3_TRANSMIT_REG = (i2c_address >> 1) & 0x00FF;

                // determine the next state, check R/W
                if(i2c_address & 0x01)
                {
                    // if this is a read we must repeat start
                    // the bus to perform a read
                    i2c3_state = S_MASTER_10BIT_RESTART;
                }
                else
                {
                    // this is a write continue writing data
                    i2c3_state = S_MASTER_SEND_DATA;
                }
            }

            break;

        case S_MASTER_10BIT_RESTART:

            if(I2C3_ACKNOWLEDGE_STATUS_BIT)
            {
                i2c3_object.i2cErrors++;
                I2C3_Stop(I2C3_MESSAGE_ADDRESS_NO_ACK);
            }
            else
            {
                // ACK Status is good
                // restart the bus
                I2C3_REPEAT_START_CONDITION_ENABLE_BIT = 1;

                // fudge the address so S_MASTER_SEND_ADDR works correctly
                // we only do this on a 10-bit address resend
                i2c_address = 0x00F0 | ((i2c_address >> 8) & 0x0006);

                // set the R/W flag
                i2c_address |= 0x0001;

                // set the address restart flag so we do not change the address
                i2c_10bit_address_restart = 1;

                // Resend the address as a read
                i2c3_state = S_MASTER_SEND_ADDR;
            }

            break;

        case S_MASTER_SEND_ADDR:

            /* Start has been sent, send the address byte */

            /* Note: 
                On a 10-bit address resend (done only during a 10-bit
                device read), the original i2c_address was modified in
                S_MASTER_10BIT_RESTART state. So the check if this is
                a 10-bit address will fail and a normal 7-bit address
                is sent with the R/W bit set to read. The flag
                i2c_10bit_address_restart prevents the  address to
                be re-written.
             */
            if(i2c_10bit_address_restart != 1)
            {
                // extract the information for this message
                i2c_address    = p_i2c3_trb_current->address;
                pi2c_buf_ptr   = p_i2c3_trb_current->pbuffer;
                i2c_bytes_left = p_i2c3_trb_current->length;
            }
            else
            {
                // reset the flag so the next access is ok
                i2c_10bit_address_restart = 0;
            }

            // check for 10-bit address
            if(i2c_address > 0x00FF)
            {
                // we have a 10 bit address
                // send bits<9:8>
                // mask bit 0 as this is always a write
                I2C3_TRANSMIT_REG = 0xF0 | ((i2c_address >> 8) & 0x0006);
                i2c3_state = S_MASTER_SEND_ADDR_10BIT_LSB;
            }
            else
            {
                // Transmit the address
                I2C3_TRANSMIT_REG = i2c_address;
                if(i2c_address & 0x01)
                {
                    // Next state is to wait for address to be acked
                    i2c3_state = S_MASTER_ACK_ADDR;
                }
                else
                {
                    // Next state is transmit
                    i2c3_state = S_MASTER_SEND_DATA;
                }
            }
            break;

        case S_MASTER_SEND_DATA:

            // Make sure the previous byte was acknowledged
            if(I2C3_ACKNOWLEDGE_STATUS_BIT)
            {
                // Transmission was not acknowledged
                i2c3_object.i2cErrors++;

                // Reset the Ack flag
                I2C3_ACKNOWLEDGE_STATUS_BIT = 0;

                // Send a stop flag and go back to idle
                I2C3_Stop(I2C3_DATA_NO_ACK);

            }
            else
            {
                // Did we send them all ?
                if(i2c_bytes_left-- == 0U)
                {
                    // yup sent them all!

                    // update the trb pointer
                    p_i2c3_trb_current++;

                    // are we done with this string of requests?
                    if(--i2c3_trb_count == 0)
                    {
                        I2C3_Stop(I2C3_MESSAGE_COMPLETE);
                    }
                    else
                    {
                        // no!, there are more TRB to be sent.
                        //I2C3_START_CONDITION_ENABLE_BIT = 1;

                        // In some cases, the slave may require
                        // a restart instead of a start. So use this one
                        // instead.
                        I2C3_REPEAT_START_CONDITION_ENABLE_BIT = 1;

                        // start the i2c request
                        i2c3_state = S_MASTER_SEND_ADDR;

                    }
                }
                else
                {
                    // Grab the next data to transmit
                    I2C3_TRANSMIT_REG = *pi2c_buf_ptr++;
                }
            }
            break;

        case S_MASTER_ACK_ADDR:

            /* Make sure the previous byte was acknowledged */
            if(I2C3_ACKNOWLEDGE_STATUS_BIT)
            {

                // Transmission was not acknowledged
                i2c3_object.i2cErrors++;

                // Send a stop flag and go back to idle
                I2C3_Stop(I2C3_MESSAGE_ADDRESS_NO_ACK);

                // Reset the Ack flag
                I2C3_ACKNOWLEDGE_STATUS_BIT = 0;
            }
            else
            {
                I2C3_RECEIVE_ENABLE_BIT = 1;
                i2c3_state = S_MASTER_ACK_RCV_DATA;
            }
            break;

        case S_MASTER_RCV_DATA:

            /* Acknowledge is completed.  Time for more data */

            // Next thing is to ack the data
            i2c3_state = S_MASTER_ACK_RCV_DATA;

            // Set up to receive a byte of data
            I2C3_RECEIVE_ENABLE_BIT = 1;

            break;

        case S_MASTER_ACK_RCV_DATA:

            // Grab the byte of data received and acknowledge it
            *pi2c_buf_ptr++ = I2C3_RECEIVE_REG;

            // Check if we received them all?
            if(--i2c_bytes_left)
            {

                /* No, there's more to receive */

                // No, bit 7 is clear.  Data is ok
                // Set the flag to acknowledge the data
                I2C3_ACKNOWLEDGE_DATA_BIT = 0;

                // Wait for the acknowledge to complete, then get more
                i2c3_state = S_MASTER_RCV_DATA;
            }
            else
            {

                // Yes, it's the last byte.  Don't ack it
                // Flag that we will nak the data
                I2C3_ACKNOWLEDGE_DATA_BIT = 1;

                I2C3_FunctionComplete();
            }

            // Initiate the acknowledge
            I2C3_ACKNOWLEDGE_ENABLE_BIT = 1;
            break;

        case S_MASTER_RCV_STOP:                
        case S_MASTER_SEND_STOP:

            // Send the stop flag
            I2C3_Stop(I2C3_MESSAGE_COMPLETE);
            break;

        default:

            // This case should not happen, if it does then
            // terminate the transfer
            i2c3_object.i2cErrors++;
            I2C3_Stop(I2C3_LOST_STATE);
            break;

    }
}

static void I2C3_FunctionComplete(void)
{

    // update the trb pointer
    p_i2c3_trb_current++;

    // are we done with this string of requests?
    if(--i2c3_trb_count == 0)
    {
        i2c3_state = S_MASTER_SEND_STOP;
    }
    else
    {
        i2c3_state = S_MASTER_RESTART;
    }

}

static void I2C3_Stop(I2C3_MESSAGE_STATUS completion_code)
{
    // then send a stop
    I2C3_STOP_CONDITION_ENABLE_BIT = 1;

    // make sure the flag pointer is not NULL
    if (p_i2c3_current->pTrFlag != NULL)
    {
        // update the flag with the completion code
        *(p_i2c3_current->pTrFlag) = completion_code;
    }

    // Done, back to idle
    i2c3_state = S_MASTER_IDLE;
    
}

void I2C3_MasterWrite(
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address,
                                I2C3_MESSAGE_STATUS *pstatus)
{
    static I2C3_TRANSACTION_REQUEST_BLOCK   trBlock;

    // check if there is space in the queue
    if (i2c3_object.trStatus.s.full != true)
    {
        I2C3_MasterWriteTRBBuild(&trBlock, pdata, length, address);
        I2C3_MasterTRBInsert(1, &trBlock, pstatus);
    }
    else
    {
        *pstatus = I2C3_MESSAGE_FAIL;
    }

}                           

void I2C3_MasterRead(
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address,
                                I2C3_MESSAGE_STATUS *pstatus)
{
    static I2C3_TRANSACTION_REQUEST_BLOCK   trBlock;


    // check if there is space in the queue
    if (i2c3_object.trStatus.s.full != true)
    {
        I2C3_MasterReadTRBBuild(&trBlock, pdata, length, address);
        I2C3_MasterTRBInsert(1, &trBlock, pstatus);
    }
    else
    {
        *pstatus = I2C3_MESSAGE_FAIL;
    }

}       

void I2C3_MasterTRBInsert(
                                uint8_t count,
                                I2C3_TRANSACTION_REQUEST_BLOCK *ptrb_list,
                                I2C3_MESSAGE_STATUS *pflag)
{

    // check if there is space in the queue
    if (i2c3_object.trStatus.s.full != true)
    {
        *pflag = I2C3_MESSAGE_PENDING;

        i2c3_object.pTrTail->ptrb_list = ptrb_list;
        i2c3_object.pTrTail->count     = count;
        i2c3_object.pTrTail->pTrFlag   = pflag;
        i2c3_object.pTrTail++;

        // check if the end of the array is reached
        if (i2c3_object.pTrTail == (i2c3_tr_queue + I2C3_CONFIG_TR_QUEUE_LENGTH))
        {
            // adjust to restart at the beginning of the array
            i2c3_object.pTrTail = i2c3_tr_queue;
        }

        // since we added one item to be processed, we know
        // it is not empty, so set the empty status to false
        i2c3_object.trStatus.s.empty = false;

        // check if full
        if (i2c3_object.pTrHead == i2c3_object.pTrTail)
        {
            // it is full, set the full status to true
            i2c3_object.trStatus.s.full = true;
        }

        // for interrupt based
        if(i2c3_state == S_MASTER_IDLE)
        {    
            // force the task to run since we know that the queue has
            // something that needs to be sent
            IFS5bits.MI2C3IF = 1;
        }           
        
    }
    else
    {
        *pflag = I2C3_MESSAGE_FAIL;
    }

}      
                                
void I2C3_MasterReadTRBBuild(
                                I2C3_TRANSACTION_REQUEST_BLOCK *ptrb,
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address)
{
    ptrb->address  = address << 1;
    // make this a read
    ptrb->address |= 0x01;
    ptrb->length   = length;
    ptrb->pbuffer  = pdata;
}
                                
void I2C3_MasterWriteTRBBuild(
                                I2C3_TRANSACTION_REQUEST_BLOCK *ptrb,
                                uint8_t *pdata,
                                uint8_t length,
                                uint16_t address)
{
    ptrb->address = address << 1;
    ptrb->length  = length;
    ptrb->pbuffer = pdata;
}

bool I2C3_MasterQueueIsEmpty(void)
{
    return((bool)i2c3_object.trStatus.s.empty);
}

bool I2C3_MasterQueueIsFull(void)
{
    return((bool)i2c3_object.trStatus.s.full);
}

/*
*//*=====================================================================*//**
**      @brief Write Command to SHT31 Sensor
**
**      @param uint8_t writeCommand[2]
**
**      @retval 'MEASUREMENT_ERROR'
**              'MEASUREMENT_OK'
**
*//*=====================================================================*//**
*/
uint8_t Write_I2C_Command (uint8_t writeCommand[2])
{
    uint16_t timeOut, slaveTimeOut;
    I2C3_MESSAGE_STATUS status;
    
    timeOut = 0;
    slaveTimeOut = 0;
    StopTimer(T_SHT31_WRITE_TIMEOUT);
    StartTimer(T_SHT31_WRITE_TIMEOUT);            
    status = I2C3_MESSAGE_PENDING;
    while(status != I2C3_MESSAGE_FAIL)
    {
        // Write 2 bytes (= Command code) to SHT31 address '0x44'
        I2C3_MasterWrite(writeCommand,2,SHT31_DeviceAddress,&status);
        // Wait for the message to be sent or status has changed
        while(status == I2C3_MESSAGE_PENDING)
        {
            if (StatusTimer(T_SHT31_WRITE_TIMEOUT) == T_ELAPSED)
            {
                slaveTimeOut = 1;
                StopTimer(T_SHT31_WRITE_TIMEOUT);
                break;
            } 
        } 
        if ((slaveTimeOut == 1) || (status == I2C3_MESSAGE_COMPLETE))
            break;

        // if status is  I2C3_MESSAGE_ADDRESS_NO_ACK,
        //               or I2C1_DATA_NO_ACK,
        // The device may be busy and needs more time for the last
        // write so we can retry writing the data, this is why we
        // use a while loop here

        // check for max retry and skip this byte
        if (timeOut == SLAVE_I2C_GENERIC_RETRY_MAX)
            break;
        else
            timeOut++;
    }
    StopTimer(T_SHT31_WRITE_TIMEOUT);

    // Write Command failed
    if (status != I2C3_MESSAGE_COMPLETE)
        return MEASUREMENT_ERROR;
    else
        return MEASUREMENT_OK;
}

/*
*//*=====================================================================*//**
**      @brief Read Command to SHT31 Sensor
**
**      @param uint8_t *res: pointer to received data buffer
**             uint8_t bytes_n: bytes number to be read         
**
**      @retval 'READ_ERROR'
**              'READ_OK'
**
*//*=====================================================================*//**
*/
uint8_t Read_I2C_Command (uint8_t *res, uint8_t bytes_n)
{
    uint16_t timeOut, slaveTimeOut;
    I2C3_MESSAGE_STATUS status;

    timeOut = 0;
    slaveTimeOut = 0;
    StopTimer(T_SHT31_WRITE_TIMEOUT);
    StartTimer(T_SHT31_WRITE_TIMEOUT);            
    status = I2C3_MESSAGE_PENDING;
    while(status != I2C3_MESSAGE_FAIL)
    {
        // Read SHT31 Temperature Humidity results
        I2C3_MasterRead(res,bytes_n,SHT31_DeviceAddress,&status);
        // Wait for the message to be sent or status has changed.
        while(status == I2C3_MESSAGE_PENDING)
        {
            if (StatusTimer(T_SHT31_WRITE_TIMEOUT) == T_ELAPSED)
            {
                slaveTimeOut = 1;
                StopTimer(T_SHT31_WRITE_TIMEOUT);
                break;
            } 
        }
        if ((slaveTimeOut == 1) || (status == I2C3_MESSAGE_COMPLETE))
            break;

        // if status is  I2C1_MESSAGE_ADDRESS_NO_ACK,
        //               or I2C1_DATA_NO_ACK,
        // The device may be busy and needs more time for the last
        // write so we can retry writing the data, this is why we
        // use a while loop here

        // check for max retry and skip this byte
        if (timeOut == SLAVE_I2C_GENERIC_RETRY_MAX)
            break;
        else
            timeOut++;
    }
    StopTimer(T_SHT31_WRITE_TIMEOUT);
    // Read Command failed
    if (status != I2C3_MESSAGE_COMPLETE)
        return READ_ERROR;
    else
        return READ_OK;
}

/*
*//*=====================================================================*//**
**      @brief Updates SHT31 Sensor Acquisition Status
**
**      @param void
**
**      @retval void
**
*//*=====================================================================*//**
*/
void I2C_Manager (void)
{
    static uint8_t Status_I2C = I2C_IDLE;
    static uint16_t SHT31_Humidity_tmp, SHT31_Temperature_tmp;
    //static float SHT31_Humidity_tmp, SHT31_Temperature_tmp;
    static uint16_t SHT31_Raw_Humidity, SHT31_Raw_Temperature;     
    static uint16_t SHT31_Status;
    uint8_t SHT31_Checksum_Temperature, SHT31_Checksum_Humidity, SHT31_Checksum_Status;
    uint8_t result;
    uint8_t writeBuffer[2];
    uint8_t Read_results[6];
    uint8_t crc;
    uint8_t byteCtr;
    uint8_t bits;

    switch (Status_I2C)
    {
        case I2C_IDLE:
            Read_results[0] = 0;
            Read_results[1] = 0;
            Read_results[2] = 0;
            Read_results[3] = 0;
            Read_results[4] = 0;
            Read_results[5] = 0;
            
            ResetProcedure(OFF);
            if (Start_New_Measurement == TRUE)
                Status_I2C = I2C_WRITE_SINGLE_SHOT_DATA_ACQUISITION;
        break;      
// -----------------------------------------------------------------------------
        // Write Temperature Humidity Command to SHT31
        case I2C_WRITE_SINGLE_SHOT_DATA_ACQUISITION:
            // Build the write buffer first: 'SINGLE SHOT DATA ACQUISITION' - "0x240B": Clock Stretching disabled + Medium Repeatability
            writeBuffer[0] = 0x24;          
            writeBuffer[1] = 0x0B;            
            result = Write_I2C_Command (writeBuffer);
            if (result == MEASUREMENT_OK)
            {
                StartTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_WAIT_READ_RESULTS;                        
            }    
            else
            {
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;                        
            }            
        break;

        // WAIT 8 msec before to Read results
        case I2C_WAIT_READ_RESULTS:
            if (StatusTimer(T_SHT31_MEASUREMENT) == T_ELAPSED)
            {
                StopTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_READ_RESULTS;                
            }
        break;

        // Read Temperature Humidity 
        case I2C_READ_RESULTS:
            result = Read_I2C_Command (Read_results, 6);            
            // Read Command failed
            if (result != MEASUREMENT_OK)
            {    
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;        
            }
            else
            {                    
                SHT31_Raw_Temperature = Read_results[0]*256 + Read_results[1]; 
//                Read_results[0] = 0xBE;
//                Read_results[1] = 0xEF;                
                SHT31_Checksum_Temperature = Read_results[2];
                // Verify Temperature Checksum
                // Calculates 8-Bit checksum with given polynomial
                crc = 0xFF;
                for (byteCtr = 0; byteCtr < 2; ++byteCtr)
                { 
                    crc ^= (Read_results[byteCtr]);
                    for (bits = 8; bits > 0; --bits)
                    { 
                        if (crc & 0x80) 
                            crc = (crc << 1) ^ POLYNOMIAL;
                        else 
                            crc = (crc << 1);
                    }
                }

                if (crc != SHT31_Checksum_Temperature) 
                {    
                    Sensor_Measurement_Error = TRUE;
                    // Set HARD RESET
                    Status_I2C = I2C_HARD_RESET;        
                }
                else
                {    
                    SHT31_Raw_Humidity = Read_results[3]*256 + Read_results[4]; 
                    SHT31_Checksum_Humidity = Read_results[5];                
                    // Verify Humidity Checksum
                    // Calculates 8-Bit checksum with given polynomial
                    crc = 0xFF;
                    for (byteCtr = 3; byteCtr < 5; ++byteCtr)
                    { 
                        crc ^= (Read_results[byteCtr]);
                        for (bits = 8; bits > 0; --bits)
                        { 
                            if (crc & 0x80) 
                                crc = (crc << 1) ^ POLYNOMIAL;
                            else 
                                crc = (crc << 1);
                        }
                    }
                    if (crc != SHT31_Checksum_Humidity) 
                    {    
                        Sensor_Measurement_Error = TRUE;
                        // Set HARD RESET
                        Status_I2C = I2C_HARD_RESET;        
                    }
                    else
                        Status_I2C = I2C_CALCULATE_HUMIDITY_TEMPERATURE;
                }                
            }            
        break;

        // Calculate Absolute values from Raw data
        case I2C_CALCULATE_HUMIDITY_TEMPERATURE:
            // RH Umidiy % x 10
            SHT31_Humidity_tmp = (((float)SHT31_Raw_Humidity / 65535)) * 1000;        
            // Temperature (°C) x 10)
            SHT31_Temperature_tmp = ((((float)SHT31_Raw_Temperature / 65535)* 175) - 45)*10; 
            
            Status_I2C = I2C_WRITE_STATUS_COMMAND;
        break;
// -----------------------------------------------------------------------------
        // Write Status Command to SHT31
        case I2C_WRITE_STATUS_COMMAND:
            // Build the write buffer first: 'READ OUT OF STATUS REGISTER' - "0xF32D"
            writeBuffer[0] = 0xF3;          
            writeBuffer[1] = 0x2D;            
            result = Write_I2C_Command (writeBuffer);
            if (result == MEASUREMENT_OK)
            {
                StartTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_WAIT_READ_STATUS;                        
            }    
            else
            {
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;                        
            }            
        break;

        // WAIT 8 msec before to Read Status
        case I2C_WAIT_READ_STATUS:
            if (StatusTimer(T_SHT31_MEASUREMENT) == T_ELAPSED)
            {
                StopTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_READ_STATUS;                
            }                
        break;

        // Read Sensor Status 
        case I2C_READ_STATUS:
            result = Read_I2C_Command (Read_results, 3);            
            // Read Command failed
            if (result != MEASUREMENT_OK)
            {    
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;        
            }
            else
            {                    
                SHT31_Status = Read_results[0]*256 + Read_results[1]; 
                SHT31_Checksum_Status = Read_results[2];
                // Last Command NOT Received (bit1) OR Checksum Error (bit0) --> Command Error
                if ( (SHT31_Status & 0x0003) != 0)
                {
                    Sensor_Measurement_Error = TRUE;
                    // Set HARD RESET
                    Status_I2C = I2C_HARD_RESET;        
                }
                else
                {    
                    // Correct Measurement and Status
                    // RH Umidiy % x 10
                    SHT31_Humidity = SHT31_Humidity_tmp;        
                    // Temperature (°C) x 10)
                    SHT31_Temperature = SHT31_Temperature_tmp;
                    Sensor_Measurement_Error = FALSE;
                    Start_New_Measurement = FALSE;
                    Status_I2C = I2C_IDLE;
//                    Status_I2C = I2C_WRITE_HEATER_ON;
                }
            }            
        break;
// -----------------------------------------------------------------------------       
        case I2C_HARD_RESET:
            // Wait Reset End            
            if (ResetProcedure(ON) == TRUE)
            { 
                Start_New_Measurement = FALSE;  
                Status_I2C = I2C_IDLE;
            }
        break;
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
        // Write HEATER ON Command to SHT31
        case I2C_WRITE_HEATER_ON:
            // Build the write buffer first: 'HEATER ON' - "0x306D"
            writeBuffer[0] = 0x30;          
            writeBuffer[1] = 0x6D;            
            result = Write_I2C_Command (writeBuffer);
            if (result == MEASUREMENT_OK)
            {
                StartTimer(T_SHT31_HEATER);
                Status_I2C = I2C_WAIT_HEATER_ON;                        
            }    
            else
            {
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;                        
            }            
        break;

        // WAIT 10" HEATER ON
        case I2C_WAIT_HEATER_ON:
            if (StatusTimer(T_SHT31_HEATER) == T_ELAPSED)
            {
                StopTimer(T_SHT31_HEATER);
                Status_I2C = I2C_WRITE_HEATER_STATUS_COMMAND;                
            }
        break;
        // Write Status Command to SHT31
        case I2C_WRITE_HEATER_STATUS_COMMAND:
            // Build the write buffer first: 'READ OUT OF STATUS REGISTER' - "0xF32D"
            writeBuffer[0] = 0xF3;          
            writeBuffer[1] = 0x2D;            
            result = Write_I2C_Command (writeBuffer);
            if (result == MEASUREMENT_OK)
            {
                StartTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_WAIT_READ_ON_STATUS;                        
            }    
            else
            {
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;                        
            }            
        break;

        // WAIT 8 msec before to Read Status
        case I2C_WAIT_READ_ON_STATUS:
            if (StatusTimer(T_SHT31_MEASUREMENT) == T_ELAPSED)
            {
                StopTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_READ_ON_STATUS;                
            }                
        break;

        // Read Sensor Status 
        case I2C_READ_ON_STATUS:
            result = Read_I2C_Command (Read_results, 3);            
            // Read Command failed
            if (result != MEASUREMENT_OK)
            {    
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;        
            }
            else
            {                    
                SHT31_Status = Read_results[0]*256 + Read_results[1]; 
                SHT31_Checksum_Status = Read_results[2];
                // Last Command NOT Received (bit1) OR Checksum Error (bit0) OR Heater OFF (bit 13) --> Command Error
                if ( ( (SHT31_Status & 0x0003) != 0) ||  ( (SHT31_Status & 0x2000) == 0) )              
                {
                    Sensor_Measurement_Error = TRUE;
                    // Set HARD RESET
                    Status_I2C = I2C_HARD_RESET;        
                }
                else
                {    
                    // HEATER is ON and WAIT time elapsed --> Send HEATER OFF
                    Sensor_Measurement_Error = FALSE;
                    Status_I2C = I2C_WRITE_HEATER_OFF;
                }
            }            
        break;
        
        // Write HEATER OFF Command to SHT31
        case I2C_WRITE_HEATER_OFF:
            // Build the write buffer first: 'HEATER ON' - "0x3066"
            writeBuffer[0] = 0x30;          
            writeBuffer[1] = 0x66;            
            result = Write_I2C_Command (writeBuffer);
            if (result == MEASUREMENT_OK)
            {
                StartTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_WAIT_READ_STATUS_OFF;                        
            }    
            else
            {
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;                        
            }            
        break;

        // WAIT 8 msec before to Read Status
        case I2C_WAIT_READ_STATUS_OFF:
            if (StatusTimer(T_SHT31_MEASUREMENT) == T_ELAPSED)
            {
                StopTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_WRITE_HEATER_OFF_STATUS_COMMAND;                
            }
        break;
        
        // Write Status Command to SHT31
        case I2C_WRITE_HEATER_OFF_STATUS_COMMAND:
            // Build the write buffer first: 'READ OUT OF STATUS REGISTER' - "0xF32D"
            writeBuffer[0] = 0xF3;          
            writeBuffer[1] = 0x2D;            
            result = Write_I2C_Command (writeBuffer);
            if (result == MEASUREMENT_OK)
            {
                StartTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_WAIT_READ_OFF_STATUS;                        
            }    
            else
            {
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;                        
            }            
        break;

        // WAIT 8 msec before to Read Status
        case I2C_WAIT_READ_OFF_STATUS:
            if (StatusTimer(T_SHT31_MEASUREMENT) == T_ELAPSED)
            {
                StopTimer(T_SHT31_MEASUREMENT);
                Status_I2C = I2C_READ_OFF_STATUS;                
            }                
        break;
            
        // Read Sensor Status 
        case I2C_READ_OFF_STATUS:
            result = Read_I2C_Command (Read_results, 3);            
            // Read Command failed
            if (result != MEASUREMENT_OK)
            {    
                Sensor_Measurement_Error = TRUE;
                // Set HARD RESET
                Status_I2C = I2C_HARD_RESET;        
            }
            else
            {                    
                SHT31_Status = Read_results[0]*256 + Read_results[1]; 
                SHT31_Checksum_Status = Read_results[2];
                // Last Command NOT Received (bit1) OR Checksum Error (bit0) OR Heater ON (bit 13) --> Command Error
                if ( (SHT31_Status & 0x2003) != 0)               
                {
                    Sensor_Measurement_Error = TRUE;
                    // Set HARD RESET
                    Status_I2C = I2C_HARD_RESET;        
                }
                else
                {    
                    // HEATER is ON and WAIT time elapsed --> Send HEATER OFF
                    Sensor_Measurement_Error = FALSE;
                    Status_I2C = I2C_IDLE;
                }
            }            
        break;
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
        default:
        break;    
    }        
}     

/*
*//*=====================================================================*//**
**      @brief SHT31 Hard reset Procedure 
**
**      @param unsigned chat 'type': ON  --> Hard Reset Activation
**                                   OFF --> StopHard Reset   
**
**      @retval unsigned int status: TRUE: Hard Reset Done or Stopped
**                                   WAIT: Hard Reset in Execution         
**
*//*=====================================================================*//**
*/
unsigned int ResetProcedure (unsigned char type)
{
    static unsigned int reset_procedure = RESET_ON;
    
    if (type == OFF)
    {
        StopTimer(T_HARD_RESET);
        RST_SHT31 = 1;
        reset_procedure = RESET_ON;
        return TRUE;
    }
    else 
    {
        switch (reset_procedure)
        {            
            case RESET_ON:
                StopTimer(T_HARD_RESET);
                StartTimer(T_HARD_RESET);
                RST_SHT31 = 0;
                reset_procedure = RESET_WAIT;
                return WAIT;
            break;
            case RESET_WAIT:
                if (StatusTimer(T_HARD_RESET) == T_ELAPSED)
                {            
                    RST_SHT31 = 1;
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

/**
 End of File
*/
