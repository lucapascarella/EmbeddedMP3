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
#include "Utilities/AsyncTimer.h"
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
    numberOfCommands = lastCommand = 0;

    custom_memset(inputLine, '\0', sizeof (inputLine));
    inputLineLength = inputLineIndex = 0;

    args = NULL;

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
                this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
            }
        } else {
            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
        }
    } else {
        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
    }

    // Flush console content
    this->reprintConsoleNew();
}

void CLI::registerCommand(CommandBase *cb) {
    commandList.push_back(cb);
}

void CLI::cliTaskHadler(void) {
    int rtn;
    char *name;
    uint16_t read;
    uint8_t inputBuffer[16];

    switch (sm) {
        case CLI_SM_HOME:
            // Initialize stuff here 
            sm = CLI_SM_WAIT_INPUT;
            break;

        case CLI_SM_WAIT_INPUT:
            // Get a copy input until the newline is reached
            if ((read = consoleRead(inputBuffer, sizeof (inputBuffer))) > 0) {
                if (this->addByteAndUpdateConsole(inputBuffer, read))
                    sm = CLI_SM_ARGS_PARSER;
            }
            break;

        case CLI_SM_ARGS_PARSER:
            // Create Arguments parser object
            args = new ArgsParser();
            // Try to extract arguments
            if (args->extractArgs(inputLine) > 0) {
                sm = CLI_SM_FIND_COMMAND;
            } else {
                sm = CLI_SM_DONE;
            }
            break;

        case CLI_SM_FIND_COMMAND:
            // Find a command matching first argument (the name of the program)
            name = args->getArgPointer(0);
            if (this->setExecutableCommand(name)) {
                // It is a command, add to history list
                this->putLastCommandInFile();
                GpioUpdateOutputState(GPIO_BIT_CMD_OK);
                sm = CLI_SM_COMMAND_TASK;
            } else {
                GpioUpdateOutputState(GPIO_BIT_CMD_ERR);
                // The command was not found
                this->verbosePrintfWrapper(VER_MIN, false, "\r\nCommand '%s' not found!", name);
                sm = CLI_SM_DONE;
            }
            break;

        case CLI_SM_COMMAND_TASK:
            // Execute the command until 0 is returned
            rtn = cmd->taskCommand(args);
            if (rtn < 0) {
                // Some error happened
                this->verbosePrintfWrapper(VER_MIN, false, "\r\nCommand '%s' terminated with error code: %d", cmd->getCommandName(), rtn);
                sm = CLI_SM_DONE;
            } else if (rtn > 0) {
                // Still working
            } else {
                // Command finished successful
                sm = CLI_SM_DONE;
            }
            break;

        case CLI_SM_DONE:
            // De-initialize stuff here
            args->~ArgsParser();
            cmd = NULL;
            this->clearCommand();
            this->reprintConsoleNew();
            sm = CLI_SM_HOME;
            break;

        default:
            sm = CLI_SM_DONE;
            break;
    }
}

bool CLI::addByteAndUpdateConsole(uint8_t *pbuf, uint16_t len) {

    int occ;
    uint8_t c;
    int i, j;

    j = 0;
    do {
        c = pbuf[j++];
        switch (c) {
            case ESCAPE:
                custom_memset(escapeSequence, '\0', sizeof (escapeSequence));
                escapeCount = 0;
                escapeSequence[escapeCount++] = c;
                break;

            case ESCAPE_ARROW_LEFT:
                // Put the cursor to left; 0x1B 0x5B 0x44
                if (inputLineIndex > 0) {
                    inputLineIndex--;
                    this->printEscapeSequence(escape_arrow_left, 1);
                }
                break;

            case ESCAPE_ARROW_RIGHT:
                // Put the cursor to right; 0x1B 0x5B 0x43
                if (inputLineIndex < inputLineLength) {
                    inputLineIndex++;
                    this->printEscapeSequence(escape_arrow_right, 1);
                }
                break;

            case ESCAPE_HOME:
                this->printEscapeSequence(escape_arrow_left, inputLineIndex);
                inputLineIndex = 0;
                break;

            case ESCAPE_END:
                this->printEscapeSequence(escape_arrow_right, inputLineLength - inputLineIndex);
                inputLineIndex = inputLineLength;
                break;

            case ESCAPE_DEL:
                if (inputLineIndex < inputLineLength) {
                    inputLineLength--;
                    for (i = inputLineIndex; i < inputLineLength; i++) {
                        inputLine[i] = inputLine[i + 1];
                    }
                    inputLine[inputLineLength] = '\0';
                    // Sposto a destra di uno e cancello
                    this->printEscapeSequence(escape_arrow_right, 1);
                    this->printBackspace();
                    // Stampa gli elementi che sono da i a len
                    consolePrint(&inputLine[inputLineIndex], inputLineLength - inputLineIndex);
                    //this->printString(inputLine, inputLineIndex, inputLineLength);
                    // Sposto a destra di uno e cancello
                    this->printEscapeSequence(escape_arrow_right, 1);
                    this->printBackspace();
                    // Torna il curso in dietro di len - i posizioni
                    this->printEscapeSequence(escape_arrow_left, inputLineLength - inputLineIndex);
                }
                break;

            case 0x08: // Backspace
            case 0x7F:
                // Left remove 
                if (inputLineIndex > 0) {
                    inputLineIndex--;
                    inputLineLength--;
                    for (i = inputLineIndex; i < inputLineLength; i++) {
                        inputLine[i] = inputLine[i + 1];
                    }
                    inputLine[inputLineLength] = '\0';
                    this->printBackspace();
                    // Print characters on the right (if present)
                    consolePrint(&inputLine[inputLineIndex], inputLineLength - inputLineIndex);
                    // Move the cursor on the right and delete
                    this->printEscapeSequence(escape_arrow_right, 1);
                    this->printBackspace();
                    // Move back the cursor of (length - index) positions
                    this->printEscapeSequence(escape_arrow_left, inputLineLength - inputLineIndex);
                }
                break;

            case ESCAPE_ARROW_UP:
                if (lastCommand < numberOfCommands) {
                    lastCommand++;
                    this->getLastCommandFromFile(lastCommand);
                    if (config.console.echo)
                        printf("\r>%s", inputLine);
                    this->printEscapeSequence(escape_clear_end_row, 1);
                }
                break;

            case ESCAPE_ARROW_DOWN:
                if (lastCommand > 1) {
                    lastCommand--;
                    this->getLastCommandFromFile(lastCommand);
                    if (config.console.echo)
                        printf("\r>%s", inputLine);
                    this->printEscapeSequence(escape_clear_end_row, 1);
                } else if (lastCommand == 1) {
                    inputLine[0] = '\0';
                    lastCommand = inputLineIndex = inputLineLength = 0;
                    if (config.console.echo)
                        printf("\r>%s", inputLine);
                    this->printEscapeSequence(escape_clear_end_row, 1);
                }
                break;

            case '\t':
                // Horizontal Tab. Completes command
                occ = this->completeCommand();
                this->verbosePrintfWrapper(VER_DBG, false, "\r\nOccurrences %d", occ);
                break;

            case '\0':
                // Null character
                // Special char '\0' do nothing
                this->verbosePrintfWrapper(VER_DBG, false, "\r\nException '\0'");
                break;

            case '\n':
                // Line feed
                // Special char '\n' do nothing
                this->verbosePrintfWrapper(VER_DBG, false, "\r\nException '\n'");
                break;

            case'\r':
                // Carriage return execute command
                //consolePrint((uint8_t*) "\r\n", 2);
                return true;
                break;

            default:
                if (escapeCount > 0) {
                    escapeSequence[escapeCount++] = c;
                    if (this->returnEscapeInternalCharacter(&c)) {
                        // The escape sequence is converted in a internal single character escape, reprocess it
                        pbuf[--j] = c;
                        escapeCount = 0;
                    }
                } else {
                    // Add a printable character to the inputLine buffer
                    if (inputLineLength < CLI_INPUT_LINE_SIZE && inputLineIndex < CLI_INPUT_LINE_SIZE) {
                        // If the buffer index is different from buffer length, shift characters to right
                        for (i = inputLineLength; i > inputLineIndex; i--)
                            inputLine[i] = inputLine[i - 1];
                        // Add the received character in the free slot
                        inputLine[inputLineIndex++] = c;
                        inputLine[++inputLineLength] = '\0';
                        // Update console
                        if (config.console.echo) {
                            // Print the char
                            consolePrint(&c, 1);
                            // Print remaining characters
                            consolePrint(&inputLine[inputLineIndex], inputLineLength - inputLineIndex);
                            // Move back the cursor of (length - index) positions
                            this->printEscapeSequence(escape_arrow_left, inputLineLength - inputLineIndex);
                        }
                    } else {
                        this->verbosePrintfWrapper(VER_ERR, true, "\r\nThe input is too long");
                    }
                }
                break;
        }
    } while (j < len);
    return false;
}

bool CLI::returnEscapeInternalCharacter(uint8_t *c) {
    bool rtn = true;
    if (escapeCount == sizeof (escape_arrow_left) - 1 && memcmp(escapeSequence, escape_arrow_left, sizeof (escape_arrow_left) - 1) == 0) {
        *c = ESCAPE_ARROW_LEFT;
    } else if (escapeCount == sizeof (escape_arrow_right) - 1 && memcmp(escapeSequence, escape_arrow_right, sizeof (escape_arrow_right) - 1) == 0) {
        *c = ESCAPE_ARROW_RIGHT;
    } else if (escapeCount == sizeof (escape_arrow_up) - 1 && memcmp(escapeSequence, escape_arrow_up, sizeof (escape_arrow_up) - 1) == 0) {
        *c = ESCAPE_ARROW_UP;
    } else if (escapeCount == sizeof (escape_arrow_down) - 1 && memcmp(escapeSequence, escape_arrow_down, sizeof (escape_arrow_down) - 1) == 0) {
        *c = ESCAPE_ARROW_DOWN;
    } else if (escapeCount == sizeof (escape_home) - 1 && memcmp(escapeSequence, escape_home, sizeof (escape_home) - 1) == 0) {
        *c = ESCAPE_HOME;
    } else if (escapeCount == sizeof (escape_page_up) - 1 && memcmp(escapeSequence, escape_page_up, sizeof (escape_page_up) - 1) == 0) {
        *c = ESCAPE_PAGE_UP;
    } else if (escapeCount == sizeof (escape_page_down) - 1 && memcmp(escapeSequence, escape_page_down, sizeof (escape_page_down) - 1) == 0) {
        *c = ESCAPE_PAGE_DOWN;
    } else if (escapeCount == sizeof (escape_end) - 1 && memcmp(escapeSequence, escape_end, sizeof (escape_end) - 1) == 0) {
        *c = ESCAPE_END;
    } else if (escapeCount == sizeof (escape_del) - 1 && memcmp(escapeSequence, escape_del, sizeof (escape_del) - 1) == 0) {
        *c = ESCAPE_DEL;
    } else {
        rtn = false;
    }
    return rtn;
}

int CLI::completeCommand(void) {

    int rtn, i;
    uint8_t *p;

    // If there is nothing written in the buffer do nothing.
    if (inputLineLength == 0)
        return 0;

    // If in the buffer there is a space, search a file, otherwise search a command
    if ((p = (uint8_t*) custom_memchr(inputLine, ' ', inputLineIndex)) == NULL) {
        p = inputLine;
        // Complete the command by searching into selected file
        rtn = this->completeCommandSearchingInFile(temporaryFileCommands, p, &inputLineIndex);
    } else {
        for (i = inputLineIndex; inputLine[i] != ' ' && i >= 0; i--);
        p = &inputLine[++i];
        rtn = this->completeCommandSearchingInFile(temporaryFileEntryList, p, &inputLineIndex);
    }
    return rtn;
}

void CLI::clearCommand(void) {
    custom_memset(inputLine, '\0', CLI_INPUT_LINE_SIZE);
    inputLineLength = inputLineIndex = 0;
}

void CLI::reprintConsoleNew(void) {
    if (inputLineLength > 0)
        printf("\r\n>%s", inputLine);
    else
        printf("\r\n>");
    this->printEscapeSequence(escape_arrow_left, inputLineLength - inputLineIndex);
}

void CLI::printEscapeSequence(const char *p, int i) {
    while (i--)
        printf((char*) p);
}

void CLI::printBackspace(void) {
    uint8_t c = 0x7F;
    //uint8_t c = 0x08; // ASCII Backspace
    consolePrint(&c, 1);
}

bool CLI::setExecutableCommand(char *name) {

    int len;
    std::list<CommandBase*>::iterator it;

    if ((len = custom_strlen(name)) == 0)
        return false;
    for (it = commandList.begin(); it != commandList.end(); it++)
        if (len == (*it)->getCommandNameLength() && memcmp(name, (*it)->getCommandName(), len) == 0) {
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
            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileCommands);
            rtn = false;
        }
        // Close the temporary file
        if ((fres = f_close(fp)) != FR_OK) {
            // Unable to close the temporary file
            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileCommands);
            rtn = false;
        }
    } else {
        // Unable to create temporary file.
        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileCommands);
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
    char *buf;

    // Allocate enough space for FILINFO, FIL, and DIR structures
    finfo = NULL;
    fp = NULL;
    dir = NULL;
    buf = NULL;
    fp = (FIL*) custom_malloc(fp, sizeof (FIL));
    buf = (char*) custom_malloc(buf, CLI_DIR_SIZE);

    // Create a temporary file where will create a list of files
    if ((fres = f_open(fp, temporaryFileEntryList, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
        // Changes the properties of the temporary file to hide it
        if ((fres = f_chmod(temporaryFileEntryList, AM_HID, AM_HID)) == FR_OK) {
            // Flush the hidden property
            if ((fres = f_sync(fp)) == FR_OK) {
                // Get the current directory name
                if ((fres = f_getcwd(buf, CLI_DIR_SIZE)) == FR_OK) {
                    // Open the directory
                    dir = (DIR*) custom_malloc((void*) dir, sizeof (DIR));
                    finfo = (FILINFO*) custom_malloc((void*) finfo, sizeof (FILINFO));
                    if ((fres = f_opendir(dir, buf)) == FR_OK) {
                        while (true) {
                            // Read a directory item
                            if ((fres = (f_readdir(dir, finfo))) != FR_OK) {
                                this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
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
                            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
                            rtn = false;
                        }
                    } else {
                        // Unable to open directory
                        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
                        rtn = false;
                    }
                    // Try to close temporary file
                    if ((fres = f_close(fp)) == FR_OK) {
                        rtn = true;
                    } else {
                        // Unable to close the temporary file
                        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
                        rtn = false;
                    }
                } else {
                    // Unable to get current directory
                    this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
                    rtn = false;
                }
            } else {
                // Unable to write property
                this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
                rtn = false;
            }
        } else {
            // Unable to change property
            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
            rtn = false;
        }
    } else {
        // Unable to create temporary file
        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileEntryList);
        rtn = false;
    }

    if (finfo != NULL)
        custom_free((void**) &finfo);
    if (fp != NULL)
        custom_free((void**) &fp);
    if (dir != NULL)
        custom_free((void**) &dir);
    if (buf != NULL)
        custom_free((void**) &buf);

    return rtn;
}

int CLI::completeCommandSearchingInFile(const char *fileName, uint8_t *p, int *index) {

    FIL *fp;
    bool found;
    uint16_t rtn;
    int size, inputLength, occ, fileLength;
    uint8_t *l, match, *buf;
    FRESULT fres;

    rtn = 0;
    fp = NULL;
    fp = (FIL*) custom_malloc(fp, sizeof (FIL));
    buf = NULL;
    size = CLI_INPUT_LINE_SIZE - inputLineLength;
    buf = (uint8_t*) custom_malloc(buf, size);

    // Search method (Files and Commands) unified
    if ((fres = f_open(fp, fileName, FA_READ)) == FR_OK) {
        do {
            found = false;
            occ = 0;
            match = '\0';
            inputLength = *index - (p - inputLine);
            while (f_gets((TCHAR*) buf, size, fp) != NULL) {
                if ((l = (uint8_t *) custom_memchr(buf, '\n', size)) != NULL) {
                    fileLength = l - buf;
                    *l = '\0';
                    if (fileLength >= inputLength && strncmp((const char*) p, (const char*) buf, inputLength) == 0) {
                        if (match == '\0' && found == false) {
                            match = buf[inputLength];
                            found = true;
                            occ++;
                        } else {
                            occ++;
                            if (match != buf[inputLength])
                                found = false;
                        }
                    }
                } else {
                    this->verbosePrintfWrapper(VER_DBG, false, "\r\nException 0");
                    found = false;
                    break;
                }
            }
            if (match != '\0' && found == true)
                this->addByteAndUpdateConsole((uint8_t*) & match, 1);
            if ((fres = f_lseek(fp, 0l)) != FR_OK)
                this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), fileName);
        } while (match != '\0' && found == true);

        if (occ > 1) {
            while (f_gets((TCHAR*) buf, size, fp) != NULL) {
                if (strncmp((const char*) p, (const char*) buf, inputLength) == 0) {
                    fileLength = custom_strlen((char*) buf) - 1;
                    if (buf[fileLength] == '\n')
                        buf[fileLength] = '\0';
                    printf("\r\n%s", buf);
                }
            }
            reprintConsoleNew();
        } else if (found == true) {
            match = ' ';
            this->addByteAndUpdateConsole((uint8_t*) & match, 1);
        }
        if ((fres = f_close(fp)) != FR_OK)
            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), fileName);
        rtn = occ;
    } else {
        // Unable to open file
        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), fileName);
        rtn = -1;
    }
    if (fp != NULL)
        custom_free((void**) &fp);
    if (buf != NULL)
        custom_free((void**) &buf);
    return rtn;
}

bool CLI::getLastCommandFromFile(int pos) {

    bool rtn;
    UINT read;
    FRESULT fres;

    // Place the file pointer at the end of the file
    if ((fres = f_lseek(fileLastCommands, f_size(fileLastCommands) - (CLI_INPUT_LINE_SIZE * pos))) == FR_OK) {
        // read last command
        if ((fres = f_read(fileLastCommands, inputLine, CLI_INPUT_LINE_SIZE, &read)) == FR_OK) {
            // Clear all indicator
            inputLineLength = inputLineIndex = custom_strlen((char*) inputLine);
            rtn = true;
        } else {
            // Unable to read
            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
            rtn = false;
        }
    } else {
        // Unable to move pointer 
        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
        rtn = false;
    }
    return rtn;
}

void CLI::putLastCommandInFile(void) {

    UINT writed;
    FRESULT fres;

    // Place the file pointer at the end of the file
    if ((fres = f_lseek(fileLastCommands, f_size(fileLastCommands))) == FR_OK) {
        custom_memset(&inputLine[inputLineLength], '\0', CLI_INPUT_LINE_SIZE - inputLineLength);
        // Write into the file last command
        if ((fres = f_write(fileLastCommands, inputLine, CLI_INPUT_LINE_SIZE, &writed)) == FR_OK) {
            // Synchronize the content of the file on the micro SD
            if ((fres = f_sync(fileLastCommands)) == FR_OK) {
                numberOfCommands++;
                lastCommand = 0;
            } else {
                this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
            }
        } else {
            this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
        }
    } else {
        this->verbosePrintfWrapper(VER_ERR, true, "\r\nError %s with %s", string_rc(fres), temporaryFileLatterCommands);
    }
}

int CLI::verbosePrintfWrapper(int level, bool reprint, const char * fmt, ...) {
    va_list ap;
    int sent = 0;

    if (level <= config.console.verbose) {
        //sent = consolePrint((uint8_t*) "\r\n", 2);
        va_start(ap, fmt);
        sent += verbosePrintfVaList(level, fmt, ap);
        va_end(ap);
        if (reprint)
            this->reprintConsoleNew();
    }
    return sent;
}

CLI::~CLI(void) {

}

