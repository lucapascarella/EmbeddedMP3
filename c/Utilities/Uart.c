/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Uart.c
 * Dependencies:    Uart.h
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

#include "Utilities/Uart.h"
#include "Utilities/Config.h"
#include <stdarg.h>
#include <stdio.h>

CIRCULAR_BUFFER tx, rx;

// UART 2 interrupt handler, it is set at priority level 5

void __ISR(_UART2_VECTOR, ipl5) IntUart2Handler(void) {

    if (INTGetFlag(INT_U2TX) && INTGetEnable(INT_U2TX)) { // transmit buffer empty
        while (UARTTransmitterIsReady(UART2) && tx.put != tx.get) {
            UARTSendDataByte(UART2, tx.buff[tx.get]);
            tx.get = (tx.get + 1) % SER_BUF_SIZE;
        }
        if (tx.put == tx.get)
            INTEnable(INT_U2TX, INT_DISABLED);
        INTClearFlag(INT_U2TX); // buffer is ready, clear interrupt flag
    }

    if (INTGetFlag(INT_U2RX) && INTGetEnable(INT_U2RX)) { // something in the receive buffer
        while (UARTReceivedDataIsAvailable(UART2) && ((rx.put + 1) % SER_BUF_SIZE) != rx.get) {
            rx.buff[rx.put] = UARTGetDataByte(UART2); // store bytes in buffer
            rx.put = ((rx.put + 1) % SER_BUF_SIZE);
        }
        INTClearFlag(INT_U2RX); // buffer is empty, clear interrupt flag
    }
}

WORD UartWrite(CHAR8 *buffer, WORD count) {

    int i;

    for (i = 0; i < count; i++) {
        // Wait so that the buffer has at least one empty position
        while (((tx.put + 1) % SER_BUF_SIZE) == tx.get) {
            Nop();
            if (!INTGetEnable(INT_U2TX))
                INTEnable(INT_U2TX, INT_ENABLED);
            else
                INTSetFlag(INT_U2TX);
        }
        tx.buff[tx.put] = buffer[i];
        tx.put = ((tx.put + 1) % SER_BUF_SIZE);

        if (i == (SER_BUF_SIZE - 1)) {
            if (!INTGetEnable(INT_U2TX))
                INTEnable(INT_U2TX, INT_ENABLED);
            else
                INTSetFlag(INT_U2TX);
        }
    }

    if (!INTGetEnable(INT_U2TX))
        INTEnable(INT_U2TX, INT_ENABLED);
    else
        INTSetFlag(INT_U2TX);

    return i;
}

WORD UartRead(CHAR8 *buffer, WORD count) {

    int i = 0;

    while (rx.put != rx.get && i < count) {
        buffer[i++] = rx.buff[rx.get];
        rx.get = ((rx.get + 1) % SER_BUF_SIZE);
    }

    return i;
}


//BOOL UartIsByteAviableInRX(void) {
//
//    if (rx.put != rx.get)
//        return TRUE;
//    else
//        return FALSE;
//}

void UartInit(DWORD BPS) {

    // Configure Serial Port
    UART_TX_TRIS = OUTPUT;
    UART_RX_TRIS = INPUT;

    //    // Disable the analog functionalities
    //    ANSELBbits.ANSB0 = 0; // RB0 --> TX
    //    ANSELBbits.ANSB1 = 0; // RB1 --> RX

    // Opens the UART 2 with the specified parameters
    UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART2, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART2, GetPeripheralClock(), BPS);
    UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    // Configure the interrupt priority, level 5
    INTSetVectorPriority(INT_UART_2_VECTOR, INT_PRIORITY_LEVEL_5); //configure UART2 interrupt
    //    INTSetVectorSubPriority(INT_VECTOR_UART(UART2), INT_SUB_PRIORITY_LEVEL_0);

    //don't enable U2TX interrupt until ready to transmit stuff
    INTClearFlag(INT_U2TX);
    tx.put = tx.get = 0;
    //    INTEnable(INT_U2TX, INT_ENABLED);

    INTClearFlag(INT_U2RX);
    rx.put = rx.get = 0;
    INTEnable(INT_U2RX, INT_ENABLED);
}