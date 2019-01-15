/* 
 * File:   spi.h
 * Author: mballerini
 *
 * Created on 24 ottobre 2018, 17.13
 */

#ifndef SPI_H
#define	SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

    /*===== INCLUSIONI ========================================================= */
/*===== DICHIARAZIONI LOCALI ================================================*/
/*===== DEFINE GENERICHE ====================================================*/

#define SPIM_PIC24
#define SPIM_BLOCKING_FUNCTION
#define SPIM_PPRE (unsigned)0
#define SPIM_SPRE (unsigned)0

    //periferiche SPI
enum
{
  SPI_1,
  SPI_2,
  SPI_3
};

/*
 * Error and Status Flags
 * SPIM_STS_WRITE_COLLISION indicates that, Write collision has occurred
 * while trying to transmit the byte.
 *
 * SPIM_STS_TRANSMIT_NOT_OVER indicates that, the transmission is
 * not yet over. This is to be checked only when non Blocking
 * option is opted.
 *
 * SPIM_STS_DATA_NOT_READY indicates that reception SPI buffer is empty
 * and there's no data avalable yet.
 *
*/

/* Commnted out here as they're defined as part of an enum in include/flash.h */
/* #define SPIM_STS_WRITE_COLLISION    1 */
/* #define SPIM_STS_TRANSMIT_NOT_OVER  2   */
/* #define SPIM_STS_DATA_NOT_READY     3   */

#define SPI_FALSE FALSE
#define SPI_TRUE  TRUE

#define BUFF_SIZE 4

typedef struct
{
  unsigned char buffer[4];
  unsigned char length;
  unsigned char index;

} Spi_buffer_t;

//static Spi_buffer_t txBufferSPI, rxBufferSPI;


extern void spi_remapping(unsigned char);
extern void spi_init(unsigned char);
extern void spi_test(unsigned char);
extern void SpiSendByte(uint8_t byte);
extern uint8_t SpiRecvByte(void);


void SpiSendData(Spi_buffer_t * txBuffer);
void SpiReadData(Spi_buffer_t * rxBuffer);
void initSpiBuffer(Spi_buffer_t * buffer);

/*extern unsigned char spi_put_byte(unsigned char, unsigned char);
extern unsigned char spi_get_byte(unsigned char);
extern unsigned char spi_is_tx_over(unsigned char);
*/


#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

