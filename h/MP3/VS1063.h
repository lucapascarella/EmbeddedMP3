/*********************************************************************
 *
 *  VS1063 Low Level Subroutines
 *
 *********************************************************************
 * FileName:        VS1063.h
 * Dependencies:    Compiler.h GenericTypeDefs.h HardwareProfile.h
 * Processor:       PIC32MX270F256B
 * Compiler:        Microchip XC32 v1.32 or higher
 * Company:         LP Systems
 * Author:          Luca Pascarella www.lucapascarella.it
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
 * File Description: Low level subroutine entry point to interact with VS1063
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/

#ifndef _VS1063_H
#define _VS1063_H

#include "HardwareProfile.h"
#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "MDD File System/Wrapper.h"



#define VLSI_STATUS_VER     0x00F0u
#define VER_VS1001          (0u<<4)
#define VER_VS1011          (1u<<4)
#define VER_VS1002          (2u<<4)
#define VER_VS1011E         (2u<<4)
#define VER_VS1003          (3u<<4)
#define VER_VS1053          (4u<<4)
#define VER_VS1033          (5u<<4)
#define VER_VS1063          (6u<<4)
#define VER_VS1103          (7u<<4)

#define VLSI_ADD_MODE           0x00
#define VLSI_ADD_STATUS         0x01
#define VLSI_ADD_BASS           0x02
#define VLSI_ADD_CLOCKF         0x03
#define VLSI_ADD_DECODE_TIME    0x04
#define VLSI_ADD_AUDATA         0x05
#define VLSI_ADD_WRAM           0x06
#define VLSI_ADD_WRAMADDR       0x07
#define VLSI_ADD_HDAT0          0x08
#define VLSI_ADD_HDAT1          0x09
#define VLSI_ADD_AIADDR         0x0A
#define VLSI_ADD_VOL            0x0B
#define VLSI_ADD_AICTRL0        0x0C
#define VLSI_ADD_AICTRL1        0x0D
#define VLSI_ADD_AICTRL2        0x0E
#define VLSI_ADD_AICTRL3        0x0F

#define VLSI_VAL_MAX_VOL        0x0000
#define VLSI_VAL_MIN_VOL        0xFEFE
#define VLSI_VAL_MUTE_VOL       0xFFFF



#define DREQ_IS_NOT_FULL        MP3_DREQ_I
#define DREQ_IS_FULL            !MP3_DREQ_I

#define SC_MULT_1063_10X 0x0000
#define SC_MULT_1063_20X 0x2000
#define SC_MULT_1063_25X 0x4000
#define SC_MULT_1063_30X 0x6000
#define SC_MULT_1063_35X 0x8000
#define SC_MULT_1063_40X 0xa000
#define SC_MULT_1063_45X 0xc000
#define SC_MULT_1063_50X 0xe000

#define SC_ADD_1063_00X 0x0000
#define SC_ADD_1063_10X 0x0800
#define SC_ADD_1063_15X 0x1000
#define SC_ADD_1063_20X 0x1800

// SC_MULT  15:13   Clock multiplier                // SC_MULT 0xA000 = 5 = x4.0
// SC_ADD   12:11   Allowed multiplier addition     // SC_ADD  0x1000 = 2 = x1.5
// SC_FREQ  10: 0   Clock frequency                 // SC_FREQ 0x0430 = 1072 = 0x0430 (12.288MHz)
// #define VLSI_VAL_CLOCKF_12MHz288       0xA000 | 0x1000 | 0x0430 // = 0xB430

// SC_MULT  15:13   Clock multiplier                // SC_MULT 0xC000 = 6 = x4.5
// SC_ADD   12:11   Allowed multiplier addition     // SC_ADD  0x0800 = 1 = x1.0
// SC_FREQ  10: 0   Clock frequency                 // SC_FREQ 0x0430 = 1072 = 0x0430 (12.288MHz)
//                                                  // 0xC000 | 0x0800 | 0x0430 = 0xCC30
#define VLSI_VAL_CLOCKF_12MHz288      0xC000 | 0x0800 | 0x0430
#define VLSI_VAL_MODE_DECODE          0x0802
#define VLSI_VAL_MODE_ENCODE_MIC      0x1802
#define VLSI_VAL_MODE_ENCODE_LINE     0x5802
#define VLSI_VAL_SM_RESET             0x0806
#define VLSI_VAL_SM_CANCEL            0x0008

#define VLSI_BIT_SM_ENCODE            0x1000


//#define VLSI_SINE_TEST                  1

// SCI_MODE is used to control the operation of VS1063a and defaults to 0x0800
typedef union {
    WORD word;

    struct __PACKED {
        BYTE SM_DIFF : 1;       //  bit 0
        BYTE SM_LAYER12 : 1;    //  bit 1
        BYTE SM_RESET : 1;      //  bit 2
        BYTE SM_CANCEL : 1;     //  bit 3
        BYTE none1 : 1;         //  bit 4
        BYTE SM_TESTS : 1;      //  bit 5
        BYTE none2 : 1;         //  bit 6
        BYTE none3 : 1;         //  bit 7
        BYTE SM_DACT : 1;       //  bit 8
        BYTE SM_SDIORD : 1;     //  bit 9
        BYTE SM_SDISHARE : 1;   //  bit 10
        BYTE SM_SDINEW : 1;     //  bit 11
        BYTE SM_ENCODE : 1;     //  bit 12
        BYTE none4 : 1;         //  bit 13
        BYTE SM_LINE1 : 1;      //  bit 14
        BYTE SM_CLK_RANGE : 1;  //  bit 15
    } bits;
} SCI_MODE;

// SCI_STATUS contains information on the current status of VS1063a.
typedef union {
    WORD word;

    struct __PACKED {
        BYTE SS_REFERENCE_SEL : 1;  //  bit 0
        BYTE SS_AD_CLOCK : 1;
        BYTE SS_APDOWN1 : 1;
        BYTE SS_APDOWN2 : 1;
        BYTE SS_VER : 4;
        BYTE none1 : 2;
        BYTE SS_VCM_DISABLE : 1;
        BYTE SS_VCM_OVERLOAD : 1;
        BYTE SS_SWING : 3;
        BYTE SS_DO_NOT_JUMP : 1;    //  bit 15
    } bits;
} SCI_STATUS;

// The external clock multiplier SCI register SCI_CLOCKF is presented in the table below.
typedef union {
    WORD word;

    struct __PACKED {
        WORD SC_FREQ : 11;
        BYTE SC_ADD : 2;
        BYTE SC_MULT : 3;
    } bits;
} SCI_CLOCK;

// Samplerate 8000. . . 48000 Hz (read at encoding startup)
typedef union {
    WORD word;

    struct __PACKED {
        WORD samplerate : 16;
    } bits;
} SCI_AICTRL0;

// Encoding gain (1024 = 1) or 0 for automatic gain control
typedef union {
    WORD word;

    struct __PACKED {
        WORD gain : 16;
    } bits;
} SCI_AICTRL1;

// Maximum autogain amplification (1024 = 1, 65535 = 64)
typedef union {
    WORD word;

    struct __PACKED {
        WORD maxAutogain : 16;
    } bits;
} SCI_AICTRL2;

// SCI_AICTRL3 in Encoding Control Mode
typedef union {
    WORD word;

    struct __PACKED {
        BYTE ADC_mode : 3;
        BYTE none1 : 1;
        BYTE EncodingFormat : 4;
        BYTE none2 : 2;
        BYTE NoRiffWAV : 1;
        BYTE RecPause : 1;
        BYTE none3 : 1;
        BYTE uartTx : 1;
        BYTE AEC : 1;
        BYTE CodecMode : 1;
    } bits;
} SCI_AICTRL3;

// SCI_WRAMADDR in Encoding Control Mode
typedef union {
    WORD word;

    struct __PACKED {
        WORD bitrateBase : 9;
        BYTE none1 : 1;
        BYTE MP3bitReservoir_OggLimitedFrameLength : 1;
        BYTE OggUseSerialNumber : 1;
        BYTE bitrateMultiplier : 2;
        BYTE bitrateMode : 2;
    } bits;
} SCI_WRAMADDR;

typedef struct {
    SCI_MODE sci_mode;
    SCI_AICTRL0 sci_aictrl0;
    SCI_AICTRL1 sci_aictrl1;
    SCI_AICTRL2 sci_aictrl2;
    SCI_AICTRL3 sci_aictrl3;
    SCI_WRAMADDR sci_wramaddr;
} _REC_PRESET;





typedef union {
    UINT32 Val;

    struct __PACKED {
        UINT16 HDAT0;
        UINT16 HDAT1;
    } word;

    struct __PACKED {
        UINT8 emphasis : 2;
        UINT8 original : 1;
        UINT8 copyright : 1;
        UINT8 extension : 2;
        UINT8 mode : 2;
        UINT8 private_bit : 1;
        UINT8 pad : 1;
        UINT8 samplerate : 2;
        UINT8 bitrate : 4;

        UINT16 protect_bit : 1;
        UINT16 layer : 2;
        UINT16 ID : 2;
        UINT16 syncword : 11;
    } bits;

} _MP3_INFO;

typedef union {
    UINT16 Val;

    struct __PACKED {
        UINT16 AICTRL3;
    } word;

    struct __PACKED {
        UINT8 ADCMode : 3;
        UINT8 reserved1 : 1;
        UINT8 EncodingForma : 4;
        UINT8 reserved2 : 2;
        UINT8 header : 1;
        UINT8 pause : 1;
        UINT8 reserved3 : 1;
        UINT8 uart : 1;
        UINT8 AEC : 1;
        UINT8 codec : 1;
    } bits;

} _MP3_RECORD;

#define PARAMETRIC_VERSION 0x0004

typedef union {
    // 42 + 22 words
    WORD w[64];

    struct __PACKED {
        // 42 words
        UINT32 chipID; // 0x1e00/01 Initialized at reset for your convenience
        UINT16 version; // 0x1e02 - structure version
        UINT16 config1; // 0x1e03 wamf ---C ppss RRRR
        INT16 playSpeed; // 0x1e04 0,1 = normal speed, 2 = twice, etc.
        UINT16 bitRatePer100; // 0x1e05 average bitrate divided by 100
        UINT16 endFillByte; // 0x1e06 which byte value to send after file
        UINT16 rateTune[2]; // 0x1e07..8 samplerate tune in +-1ppm steps. V4
        UINT16 playMode; // 0x1e09 play and processing enables V4
        INT32 sampleCounter; // 0x1e0a..b sample counter. V4
        UINT16 vuMeter; // 0x1e0c VU meter result V4
        UINT16 adMixerGain; // 0x1e0d AD mixer attenuation in 3dB steps -3..-31
        UINT16 adMixerConfig; // 0x1e0e AD mixer config, bits 5-4=rate, 7-6=mode
        UINT16 pcmMixerRate; // 0x1e0f PCM mixer samplerate (read when enabled)
        UINT16 pcmMixerFree; // 0x1e10 PCM mixer FIFO free state
        UINT16 pcmMixerVol; // 0x1e11 PCM mixer volume 0..191 (-0.5dB steps)
        UINT16 eq5Params[10]; // 0x1e12..0x1e1b 5-channel EQ parameters
        UINT16 eq5Updated; // 0x1e1c write as non-zero to recalculate filters.
        UINT16 speedShifter; // 0x1e1d Speed Shifter speed 0x4000 == 1.0x V4
        UINT16 earSpeakerLevel; // 0x1e1e EarSpeaker level, 0 = off. V4
        UINT16 sdiFree; // 0x1e1f SDI FIFO free in words. V4
        UINT16 audioFill; // 0x1e20 Audio buffer fill in stereo samples. V4
        UINT16 reserved[4]; // 0x1e21..24
        UINT16 latestSOF[2]; // 0x1e25/1e26 latest start of frame V4
        UINT16 positionMsec[2]; // 0x1e27-28 play position if known. V3
        INT16 resync; // 0x1e29 > 0 for automatic m4a, ADIF, WMA resyncs

        // 22 words

        union { /* 22 available -- these are not cleared at software reset! */

            struct {
                INT16 txUartDiv; // 1e2a direct set of UART divider
                INT16 txUartByteSpeed; // 1e2b set UART byte speed (txUartDiv=0)
                UINT16 txPauseGpio; // 1e2c mask: a high level pauses tx
                INT16 aecAdaptMultiplier; // 2 for default
                INT16 reserved[14];
                UINT16 channelMax[2]; // 1e3c,1e3d for record level monitoring
                UINT32 serialNumber; // 1e3e,1e3f for Ogg Vorbis if enabled in WRAMADDR(11)
            } encoding;

            struct {
                UINT32 curPacketSize;
                UINT32 packetSize;
            } wma; // 4

            struct {
                UINT16 sceFoundMask; // 1e2a single-channel-el. found since last clr
                UINT16 cpeFoundMask; // 1e2b channel-pair-el. found since last clr
                UINT16 lfeFoundMask; // 1e2c low-frequency-el. found since last clr
                UINT16 playSelect; // 1e2d 0 = first any, initialized at aac init
                INT16 dynCompress; // 1e2e -8192=1.0, initialized at aac init
                INT16 dynBoost; // 1e2f 8192=1.0, initialized at aac init
                // playSelect: 0 = first sce or cpe or lfe
                // xxxx0001 first sce xxxx0010 first cpe
                // xxxx0011 first lfe eeee0101 sce eeee
                // eeee0110 cpe eeee eeee0111 lfe eeee
                UINT16 sbrAndPsStatus; // 0x1e30 V3 gotSBR/upsampling/gotPS/PSactive
                UINT16 sbrPsFlags; // 0x1e31 V4
            } aac; // 3

            struct {
                INT16 gain; // 0x1e2a proposed gain offset, default = -12
            } vorbis;
        } i;
    } word;
} _EXTRA_PAR;


#define PLAYMODE_SPEEDSHIFTER_ON    0b01000000
#define PLAYMODE_EQ5_ON             0b00100000
#define PLAYMODE_PCMMIXER_ON        0b00010000
#define PLAYMODE_ADMIXER_ON         0b00001000
#define PLAYMODE_VUMETER_ON         0b00000100
#define PLAYMODE_PAUSE_ON           0b00000010
#define PLAYMODE_MONO_OUTPUT_ON     0b00000001

#define PLAYMODE_SPEEDSHIFTER_OFF   0b01111111
#define PLAYMODE_EQ5_OFF            0b10111111
#define PLAYMODE_PCMMIXER_OFF       0b11011111
#define PLAYMODE_ADMIXER_OFF        0b11110111
#define PLAYMODE_VUMETER_OFF        0b11111011
#define PLAYMODE_PAUSE_OFF          0b11111101
#define PLAYMODE_MONO_OUTPUT_OFF    0b11111110







void InitVLSI(void);
void VLSI_SoftReset(void);

WORD VLSIReadReg(BYTE vAddress, WORD brg);
void VLSIWriteReg(BYTE vAddress, WORD wValue, WORD brg);
WORD VLSIPutArray(BYTE*, int);
UINT VLSIGetArray(BYTE*, UINT, UINT);

void VLSI_FinishPlaying(void);
void VLSI_SendFinishRecording(void);

void VLSI_InitRecording(_REC_PRESET *recPreSet);
void VLSI_GetRecordingInfo(_REC_PRESET *recPreSet);
void VLSI_SetBitRecMode(void);
void VLSI_ClearBitRecMode(void);
WORD VLSI_GetPlayRecMode();





void VLSI_SetBassBoost(BYTE bass, BYTE gfreq);
void VLSI_SetTrebleBoost(CHAR treble, BYTE gfreq);
void VLSI_SetVolume(SHORT left, SHORT right);
void VLSI_SetBalance(BYTE volume, CHAR balance);

BYTE VLSI_GetBass(void);
BYTE VLSI_GetBassFreq(void);
CHAR VLSI_GetTreble(void);
BYTE VLSI_GetTrebleFreq(void);

BYTE VLSI_GetLeft(void);
BYTE VLSI_GetRight(void);
CHAR VLSI_GetBalance(void);




void VLSI_SetFastSpeed(WORD speed);
WORD VLSI_GetFastSpeed(void);
void VLSI_SetBitPlayMode(BYTE mode);
void VLSI_ClearBitPlayMode(BYTE mode);
WORD VLSI_GetPlayMode();
BOOL VLSI_GetEndFillByte();
BOOL VLSI_IsClearedSmCancel();


void VLSI_GetHDAT();

const char * VLSI_GetBitrateOthersFormat(WORD *bitrate);
WORD VLSI_GetBitrateMP3Format(void);
WORD VLSI_GetSamplerate(void);
const char* VLSI_GetLayer(void);





void VLSI_GetExtra();

#endif // _VS1063_H
