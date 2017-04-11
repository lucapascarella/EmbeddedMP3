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

#ifndef ARGSPARSER_H
#define	ARGSPARSER_H

#include <stdint.h>
#include <stdbool.h>

#define _MAX_CMD_LINE_ARGS  128

class ArgsParser {
private:
    int argc;
    char * argv[_MAX_CMD_LINE_ARGS + 1];
    char *cmdLineCopy;

public:
    ArgsParser(void);
    int extractArgs(char *input);
    char * getArgPointer(int pos);
    int getArgc(void);
    char ** getArgv(void);
    char * getLastArgPointer(void);
    ~ArgsParser(void);

};

#endif	/* ARGSPARSER_H */

