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

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "I2CSlave.h"
#include "Delay/Delay.h"
#include "MP3/MP3.h"
#include <plib.h>


#define Fsck        50000
#define BRG_VAL 	((GetPeripheralClock()/2/Fsck)-2)


I2C_SPECIAL_BUFFER i2c;

///////////////////////////////////////////////////////////////////
//
//	InitI2C
//
// 	Perform initialization of the I2C module to operate as a slave
//
///////////////////////////////////////////////////////////////////

void InitI2C(void) {

    // Enable the I2C module with clock stretching enabled
    OpenI2C1(I2C_ON | I2C_7BIT_ADD | I2C_STR_EN, BRG_VAL);

    // set the address of the slave module, address matching is with bits
    // 7:1 of the message compared with bits 6:0 of the ADD SFR so we
    // need to shift the desired address 1 bit.
    I2C1ADD = SLAVE_ADDRESS >> 1; // >> 1;
    I2C1MSK = 0x0000;

    // configure the interrupt priority for the I2C peripheral
    mI2C1SetIntPriority(I2C_INT_PRI_3 | I2C_INT_SLAVE);

    // clear pending interrupts and enable I2C interrupts
    mI2C1SClearIntFlag();
    EnableIntSI2C1;

    // Reset queue
    i2c.txHead = i2c.txTail = 0;
    i2c.rxHead = i2c.rxTail = 0;
    //i2c.rxIndex = 0;
}

///////////////////////////////////////////////////////////////////
//
// Slave I2C interrupt handler
// This handler is called when a qualifying I2C events occurs
// this means that as well as Slave events
// Master and Bus Collision events will also trigger this handler.
//
///////////////////////////////////////////////////////////////////

void __ISR(_I2C_1_VECTOR, IPL3AUTO) slaveI2CHandler(void) {
    //	mLED_1_On();
    volatile uint8_t temp;
    //static unsigned int dIndex;

    // check for MASTER and Bus events and respond accordingly
    if (mI2C1MGetIntFlag() == 1) {
        mI2C1MClearIntFlag();
        return;
    }
    if (mI2C1BGetIntFlag() == 1) {
        mI2C1BClearIntFlag();
        return;
    }

    // handle the incoming message
    if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 0)) {
        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 0 --> indicates last byte was address

        // reset any state variables needed by a message sequence
        // perform a dummy read of the address
        //        i2c.rxIndex = 0x00;
        //        i2c.uni.rxBuffer[i2c.rxIndex++] = SlaveReadI2C1();

        i2c.regFlag = true;
        i2c.address = SlaveReadI2C1();

        // release the clock to restart I2C
        I2C1CONbits.SCLREL = 1; // release the clock

    } else if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1)) {
        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 1 --> indicates last byte was data

        // writing data to our module, just store it in I2CData array
        // The first byte represents the total length
        // The second byte represents the command
        // The nth byte represents the optional data of the command
        //i2c.uni.rxBuffer[i2c.rxIndex++] = SlaveReadI2C1();
        if (i2c.regFlag) {
            i2c.regFlag = false;
            i2c.regLen = SlaveReadI2C1();
        } else {
            if (((i2c.rxTail + 1) & (I2C_DATA_SIZE - 1)) == i2c.rxHead) {
                // Discard the first byte in FIFO queue
                i2c.rxHead = (i2c.rxHead + 1) & (I2C_DATA_SIZE - 1);
            }
            i2c.rxBuf[i2c.rxTail] = SlaveReadI2C1();
            i2c.rxTail = (i2c.rxTail + 1) & (I2C_DATA_SIZE - 1);
        }
        // release the clock to restart I2C
        I2C1CONbits.SCLREL = 1; // release clock stretch bit

    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0)) {
        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 0 --> indicates last byte was address

        // read of the slave device, read the address
        i2c.address = SlaveReadI2C1();
        //I2COperation = I2C_OP_READ;
        //I2CIndexRead = I2C_index_reset;
        // Send immediately the length of the previous command, if 0 the response is not ready

        //if (i2c.uni.send.reg == 0xFD) {
        if (i2c.regLen == 0xFD) {
            // Get number of available bytes
            temp = ((i2c.txHead > i2c.txTail) ? I2C_DATA_SIZE : 0) + i2c.txTail - i2c.txHead;
        } else {
            if (i2c.txHead != i2c.txTail) {
                // If buffer is not empty take the first byte in it
                temp = i2c.txBuf[i2c.txHead];
                i2c.txHead = (i2c.txHead + 1) & (I2C_DATA_SIZE - 1);
            } else {
                // Buffer empty, the user asks more bytes than the availables
                temp = 0;
            }
        }
        SlaveWriteI2C1(temp);

    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 1)) {
        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 1 --> indicates last byte was data

        // output the data until the MASTER terminates the
        // transfer with a NACK, continuing reads return 0
        temp = 0;
        //if (i2c.uni.send.reg == 0xFF) {
        if (i2c.regLen == 0xFF) {
            if (i2c.txHead != i2c.txTail) {
                // If buffer is not empty take the first byte in it
                temp = i2c.txBuf[i2c.txHead];
                i2c.txHead = (i2c.txHead + 1) & (I2C_DATA_SIZE - 1);
            }
        }
        SlaveWriteI2C1(temp);
    }

    // finally clear the slave interrupt flag
    mI2C1SClearIntFlag();
}

// Example of commnad with Bus Pirate v4
// To configure tha Bus Pirate in I2C at 1MHz
// m -> 4 -> 2 -> 3
// To ask the current volume
// S Address Length Command P
// [0x40 0x01 "g"]
// Read the response
// S Address Length Byte1 Byte 2 P
// [0x41 r rr]

//void I2CHandler(void) {
//    Nop();
//}

uint16_t I2CWrite(uint8_t *buffer, uint16_t count) {

    int i;

    for (i = 0; i < count; i++) {
        if (((i2c.txTail + 1) & (I2C_DATA_SIZE - 1)) == i2c.txHead) {
            // Discard the first byte in FIFO queue
            i2c.txHead = (i2c.txHead + 1) & (I2C_DATA_SIZE - 1);
        }
        //        while (((i2c.tail + 1) & (I2C_DATA_SIZE - 1)) == i2c.head) {
        //            Nop();
        //        }
        i2c.txBuf[i2c.txTail] = buffer[i];
        i2c.txTail = (i2c.txTail + 1) & (I2C_DATA_SIZE - 1);
    }
    return i;
}

uint16_t I2CRead(uint8_t *buffer, uint16_t count) {

    int i = 0;

    while (i2c.rxHead != i2c.rxTail && i < count) {
        // If buffer is not empty take the first byte in it
        buffer[i++] = i2c.rxBuf[i2c.rxHead];
        i2c.rxHead = (i2c.rxHead + 1) & (I2C_DATA_SIZE - 1);
    }
    return i;
}