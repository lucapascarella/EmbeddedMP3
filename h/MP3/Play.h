/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Play.h
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

#ifndef PLAY_H
#define	PLAY_H

#include <string.h>
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "FatFS/ff.h"

#include <stdint.h>
#include <stdbool.h>


#define PLAY_IDLE               0

typedef enum {
    MP3_PLAY_HOME = 0,

    MP3_PLAY_OPEN_PLAYLIST,
    MP3_PLAY_PL_OPENED_SUCCESSFUL,
    MP3_PLAY_PL_OPENED_FAILED,

    MP3_PLAY_OPEN_FILE,
    MP3_PLAY_OPENED_SUCCESSFUL,
    MP3_PLAY_OPENED_FAILED,

    MP3_PLAY_PL_GET_NEXT_TRACK,

    MP3_PLAY_READ_BUFFER,
    MP3_PLAY_WRITE_BUFFER,

    MP3_PLAY_PAUSE_WAIT_ENTERING,
    MP3_PLAY_PAUSE_WAIT,
    MP3_PLAY_PAUSE_DELAY_ENTERING,
    MP3_PLAY_PAUSE_DELAY,
    MP3_PLAY_PAUSE_EXIT,

    MP3_PLAY_FINISH_PLAING,
    MP3_PLAY_CLOSE_FILE,
    MP3_PLAY_CLOSED_SUCCESSFUL,
    MP3_PLAY_CLOSED_FAILED,

    MP3_PLAY_PL_NEXT,
            
} PLAY_STATE_MACHINE;

typedef struct __attribute__((__packed__)) {

    DWORD reconnectionDelayTick;        // Reconnection delay multiply by TICK_SECOND
    DWORD reconnectionDelay;            // Reconnection delay
    DWORD connectionTimeoutTick;        // Connection timeout multiply by TICK_SECOND
    DWORD connectionTimeout;            // Connection timeout
    
    UINT read;
    UINT write;
    UINT offset;
    DWORD timeout;                      // Timeout indicator
    DWORD tled;                         // Timeout led indicator
    
    BYTE *buffer;                       // Buffer pointer
    FIL *fil;                           // File pointer
    
    PLAY_STATE_MACHINE sm;              // Play state machine indicator
    BYTE filename[_MAX_LFN + 1];        // USB Filename
    
    union {
        DWORD allFlags;                         // 32 bits reserved for flags field
        struct __PACKED {
            // LSB
            DWORD boolConnectionState : 1;      // Web Radio connection state: True = Connected and False = Disconnected
            DWORD bConnIsLost : 1;              // Connection is lost, try ricconection
            // Expand here
            // MSB
        } bits;
    } flags;
    
} PLAY_CONFIG;

void PlayTaskInit(void);
int PlayTaskHandler(void);

void startPlay(char *ptr);
bool isPlaying(void);

int Play(int, char **);
BOOL PausePlay(int, char **);
BOOL StopPlay(int, char **);
BOOL InfoPlay(int, char **);

#endif // RECORD_H