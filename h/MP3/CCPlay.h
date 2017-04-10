/* 
 * File:   CCPlay.h
 * Author: luca
 *
 * Created on April 10, 2017, 11:49 AM
 */

#ifndef CCPLAY_H
#define	CCPLAY_H

#include "HardwareProfile.h"
#include "FatFS/ff.h"
#include <stdint.h>
#include <stdbool.h>


typedef enum {
    MP3_CCPLAY_HOME = 0,

    MP3_CCPLAY_OPEN_PLAYLIST,
    MP3_CCPLAY_PL_OPENED_SUCCESSFUL,
    MP3_CCPLAY_PL_OPENED_FAILED,

    MP3_CCPLAY_OPEN_FILE,
    MP3_CCPLAY_OPENED_SUCCESSFUL,
    MP3_CCPLAY_OPENED_FAILED,

    MP3_CCPLAY_PL_GET_NEXT_TRACK,

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

} CCPLAY_STATE_MACHINE;

class CCPlay {
    
private:
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
    
    CCPLAY_STATE_MACHINE sm;              // Play state machine indicator
    BYTE filename[_MAX_LFN + 1];        // USB Filename
    
    union {
        DWORD allFlags;                         // 32 bits reserved for flags field
        struct __PACKED {
            // LSB
            DWORD boolConnectionState : 1;      // Web Radio connection state: True = Connected and False = Disconnected
            DWORD bConnIsLost : 1;              // Connection is lost, try reconnection
            // Expand here
            // MSB
        } bits;
    } flags;
    
public:
    CCPlay();
    CCPlay(const CCPlay& orig);
    virtual ~CCPlay();
private:

};

#endif	/* CCPLAY_H */

