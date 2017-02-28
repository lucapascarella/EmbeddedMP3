/*********************************************************************
 *
 *  MP3 MP3 frame Application Entry Point
 *
 *********************************************************************
 * FileName:        MP3Frame.h
 * Dependencies:
 * Processor:       PIC32MX795F512L
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         xxx
 * Author:	    Luca Pascarella www.lucapascarella.it
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
 * 
 ********************************************************************/

#ifndef MP3_FRAME_H
#define	MP3_FRAME_H

typedef union {
    //DWORD dword;

    struct __PACKED {
        BYTE LB;
        BYTE HB;
        BYTE UB;
        BYTE MB;
    } byte;

    // little endin correction

    struct {
        // First 8 bit
        BYTE sync1 : 8;

        // Second 8 bit
        BYTE protection : 1;
        BYTE layerDesc : 2;
        BYTE versionID : 2;
        BYTE sync2 : 3;

        // Third 8 bit
        BYTE reserved : 1;
        BYTE padding : 1;
        BYTE sampling : 2;
        BYTE bitrate : 4;

        // Fourth 8 bit
        BYTE emphasis : 2;
        BYTE original : 1;
        BYTE copyright : 1;
        BYTE modeExtension : 2;
        BYTE channelMode : 2;
    } frame;
} MP3_FRAME;

DWORD CalcFrameSize(BYTE *buffer, double *duration);
BOOL CheckFrameSyncBufferHead(BYTE *buffer);

#endif