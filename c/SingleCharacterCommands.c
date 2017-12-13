/*********************************************************************
 *
 *  Single Character Commands
 *
 *********************************************************************
 * FileName:        SingleCharacterCommands.c
 * Dependencies:    SingleCharacterCommands.h
 *                  HardwareProfile.h
 *                  Compiler.h
 *                  GenericTypeDefs.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.21 or higher
 * Company:         LP Systems
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
 * This file contains the routines to do a command execution
 *
 * Change History:
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/

#include "Utilities/Uart.h"
#include "Delay/Delay.h"
#include "Utilities/Config.h"
#include "Utilities/printer.h"
#include "Utilities/GPIO.h"
#include "FatFS/ff.h"
#include "Commands.h"
#include "SingleCharacterCommands.h"
#include "CommandLineInterpreter.h"
#include "MP3/Play.h"
#include "MP3/VS1063.h"

BYTE SCCmode;
extern _command_line cl;

BOOL InitSCC(BYTE mode) {
    SCCmode = mode;
}

void SCCHandler() {

#define MAX_TOKEN 32

    static BOOL number = FALSE;
    static char command;
    char c, *argv[MAX_TOKEN];
    int vol;

    if (CliGetCharFromConsole(&c)) {
        if (SCCmode == SCC_ECHO_MODE)
            putc(c);

        // Reset all previous commands
        if (c == '\n')
            number = FALSE;

        if (!number)
            command = c;

        switch (command) {

                // b = bass
                // t = treble
                // f = fast play
                // t = rate tune
                // e = reboot (soft rate)
                // m = bookmark

            case 'k':
            case 'K':
                // Kill
                //Stop(1, NULL);
                break;

            case 's':
            case 'S':
                // Start a new song
                if (number) {
                    if (c != '#') {
                        cl.cmd[cl.cmdi++] = c;
                    } else {
                        cl.cmd[cl.cmdi] = '\0';
                        argv[1] = cl.cmd;
                        Play(2, argv);
                        number = FALSE;
                    }
                } else {
                    cl.cmdi = 0;
                    cl.cmd[cl.cmdi] = '\0';
                    number = TRUE;
                }
                break;

            case 'l':
            case 'L':
                // Start a new playlist
                if (number) {
                    if (c != '#') {
                        cl.cmd[cl.cmdi++] = c;
                    } else {
                        if (cl.cmdi == 0) {
                            Playlist(1, NULL);
                        } else {
                            cl.cmd[cl.cmdi] = '\0';
                            argv[1] = cl.cmd;
                            Playlist(2, argv);
                        }
                        number = FALSE;
                    }
                } else {
                    cl.cmdi = 0;
                    cl.cmd[cl.cmdi] = '\0';
                    number = TRUE;
                }
                break;

            case 'r':
            case 'R':
                // Recording
                if (number) {
                    if (c != '#') {
                        cl.cmd[cl.cmdi++] = c;
                    } else {
                        cl.cmd[cl.cmdi] = '\0';
                        argv[1] = cl.cmd;
                        Record(2, argv);
                        number = FALSE;
                    }
                } else {
                    cl.cmdi = 0;
                    number = TRUE;
                }
                break;

            case 'p':
            case 'P':
                // Pause
                // Modificare p in modo da poter prendere anche il tempo
                // se p# allora toggle
                // se p100# pausa per 100ms
                Pause(1, NULL);
                break;


            case 'u':
            case 'U':
                // Up volume
                vol = min(VLSI_GetLeft(), VLSI_GetRight());
                if (vol-- > 0)
                    VLSI_SetVolume(vol, vol);
                break;

            case 'd':
            case 'D':
                // Down volume
                vol = min(VLSI_GetLeft(), VLSI_GetRight());
                if (vol++ < 255)
                    VLSI_SetVolume(vol, vol);
                break;

            case 'v':
            case 'V':
                // volume insert
                if (number) {
                    if (c != '#') {
                        cl.cmd[cl.cmdi++] = c;
                    } else {
                        cl.cmd[cl.cmdi] = '\0';
                        vol = atoimm(cl.cmd, 0, 255, 10);
                        VLSI_SetVolume(vol, vol);
                        number = FALSE;
                    }
                } else {
                    cl.cmdi = 0;
                    number = TRUE;
                }
                break;

            case 'g':
            case 'G':
                // Get volume
                printf("%d", min(VLSI_GetLeft(), VLSI_GetRight()));
                //Volume();
                break;

        }
    }
}
