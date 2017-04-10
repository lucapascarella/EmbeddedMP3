/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        MP3.h
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

#ifndef MP3_H
#define	MP3_H

//#include <string.h>
//#include "Compiler.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"

#define STREAM_BUF_SIZE_PLAY            0x1000
#define STREAM_BUF_SIZE_RECORD          0x4000

#define STREAM_BUF_SIZE                 max(STREAM_BUF_SIZE_PLAY , STREAM_BUF_SIZE_RECORD)

#ifndef min
#define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

int Play(int argc, char **argv);
int Record(int argc, char **argv);

int Pause(int argc, char **argv);
int Stop(int argc, char **argv);
int Info(int argc, char **argv);
int FPlay(int argc, char **argv);
int SpeedShifter(int argc, char **argv);
int RateTune(int argc, char **argv);
int Semitone(int argc, char **argv);
int EarSpeaker(int argc, char **argv);

int Volume(int argc, char **argv);
int Treble(int argc, char **argv);
int Bass(int argc, char **argv);

int Playlist(int argc, char **argv);
int Bookmark(int argc, char **argv);

BOOL IsConvertible(char *);
long atoimm(char*, long, long, long);

void NoTrackInExecution(void);



#endif	/* MP3_H */

