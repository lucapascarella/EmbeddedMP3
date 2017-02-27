/* 
 * File:   I2CSlave.h
 * Author: Luca
 *
 * Created on 4 febbraio 2014, 21.57
 */

#ifndef I2CSLAVE_H
#define	I2CSLAVE_H


typedef enum {
    I2C_OP_NONE = 0,
    I2C_OP_READ,
    I2C_OP_WRITE
} I2C_OPERATION;

#define I2C_index_reset             0x01
#define I2C_operation_in_progress   0x00

#define isI2CEnabled()      (config.gpio[GPIO_0_SCL].mode == GPIO_S_I2C && config.gpio[GPIO_1_SDA].mode == GPIO_S_I2C)

void InitI2C(void);
void I2CHandler(void);


#endif	/* I2CSLAVE_H */

