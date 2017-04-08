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
#include "Commands/CommandBase.h"
//#include "CommandLineInterpreter.h"

CommandBase::CommandBase(void) {
    tooFewArgsCounter = 0;
    tooManyArgsCounter = 0;
}

bool CommandBase::checkParameters(int argc, char **argv, int lowLimit, int upperLimit) {
    
    if (argc < lowLimit) {
        // Too few arguments passed
        //CliTooFewArgumnets(argv[0]);
        return false;
    } else if (argc > upperLimit) {
        // Too many arguments passed
        //CliTooManyArgumnets(argv[0]);
        return false;
    } else {
        return true;
    }
}
