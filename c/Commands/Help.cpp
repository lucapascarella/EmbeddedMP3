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

#include "Commands/Help.hpp"
#include "Utilities/ArgsParser.hpp"
#include "Utilities/CustomFunctions.h"

Help::Help(CLI *cli) {
    this->cli = cli;
    calculateNameLength();
}

int Help::command(void) {

    int rtn;

    rtn = COMMAND_BASE_EXECUTING;
    switch (sm) {
        case SM_HELP_HOME:
            // Check flag options
            path = NULL;
            if (opt->isOptionPresent('&'))
                path = opt->getFirstArgumentForOption('&');
            sm = SM_CHECK;
            //break; // no break required 

        case SM_CHECK:
            if (argc == 1) {
                sm = SM_PRINT_HELP;
            } else if (path != NULL && custom_strlen(path) >= 1) {
                rtn = COMMAND_BASE_PRINT_HELP;
                sm = SM_HELP_HOME;
            } else {
                sm = SM_HELP_ERROR;
            }
            break;

        case SM_PRINT_HELP:
            // Command help informations
            printf("Returns the help informations.\r\n");
            this->usageCommand("help [cmd]");
            printf("cmd\thelp of 'cmd' commnad\r\n");
            this->usageExample("help play");
            sm = SM_HELP_END;
            break;

        case SM_HELP_ERROR:
            rtn = COMMAND_BASE_ERROR;
            sm = SM_HELP_HOME;
            break;

        case SM_HELP_END:
            rtn = COMMAND_BASE_TERMINATED;
            sm = SM_HELP_HOME;
            break;

    }
    return rtn;
}

const char * Help::getCommandOptions(void) {
    return options;
}

const char * Help::getCommandName(void) {
    return name;
}

int Help::helper(void) {
    return COMMAND_BASE_TERMINATED;
}