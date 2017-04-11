/* 
 * File:   CLI.hpp
 * Author: luca
 *
 * Created on April 11, 2017, 12:53 PM
 */

#ifndef CLI_HPP
#define	CLI_HPP


#include "Commands/CommandBase.h"
#include <stdint.h>
#include <stdbool.h>
#include <list>

#define CLI_MAX_DIR_SIZE    128
#define CLI_MAX_BUF_SIZE    1024 * 4

//typedef enum {
//    CLI_SM_HOME = 0,
//} CLI_STATE_MACHINE;

class CLI {
private:
    // Some useful constants
    //const char temporaryFileEntryList[] = "/lst.tmp";
    //const char temporaryFileCommands[] = "/cmd.tmp";

    std::list<CommandBase> commandList;
    CommandBase *cmd;

    // State Machine for task handler
    enum {
        CLI_SM_HOME = 0,
    } sm;

public:
    CLI(void);

    void registerCommand(CommandBase &cb);
    void cliTaskHadler(void);
    //    bool CLI::CliCreateFileListOfCommands(void);
    //    bool CLI::CliCreateFileListOfFilesEntry(void);
    virtual ~CLI(void);

private:
    CommandBase * searchCommand(char *name);
};

#endif	/* CLI_HPP */

