/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Record.h
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

#ifndef _RECORD_H
#define	_RECORD_H

#include <string.h>
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "FatFS/ff.h"
#include "Utilities/RTCC.h"




// Samplerate: 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100 and 48000 (Hz)
#define SAMPLERATE_8000     8000
#define SAMPLERATE_11025    11025
#define SAMPLERATE_12000    12000
#define SAMPLERATE_16000    16000
#define SAMPLERATE_22050    22050
#define SAMPLERATE_24000    24000
#define SAMPLERATE_32000    32000
#define SAMPLERATE_44100    44100
#define SAMPLERATE_48000    48000

// Bitrate: 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256 and 320 (kbit/s)
#define BITRATE_32          32
#define BITRATE_40          40
#define BITRATE_48          48
#define BITRATE_56          56
#define BITRATE_64          64
#define BITRATE_80          80
#define BITRATE_96          96
#define BITRATE_112         112
#define BITRATE_128         128
#define BITRATE_160         160
#define BITRATE_192         192
#define BITRATE_224         224
#define BITRATE_256         256
#define BITRATE_320         320

// Quality mode: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
#define QUALITY_0           0
#define QUALITY_1           1
#define QUALITY_2           2
#define QUALITY_3           3
#define QUALITY_4           4
#define QUALITY_5           5
#define QUALITY_6           6
#define QUALITY_7           7
#define QUALITY_8           8
#define QUALITY_9           9

// Encoding gain: from 1x to 63x or 0 for automatic gain control.
#define GAIN_0              0
#define GAIN_1              1
#define GAIN_2              2
#define GAIN_3              3
#define GAIN_4              4
#define GAIN_5              5
#define GAIN_6              6
#define GAIN_7              7
#define GAIN_8              8
#define GAIN_9              9
#define GAIN_10             10
#define GAIN_11             11
#define GAIN_12             12
#define GAIN_13             13
#define GAIN_14             14
#define GAIN_15             15
#define GAIN_16             16
#define GAIN_17             17
#define GAIN_18             18
#define GAIN_19             19
#define GAIN_20             20
#define GAIN_21             21
#define GAIN_22             22
#define GAIN_23             23
#define GAIN_24             24
#define GAIN_25             25
#define GAIN_26             26
#define GAIN_27             27
#define GAIN_28             28
#define GAIN_29             29
#define GAIN_30             30
#define GAIN_31             31
#define GAIN_32             32
#define GAIN_33             33
#define GAIN_34             34
#define GAIN_35             35
#define GAIN_36             36
#define GAIN_37             37
#define GAIN_38             38
#define GAIN_39             39
#define GAIN_40             40
#define GAIN_41             41
#define GAIN_42             42
#define GAIN_43             43
#define GAIN_44             44
#define GAIN_45             45
#define GAIN_46             46
#define GAIN_47             47
#define GAIN_48             48
#define GAIN_49             49
#define GAIN_50             50
#define GAIN_51             51
#define GAIN_52             52
#define GAIN_53             53
#define GAIN_54             54
#define GAIN_55             55
#define GAIN_56             56
#define GAIN_57             57
#define GAIN_58             58
#define GAIN_59             59
#define GAIN_60             60
#define GAIN_61             61
#define GAIN_62             62
#define GAIN_63             63

// Maximum autogain amplification: from 1x to 63x.
#define MAX_GAIN_0          0
#define MAX_GAIN_1          1
#define MAX_GAIN_2          2
#define MAX_GAIN_3          3
#define MAX_GAIN_4          4
#define MAX_GAIN_5          5
#define MAX_GAIN_6          6
#define MAX_GAIN_7          7
#define MAX_GAIN_8          8
#define MAX_GAIN_9          9
#define MAX_GAIN_10         10
#define MAX_GAIN_11         11
#define MAX_GAIN_12         12
#define MAX_GAIN_13         13
#define MAX_GAIN_14         14
#define MAX_GAIN_15         15
#define MAX_GAIN_16         16
#define MAX_GAIN_17         17
#define MAX_GAIN_18         18
#define MAX_GAIN_19         19
#define MAX_GAIN_20         20
#define MAX_GAIN_21         21
#define MAX_GAIN_22         22
#define MAX_GAIN_23         23
#define MAX_GAIN_24         24
#define MAX_GAIN_25         25
#define MAX_GAIN_26         26
#define MAX_GAIN_27         27
#define MAX_GAIN_28         28
#define MAX_GAIN_29         29
#define MAX_GAIN_30         30
#define MAX_GAIN_31         31
#define MAX_GAIN_32         32
#define MAX_GAIN_33         33
#define MAX_GAIN_34         34
#define MAX_GAIN_35         35
#define MAX_GAIN_36         36
#define MAX_GAIN_37         37
#define MAX_GAIN_38         38
#define MAX_GAIN_39         39
#define MAX_GAIN_40         40
#define MAX_GAIN_41         41
#define MAX_GAIN_42         42
#define MAX_GAIN_43         43
#define MAX_GAIN_44         44
#define MAX_GAIN_45         45
#define MAX_GAIN_46         46
#define MAX_GAIN_47         47
#define MAX_GAIN_48         48
#define MAX_GAIN_49         49
#define MAX_GAIN_50         50
#define MAX_GAIN_51         51
#define MAX_GAIN_52         52
#define MAX_GAIN_53         53
#define MAX_GAIN_54         54
#define MAX_GAIN_55         55
#define MAX_GAIN_56         56
#define MAX_GAIN_57         57
#define MAX_GAIN_58         58
#define MAX_GAIN_59         59
#define MAX_GAIN_60         60
#define MAX_GAIN_61         61
#define MAX_GAIN_62         62
#define MAX_GAIN_63         63

// kiloByte MegaByte GigaByte
#define x_KILOBYTE          1024
#define x_MEGABYTE          1024*1024
#define x_GIGABYTE          1024*1024*1024


#define INPUT_MIC           0
#define INPUT_LINE          1

// Bitrate mode: 0 = Quality Mode, 1 = VBR, 2 = ABR, 3 = CBR.
// ABR = VBR
#define QUALITY_MODE        0
#define VBR_MODE            1
#define ABR_MODE            2
#define CBR_MODE            3

// ADC mode: 0 = Joint stereo (common AGC), 1 = Dual channel (separate AGC), 2 = Left, 3 = Right, 4 = Mono
#define JOINT_STEREO        0
#define DUAL_CHANNEL        1
#define LEFT_CHANNEL        2
#define RIGHT_CHANNEL       3
#define MONO                4

#define REC_MP3             0
#define REC_OGG             1

#define MIN_DISABLED_REC    0          // 0 Disabled
#define MIN_INT_REC         15          // 15 Seconds
#define MID_INT_REC         60          // 1 Minute
#define MAX_INT_REC         1*60*60     // 1 Hour

#define BITRATE_MULTIPLIER_X10      0
#define BITRATE_MULTIPLIER_X100     1
#define BITRATE_MULTIPLIER_X1000    2
#define BITRATE_MULTIPLIER_X10000   3

// Bit reservoir
// Ogg Vorbis: 1 = limited frame length
// mp3: 1 = do not use bit-reservoir
#define BIT_RESERVOIR_ON            0
#define BIT_RESERVOIR_OFF           1

#define REC_IDLE            0

typedef enum {
    SM_REC_HOME = 0,

    SM_REC_OPEN_FILE,
    SM_REC_OPEN_NEXT_FILE,
    SM_REC_OPENED_SUCCESSFUL,
    SM_REC_OPENED_FAILED,

    SM_REC_READ_BUFFER,
    SM_REC_WRITE_BUFFER,

    SM_REC_PAUSE_WAIT_ENTERING,
    SM_REC_PAUSE_WAIT,
    SM_REC_PAUSE_DELAY_ENTERING,
    SM_REC_PAUSE_DELAY,
    SM_REC_PAUSE_EXIT,

    SM_REC_SEND_FINISH_RECORDING,
    SM_REC_FINISH_RECORDING,
    SM_REC_FINALIZE,

    SM_REC_CLOSE_FILE,
    SM_REC_CLOSED_SUCCESSFUL,
    SM_REC_CLOSED_FAILED,

    SM_REC_PUT_ERROR,

} REC_STATE_MACHINE;

typedef enum {
    SM_REC_SUB_CLOSE = 0,
    SM_REC_SUB_FS_STAT,
    SM_REC_SUB_OPEN_DIR,
    SM_REC_SUB_READ_DIR,
    SM_REC_SUB_DELETE_DIR,
    SM_REC_SUB_CLOSE_DIR,
    SM_REC_SUB_MKDIR,
    SM_REC_SUB_OPEN_FILE,
    SM_REC_SUB_DEFAULT,
} SM_REC_SUB;

typedef struct __attribute__((__packed__)) {

    REC_STATE_MACHINE sm;
    SM_REC_SUB smSub;
    DIR dir;
    FILINFO fno;
    FRESULT fres;
    DWORD tick_delay;
    DWORD tick_max;
    FIL * fp[2];
    TCHAR filename[FF_MAX_LFN + 1];
    FAT_TIME fat_time;
    BOOL ffind;
    UINT16 intervalRec;
    WORD read;
    WORD toWrite;
    DWORD alt;
    DWORD nextFrame;
    DWORD frameCount;
    DWORD fileLength;
    double longDuration;
    UINT16 year;
    BYTE fpIndex;
    BYTE mon;
    BYTE day;
    BYTE hour;
    BYTE mins;
    BYTE sec;
}
REC_CONFIG;


void RecordTaskInit(void);
int RecordTaskHandler(void);

int Record(int, char **);
BOOL PauseRecord(int, char **);
BOOL StopRecord(int, char **);
BOOL InfoRecord(int argc, char **argv);

void SetBitRate(long samplerate, int bitrate, int gain, int maxgain, BOOL input, char mode, char format, char adc, BOOL reservoir);


void TestRecInternalGet(void);
#endif // _RECORD_H