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

#ifndef LIST_HPP
#define	LIST_HPP

#include "CommandBase.hpp"
#include "FatFS/ff.h"

#define LIST_BUFFER_SIZE            128

class List : public CommandBase {
private:
    static constexpr const char* name = "ls";
    static constexpr const char* options = "alh&";

    enum LIST_SM {
        SM_LIST_HOME = 0,
        SM_LIST_INITIALIZE,
        SM_LIST_OPEN_DIR,
        SM_LIST_READ_DIR,
        SM_LIST_CLOSE_DIR,
        SM_LIST_NO_PATH,
        SM_LIST_ERROR,
        SM_LIST_END,
    } sm;

    char *path;
    char *buf;

    FRESULT fres;
    DIR *dir;
    FILINFO *finfo;

    WORD countTotObj;
    WORD countFile;
    WORD countDir;
    DWORD totalSize;

    struct {
        uint8_t hidden : 1;
        uint8_t list : 1;
        uint8_t human : 1;
        uint8_t relative : 1;
    } flags;

public:
    List(CLI *cli);
    const char * getCommandOptions(void);
    const char * getCommandName(void);
    int command(void); // pure specifier (Abstract implementation)
    int helper(void); // pure specifier (Abstract implementation)

private:
    const char * byteToFatAttributes(uint8_t att);
};


#endif	/* LIST_HPP */

