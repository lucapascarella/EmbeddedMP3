/* 
 * File:   I2CSlave.h
 * Author: Luca
 *
 * Created on 4 febbraio 2014, 21.57
 */

#ifndef I2CSLAVE_H
#define	I2CSLAVE_H

// this is the modules Slave Address
#define SLAVE_ADDRESS 0x40

// I2C Buffer size
#define I2C_DATA_SIZE	    128

typedef struct {
    BYTE address;
    BYTE regLen;
    BOOL regFlag;

    CHAR rxBuf[I2C_DATA_SIZE];
    WORD rxHead;
    WORD rxTail;

    CHAR txBuf[I2C_DATA_SIZE];
    WORD txHead;
    WORD txTail;

} I2C_SPECIAL_BUFFER;

#define isI2CEnabled()      (config.gpio[GPIO_0_SCL].mode == GPIO_S_I2C && config.gpio[GPIO_1_SDA].mode == GPIO_S_I2C)

void InitI2C(void);
void I2CHandler(void);

WORD I2CWrite(CHAR8 *buffer, WORD count);
WORD I2CRead(CHAR8 *buffer, WORD count);

//BOOL isFull(void);
//int getBusy(void);
//BOOL isEmpty(void);
//void clear(void);
//int getCapacity(void);
//void dequeue(BYTE *byte);
//int peek(const int i, BYTE *byte);
//void enqueue(BYTE *byte);

#endif	/* I2CSLAVE_H */

