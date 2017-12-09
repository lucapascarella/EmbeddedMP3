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

#ifndef COMMAND_ABSTRACT_HPP
#define	COMMAND_ABSTRACT_HPP

#include "Utilities/ArgsParser.hpp"
#include "Utilities/Optlist.hpp"
#include "Utilities/printer.h"
#include <stdint.h>
#include <stdbool.h>

#define COMMAND_BASE_EXECUTING              +1
#define COMMAND_BASE_TERMINATED             0
#define COMMAND_BASE_ERROR                  -1

class CommandBase {
protected:
    // Class property
    int commandNameLength;
    int argc;
    char **argv;
    Optlist *opt;
    int numOfOpt;

    enum SM_COMMAND {
        COMMAND_STATE_HOME = 0,
        COMMAND_STATE_PARSE_ARGS,
        COMMAND_STATE_EXECUTE,
        COMMAND_STATE_DONE,
    } commandState;

    enum SM {
        COMMAND_SM_PARSE_ARGS = 0,
        COMMAND_SM_CREATE_OPTLIST,
        COMMAND_SM_EXECUTE,
        COMMAND_SM_DESTROY_OPTLIST,
        COMMAND_SM_DONE,
    } sm;

public:
    CommandBase(void);
    int taskCommand(ArgsParser *args);
    int getCommandNameLength(void);
    virtual const char* getCommandOptions(void) = 0; // pure specifier
    virtual const char* getCommandName(void) = 0; // pure specifier
    ~CommandBase(void);

protected:
    void calculateNameLength(void);
    bool isConvertible(char *pstr);
    long atolmm(char *str, long min, long max, long def);
    bool checkRequiredOptions(const char * opts);
    void printUnexpectedNumberOfOptions(void);
    void printUnexpectedOptions(const char *opts);
    void printOptions(void);
    virtual int command(void); // pure specifier
};


#endif	/* COMMAND_ABSTRACT_HPP */

