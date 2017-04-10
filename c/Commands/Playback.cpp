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

#include "Commands/Playback.h"
#include "MP3/Play.h"

Playback::Playback(void) {

}

int Playback::playback(int argc, char **argv) {

    if (checkParameters(argc, argv, 2, 2)) {
        startPlay(argv[1]);
        return 0;
    } else {
        return -1;
    }
}

int Playback::stopPlayback(int argc, char **argv) {

    if (checkParameters(argc, argv, 2, 2)) {
        // TODO Add code here
        return 0;
    } else {
        return -1;
    }
}

int Playback::pausePlayback(int argc, char **argv) {

//    LONG delay;
//
//    if (argc == 1) {
//        if (isPlaying()) {
//            // Enter in pause
//            play.sm = MP3_PLAY_PAUSE_WAIT_ENTERING;
//            printf("Pause: ON\r\n");
//            return TRUE;
//        } else if (play.sm >= MP3_PLAY_PAUSE_WAIT && play.sm <= MP3_PLAY_PAUSE_DELAY) {
//            // Exit from pause
//            play.sm = MP3_PLAY_PAUSE_EXIT;
//            printf("Pause: OFF\r\n");
//            return TRUE;
//        }
//
//    } else if (argc == 2) {
//        if (isPlaying()) {
//            delay = atoimm(argv[1], 0, 10000000, 1000);
//
//            tick_delay = TickGet();
//            tick_max = TICK_SECOND / 1000 * delay;
//
//            play.sm = MP3_PLAY_PAUSE_DELAY_ENTERING;
//            //            VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
//            //            play.sm = MP3_PLAY_PAUSE_DELAY;
//            //            GpioUpdateState(GPIO_BIT_PAUSE_PLAY);
//
//            printf("Pause for %ld ms\r\n", delay);
//            return TRUE;
//        }
//
//    } else {
//        CliTooManyArgumnets(argv[0]);
//    }

    return -1;
}