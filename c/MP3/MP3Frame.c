/*********************************************************************
 *
 *  MP3Frame Low Level Subroutines
 *
 *********************************************************************
 * FileName:        MP3Frame.c
 * Dependencies:    MP3Frame.h
 * Processor:       PIC32MX795F512L
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         xxx
 * Author:          Luca Pascarella www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 All rights reserved.
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
 * File Description:
 * Low level subroutine entry point
 *
 * Change History:
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/

#include "HardwareProfile.h"
#include "Compiler.h"
#include "MP3/MP3Frame.h"

// sampling rates in hertz: 1. index = MPEG Version ID, 2. index = sampling rate index
const DWORD m_dwSamplingRates[4][3] = {
    {11025, 12000, 8000,}, // MPEG 2.5
    {0, 0, 0,}, // reserved
    {22050, 24000, 16000,}, // MPEG 2
    {44100, 48000, 32000} // MPEG 1
};

// bitrates: 1. index = LSF, 2. index = Layer, 3. index = bitrate index
const DWORD m_dwBitrates[2][3][15] = {
    { // MPEG 1
        {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448,}, // Layer1
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384,}, // Layer2
        {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320,} // Layer3
    },
    { // MPEG 2, 2.5
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256,}, // Layer1
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160,}, // Layer2
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160,} // Layer3
    }
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
const DWORD m_dwSamplesPerFrames[2][3] = {
    { // MPEG 1
        384, // Layer1
        1152, // Layer2
        1152 // Layer3
    },
    { // MPEG 2, 2.5
        384, // Layer1
        1152, // Layer2
        576 // Layer3
    }
};

// allowed combination of bitrate (1.index) and mono (2.index)
const BOOL m_bAllowedModes[15][2] = {
    // {stereo, intensity stereo, dual channel allowed,single channel allowed}
    {TRUE, TRUE}, // free mode
    {FALSE, TRUE}, // 32
    {FALSE, TRUE}, // 48
    {FALSE, TRUE}, // 56
    {TRUE, TRUE}, // 64
    {FALSE, TRUE}, // 80
    {TRUE, TRUE}, // 96
    {TRUE, TRUE}, // 112
    {TRUE, TRUE}, // 128
    {TRUE, TRUE}, // 160
    {TRUE, TRUE}, // 192
    {TRUE, FALSE}, // 224
    {TRUE, FALSE}, // 256
    {TRUE, FALSE}, // 320
    {TRUE, FALSE} // 384
};

// Samples per Frame / 8
const DWORD m_dwCoefficients[2][3] = {
    { // MPEG 1
        12, // Layer1	(must be multiplied with 4, because of slot size)
        144, // Layer2
        144 // Layer3
    },
    { // MPEG 2, 2.5
        12, // Layer1	(must be multiplied with 4, because of slot size)
        144, // Layer2
        72 // Layer3
    }
};

// slot size per layer
const DWORD m_dwSlotSizes[3] = {
    4, // Layer1
    1, // Layer2
    1 // Layer3
};

DWORD CalcFrameSize(BYTE *buffer, double *duration) {

    BYTE bBitrateIndex;
    BYTE bSamplingIndex;

    BOOL m_bLSF; // true means lower sampling frequencies (=MPEG2/MPEG2.5)
    BOOL m_bCRC;

    DWORD m_dwBitrate; // in bit per second (1 kb = 1000 bit, not 1024)
    DWORD m_dwSamplesPerSec;
    DWORD m_dwPaddingSize;
    DWORD m_dwSamplesPerFrame;
    DWORD frameSize;
    DWORD tmp;
    static MP3_FRAME *frame; // Canont be reg variable due to alignment access limitaion of MIPS MCU

    enum MPAVersion {
        MPEG25 = 0,
        MPEGReserved,
        MPEG2,
        MPEG1
    } m_Version;

    enum MPALayer {
        Layer1,
        Layer2,
        Layer3,
        LayerReserved
    } m_Layer;

    frame = (MP3_FRAME*) buffer;

    // MP3 Frame sync check
    if (frame->frame.sync1 != 0xFF && frame->frame.sync2 != 0x07)
        return 0; // Frame sync error

    // MPEG Audio version ID check
    // 00 - MPEG Version 2.5
    // 01 - reserved
    // 10 - MPEG Version 2 (ISO/IEC 13818-3)
    // 11 - MPEG Version 1 (ISO/IEC 11172-3)
    if (frame->frame.versionID == 0x01)
        return 0; // versionID error

    // Layer description check
    // 00 - reserved
    // 01 - Layer III
    // 10 - Layer II
    // 11 - Layer I
    if (frame->frame.layerDesc == 0x00)
        return 0; // description error

    // Bitrate index check
    if (frame->frame.bitrate == 0x0F)
        return 0; // bitrate error

    // Sampling rate frequency index
    // bits   MPEG1    MPEG2    MPEG2.5
    //  00    44100    22050    11025
    //  01    48000    24000    12000
    //  10    32000    16000    8000
    //  11    reserv.  reserv.  reserv.
    if (frame->frame.sampling == 0x03)
        return 0; // sampling error

    // Get MPEG1 or MPEG2.x version
    m_Version = frame->frame.versionID;
    m_bLSF = (m_Version == MPEG1) ? FALSE : TRUE;


    // Get layer (0 = layer1, 2 = layer2, ...)  [bit 13,14]
    m_Layer = 3 - frame->frame.layerDesc;

    // protection bit (inverted) [bit 15]
    //m_bCRC = !((pHeader[1]) & 0x01);
    //m_bCRC = frame->frame.protection;

    // bitrate [bit 16..19]
    bBitrateIndex = frame->frame.bitrate;
    m_dwBitrate = m_dwBitrates[m_bLSF][m_Layer][bBitrateIndex] * 1000; // convert from kbit to bit


    // sampling rate [bit 20,21]
    bSamplingIndex = frame->frame.sampling;
    m_dwSamplesPerSec = m_dwSamplingRates[m_Version][bSamplingIndex];

    // padding bit [bit 22]
    m_dwPaddingSize = frame->frame.padding; // in Slots (always 1)

    //m_dwSamplesPerFrame = m_dwSamplesPerFrames[m_bLSF][m_Layer];

    // avoid trap diviede by zero
    if (m_dwSamplesPerSec == 0 || m_dwBitrate == 0)
        return 0;

    tmp = (m_dwCoefficients[m_bLSF][m_Layer] * m_dwBitrate / m_dwSamplesPerSec) + m_dwPaddingSize;
    frameSize = tmp * m_dwSlotSizes[m_Layer];

    tmp = frameSize * 8;
    *duration = (double) ((double) tmp / (double) m_dwBitrate);
    return frameSize;
}

BOOL CheckFrameSyncBufferHead(BYTE *buffer) {

    static MP3_FRAME *frame; // Canont be reg variable due to alignment access limitaion of MIPS MCU
    frame = (MP3_FRAME*) buffer;

    // MP3 Frame sync check
    if (frame->frame.sync1 != 0xFF && frame->frame.sync2 != 0x07)
        return FALSE; // Frame sync error

    // MPEG Audio version ID check
    // 00 - MPEG Version 2.5
    // 01 - reserved
    // 10 - MPEG Version 2 (ISO/IEC 13818-3)
    // 11 - MPEG Version 1 (ISO/IEC 11172-3)
    if (frame->frame.versionID == 0x01)
        return FALSE; // versionID error

    // Layer description check
    // 00 - reserved
    // 01 - Layer III
    // 10 - Layer II
    // 11 - Layer I
    if (frame->frame.layerDesc == 0x00)
        return FALSE; // description error

    // Bitrate index check
    if (frame->frame.bitrate == 0x0F)
        return FALSE; // bitrate error

    // Sampling rate frequency index
    // bits   MPEG1    MPEG2    MPEG2.5
    //  00    44100    22050    11025
    //  01    48000    24000    12000
    //  10    32000    16000    8000
    //  11    reserv.  reserv.  reserv.
    if (frame->frame.sampling == 0x03)
        return FALSE; // sampling error

    return TRUE;
}