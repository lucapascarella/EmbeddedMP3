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


#include "MP3/Play.h"
#include "MP3/MP3.h"
#include "Delay/Tick.h"
#include "Commands.h"
#include "MP3/VS1063.h"

#include "Utilities/printer.h"
#include "Utilities/GPIO.h"

#include "Utilities/Config.h"
#include <math.h>

PLAY_CONFIG play;

void PlayTaskInit(void) {
    play.sm = MP3_PLAY_HOME;
    memset(play.filename, sizeof (play.filename), '\0');
}

BOOL playIndicator, playlistIndicator, stopIndicator;
DWORD tick_delay, tick_max;

int PlayTaskHandler(void) {

    static char stream[4096];
    extern FIL fstream2, ftmp2;
    static FIL *fplaylist = &ftmp2;
    FRESULT fres;
    static UINT read, write;
    static WORD playlistNumber = 0;
    static DWORD t = 0;

    switch (play.sm) {

        case MP3_PLAY_HOME:
            // Do nothing, wait a play command
            Nop();
            break;



        case MP3_PLAY_OPEN_PLAYLIST:
            verbosePrintf(VER_DBG, "Try to open playlist: %s", play.filename);
            // Open a playlist in read mode
            fres = f_open(fplaylist, play.filename, FA_READ);
            if (fres != FR_OK)
                play.sm = MP3_PLAY_PL_OPENED_FAILED;
            else
                play.sm = MP3_PLAY_PL_OPENED_SUCCESSFUL;
            break;

        case MP3_PLAY_PL_OPENED_SUCCESSFUL:
            verbosePrintf(VER_DBG, "Playlist opened successful");
            playlistNumber = 0;
            playlistIndicator = TRUE;
            play.sm = MP3_PLAY_PL_GET_NEXT_TRACK;
            break;

        case MP3_PLAY_PL_OPENED_FAILED:
            // Signal and handler the error
            verbosePrintf(VER_MIN, "Playlist: %s not found", play.filename);
            GpioUpdateOutputState(GPIO_BIT_FILE_NOT_FOUND);
            FlashLight(100, 10, FALSE);
            // Return to idle state
            play.sm = MP3_PLAY_HOME;
            break;


        case MP3_PLAY_PL_GET_NEXT_TRACK:

            if (playlistIndicator && f_gets(play.filename, _MAX_LFN, fplaylist) != NULL) {
                playlistNumber++;
                verbosePrintf(VER_DBG, "Playlist execute: %s", play.filename);
                // Goto next stage
                play.sm = MP3_PLAY_OPEN_FILE;
            } else {
                playlistNumber = 0;
                playlistIndicator = FALSE;
                verbosePrintf(VER_DBG, "Playlist ended");
                if (f_close(fplaylist) != FR_OK)
                    FlashLight(100, 10, FALSE);
                play.sm = MP3_PLAY_HOME;
            }
            break;



        case MP3_PLAY_OPEN_FILE:
            verbosePrintf(VER_DBG, "Try to open: %s", play.filename);
            // Open a file in read mode
            //fp = fopen(fileName, FS_READ);
            fres = f_open(&fstream2, play.filename, FA_READ);
            if (fres != FR_OK)
                play.sm = MP3_PLAY_OPENED_FAILED;
            else
                play.sm = MP3_PLAY_OPENED_SUCCESSFUL;
            break;



        case MP3_PLAY_OPENED_SUCCESSFUL:
            verbosePrintf(VER_DBG, "File opened successful");
            // Initialize the decoder
            // Todo spostare questa funzione in VLSI
            VLSIWriteReg(VLSI_ADD_MODE, VLSI_VAL_MODE_DECODE, SPI_BRG_12_5MHZ);
            // Initialize the buffer counter and the plaing indicator
            read = write = 0;
            playIndicator = TRUE;
            stopIndicator = FALSE;

            GpioUpdateOutputState(GPIO_BIT_STARTS_PLAY);

            // Goto next stage
            play.sm = MP3_PLAY_READ_BUFFER;
            break;



        case MP3_PLAY_OPENED_FAILED:
            // Signal and handler the error
            verbosePrintf(VER_MIN, "File: %s not found", play.filename);
            GpioUpdateOutputState(GPIO_BIT_FILE_NOT_FOUND);
            FlashLight(100, 10, FALSE);
            // Return to idle state
            play.sm = MP3_PLAY_HOME;
            break;



        case MP3_PLAY_READ_BUFFER:
            // Reads buffer bytes and trasfer to VS1063 decoder
            LED_BLUE_ON();
            //read = fread(stream, sizeof (char), STREAM_BUF_SIZE_PLAY, fp);
            fres = f_read(&fstream2, stream, STREAM_BUF_SIZE_PLAY, &read);
            LEDs_OFF();

            if (read <= 0) {
                play.sm = MP3_PLAY_FINISH_PLAING;
            } else {
                play.sm = MP3_PLAY_WRITE_BUFFER;
            }
            break;



        case MP3_PLAY_WRITE_BUFFER:
            // Write a small quantities of data in the decoder buffer
            write += VLSIPutArray(&stream[write], read - write);
            if (write == read) {
                write = 0;
                play.sm = MP3_PLAY_READ_BUFFER;
            }
            break;




        case MP3_PLAY_PAUSE_WAIT_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_PLAY);
            // Put the decoder in pause mode
            VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
            play.sm = MP3_PLAY_PAUSE_WAIT;
            break;

        case MP3_PLAY_PAUSE_WAIT:
            // Wait until pause is resent
            Toggle1Second();
            break;


        case MP3_PLAY_PAUSE_DELAY_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_PLAY);
            // Put the decoder in pause mode
            VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
            play.sm = MP3_PLAY_PAUSE_DELAY;
            break;

        case MP3_PLAY_PAUSE_DELAY:
            if (TickGet() - tick_delay >= tick_max)
                play.sm = MP3_PLAY_PAUSE_EXIT;
            Toggle1Second();
            break;

        case MP3_PLAY_PAUSE_EXIT:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_PLAY);
            VLSI_ClearBitPlayMode(PLAYMODE_PAUSE_OFF);
            play.sm = MP3_PLAY_WRITE_BUFFER;
            break;




        case MP3_PLAY_FINISH_PLAING:

            GpioUpdateOutputState(GPIO_BIT_STOP_PLAY);
            verbosePrintf(VER_DBG, "Finish playing");
            // Finish to plaing the current song
            VLSI_FinishPlaying();
            playIndicator = FALSE;
            play.sm = MP3_PLAY_CLOSE_FILE;
            break;



        case MP3_PLAY_CLOSE_FILE:
            verbosePrintf(VER_DBG, "Try to close file");
            // Close the current file pointer
            if (f_close(&fstream2) != FR_OK)
                play.sm = MP3_PLAY_CLOSED_FAILED;
            else
                play.sm = MP3_PLAY_CLOSED_SUCCESSFUL;
            break;



        case MP3_PLAY_CLOSED_SUCCESSFUL:
            verbosePrintf(VER_DBG, "File closed successful");
            // Goto idle state machine
            play.sm = MP3_PLAY_PL_NEXT;
            break;



        case MP3_PLAY_CLOSED_FAILED:
            // Signal and handler the error
            verbosePrintf(VER_DBG, "File: %s not closed", play.filename);
            FlashLight(100, 100, FALSE);
            // Goto idle state machine
            play.sm = MP3_PLAY_PL_NEXT;
            break;


        case MP3_PLAY_PL_NEXT:
            // Manage the next track in execution into the playlist
            if (playlistNumber) {
                // Goto the handler of next stage of playlist
                play.sm = MP3_PLAY_PL_GET_NEXT_TRACK;
            } else if (config.play.repeat == 1 && stopIndicator == FALSE) {
                verbosePrintf(VER_DBG, "Repeat enabled");
                play.sm = MP3_PLAY_OPEN_FILE;
            } else {
                // Goto idle state machine
                play.sm = MP3_PLAY_HOME;
            }

            break;



        default:
            verbosePrintf(VER_DBG, "Return to idle state");
            // Goto idle state machine
            play.sm = MP3_PLAY_HOME;
            break;
    }

    return play.sm;
}

void startPlay(char *ptr) {
    strncpy(play.filename, ptr, _MAX_LFN);
    // Turn on the player
    play.sm = MP3_PLAY_OPEN_FILE;
}

bool isPlaying(void) {
    if (play.sm >= MP3_PLAY_READ_BUFFER && play.sm <= MP3_PLAY_WRITE_BUFFER)
        return true;
    return false;
}

int Play(int argc, char **argv) {

    if (argc == 1) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 2) {
        startPlay(argv[1]);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

BOOL PausePlay(int argc, char **argv) {

    LONG delay;

    if (argc == 1) {

        if (isPlaying()) {
            // Enter in pause
            play.sm = MP3_PLAY_PAUSE_WAIT_ENTERING;
            printf("Pause: ON\r\n");
            return TRUE;
        } else if (play.sm >= MP3_PLAY_PAUSE_WAIT && play.sm <= MP3_PLAY_PAUSE_DELAY) {
            // Exit from pause
            play.sm = MP3_PLAY_PAUSE_EXIT;
            printf("Pause: OFF\r\n");
            return TRUE;
        }

    } else if (argc == 2) {

        if (isPlaying()) {
            delay = atoimm(argv[1], 0, 10000000, 1000);

            tick_delay = TickGet();
            tick_max = TICK_SECOND / 1000 * delay;

            play.sm = MP3_PLAY_PAUSE_DELAY_ENTERING;
            //            VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
            //            play.sm = MP3_PLAY_PAUSE_DELAY;
            //            GpioUpdateState(GPIO_BIT_PAUSE_PLAY);

            printf("Pause for %ld ms\r\n", delay);
            return TRUE;
        }

    } else {
        CliTooManyArgumnets(argv[0]);
    }

    return FALSE;
}

BOOL StopPlay(int argc, char **argv) {

    // posso aggiungere un ritardo ricopiando la struttura di Pause
    // anche kill
    if (argc == 1) {
        if (play.sm >= MP3_PLAY_OPEN_FILE && play.sm < MP3_PLAY_FINISH_PLAING) {
            if (play.sm == MP3_PLAY_PAUSE_WAIT)
                VLSI_ClearBitPlayMode(PLAYMODE_PAUSE_OFF);
            play.sm = MP3_PLAY_FINISH_PLAING;
            return TRUE;
        }
    } else if (argc == 2) {
        if (strcmp(argv[1], "-all") == 0) {
            if (play.sm >= MP3_PLAY_OPEN_FILE && play.sm < MP3_PLAY_FINISH_PLAING) {
                if (play.sm == MP3_PLAY_PAUSE_WAIT)
                    VLSI_ClearBitPlayMode(PLAYMODE_PAUSE_OFF);
                play.sm = MP3_PLAY_FINISH_PLAING;
                // Also, and playlist execution
                playlistIndicator = FALSE;
                stopIndicator = TRUE;
                return TRUE;
            }
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return FALSE;
}

BOOL InfoPlay(int argc, char **argv) {

    const char str_yes[] = {"Yes"};
    const char str_no[] = {"No"};
    WORD bitrate;

    if (argc == 1) {
        // Print the track name in play otherwise print no track in execution
        if (play.sm > MP3_PLAY_OPENED_SUCCESSFUL && play.sm <= MP3_PLAY_FINISH_PLAING) {
            // First uUpdate the HDAT

            printf("File: %s;\r\n", play.filename);
            VLSI_GetHDAT();
            printf("Encodig format: %s\r\n", VLSI_GetBitrateOthersFormat(&bitrate));

            if (bitrate == 0) {
                // Then take all updatetd info (MP3)
                printf("Bitrate: %dkbit/s;\r\n", VLSI_GetBitrateMP3Format());
                //printf("Bitrate avg: %dkbit/s;", (VLSI_GetBitRatePer100() * 8 / 100));
                printf("Samplerate: %dHz;\r\n", VLSI_GetSamplerate());
                printf("Mode: %s;\r\n", VLSI_GetMode());
                printf("ID: %s;\r\n", VLSI_GetID());
                printf("Layer: %s;\r\n", VLSI_GetLayer());
                printf("Emphasis: %s;\r\n", VLSI_GetEmphasis());
                printf("Protect: %s;\r\n", (VLSI_GetProtect() ? str_yes : str_no));
                printf("Original: %s;\r\n", (VLSI_GetOriginal() ? str_yes : str_no));
                printf("Pad: %s;\r\n", (VLSI_GetPad() ? str_yes : str_no));
                printf("Copyright: %s;\r\n", (VLSI_GetCopyright() ? str_yes : str_no));
                bitrate = VLSI_GetBitRatePer100() * 8;
            }
            printf("Bitrate avg: %dkbit/s;\r\n", bitrate / 100);

            return TRUE;
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }

    return FALSE;
}

int FPlay(int argc, char **argv) {

    int speed = 0;

    if (argc == 1) {
        speed = VLSI_GetFastSpeed();
        if (speed == 0)
            speed++;
        printf("Fast speed: %dx\r\n", speed);
    } else if (argc == 2) {
        if (play.sm >= MP3_PLAY_OPENED_SUCCESSFUL && play.sm <= MP3_PLAY_FINISH_PLAING) {
            speed = atoimm(argv[1], 0, 10, 0);
            VLSI_SetFastSpeed(speed);
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}
// Speed Shifter allows the playback tempo to be changed without changing the playback pitch.
// The playback tempo is speedShifter / 16384, i.e. 16384 is the normal speed. The minimum speed is
// 0.68x (11141) and maximum speed 1.64x (26869).
//
// If you want to change pitch without changing tempo, adjust the speed and compensate by also
// adjusting the samplerate. For example two semitones is 2^(-2/12) = 0.8909, so set the Speed
// Shifter to 2^(-2/12) * 16384 = 14596 and set rateTune to (2^2/12 - 1) * 1000000 = 122462.
//
// Speed Shifter and EarSpeaker can not be used at the same time. Speed Shifter overrides EarSpeaker.

int SpeedShifter(int argc, char **argv) {

    float f;
    long speed = 0;
    int rtn;

    if (argc == 1) {
        //CliTooFewArgumnets(argv[0]);
        rtn = VLSI_GetSpeedShifter();
        if (rtn == 0)
            rtn = 16384;
        f = rtn / 16384.0;
        printf("Speed Shifter: [%d] (0x%X) %.3fx\r\n", rtn, rtn, f);
    } else if (argc == 2) {
        if (play.sm >= MP3_PLAY_OPENED_SUCCESSFUL && play.sm <= MP3_PLAY_FINISH_PLAING) {
            speed = atoimm(argv[1], 680, 1640, 1000);
            speed = speed * 16384 / 1000;
            VLSI_SetSpeedShifter(speed);
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

// rateTune finetunes the samplerate in 1 ppm steps. This is useful in streaming applications
// where long-term buffer fullness is used to adjust the samplerate very accurately. Zero is normal
// speed, positive values speed up, negative values slow down. To calculate rateTune for a speed,
// use (x - 1.0) * 1000000. For example 5.95% speedup (1:0595 - 1.0) * 1000000 = 59500.

int RateTune(int argc, char **argv) {

    float f;
    LONG speed = 0;

    if (argc == 1) {
        speed = VLSI_GetRateTune();
        f = speed / 10000.0;
        printf("Rate finetune: [%d] (0x%X) %.2f%%\r\n", speed, speed, f);
    } else if (argc == 2) {
        if (play.sm >= MP3_PLAY_OPENED_SUCCESSFUL && play.sm <= MP3_PLAY_FINISH_PLAING) {
            speed = atoimm(argv[1], -1000000, 1000000, 0);
            VLSI_SetRateTune(speed);
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Semitone(int argc, char **argv) {

    LONG semitone;
    double result, exponent;
    WORD speed;
    DWORD rate;

    if (argc == 1) {
        SpeedShifter(1, NULL);
        RateTune(1, NULL);
    } else if (argc == 2) {
        if (play.sm >= MP3_PLAY_OPENED_SUCCESSFUL && play.sm <= MP3_PLAY_FINISH_PLAING) {

            semitone = atoimm(argv[1], -24, 24, 0);

            exponent = (0 - semitone) / 12.0;
            result = pow(2, exponent);
            speed = result * 16384;

            exponent = semitone / 12.0;
            result = pow(2, exponent);
            rate = (result - 1) * 1000000;

            VLSI_SetSpeedShifter(speed);
            VLSI_SetRateTune(rate);
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int EarSpeaker(int argc, char **argv) {

    LONG ear = 0;

    if (argc == 1) {
        ear = VLSI_GetEarSpeaker();
        printf("Ear Speaker: [%d] (0x%X)\r\n", ear, ear);
    } else if (argc == 2) {
        if (play.sm >= MP3_PLAY_OPENED_SUCCESSFUL && play.sm <= MP3_PLAY_FINISH_PLAING) {
            ear = atoimm(argv[1], 0, 50000, 0);
            VLSI_SetEarSpeaker(ear);
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Playlist(int argc, char **argv) {

    if (argc == 1) {
        // Copy in play.filename gloabal variable the name of the default file
        strncpy(play.filename, config.play.playlist, _MAX_LFN);
        // Turn on the playlist
        play.sm = MP3_PLAY_OPEN_PLAYLIST;
    } else if (argc == 2) {
        // Copy in play.filename gloabal variable the name of the passed file
        strncpy(play.filename, argv[1], _MAX_LFN);
        // Turn on the playlist
        play.sm = MP3_PLAY_OPEN_PLAYLIST;
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Bookmark(int argc, char **argv) {
    printf("The command is not implemented yet\r\n");
    return 0;
}
