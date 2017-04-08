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

#include <lega-c/string.h>

#include "Commands/Playback.h"

Playback::Playback(int x, int y) {

}

int Playback::playback(int argc, char **argv) {

    if (checkParameters(argc, argv, 2, 2)) {
        // Copy in fileName gloabal variable the name of the passed file
        //strncpy(play.filename, argv[1], _MAX_LFN);
        // Turn on the player
        //play.sm = MP3_PLAY_OPEN_FILE;
        return 0;
    } else {
        return -1;
    }
}