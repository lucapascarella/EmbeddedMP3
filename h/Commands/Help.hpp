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

#ifndef HELP_HPP
#define	HELP_HPP

#include "CommandBase.hpp"

class Help : public CommandBase {
private:
    static constexpr const char* name = "help";
    static constexpr const char* options = "&";

    enum HELP_SM {
        SM_HELP_HOME = 0,
        SM_CHECK,
        SM_PRINT_HELP,
        SM_HELP_ERROR,
        SM_HELP_END,
    } sm;

    struct {
        uint8_t binary : 1;
    } flags;

    char *path;

public:
    Help(CLI *cli);
    const char * getCommandOptions(void);
    const char * getCommandName(void);
    int command(void); // pure specifier (Abstract implementation)
    int helper(void); // pure specifier (Abstract implementation)

private:
    int verbosePrintf(int level, bool reprint, const char * fmt, ...); // pure specifier (Abstract implementation)
};

#endif	/* HELP_HPP */

