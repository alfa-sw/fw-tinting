#ifndef SPI3_H
#define	SPI3_H

typedef enum
{
    SPI_IDLE,
    SPI_WRITE_MULTIPLE_BYTE_TRANSFER,
    SPI_WAIT_READ_RESULTS,
    SPI_READ_RESULTS,
    SPI_CALCULATE_TEMPERATURE,
    SPI_HARD_RESET    
} SPI3_ACQUISITION_STATUS;

extern void SPI3_Initialize(void);
extern void SPI3_Manager(void);
extern unsigned int TemperatureResetProcedure (unsigned char type);
extern void Write_SPI3_Command( uint8_t *pTransmitData);
extern void Read_SPI3_Command(uint8_t *pTransmitData, uint8_t *pReceiveData);

#endif	/* SPI3_H */

