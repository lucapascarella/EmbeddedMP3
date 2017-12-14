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

#ifndef PLAYLIST_HPP
#define	PLAYLIST_HPP

#include "CommandBase.hpp"

class Playlist : public CommandBase {
private:
    static constexpr const char* name = "playlist";
    static constexpr const char* options = "";

public:
    Playlist(CLI *cli);
    virtual const char * getCommandOptions(void);
    virtual const char * getCommandName(void);
    int command(void); // pure specifier (Abstract implementation)
    int helper(void); // pure specifier (Abstract implementation)
};


#endif	/* PLAYLIST_HPP */

