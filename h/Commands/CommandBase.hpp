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

#include <stdint.h>
#include <stdbool.h>
#include "Utilities/ArgsParser.hpp"

class CommandBase {
private:
    // Statistics information
    long tooFewArgsCounter;
    long tooManyArgsCounter;
    long correctArgsCounter;
    long lastCommandArgsCounter;
    // Class property
    int commandNameLength;

public:
    CommandBase(void);
    void getStatistics(void);
    
    virtual const char* getCommandOptions(void); // pure specifier
    virtual const char* getCommandName(void); // pure specifier
    int getCommandNameLength(void);
    int taskCommand(ArgsParser *args); // pure specifier
    virtual int command(int argc, char **argv); // pure specifier

    ~CommandBase(void);
    
protected:
    void initializeStatistics(void);
    bool checkParameters(int argc, char **argv, int lowLimit, int upperLimit);
};


#endif	/* COMMAND_ABSTRACT_HPP */

