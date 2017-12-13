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

#ifndef CAT_HPP
#define	CAT_HPP

#include "CommandBase.hpp"
#include "../CLI.hpp"
#include "FatFS/ff.h"

#define CAT_BUFFER_SIZE            1024

class Cat : public CommandBase {
private:
    static constexpr const char* name = "cat";
    static constexpr const char* options = "b&";

    CLI *cli;

    enum CAT_SM {
        SM_CAT_HOME = 0,
        SM_CAT_INITIALIZE,
        SM_CAT_CHECK_FILE,
        SM_CAT_OPEN_FILE,
        SM_CAT_READ_FILE,
        SM_CAT_PRINT,
        SM_CAT_CLOSE_FILE,
        SM_CAT_ERROR,
        SM_CAT_END,
    } sm;

    char *path;
    char *buf;

    FRESULT fres;
    FIL *fil;
    FILINFO *finfo;

    struct {
        uint8_t binary : 1;
    } flags;

public:
    Cat(CLI *cli);
    const char * getCommandOptions(void);
    const char * getCommandName(void);
    int command(void); // pure specifier (Abstract implementation)
    int helper(void); // pure specifier (Abstract implementation)

private:
    int verbosePrintf(int level, bool reprint, const char * fmt, ...); // pure specifier (Abstract implementation)
};


#endif	/* CAT_HPP */

