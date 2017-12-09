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
    uint16_t expected, counted;

    expected = counted = 0;
    if (opts != NULL) {
        while ((p = opts[expected++]) != '\0') {
            if (opt->getFirstArgumentForOption(p) != NULL)
                counted++;
        }
        if (strlen(opts) == counted)
            return true;
    }
    return false;
}

void CommandBase::printUnexpectedNumberOfOptions(void) {
    int i;
    Option *p;
    verbosePrintf(VER_MIN, "\r\nUnexpected number of options (%d)", numOfOpt);
    for (i = 0; i < numOfOpt; i++) {
        p = opt->getOptionNumber(i);
        if (p != NULL) {
            if (p->isArgumentRequired() && p->getArgument() != NULL)
                verbosePrintf(VER_ERR, "\r\n-%c %s", p->getFoundOption(), p->getArgument());
            else
                verbosePrintf(VER_ERR, "\r\n-%c", p->getFoundOption());
        }
    }
}

void CommandBase::printUnexpectedOptions(const char *opts) {
    int i;
    Option *p;
    verbosePrintf(VER_MIN, "\r\nUnexpected option(s):");
    for (i = 0; i < numOfOpt; i++) {
        p = opt->getOptionNumber(i);
        if (p != NULL && p->getGivenOption() == '\0')
            verbosePrintf(VER_ERR, "\r\n-%c", p->getFoundOption());
    }
}

void CommandBase::printOptions(void) {
    int i;
    Option *p;

    verbosePrintf(VER_MIN, "\r\nFound option(s): %d", numOfOpt);
    verbosePrintf(VER_MIN, "\r\nRequired options: %s", this->getCommandOptions());
    for (i = 0; i < numOfOpt; i++) {
        p = opt->getOptionNumber(i);
        if (p != NULL) {
            if (p->isArgumentRequired() && p->getArgument() != NULL)
                verbosePrintf(VER_ERR, "\r\nExpected %s -%c %s", p->isOptionExpected() ? "Yes" : "No", p->getFoundOption(), p->getArgument());
            else
                verbosePrintf(VER_ERR, "\r\nExpected %s -%c", p->isOptionExpected() ? "Yes" : "No", p->getFoundOption());
        }
    }
}

int CommandBase::taskCommand(ArgsParser *args) {

    int rtn;

    rtn = COMMAND_BASE_TERMINATED;
    switch (sm) {
        case COMMAND_SM_PARSE_ARGS:
            argc = args->getArgc();
            argv = args->getArgv();
            numOfOpt = 0;
            rtn = COMMAND_BASE_EXECUTING;
            sm = COMMAND_SM_CREATE_OPTLIST;
            // break; // no break needed

        case COMMAND_SM_CREATE_OPTLIST:
            // Create a list of options
            opt = new Optlist();
            opt->createOptionList(argc, argv, this->getCommandOptions());
            numOfOpt = opt->getNumberOfOptions();
            printOptions();
            rtn = COMMAND_BASE_EXECUTING;
            sm = COMMAND_SM_EXECUTE;
            break;

        case COMMAND_SM_EXECUTE:
            if ((rtn = this->command()) > COMMAND_BASE_TERMINATED)
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

    return COMMAND_BASE_TERMINATED;
}

CommandBase::~CommandBase(void) {

}
