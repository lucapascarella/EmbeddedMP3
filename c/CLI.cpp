/* 
 * File:   CLI.cpp
 * Author: luca
 * 
 * Created on April 11, 2017, 12:53 PM
 */

#include "CLI.hpp"
#include "Utilities/printer.h"
#include "Utilities/Utilities.h"
#include "Utilities/CustomFunctions.h"
#include "Commands/CommandBase.hpp"
#include "Utilities/ArgsParser.hpp"
#include "Utilities/GPIO.h"
#include "Utilities/Config.h"
#include "CommandLineInterpreter.h"
#include <iterator>

const char temporaryFileLatterCommands[] = "/cmds.tmp";
const char temporaryFileEntryList[] = "/lst.tmp";
const char temporaryFileCommands[] = "/cmd.tmp";


const char escape_arrow_left[] = {0x1B, 0x5B, 0x44, 0x00};
const char escape_arrow_right[] = {0x1B, 0x5B, 0x43, 0x00};
const char escape_arrow_up[] = {0x1B, 0x5B, 0x41, 0x00};
const char escape_arrow_down[] = {0x1B, 0x5B, 0x42, 0x00};

const char escape_home[] = {0x1B, 0x5B, 0x31, 0x7E, 0x00};
const char escape_page_up[] = {0x1B, 0x5B, 0x35, 0x7E, 0x00};
const char escape_page_down[] = {0x1B, 0x5B, 0x36, 0x7E, 0x00};
const char escape_end[] = {0x1B, 0x5B, 0x34, 0x7E, 0x00};

const char escape_del[] = {0x1B, 0x5B, 0x33, 0x7E, 0x00};

const char escape_clear_screen[] = {0x1B, 0x5B, 0x48, 0x1B, 0x5B, 0x4A, 0x00};
const char escape_clear_end_row[] = {0x1B, 0x5B, 0x4A, 0x00};

CLI::CLI(void) {

    FRESULT fres;

    // Initialize global variables
    cmd = NULL;
    sm = CLI_SM_HOME;
    // Reset the last command indicator
    nCmd = lastCmd = 0;

    custom_memset(inputLine, '\0', sizeof (inputLine));
    inputLineLength = inputLineIndex = 0;
    custom_memset(tmp, '\0', sizeof (tmp));
    tmpLength = tmpIndex = 0;

    // Create Arguments parser object
    args = new ArgsParser();

    // Create escape sequence counter
    custom_memset(escapeSequence, '0', escapeCount);
    escapeCount = 0;

    // Create two hidden files with the list of commands and entry list of current directory
    //this->createFileListOfCommands();
    this->createFileListOfFilesEntry();

    // Creates a temporary file where put a list of latter commands
    fileLastCommands = NULL;
    fileLastCommands = (FIL*) custom_malloc(fileLastCommands, sizeof (FIL));
    if ((fres = f_open(fileLastCommands, temporaryFileLatterCommands, FA_READ | FA_WRITE | FA_OPEN_ALWAYS)) == FR_OK) {
        // Changes the properties of the temporary file to hide it
        if ((fres = f_chmod(temporaryFileLatterCommands, AM_HID, AM_HID)) == FR_OK) {
            // Flush hidden property
            if ((fres = f_sync(fileLastCommands)) != FR_OK) {
                verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
            }
        } else {
            verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
        }
    } else {
        verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
    }

    // Flush console content
    this->reprintConsole();
}

void CLI::registerCommand(CommandBase *cb) {
    commandList.push_back(cb);
}

void CLI::cliTaskHadler(void) {
    int rtn;
    char *name;

    switch (sm) {
        case CLI_SM_HOME:
            // Initialize stuff here 
            sm = CLI_SM_WAIT_INPUT;
            break;

        case CLI_SM_WAIT_INPUT:
            // Get a copy input until the newline is reached
            if (this->cliInputHadler())
                sm = CLI_SM_ARGS_PARSER;
            break;

        case CLI_SM_ARGS_PARSER:
            // Try to extract arguments
            if (this->cliArgsParse()) {
                sm = CLI_SM_FIND_COMMAND;
            } else {
                sm = CLI_SM_DONE;
            }
            break;

        case CLI_SM_FIND_COMMAND:
            // Find a command matching first argv
            name = args->getArgPointer(0);
            if (this->searchExecutableCommand(name)) {
                // It is a command, print it
                this->putLastCommandInFile();
                GpioUpdateOutputState(GPIO_BIT_CMD_OK);
                sm = CLI_SM_COMMAND_TASK;
            } else {
                GpioUpdateOutputState(GPIO_BIT_CMD_ERR);
                // The command was not found
                printf("Command '%s' not found!\r\n", name);
                // Clear the command buffer and reprint the console indicator
                this->clearCommand();
                sm = CLI_SM_DONE;
            }
            break;

        case CLI_SM_COMMAND_TASK:
            // Execute the command until 0 is returned
            rtn = cmd->taskCommand(args);
            if (rtn < 0) {
                // Some error happened
                printf("Command '%s' terminated with error code %d\r\n", cmd->getCommandName(), rtn);
                sm = CLI_SM_DONE;
            } else if (rtn > 0) {
                // Still working
            } else {
                // Command finished successful
                sm = CLI_SM_DONE;
            }
            break;

        case CLI_SM_COMMAND_NOT_FOUND:
            break;

        case CLI_SM_DONE:
            // De-initialize stuff here
            cmd = NULL;
            sm = CLI_SM_HOME;
            break;

        default:
            sm = CLI_SM_DONE;
            break;
    }
}

bool CLI::cliArgsParse(void) {
    if (args->extractArgs(inputLine) > 0)
        return true;
    return false;
}

bool CLI::cliInputHadler(void) {
    bool rtn;
    uint8_t c;

    rtn = false;
    // Check if commandsTask is free to manage another command
    // and if a character is available, otherwise wait to next cycle
    if (this->copyInputInLocalBuffer(&c)) {
        if (c == '\t') {
            // Horizontal Tab. Completes command
            this->completeCommand();
        } else if (c == '\0') {
            // Null character
            // Special char '\0' do nothing
        } else if (c == '\n') {
            // Line feed
            // Special char '\n' do nothing
        } else if (c == '\r') {
            // Carriage return
            printf("\r\n");
            rtn = true;
        } else {
            // Any other character (printable or control), add it to the buffer and update screen console
            this->addCharAndUpdateConsole(c);
        }
    }
    return rtn;
}

bool CLI::copyInputInLocalBuffer(uint8_t *p) {

    // Read a normal character or the entire escape sequence
    while (consoleRead(p, 1) != 0) {
        if (escapeCount == 0 && *p != ESCAPE) {
            return true;
        } else {
            escapeSequence[escapeCount++] = *p;
            if (escapeCount > sizeof (escapeSequence)) {
                custom_memset(escapeSequence, '0', escapeCount);
                escapeCount = 0;
                return false;
            }
        }
    }

    if (escapeCount > 2) {
        if (memcmp(escapeSequence, escape_arrow_left, sizeof (escape_arrow_left)) == 0) {
            *p = ESCAPE_ARROW_LEFT;
        } else if (memcmp(escapeSequence, escape_arrow_right, sizeof (escape_arrow_right)) == 0) {
            *p = ESCAPE_ARROW_RIGHT;
        } else if (memcmp(escapeSequence, escape_arrow_up, sizeof (escape_arrow_up)) == 0) {
            *p = ESCAPE_ARROW_UP;
        } else if (memcmp(escapeSequence, escape_arrow_down, sizeof (escape_arrow_down)) == 0) {
            *p = ESCAPE_ARROW_DOWN;
        } else if (memcmp(escapeSequence, escape_home, sizeof (escape_home)) == 0) {
            *p = ESCAPE_HOME;
        } else if (memcmp(escapeSequence, escape_page_up, sizeof (escape_page_up)) == 0) {
            *p = ESCAPE_PAGE_UP;
        } else if (memcmp(escapeSequence, escape_page_down, sizeof (escape_page_down)) == 0) {
            *p = ESCAPE_PAGE_DOWN;
        } else if (memcmp(escapeSequence, escape_end, sizeof (escape_end)) == 0) {
            *p = ESCAPE_END;
        } else if (memcmp(escapeSequence, escape_del, sizeof (escape_del)) == 0) {
            *p = ESCAPE_DEL;
        } else {
            return false;
        }
        custom_memset(escapeSequence, '0', escapeCount);
        escapeCount = 0;
        return true;
    }
    return false;
}

uint8_t CLI::completeCommand(void) {

    char *p, *fileName;

    // If there is nothing written in the buffer do nothing.
    if (inputLineLength == 0)
        return 0;

    // If in the buffer there is a space, search a file, otherwise search a command
    if ((p = strrchr(inputLine, ' ')) == NULL) {
        p = inputLine;
        fileName = (char*) temporaryFileCommands;
    } else {
        // p pointer refer to "strrchr(cmd2, ' ')"
        p++;
        fileName = (char*) temporaryFileEntryList;
    }

    // Complete the command by searching into selected file
    return this->CliCompleteCommandSearchInFile(fileName, p);
}

void CLI::addCharAndUpdateConsole(uint8_t c) {

    int i;

    switch (c) {
        case ESCAPE_ARROW_LEFT:
            // Put the cursor to left; 0x1B 0x5B 0x44
            if (inputLineIndex > 0) {
                inputLineIndex--;
                this->printEscape(escape_arrow_left, 1);
            }
            break;

        case ESCAPE_ARROW_RIGHT:
            // Put the cursor to right; 0x1B 0x5B 0x43
            if (inputLineIndex < inputLineLength) {
                inputLineIndex++;
                this->printEscape(escape_arrow_right, 1);
            }
            break;

        case ESCAPE_HOME:
            this->printEscape(escape_arrow_left, inputLineIndex);
            inputLineIndex = 0;
            break;

        case ESCAPE_END:
            this->printEscape(escape_arrow_right, inputLineLength - inputLineIndex);
            inputLineIndex = inputLineLength;
            break;

        case ESCAPE_DEL:
            if (inputLineIndex < inputLineLength) {
                for (i = inputLineIndex; i < inputLineLength; i++) {
                    inputLine[i] = inputLine[i + 1];
                }
                // Sposto a destra di uno e cancello
                this->printEscape(escape_arrow_right, 1);
                this->printBackspace();

                // Decrementa la lunghezza del buffer in accordo al DEL
                inputLineLength--;

                // Stampa gli elementi che sono da i a len
                this->printFor(inputLine, inputLineIndex, inputLineLength);

                // Sposto a destra di uno e cancello
                this->printEscape(escape_arrow_right, 1);
                printBackspace();

                // Torna il curso in dietro di len - i posizioni
                this->printEscape(escape_arrow_left, inputLineLength - inputLineIndex);
            }
            break;

        case 0x7F:
            // Rimuove un elmento dal buffer
            if (inputLineIndex > 0) {
                inputLineIndex--;
                for (i = inputLineIndex; i < inputLineLength; i++) {
                    inputLine[i] = inputLine[i + 1];
                }
                printBackspace();
                inputLineLength--;

                // Stampa gli elementi che sono da i a len
                this->printFor(inputLine, inputLineIndex, inputLineLength);

                // Sposto a destra di uno e cancello
                this->printEscape(escape_arrow_right, 1);
                this->printBackspace();

                // Torna il curso in dietro di len - i posizioni
                this->printEscape(escape_arrow_left, inputLineLength - inputLineIndex);
            }

            break;

        case ESCAPE_ARROW_UP:
            if (lastCmd < nCmd) {
                lastCmd++;
                this->getLastCommandFromFile(lastCmd);
                if (config.console.echo)
                    printf("\r>%s", inputLine);
                this->printEscape(escape_clear_end_row, 1);
            }
            break;

        case ESCAPE_ARROW_DOWN:
            if (lastCmd > 1) {
                lastCmd--;
                this->getLastCommandFromFile(lastCmd);
                if (config.console.echo)
                    printf("\r>%s", inputLine);
                this->printEscape(escape_clear_end_row, 1);
            } else if (lastCmd == 1) {
                inputLine[0] = '\0';
                lastCmd = inputLineIndex = inputLineLength = 0;
                if (config.console.echo)
                    printf("\r>%s", inputLine);
                this->printEscape(escape_clear_end_row, 1);
            }
            break;

        default:
            // Other printable character
            if (inputLineLength < CLI_MAX_BUF_SIZE && inputLineIndex < CLI_MAX_BUF_SIZE) {
                // Sposta gli elementi all'interno del buffer
                if (inputLineIndex < inputLineLength) {
                    for (i = inputLineLength; i > inputLineIndex; i--)
                        inputLine[i] = inputLine[i - 1];
                }
                // Aggiunge il carattere ricevuto al buffer e lo stampa
                inputLine[inputLineIndex++] = c;
                inputLine[++inputLineLength] = '\0';
                if (config.console.echo)
                    putc(c);

                // Stampa gli elementi che sono da i a len
                this->printFor(inputLine, inputLineIndex, inputLineLength);
                // Torna il curso in dietro di len - i posizioni
                this->printEscape(escape_arrow_left, inputLineLength - inputLineIndex);
            }
            break;
    }
}

void CLI::clearCommand(void) {
    //cmd2[0] = '\0';
    inputLineLength = inputLineIndex = 0;
}

void CLI::reprintConsole(void) {
    this->printEscape(escape_arrow_left, inputLineLength - inputLineIndex);
    if (inputLineLength)
        printf(">%s", inputLine);
    else
        putc('>');
}

void CLI::CliAddStringAndUpdateConsole(char *str) {
    while (*str != '\0')
        this->addCharAndUpdateConsole(*str++);
}

void CLI::printEscape(const char *p, int i) {
    while (i--)
        printf((char*) p);
}

void CLI::printBackspace(void) {
    char c = 0x7F;
    putc(c);
}

void CLI::printFor(char *p, int i, int len) {
    // Stampa gli elementi che sono da i a len
    while (i < len)
        putc(p[i++]);
}

bool CLI::searchExecutableCommand(char *name) {

    int len;
    std::list<CommandBase*>::iterator it;

    len = strlen(name);
    for (it = commandList.begin(); it != commandList.end(); it++)
        if (len == (*it)->getCommandNameLength() && strcmp(name, (*it)->getCommandName()) == 0) {
            cmd = *it;
            return true;
        }
    return false;
}

bool CLI::createFileListOfCommands(void) {

    FIL *fp;
    FRESULT fres;
    bool rtn;
    UINT len, written;
    std::list<CommandBase*>::iterator it;

    // Allocate enough space for FIL structure
    fp = NULL;
    fp = (FIL*) custom_malloc((void*) fp, sizeof (FIL));

    rtn = true;
    // Create a temporary file where will create a list of commands
    if ((fres = f_open(fp, temporaryFileCommands, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
        // Changes the properties of the temporary file to hide it
        if ((fres = f_chmod(temporaryFileCommands, AM_HID, AM_HID)) == FR_OK) {
            // Prints all commands in the file
            for (it = commandList.begin(); it != commandList.end(); it++) {
                len = custom_strlen((char*) (*it)->getCommandName());
                fres = f_write(fp, (*it)->getCommandName(), len, &written);
                fres = f_write(fp, "\n", 1, &written);
                //f_printf(fp, "%s\n", it->getCommandName());
            }
        } else {
            verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileCommands);
            rtn = false;
        }
        // Close the temporary file
        if ((fres = f_close(fp)) != FR_OK) {
            // Unable to close the temporary file
            verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileCommands);
            rtn = false;
        }
    } else {
        // Unable to create temporary file.
        verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileCommands);
        rtn = false;
    }

    if (fp != NULL)
        custom_free((void**) &fp);

    return rtn;
}

bool CLI::createFileListOfFilesEntry(void) {

    FILINFO *finfo;
    FIL *fp;
    DIR *dir;
    FRESULT fres;
    UINT len, written;
    bool rtn;

    // Allocate enough space for FILINFO, FIL, and DIR structures
    finfo = NULL;
    fp = NULL;
    dir = NULL;
    fp = (FIL*) custom_malloc(fp, sizeof (FIL));

    // Create a temporary file where will create a list of files
    if ((fres = f_open(fp, temporaryFileEntryList, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
        // Changes the properties of the temporary file to hide it
        if ((fres = f_chmod(temporaryFileEntryList, AM_HID, AM_HID)) == FR_OK) {
            // Flush the hidden property
            if ((fres = f_sync(fp)) == FR_OK) {
                // Get the current directory name
                if ((fres = f_getcwd(tmp, CLI_MAX_DIR_SIZE)) == FR_OK) {
                    // Open the directory
                    dir = (DIR*) custom_malloc((void*) dir, sizeof (DIR));
                    finfo = (FILINFO*) custom_malloc((void*) finfo, sizeof (FILINFO));
                    if ((fres = f_opendir(dir, tmp)) == FR_OK) {
                        while (true) {
                            // Read a directory item
                            if ((fres = (f_readdir(dir, finfo))) != FR_OK) {
                                verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
                                break;
                            }
                            // Break on end of directory
                            if (finfo->fname[0] == 0)
                                break;
                            // Ignore dot entry
                            if (finfo->fname[0] == '.')
                                continue;

                            if (!(finfo->fattrib & AM_HID)) {
                                // It is a file print it in the hidden file
                                len = custom_strlen(finfo->fname);
                                fres = f_write(fp, finfo->fname, len, &written);
                                fres = f_write(fp, "\n", 1, &written);
                                //f_printf(fp, "%s\n", finfo->fname);
                            }
                        }
                        if ((fres = f_closedir(dir)) != FR_OK) {
                            // Unable to close directory
                            verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
                            rtn = false;
                        }
                    } else {
                        // Unable to open directory
                        verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
                        rtn = false;
                    }
                    // Try to close temporary file
                    if ((fres = f_close(fp)) == FR_OK) {
                        rtn = true;
                    } else {
                        // Unable to close the temporary file
                        verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
                        rtn = false;
                    }
                } else {
                    // Unable to get current directory
                    verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
                    rtn = false;
                }
            } else {
                // Unable to write property
                verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
                rtn = false;
            }
        } else {
            // Unable to change property
            verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
            rtn = false;
        }
    } else {
        // Unable to create temporary file
        verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileEntryList);
        rtn = false;
    }

    if (finfo != NULL)
        custom_free((void**) &finfo);
    if (fp != NULL)
        custom_free((void**) &fp);
    if (dir != NULL)
        custom_free((void**) &dir);

    return rtn;
}

uint8_t CLI::CliCompleteCommandSearchInFile(char *fileName, char *p) {

    FIL *fp;
    bool found, rtn;
    int len, occ, tmpLen;
    char match;
    FRESULT fres;

    fp = NULL;
    fp = (FIL*) custom_malloc(fp, sizeof (FIL));

    // Search method (Files and Commands) unified
    if ((fres = f_open(fp, fileName, FA_READ)) == FR_OK) {
        found = FALSE;
        do {
            occ = 0;
            ////        len = strlen(p);
            len = inputLineIndex - (p - inputLine);
            while (f_gets(tmp, sizeof (tmp), fp) != NULL) {
                tmpLen = strlen(tmp) - 1;
                if (tmp[tmpLen] == '\n')
                    tmp[tmpLen] = '\0';
                if (tmpLen >= len && strncmp(p, tmp, len) == 0) {
                    if (tmp[len] == '\0') {
                        occ = 0;
                        found = FALSE;
                        break;
                    }
                    if (occ == 0) {
                        match = tmp[len];
                        occ++;
                        found = TRUE;
                    } else {
                        if (match != tmp[len]) {
                            found = FALSE;
                        }
                    }
                }
            }
            if (found && occ > 0)
                this->addCharAndUpdateConsole(match);
            if ((fres = f_lseek(fp, 0l)) != FR_OK)
                verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), fileName);
        } while (found && occ != 0);

        if (occ != 0) {
            while (f_gets(tmp, sizeof (tmp), fp) != NULL) {
                if (strncmp(p, tmp, len) == 0) {
                    tmpLen = strlen(tmp) - 1;
                    if (tmp[tmpLen] == '\n')
                        tmp[tmpLen] = '\0';
                    printf("\r\n%s", tmp);
                }
            }
            printf("\r\n");
            reprintConsole();
        }
        if ((fres = f_close(fp)) != FR_OK)
            verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), fileName);
        return occ;
    } else {
        // Unable to open file
        verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), fileName);
        rtn = false;
    }
    if (fp != NULL)
        custom_free((void**) &fp);
    return rtn;
}

bool CLI::getLastCommandFromFile(int pos) {

    bool rtn;
    UINT read;
    FRESULT fres;

    // Place the file pointer at the end of the file
    if ((fres = f_lseek(fileLastCommands, f_size(fileLastCommands) - (CLI_MAX_BUF_SIZE * pos))) == FR_OK) {
        // read last command
        if ((fres = f_read(fileLastCommands, inputLine, CLI_MAX_BUF_SIZE, &read)) == FR_OK) {
            // Clear all indicator
            inputLineLength = inputLineIndex = custom_strlen(inputLine);
            rtn = true;
        } else {
            // Unable to read
            verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
            rtn = false;
        }
    } else {
        // Unable to move pointer 
        verbosePrintf(VER_DBG, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
        rtn = false;
    }
    return rtn;
}

void CLI::putLastCommandInFile(void) {

    UINT writed;
    FRESULT fres;

    // Place the file pointer at the end of the file
    if ((fres = f_lseek(fileLastCommands, f_size(fileLastCommands))) == FR_OK) {
        //for (i = inputLineLength; i < CLI_MAX_BUF_SIZE; i++)
        //    inputLine[i] = '\0';
        custom_memset(&inputLine[inputLineLength], '\0', CLI_MAX_BUF_SIZE - inputLineLength);
        // Write into the file last command
        if ((fres = f_write(fileLastCommands, inputLine, CLI_MAX_BUF_SIZE, &writed)) == FR_OK) {
            // Synchronize the content of the file on the micro SD
            if ((fres = f_sync(fileLastCommands)) == FR_OK) {
                nCmd++;
                lastCmd = 0;
            } else {
                verbosePrintf(VER_ERR, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
            }
        } else {
            verbosePrintf(VER_ERR, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
        }
    } else {
        verbosePrintf(VER_ERR, "Error %s with %s", string_rc(fres), temporaryFileLatterCommands);
    }
}

CLI::~CLI(void) {

}

