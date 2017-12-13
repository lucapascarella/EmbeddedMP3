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

#include "Commands/Playback.hpp"
#include "Utilities/ArgsParser.hpp"

Playback::Playback(CLI *cli) {
    this->cli = cli;
    calculateNameLength();
}

int Playback::command(void) {

////    return_t *opt;
//
//    if (checkParameters(argc, argv, 2, 2)) {
////        opt = GetOptList(argc, argv, "C");
//        startPlay(argv[1]);
//  //      FreeRtnList(opt);
//        
//        return 0;
//    } else {
//        return -1;
//    }
    return COMMAND_BASE_TERMINATED;
}

const char * Playback::getCommandOptions(void) {
    return options;
}

const char * Playback::getCommandName(void) {
    return name;
}

int Playback::verbosePrintf(int level, bool reprint, const char * fmt, ...) {
    int rtn;
    va_list ap;
    va_start(ap, fmt);
    rtn = cli->verbosePrintfWrapper(level, reprint, fmt, ap);
    va_end(ap);

    return rtn;
}

int Playback::helper(void){
    return COMMAND_BASE_TERMINATED;
}