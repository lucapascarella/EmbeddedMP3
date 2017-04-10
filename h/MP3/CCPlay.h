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

#ifndef CCPLAY_H
#define	CCPLAY_H

#include "HardwareProfile.h"
#include "FatFS/ff.h"
#include <stdint.h>
#include <stdbool.h>

#define CCPLAY_BUF_SIZE_PLAY        1024u

typedef enum {
    MP3_CCPLAY_HOME = 0,

    MP3_CCPLAY_OPEN_PLAYLIST,
    MP3_CCPLAY_PL_GET_NEXT_TRACK,

    MP3_CCPLAY_OPEN_FILE,

    MP3_CCPLAY_READ_BUFFER,
    MP3_CCPLAY_WRITE_BUFFER,

    MP3_CCPLAY_PAUSE_WAIT_ENTERING,
    MP3_CCPLAY_PAUSE_WAIT,
    MP3_CCPLAY_PAUSE_DELAY_ENTERING,
    MP3_CCPLAY_PAUSE_DELAY,
    MP3_CCPLAY_PAUSE_EXIT,

    MP3_CCPLAY_FINISH_PLAING,
    MP3_CCPLAY_CLOSE_FILE,
    MP3_CCPLAY_CLOSED_SUCCESSFUL,
    MP3_CCPLAY_CLOSED_FAILED,

    MP3_CCPLAY_PL_NEXT,

    MP3_CCPLAY_FINISH

} CCPLAY_STATE_MACHINE;

class CCPlay {
private:
    uint16_t read;
    uint16_t write;

    uint8_t buffer[CCPLAY_BUF_SIZE_PLAY]; // Buffer pointer

    FIL *pfil; // File pointer
    FIL *pfilPlaylist; // Pointer for play-list file

    char filename[_MAX_LFN + 1]; // File name
    char playlistFilename[_MAX_LFN + 1]; // Play-list File name

    uint32_t tick_delay;
    uint32_t tick_max;
    long playlistNumber;

    CCPLAY_STATE_MACHINE sm; // Play state machine indicator

    union {
        uint32_t allFlags; // 32 bits reserved for flags field

        struct __PACKED {
            // LSB
            uint32_t boolConnectionState : 1; // Web Radio connection state: True = Connected and False = Disconnected
            uint32_t bConnIsLost : 1; // Connection is lost, try reconnection
            uint32_t playlistIndicator : 1; // True if playlist is correctly opened
            uint32_t playIndicator : 1; // True if playback
            uint32_t stopIndicator : 1; // True if stopped
            // Expand here
            // MSB
        } bits;
    } flags;

public:
    CCPlay(void);
    
    int playTaskHandler(void);
    void startPlayback(char *ptr);
    void stopPlayback(void);
    bool isPlaying(void);

    virtual ~CCPlay(void);
private:

};

#endif	/* CCPLAY_H */

