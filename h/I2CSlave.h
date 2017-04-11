/*
 * Copyright (C) 2017 LP Systems
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 * 
 * Author: Luca Pascarella www.lucapascarella.it
 */

#ifndef I2CSLAVE_H
#define	I2CSLAVE_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include "HardwareProfile.h"
#include <stdint.h>
#include <stdbool.h>

#define isI2CEnabled()      (config.gpio[GPIO_0_SCL].mode == GPIO_S_I2C && config.gpio[GPIO_1_SDA].mode == GPIO_S_I2C)

    // this is the modules Slave Address
#define SLAVE_ADDRESS 0x40

    // I2C Buffer size
#define I2C_DATA_SIZE	    128

    typedef struct {
        uint8_t address;
        uint8_t regLen;
        bool regFlag;

        uint8_t rxBuf[I2C_DATA_SIZE];
        uint16_t rxHead;
        uint16_t rxTail;

        uint8_t txBuf[I2C_DATA_SIZE];
        uint16_t txHead;
        uint16_t txTail;

    } I2C_SPECIAL_BUFFER;


    void InitI2C(void);
    void I2CHandler(void);

    uint16_t I2CWrite(uint8_t *buffer, uint16_t count);
    uint16_t I2CRead(uint8_t *buffer, uint16_t count);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif	/* I2CSLAVE_H */

