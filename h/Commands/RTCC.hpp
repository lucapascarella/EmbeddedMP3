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

#ifndef RTCC_HPP
#define	RTCC_HPP

#include "CommandBase.hpp"

class RTCC : public CommandBase {
private:
    static constexpr const char* name = "rtcc";
    static constexpr const char* options = "h:m:s:D:M:Y:";

public:
    RTCC(void);
    const char * getCommandOptions(void);
    const char * getCommandName(void);
    int command(void); // pure specifier
};


#endif	/* RTCC_HPP */

