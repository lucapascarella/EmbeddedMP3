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

#ifndef PLAYBACK_HPP
#define	PLAYBACK_HPP

#include "CommandBase.hpp"
#include "../CLI.hpp"

class Playback : public CommandBase {
private:
    static constexpr const char* name = "play";
    static constexpr const char* options = "";

    CLI *cli;

public:
    Playback(CLI *cli);
    virtual const char * getCommandOptions(void);
    virtual const char * getCommandName(void);
    virtual int command(void); // pure specifier (Abstract implementation)
    int helper(void); // pure specifier (Abstract implementation)
};


#endif	/* PLAYBACK_HPP */

