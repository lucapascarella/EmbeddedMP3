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

#include "Commands/CommandBase.hpp"
#include "Utilities/CustomFunctions.h"
#include "Utilities/ArgsParser.hpp"
#include "Utilities/CustomFunctions.h"
#include <string.h>
#include <cctype>

CommandBase::CommandBase(void) {
    this->initializeStatistics();
    commandNameLength = 0;
}

void CommandBase::calculateNameLength(void) {
    char *p;
    if ((p = (char*) getCommandName()) != NULL)
        commandNameLength = strlen(p);
}

void CommandBase::getStatistics(void) {

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
        this->argumnetsProblem();
        tooFewArgsCounter++;
        return false;
    } else if (argc > upperLimit) {
        // Too many arguments passed
        this->argumnetsProblem();
        tooManyArgsCounter++;
        return false;
    } else {
        correctArgsCounter++;
        return true;
    }
}

const char* CommandBase::getCommandOptions(void) {
    return NULL;
}

const char * CommandBase::getCommandName(void) {
    return NULL;
}

int CommandBase::getCommandNameLength(void) {
    return commandNameLength;
}

bool CommandBase::isConvertible(char *pstr) {

    int num;
    char *p = pstr;

    num = 0;
    while (*p != '\0') {
        if (isdigit(*p) || *p == '-')
            num++;
        p++;
    }

    if (num == custom_strlen(pstr))
        return true;
    return false;
}

long CommandBase::atolmm(char *str, long min, long max, long def) {

    long tmp;

    tmp = def;
    if (str != NULL)
        if (this->isConvertible(str)) {
            tmp = atol(str);
            if (tmp < min)
                tmp = min;
            if (tmp > max)
                tmp = max;
        }
    return tmp;
}

void CommandBase::argumnetsProblem(void) {
    this->getCommandName();
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
