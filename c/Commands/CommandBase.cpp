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

#include "Commands/CommandBase.h"
#include "CommandLineInterpreter.h"
#include "Utilities/Utilities.h"
#include "Utilities/ArgsParser.h"

CommandBase::CommandBase(void) {
    this->initializeStatistics();
    commandNameLength = custom_strlen((char*) getCommandName());
}

void CommandBase::getStatistics(void) {
    // Initialize statistical information

}

void CommandBase::initializeStatistics(void) {
    // Initialize statistical information
    tooFewArgsCounter = 0;
    tooManyArgsCounter = 0;
    correctArgsCounter = 0;
    lastCommandArgsCounter = 0;
}

bool CommandBase::checkParameters(int argc, char **argv, int lowLimit, int upperLimit) {

    lastCommandArgsCounter = argc;
    if (argc < lowLimit) {
        // Too few arguments passed
        CliTooFewArgumnets(argv[0]);
        tooFewArgsCounter++;
        return false;
    } else if (argc > upperLimit) {
        // Too many arguments passed
        CliTooManyArgumnets(argv[0]);
        tooManyArgsCounter++;
        return false;
    } else {
        correctArgsCounter++;
        return true;
    }
}

const char* CommandBase::getCommandOptions(void) {
    return 0;
}

const char * CommandBase::getCommandName(void) {
    return 0;
}

int CommandBase::getCommandNameLength(void) {
    return commandNameLength;
}

int CommandBase::taskCommand(ArgsParser *args) {
    int argc;
    char **argv;
    argc = args->getArgc();
    argv = args->getArgv();
    return this->command(argc, argv);
}

int CommandBase::command(int argc, char **argv) {
    return 0;
}

CommandBase::~CommandBase(void) {

}
