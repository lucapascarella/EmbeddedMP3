/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Uart.h
 * Dependencies:    Compiler.h GenericTypeDefs.h HardwareProfile.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         LP Systems
 * Author:	    Luca Pascarella luca.pascarella@gmail.com
 * Web Site:        www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 LP Systems  All rights reserved.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 * File Description: Encoder and Decoder state finite machine
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release (1 September 2013, 16.00)
 *
 ********************************************************************/

#ifndef UART_H
#define	UART_H

#include <string.h>
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"

#define DISABLED        0
#define ENABLED         1

#define isUARTEnabled() (config.gpio[GPIO_4_RX].mode == GPIO_S_UART && config.gpio[GPIO_5_TX].mode == GPIO_S_UART)
#define UART_BUFFER_SIZE         1024

typedef struct __attribute__((__packed__)) {
    CHAR txBuf[UART_BUFFER_SIZE];
    WORD txLen;
    CHAR rxBuf[UART_BUFFER_SIZE];
    WORD rxHead;
    WORD rxTail;
    struct {
        BYTE dmaTXBusyFlag : 1; // DMA TX busy flag
    } flags;
} UART_CONFIG;

void UartInit(DWORD);
WORD UartWrite(CHAR8 *buffer, WORD count);
WORD UartWriteDirectly(CHAR8 *buffer, WORD count);
WORD UartRead(CHAR8 *buffer, WORD count);

#endif // UART_H
