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

#include "MP3/CCPlay.h"
#include "Utilities/printer.h"
#include "Utilities/Utilities.h"
#include "Utilities/GPIO.h"
#include "MP3/VS1063.h"
#include "Utilities/Config.h"
#include "Delay/Tick.h"

CCPlay::CCPlay(void) {
    sm = MP3_CCPLAY_HOME;

    // Counters and other global variables
    playlistNumber = 0;

    // Initialize static buffers
    custom_memset(filename, sizeof (filename), '\0');
    custom_memset(playlistFilename, sizeof (playlistFilename), '\0');

    // Initialize pointers
    pfilPlaylist = NULL;
}

int CCPlay::playTaskHandler(void) {

    FRESULT fres;

    switch (sm) {

        case MP3_CCPLAY_HOME:
            // Do nothing, wait a play command
            Nop();
            break;



        case MP3_CCPLAY_OPEN_PLAYLIST:
            verbosePrintf(VER_DBG, "Try to open playlist file: %s", playlistFilename);
            // Open a play-list in read mode
            custom_malloc((void**) pfilPlaylist, sizeof (FIL));
            fres = f_open(pfilPlaylist, playlistFilename, FA_READ);
            if (fres != FR_OK) {
                // Signal and handler the error
                verbosePrintf(VER_MIN, "File %s error %s", playlistFilename, string_rc(fres));
                GpioUpdateOutputState(GPIO_BIT_FILE_NOT_FOUND);
                FlashLight(100, 10, false);
                flags.bits.playlistIndicator = false;
                sm = MP3_CCPLAY_HOME;
            } else {
                verbosePrintf(VER_DBG, "Playlist opened successful");
                playlistNumber = 0;
                flags.bits.playlistIndicator = true;
                sm = MP3_CCPLAY_PL_GET_NEXT_TRACK;
            }
            break;



        case MP3_CCPLAY_PL_GET_NEXT_TRACK:
            verbosePrintf(VER_DBG, "Try to open next file in playlist file: %s", playlistFilename);
            if (flags.bits.playlistIndicator && f_gets(filename, _MAX_LFN, pfilPlaylist) != NULL) {
                playlistNumber++;
                verbosePrintf(VER_DBG, "Playlist error %s", filename);
                sm = MP3_CCPLAY_OPEN_FILE;
            } else {
                playlistNumber = 0;
                flags.bits.playlistIndicator = false;
                verbosePrintf(VER_DBG, "Playlist ended");
                if (f_close(pfilPlaylist) != FR_OK)
                    FlashLight(100, 10, false);
                sm = MP3_CCPLAY_FINISH;
            }
            break;



        case MP3_CCPLAY_OPEN_FILE:
            verbosePrintf(VER_DBG, "Try to open: %s", filename);
            // Open a file in read mode
            custom_malloc((void**) pfil, sizeof (FIL));
            fres = f_open(pfil, filename, FA_READ);
            if (fres != FR_OK) {
                // Signal and handler the error
                verbosePrintf(VER_MIN, "File %s error %s", filename, string_rc(fres));
                GpioUpdateOutputState(GPIO_BIT_FILE_NOT_FOUND);
                FlashLight(100, 10, false);
                sm = MP3_CCPLAY_FINISH;
            } else {
                verbosePrintf(VER_DBG, "File opened successful");
                // Initialize the decoder
                // Todo spostare questa funzione in VLSI
                VLSIWriteReg(VLSI_ADD_MODE, VLSI_VAL_MODE_DECODE, SPI_BRG_12_5MHZ);
                // Initialize the buffer counter and the plaing indicator
                read = write = 0;
                flags.bits.playIndicator = true;
                flags.bits.stopIndicator = false;
                GpioUpdateOutputState(GPIO_BIT_STARTS_PLAY);
                sm = MP3_CCPLAY_READ_BUFFER;
            }
            break;






        case MP3_CCPLAY_READ_BUFFER:
            // Read buffer_size bytes and transfer to VS1063 decoder
            LED_BLUE_ON();
            fres = f_read(pfil, buffer, CCPLAY_BUF_SIZE_PLAY, (UINT*) &read);
            LEDs_OFF();

            if (read <= 0) {
                sm = MP3_CCPLAY_FINISH_PLAING;
            } else {
                sm = MP3_CCPLAY_WRITE_BUFFER;
            }
            break;



        case MP3_CCPLAY_WRITE_BUFFER:
            // Write a small quantities of data in the decoder buffer
            write += VLSIPutArray((BYTE*) & buffer[write], read - write);
            if (write == read) {
                write = 0;
                sm = MP3_CCPLAY_READ_BUFFER;
            }
            break;




        case MP3_CCPLAY_PAUSE_WAIT_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_PLAY);
            // Put the decoder in pause mode
            VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
            sm = MP3_CCPLAY_PAUSE_WAIT;
            break;

        case MP3_CCPLAY_PAUSE_WAIT:
            // Wait until pause is resent
            Toggle1Second();
            break;


        case MP3_CCPLAY_PAUSE_DELAY_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_PLAY);
            // Put the decoder in pause mode
            VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
            sm = MP3_CCPLAY_PAUSE_DELAY;
            break;

        case MP3_CCPLAY_PAUSE_DELAY:
            if (TickGet() - tick_delay >= tick_max)
                sm = MP3_CCPLAY_PAUSE_EXIT;
            Toggle1Second();
            break;

        case MP3_CCPLAY_PAUSE_EXIT:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_PLAY);
            VLSI_ClearBitPlayMode(PLAYMODE_PAUSE_OFF);
            sm = MP3_CCPLAY_WRITE_BUFFER;
            break;




        case MP3_CCPLAY_FINISH_PLAING:
            GpioUpdateOutputState(GPIO_BIT_STOP_PLAY);
            verbosePrintf(VER_DBG, "Finish playing");
            // Finish to playing the current song
            VLSI_FinishPlaying();
            flags.bits.playIndicator = false;
            sm = MP3_CCPLAY_CLOSE_FILE;
            break;



        case MP3_CCPLAY_CLOSE_FILE:
            verbosePrintf(VER_DBG, "Try to close file");
            // Close the current file pointer
            fres = f_close(pfil);
            if (fres != FR_OK) {
                // Signal and handler the error
                verbosePrintf(VER_DBG, "File %s error %s", filename, string_rc(fres));
                FlashLight(100, 100, false);
                // Goto idle state machine
                sm = MP3_CCPLAY_PL_NEXT;
            } else {
                verbosePrintf(VER_DBG, "File closed successful");
                // Goto idle state machine
                sm = MP3_CCPLAY_PL_NEXT;
            }
            break;


        case MP3_CCPLAY_PL_NEXT:
            // Manage the next track in execution into the playlist
            if (playlistNumber) {
                // Goto the handler of next stage of playlist
                sm = MP3_CCPLAY_PL_GET_NEXT_TRACK;
            } else if (config.play.repeat == 1 && flags.bits.stopIndicator == false) {
                verbosePrintf(VER_DBG, "Repeat enabled");
                sm = MP3_CCPLAY_OPEN_FILE;
            } else {
                // Goto idle state machine
                sm = MP3_CCPLAY_FINISH;
            }

            break;

        case MP3_CCPLAY_FINISH:
            // TODO free mem
            // Deallocate dynamic memory
            custom_free((void**) pfil);
            custom_free((void**) pfilPlaylist);
            sm = MP3_CCPLAY_HOME;
            break;


        default:
            verbosePrintf(VER_DBG, "Return to idle state");
            // Goto idle state machine
            sm = MP3_CCPLAY_HOME;
            break;
    }

    return sm;
}

void CCPlay::startPlayback(char *ptr) {
    strncpy(filename, ptr, _MAX_LFN);
    // Turn on the player
    sm = MP3_CCPLAY_OPEN_FILE;
}

void CCPlay::stopPlayback(void) {
    // Stop command
}

bool CCPlay::isPlaying(void) {
    if (sm >= MP3_CCPLAY_READ_BUFFER && sm <= MP3_CCPLAY_WRITE_BUFFER)
        return true;
    return false;
}

CCPlay::~CCPlay(void) {
}

