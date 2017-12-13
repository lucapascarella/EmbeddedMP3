/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        MP3.c
 * Dependencies:    MP3.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         LP Systems
 * Author:			Luca Pascarella www.lucapascarella.it
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
 * 1.0   Initial release
 *
 ********************************************************************/

#define __MP3_C
#ifdef __MP3_C

#include "MP3/MP3.h"
#include "Compiler.h"
#include "Utilities/printer.h"
#include "MP3/VS1063.h"
#include <stdlib.h>
#include <math.h>

int Pause(int argc, char **argv) {

    BOOL play, rec;

    play = PausePlay(argc, argv);
    rec = PauseRecord(argc, argv);

    if (play == FALSE && rec == FALSE)
        NoTrackInExecution();
    return 0;
}

int StopOld(int argc, char **argv) {

    BOOL play, rec;

    play = StopPlay(argc, argv);
    rec = StopRecord(argc, argv);

    if (play == FALSE && rec == FALSE)
        NoTrackInExecution();
    return 0;
}

int Info(int argc, char **argv) {

    BOOL play, rec;
    //    SCI_MODE sci_mode;
    //    SCI_STATUS sci_status;
    //    SCI_CLOCK sci_clock;

    play = InfoPlay(argc, argv);
    rec = InfoRecord(argc, argv);

    //    if (argc == 2) {
    //	if (strcmp(argv[2], "-vs") == 0) {
    //	    sci_mode.word = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_6_66MHZ);
    //	    sci_status.word = VLSIReadReg(VLSI_ADD_STATUS, SPI_BRG_6_66MHZ);
    //	    sci_clock.word = VLSIReadReg(VLSI_ADD_CLOCKF, SPI_BRG_6_66MHZ);
    //	}
    //    }

    if (play == FALSE && rec == FALSE)
        NoTrackInExecution();
    return 0;
}

int Volume(int argc, char **argv) {

    int left, right, vol, balance;

    if (argc == 1) {
        left = VLSI_GetLeft();
        right = VLSI_GetRight();
        balance = VLSI_GetBalance();
        printf("Volume: -%.1fdB balance: %d%%\r\n", (float) (min(left, right) / 2.0), balance);
        printf("Left: %dpt\r\nRight: %dpt\r\n", left, right);
    } else if (argc == 2) {
        vol = atoimm(argv[1], 0, 255, 10);
        VLSI_SetVolume(vol, vol);
    } else if (argc == 3) {
        vol = atoimm(argv[1], 0, 255, 10);
        balance = atoimm(argv[2], -100, 100, 0);
        VLSI_SetBalance(vol, balance);
    } else {
        CliTooManyArgumnets(argv[1]);
    }
    return 0;
}

int Treble(int argc, char **argv) {

    int treble_db, treble_freq;

    treble_db = 0;
    treble_freq = 2;
    if (argc == 1) {
        printf("Treble: %.1fdB @ %dHz\r\n", VLSI_GetTreble() * 1.5, VLSI_GetTrebleFreq() * 1000);
    } else if (argc == 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 3) {
        treble_db = atoimm(argv[1], -8, 7, 0);
        treble_freq = atoimm(argv[2], 1, 15, 1);
        VLSI_SetTrebleBoost(treble_db, treble_freq);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Bass(int argc, char **argv) {

    int bass_db, bass_freq;

    if (argc == 1) {
        printf("Bass enhancement: %ddB @ %dHz.\r\n", VLSI_GetBass(), VLSI_GetBassFreq() * 10);
    } else if (argc == 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 3) {
        bass_db = atoimm(argv[1], 0, 15, 0);
        bass_freq = atoimm(argv[2], 2, 15, 2);
        VLSI_SetBassBoost(bass_db, bass_freq);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

BOOL IsConvertible(char *pstr) {

    int num;
    char *p = pstr;

    num = 0;
    while (*p != '\0') {
        if (isdigit(*p) || *p == '-')
            num++;
        p++;
    }

    if (num == strlen(pstr))
        return TRUE;
    return FALSE;
}

long atoimm(char *str, long min, long max, long def) {

    long tmp;
    //    int i;

    tmp = def;

    if (IsConvertible(str)) {
        tmp = atol(str);

        if (tmp < min)
            tmp = min;
        if (tmp > max)
            tmp = max;
    }

    //    i = atoi(str);

    return tmp;
}

void NoTrackInExecution() {
    printf("No track in execution\r\n");
}

#endif
