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

#ifndef UART_H
#define	UART_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#include "HardwareProfile.h"
#include <stdint.h>
#include <stdbool.h>

#define DISABLED        0
#define ENABLED         1

#define isUARTEnabled() (config.gpio[GPIO_4_RX].mode == GPIO_S_UART && config.gpio[GPIO_5_TX].mode == GPIO_S_UART)
#define UART_BUFFER_SIZE         1024

    typedef struct __attribute__((__packed__)) {
        uint8_t txBuf[UART_BUFFER_SIZE];
        uint16_t txLen;
        uint8_t rxBuf[UART_BUFFER_SIZE];
        uint16_t rxHead;
        uint16_t rxTail;

        struct {
            uint8_t dmaTXBusyFlag : 1; // DMA TX busy flag
        } flags;
    }
    UART_CONFIG;

    void UartInit(void);
    uint16_t UartWrite(uint8_t *buffer, uint16_t count);
    uint16_t UartWriteDirectly(uint8_t *buffer, uint16_t count);
    uint16_t UartRead(uint8_t *buffer, uint16_t count);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif // UART_H
