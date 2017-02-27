/***********************************************************************
 * PIC32 I2C Slave Code
 ***********************************************************************/

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "I2CSlave.h"
#include "Delay/Delay.h"
#include "MP3/MP3.h"
#include <plib.h>


#define Fsck        50000
#define BRG_VAL 	((GetPeripheralClock()/2/Fsck)-2)

// this is the modules Slave Address
#define SLAVE_ADDRESS 0x40

//
#define I2C_DATA_SIZE	    64
BYTE I2CIndexWrite, I2CIndexRead, I2CData[I2C_DATA_SIZE], *I2CLength, *I2CCommand;
I2C_OPERATION I2COperation = I2C_OP_NONE;

///////////////////////////////////////////////////////////////////
//
//	InitI2C
//
// 	Perform initialisation of the I2C module to operate as a slave
//
///////////////////////////////////////////////////////////////////

void InitI2C(void) {

    // Assign the length and the commands to the respective pointers
    I2CLength = &I2CData[0];
    I2CCommand = &I2CData[1];

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
}

///////////////////////////////////////////////////////////////////
//
// Slave I2C interrupt handler
// This handler is called when a qualifying I2C events occurs
// this means that as well as Slave events
// Master and Bus Collision events will also trigger this handler.
//
///////////////////////////////////////////////////////////////////

void __ISR(_I2C_1_VECTOR, ipl3) _SlaveI2CHandler(void) {
    //	mLED_1_On();
    BYTE temp;
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
        temp = SlaveReadI2C1();

        // Reset address indicators
        I2CIndexWrite = 0x00;

        // release the clock to restart I2C
        I2C1CONbits.SCLREL = 1; // release the clock

    } else if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1)) {
        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 1 --> indicates last byte was data

        // writing data to our module, just store it in I2CData array
        // The first byte represents the total length
        // The second byte represents the command
        // The nth byte represents the optional data of the command
        I2CData[I2CIndexWrite++] = SlaveReadI2C1();
        I2COperation = I2C_OP_WRITE;

        // release the clock to restart I2C
        I2C1CONbits.SCLREL = 1; // release clock stretch bit

    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0)) {
        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 0 --> indicates last byte was address

        // read of the slave device, read the address
        temp = SlaveReadI2C1();
        I2COperation = I2C_OP_READ;
        I2CIndexRead = I2C_index_reset;
        // Send immediately the length of the previous command, if 0 the response is not ready
        SlaveWriteI2C1(*I2CLength);

    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 1)) {
        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 1 --> indicates last byte was data

        // output the data until the MASTER terminates the
        // transfer with a NACK, continuing reads return 0

        //if (I2CIndex >= sizeof (I2CData))
        //    I2CIndex = 0;
        SlaveWriteI2C1(I2CData[I2CIndexRead++]);
    }

    // finally clear the slave interrupt flag
    mI2C1SClearIntFlag();

    //handlerFlag = TRUE;
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

void I2CHandler(void) {

#define MAX_TOKEN 32
    int index, vol, length;
    char *argv[MAX_TOKEN];
    ;

    if (I2COperation == I2C_OP_WRITE) {
        // Check if a Write operation is been asked
        if (*I2CLength == I2CIndexWrite || I2C1STATbits.P) {
            // The command can be start if the expected total length is reached or if the stop bit is been sent
            length = *I2CLength;
            *I2CLength = I2C_operation_in_progress;
            index = I2C_index_reset;
            switch (*I2CCommand) {
                case 'g':
                case 'G':
                    // Get current volume levels
                    // Example: [0x40 0x02 "g"]
                    // Response1: [0x41 r
                    // Response2: rr]
                    I2CData[index++] = VLSI_GetLeft();
                    I2CData[index] = VLSI_GetRight();
                    *I2CLength = index;
                    break;

                case 'u':
                case 'U':
                    // Up volume
                    // Example: [0x40 0x02 "u"]
                    vol = min(VLSI_GetLeft(), VLSI_GetRight());
                    if (vol-- > 0)
                        VLSI_SetVolume(vol, vol);
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01;
                    *I2CLength = 0x01;
                    break;

                case 'd':
                case 'D':
                    // Down volume
                    vol = min(VLSI_GetLeft(), VLSI_GetRight());
                    if (vol++ < 255)
                        VLSI_SetVolume(vol, vol);
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01;
                    *I2CLength = 0x01;
                    break;

                case 'v':
                case 'V':
                    // volume insert
                    // Example: [0x40 0x04 "v" 0x0A 0x0A]
                    VLSI_SetVolume(I2CData[index + 1], I2CData[index + 2]);
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01;
                    *I2CLength = 0x01;
                    break;

                case 's':
                case 'S':
                    // Start the execution of a track
                    // Example: [0x40 0x0B "S" "r" "e" "b" "e" "l" "." "m" "p" "3"]
                    // Response: [0x41 r r]
                    I2CData[length + 1] = '\0';
                    argv[1] = &I2CData[index + 1];
                    Play(2, argv);
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01;
                    *I2CLength = 0x01;
                    break;

                case 'k':
                case 'K':
                    // Kill
                    // Example: [0x40 0x02 "k"]
                    Stop(1, NULL);
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01; // Response with 1 byte
                    *I2CLength = 0x01; // 0x01 -> OK
                    break;

                case 'l':
                case 'L':
                    // Start a new playlist
                    // Example: [0x40 0x0e "l" "playlist.pls"]
                    if (length == 2) {
                        Playlist(1, NULL);
                    } else {
                        I2CData[length + 1] = '\0';
                        argv[1] = &I2CData[index + 1];
                        Playlist(2, argv);
                    }
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01;
                    *I2CLength = 0x01;
                    break;

                case 'r':
                case 'R':
                    // Recording
                    if (length == 2) {
                        Record(1, NULL);
                    } else {
                        I2CData[length + 1] = '\0';
                        argv[1] = &I2CData[index + 1];
                        Record(2, argv);
                    }
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01;
                    *I2CLength = 0x01;
                    break;

                case 'p':
                case 'P':
                    // Pause
                    // Modificare p in modo da poter prendere anche il tempo
                    // se p# allora toggle
                    // se p100# pausa per 100ms
                    Pause(1, NULL);
                    // If user asks to receive the response, will send 1 byte with indication of operation completed successful
                    I2CData[index] = 0x01;
                    *I2CLength = 0x01;
                    break;
            }
            I2COperation = I2C_OP_NONE;
        }
    } else if (I2COperation == I2C_OP_READ) {
        // Check if a Read operation is been asked
        if (*I2CLength == I2CIndexRead || I2C1STATbits.P) {
            // Resets the length indicator after that a response is sent
            *I2CLength = I2C_operation_in_progress;
            I2COperation = I2C_OP_NONE;
        }
    }

}