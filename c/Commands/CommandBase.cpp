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
    commandNameLength = 0;
    argc = 0;
    argv = NULL;
    opt = NULL;
    sm = COMMAND_SM_PARSE_ARGS;
}

void CommandBase::calculateNameLength(void) {
    char *p;
    if ((p = (char*) getCommandName()) != NULL)
        commandNameLength = strlen(p);
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

bool CommandBase::checkRequiredOptions(const char * opts) {
    char p;
    int expected, counted;

    expected = counted = 0;
    if (opts != NULL)
        while ((p = opts[expected++]) != '\0') {
            if (opt->getFirstArgumentForOption(p) != NULL)
                counted++;
        }
    if (expected == counted)
        return true;
    return false;
}

void CommandBase::printUnexpectedNumberOfOptions(void) {
    verbosePrintf(VER_ERR, "Unexpected number of options (%d)", numOfOpt);
}

void CommandBase::printUnexpectedOptions(const char *opts) {
    verbosePrintf(VER_ERR, "Unexpected options %d", numOfOpt);
}

int CommandBase::taskCommand(ArgsParser *args) {

    int rtn;

    rtn = 0;
    switch (sm) {
        case COMMAND_SM_PARSE_ARGS:
            argc = args->getArgc();
            argv = args->getArgv();
            numOfOpt = 0;
            rtn = 1;
            sm = COMMAND_SM_CREATE_OPTLIST;
            // break; // no break needed

        case COMMAND_SM_CREATE_OPTLIST:
            // Create a list of options
            opt = new Optlist();
            if (opt->createOptionList(argc, argv, this->getCommandOptions()) == true) {
                numOfOpt = opt->getNumberOfOptions();
                rtn = 1;
                sm = COMMAND_SM_EXECUTE;
            } else {
                rtn = 0;
                sm = COMMAND_SM_DESTROY_OPTLIST;
            }
            break;

        case COMMAND_SM_EXECUTE:
            if ((rtn = this->command()) > 0)
                break;
            sm = COMMAND_SM_DESTROY_OPTLIST;
            // break; // No break here

        case COMMAND_SM_DESTROY_OPTLIST:
            opt->~Optlist();
            sm = COMMAND_SM_DONE;
            // break; // no break here

        case COMMAND_SM_DONE:
            sm = COMMAND_SM_PARSE_ARGS;
            break;
    }
    return rtn;
}

int CommandBase::command(void) {
    return 0;
}

CommandBase::~CommandBase(void) {

}
