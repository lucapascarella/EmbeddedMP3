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
#include <stdint.h>
#include <stdbool.h>

#define COMMAND_BASE_EXECUTING              +1
#define COMMAND_BASE_TERMINATED             0
#define COMMAND_BASE_ERROR                  -1
#define COMMAND_BASE_PRINT_HELP             -10

class CLI; // Forward declaration

class CommandBase {

protected:
    int commandNameLength;
    int argc;
    char **argv;
    Optlist *opt;
    int numOfOpt;
    CLI *cli;

    enum SM {
        COMMAND_SM_PARSE_ARGS = 0,
        COMMAND_SM_CREATE_OPTLIST,
        COMMAND_SM_EXECUTE,
        COMMAND_SM_DESTROY_OPTLIST,
        COMMAND_SM_DONE,
    } sm;

public:
    CommandBase(CLI *cli);
    int taskCommand(ArgsParser *args);
    int getCommandNameLength(void);
    virtual const char* getCommandOptions(void) = 0; // pure specifier requires concrete implementation
    virtual const char* getCommandName(void) = 0; // pure specifier requires concrete implementation
    ~CommandBase(void);

protected:
    void calculateNameLength(void);
    bool isConvertible(char *pstr);
    long atolmm(char *str, long min, long max, long def);
    bool checkRequiredOptions(const char * opts);
    void printUnexpectedNumberOfOptions(void);
    void printUnexpectedOptions(const char *opts);
    void printOptions(void);
    void usageExample(const char *str);
    void usageCommand(const char *str);
    virtual int command(void) = 0; // pure specifier requires concrete implementation
    virtual int helper(void) = 0; // pure specifier requires concrete implementation
};

#endif	/* COMMAND_ABSTRACT_HPP */

