/* 
 * File:   CLI.hpp
 * Author: luca
 *
 * Created on April 11, 2017, 12:53 PM
 */

#ifndef CLI_HPP
#define	CLI_HPP


#include "Commands/CommandBase.hpp"
#include "FatFS/ff.h"
#include <stdint.h>
#include <stdbool.h>
#include <list>

#define CLI_MAX_DIR_SIZE    128
#define CLI_MAX_BUF_SIZE    1024 * 4

#define CLI_MAX_LINE_INPUT_SIZE     1024

#define ESCAPE                  0x1B
#define ESCAPE_ARROW_LEFT       0x80
#define ESCAPE_ARROW_RIGHT      0x81
#define ESCAPE_ARROW_UP         0x82
#define ESCAPE_ARROW_DOWN       0x83

#define ESCAPE_PAGE_UP          0x84
#define ESCAPE_PAGE_DOWN        0x85
#define ESCAPE_HOME             0x86
#define ESCAPE_END              0x87

#define ESCAPE_DEL              0x88

#define ESCAPE_F1               0x89

class CLI {
private:

    // Escape sequence
    uint16_t escapeCount;
    char escapeSequence[10];

    // Received input
    char inputLine[CLI_MAX_BUF_SIZE];
    int inputLineIndex;
    int inputLineLength;

    char tmp[CLI_MAX_BUF_SIZE];
    int tmpIndex;
    int tmpLength;

    int lastCmd;
    int nCmd;

    // Args parser
    ArgsParser *args;

    // Commands
    std::list<CommandBase*> commandList;
    CommandBase *cmd;

    // Global file pointer for latter commands
    FIL *fileLastCommands;

    // State Machine for task handler

    enum SM {
        CLI_SM_HOME = 0,
        CLI_SM_WAIT_INPUT,
        CLI_SM_ARGS_PARSER,
        CLI_SM_FIND_COMMAND,
        CLI_SM_COMMAND_TASK,
        CLI_SM_COMMAND_NOT_FOUND,
        CLI_SM_DONE,
    } sm;

public:
    CLI(void);
    void registerCommand(CommandBase *cb);
    bool createFileListOfCommands(void);
    void cliTaskHadler(void);
    virtual ~CLI(void);

private:
    bool cliArgsParse(void);
    bool cliInputHadler(void);

    bool copyInputInLocalBuffer(uint8_t *p);
    uint8_t completeCommand(void);
    void addCharAndUpdateConsole(uint8_t c);
    void clearCommand(void);
    void reprintConsole(void);
    void CliAddStringAndUpdateConsole(char *str);
    void printEscape(const char *p, int i);
    void printBackspace(void);
    void printFor(char *p, int i, int len);
    bool searchExecutableCommand(char *name);
    bool createFileListOfFilesEntry(void);
    uint8_t CliCompleteCommandSearchInFile(char *fileName, char *p);
    bool getLastCommandFromFile(int pos);
    void putLastCommandInFile(void);
};

#endif	/* CLI_HPP */

