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


#include "Commands/CommandsList.hpp"
#include "Commands/List.hpp"
#include "Commands/Playback.hpp"
#include "Commands/Stop.hpp"
#include "Commands/Playlist.hpp"
#include "Commands/RTCC.hpp"

CommandsList::CommandsList(CLI *cli) {
    // Initialize command class
    //List *list = new List(cli);
    //Playback *playback = new Playback();
    //Stop *stop = new Stop();
    //Playlist *playlist = new Playlist();
    //RTCC *rtcc = new RTCC();
    
    // Add commands to list
    //cli->registerCommand(list);
    //cli->registerCommand(playback);
    //cli->registerCommand(stop);
    //cli->registerCommand(playlist);
    //cli->registerCommand(rtcc);
    
    // Create file list of commands
    //cli->createFileListOfCommands();
}

