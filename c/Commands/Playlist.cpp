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

#include "Commands/Playlist.hpp"
#include "MP3/Play.h"
#include "Utilities/ArgsParser.hpp"

Playlist::Playlist(void) {
    calculateNameLength();
}

int Playlist::command(int argc, char **argv) {

//    //    return_t *opt;
//
//    if (checkParameters(argc, argv, 2, 2)) {
//        //        opt = GetOptList(argc, argv, "C");
//        startPlay(argv[1]);
//        //      FreeRtnList(opt);
//
//        return 0;
//    } else {
//        return -1;
//    }
    return 0;
}

const char * Playlist::getCommandOptions(void) {
    return options;
}

const char * Playlist::getCommandName(void) {
    return name;
}