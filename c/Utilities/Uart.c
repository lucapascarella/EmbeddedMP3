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

#include "Utilities/Uart.h"
#include "Utilities/Config.h"
#include <stdarg.h>
#include <stdio.h>

UART_CONFIG uart;

// UART 2 interrupt handler, it is set at priority level 5

void __ISR(_UART2_VECTOR, IPL5AUTO) IntUart2Handler(void) {

    //if (INTGetFlag(INT_U2RX) && INTGetEnable(INT_U2RX)) { // something in the receive buffer
    while (UARTReceivedDataIsAvailable(UART2)) {
        if (((uart.rxTail + 1) & (UART_BUFFER_SIZE - 1)) == uart.rxHead) {
            // Discard the first byte in FIFO queue
            uart.rxHead = (uart.rxHead + 1) & (UART_BUFFER_SIZE - 1);
        }
        uart.rxBuf[uart.rxTail] = UARTGetDataByte(UART2); // store bytes in buffer
        uart.rxTail = (uart.rxTail + 1) & (UART_BUFFER_SIZE - 1);
    }
    INTClearFlag(INT_U2RX); // buffer is empty, clear interrupt flag
    //}
}

//void __ISR(_DMA2_VECTOR, IPL5AUTO) DmaHandler2(void) {
//    int evFlags; // event flags when getting the interrupt
//
//    INTClearFlag(INT_SOURCE_DMA(DMA_CHANNEL2)); // release the interrupt in the INT controller, we're servicing int
//
//    evFlags = DmaChnGetEvFlags(DMA_CHANNEL2); // get the event flags
//
//    if (evFlags & DMA_EV_BLOCK_DONE) { // just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
//        uart.flags.dmaTXBusyFlag = FALSE;
//        DmaChnClrEvFlags(DMA_CHANNEL2, DMA_EV_BLOCK_DONE);
//    }
//}

//// true - if queue if full
//
//inline BOOL isFull(void) {
//    return ( ((uart.rxTail + 1) & (UART_BUFFER_SIZE - 1)) == uart.rxHead);
//}
//
//// Count elements
//
//inline int getBusy(void) {
//    return ((uart.rxHead > uart.rxTail) ? UART_BUFFER_SIZE : 0) +uart.rxTail - uart.rxHead;
//}
//
//// true - if queue if empty
//
//inline BOOL isEmpty(void) {
//    return (uart.rxHead == uart.rxTail);
//}
//
//inline void clear(void) {
//    uart.rxHead = uart.rxTail = 0;
//}
//
//inline int getCapacity(void) {
//    return (UART_BUFFER_SIZE - 1);
//}
//
//// Retrieve the item from the queue
//
//int dequeue(BYTE *byte) {
//    if (isEmpty()) {
//        return 0;
//    }
//
//    *byte = uart.rxBuf[uart.rxHead];
//    uart.rxHead = (uart.rxHead + 1) & (UART_BUFFER_SIZE - 1);
//}
//
//// Get i-element with not delete
//
//int peek(const int i, BYTE *byte) {
//    int j = 0;
//    int k = uart.rxHead;
//    while (k != uart.rxTail) {
//        if (j == i)
//            break;
//        j++;
//        k = (k + 1) & (UART_BUFFER_SIZE - 1);
//    }
//    if (k == uart.rxTail)
//        return;
//    *byte = uart.rxBuf[k];
//}
//
//int enqueue(BYTE *byte) {
//    if (isFull()) {
//        return 0;
//    }
//
//    uart.rxBuf[uart.rxTail] = *byte;
//    uart.rxTail = (uart.rxTail + 1) & (UART_BUFFER_SIZE - 1);
//}

uint16_t UartWrite(uint8_t *buffer, uint16_t count) {

    while (UART_TX_DMA_WORKING());

    UART_TX_DMA_CLR_BTC();
    //memcpy(uart.txBuf, buffer, count);
    DmaChnSetTxfer(UART_TX_DMA_CHANNEL, buffer, uart.txBuf, count, count, count);
    DmaChnStartTxfer(UART_TX_DMA_CHANNEL, DMA_WAIT_NOT, 0);
    while (UART_TX_DMA_WORKING());

    UART_TX_DMA_CLR_BTC();
    uart.txLen = count;
    DmaChnSetTxfer(UART_TX_DMA_CHANNEL, uart.txBuf, (void*) &U2TXREG, uart.txLen, 1, 1);
    DmaChnStartTxfer(UART_TX_DMA_CHANNEL, DMA_WAIT_NOT, 0);
    return count;
}

uint16_t UartWriteDirectly(uint8_t *buffer, uint16_t count) {

    if (UART_TX_DMA_WORKING())
        return 0;
    UART_TX_DMA_CLR_BTC();
    DmaChnSetTxfer(UART_TX_DMA_CHANNEL, buffer, (void*) &U2TXREG, count, 1, 1);
    DmaChnStartTxfer(UART_TX_DMA_CHANNEL, DMA_WAIT_NOT, 0);
    return count;
}

uint16_t UartRead(uint8_t *buffer, uint16_t count) {
    int i = 0;
    while (uart.rxHead != uart.rxTail && i < count) {
        // If buffer is not empty take the first byte in it
        buffer[i++] = uart.rxBuf[uart.rxHead];
        uart.rxHead = (uart.rxHead + 1) & (UART_BUFFER_SIZE - 1);
    }
    return i;
}

void UartInit(void) {

    // Configure Serial Port
    UART_TX_TRIS = OUTPUT;
    UART_RX_TRIS = INPUT;

    //    // Disable the analog functionalities
    //    ANSELBbits.ANSB0 = 0; // RB0 --> TX
    //    ANSELBbits.ANSB1 = 0; // RB1 --> RX

    // Opens the UART 2 with the specified parameters
    UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED);
    UARTSetFifoMode(UART2, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART2, GetPeripheralClock(), config.console.baudrate);
    UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    // Configure the interrupt priority, level 5
    INTSetVectorPriority(INT_UART_2_VECTOR, INT_PRIORITY_LEVEL_5); //configure UART2 interrupt
    INTSetVectorSubPriority(INT_UART_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);

    //don't enable U2TX interrupt until ready to transmit stuff
    INTClearFlag(INT_U2TX);
    //INTEnable(INT_U2TX, INT_ENABLED);

    INTClearFlag(INT_U2RX);
    //uart.rxPut = uart.rxGet = 0;
    uart.rxHead = uart.rxTail = 0;
    INTEnable(INT_U2RX, INT_ENABLED);

    // Configure the DMA Channel 2 assigned to TX UART 4
    DmaChnOpen(UART_TX_DMA_CHANNEL, DMA_CHN_PRI2, DMA_OPEN_DEFAULT);
    // set the events: now the start event is the UART tx being empty
    // we maintain the pattern match mode
    DmaChnSetEventControl(UART_TX_DMA_CHANNEL, DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(_UART2_TX_IRQ));
    //    // set the transfer source and dest addresses, source and dest size and cell size
    DmaChnSetTxfer(UART_TX_DMA_CHANNEL, uart.txBuf, (void*) &U2TXREG, NULL, 1, 1);
    // enable the transfer done interrupt: all the characters transferred
    //DmaChnSetEvEnableFlags(UART_DMA_CHANNEL, DMA_EV_BLOCK_DONE);
    // enable system wide multi vectored interrupts
    //    INTSetVectorPriority(INT_VECTOR_DMA(UART_DMA_CHANNEL), INT_PRIORITY_LEVEL_5); // set INT controller priority
    //    INTSetVectorSubPriority(INT_VECTOR_DMA(UART_DMA_CHANNEL), INT_SUB_PRIORITY_LEVEL_3); // set INT controller sub-priority
    // enable the chn interrupt in the INT controller
    //INTEnable(INT_SOURCE_DMA(UART_DMA_CHANNEL), INT_ENABLED);
    // Reset busy flag indicator
    UART_TX_DMA_SET_BTC();
}