/* 
 * File:   CLI.cpp
 * Author: luca
 * 
 * Created on April 11, 2017, 12:53 PM
 */

#include "CLI.hpp"
#include "Utilities/printer.h"
#include "Utilities/Utilities.h"
#include "Commands/CommandBase.h"
#include "Utilities/ArgsParser.h"
#include "Utilities/GPIO.h"
#include "Utilities/Config.h"
#include <iterator>

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

    cmd = NULL;
    custom_memset(inputLine, '\0', sizeof (inputLine));
    args = new ArgsParser();
    sm = CLI_SM_HOME;

    custom_memset(escapeSequence, '0', countEscape);
    countEscape = 0;

    //    // Create two hidden files with the list of commands and entry list of current directory
    //    this->CliCreateFileListOfCommands();
    //    this->CliCreateFileListOfFilesEntry();
}

void CLI::registerCommand(CommandBase &cb) {
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
            if (this->searchCommand(name)) {
                // It is a command, print it
                this->putLastCommandInFile();
                GpioUpdateOutputState(GPIO_BIT_CMD_OK);
                sm = CLI_SM_COMMAND_TASK;
            } else {
                ////                GpioUpdateOutputState(GPIO_BIT_CMD_ERR);
                ////                // The command was not found
                ////                this->commandNotFound(cmd2);
                ////                // Clear the command buffer and reprint the console indicator
                ////                this->clearCommand();
                sm = CLI_SM_DONE;
            }
            break;

        case CLI_SM_COMMAND_TASK:
            // Execute the command until 0 is returned
            if ((rtn = cmd->taskCommand(args)) == 0)
                sm = CLI_SM_DONE;
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
    if (this->copyInputs(&c)) {
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

bool CLI::copyInputs(uint8_t *p) {

    // Read a normal character or the entire escape sequence
    while (consoleRead(p, 1) != 0) {
        if (countEscape == 0 && *p != ESCAPE) {
            return true;
        } else {
            escapeSequence[countEscape++] = *p;
            if (countEscape > sizeof (escapeSequence)) {
                custom_memset(escapeSequence, '0', countEscape);
                countEscape = 0;
                return false;
            }
        }
    }

    if (countEscape > 2) {
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
        custom_memset(escapeSequence, '0', countEscape);
        countEscape = 0;
        return true;
    }
    return false;
}

uint8_t CLI::completeCommand(void) {

//    char *p, *fileName;
//
//    // If there is nothing written in the buffer do nothing.
//    if (cmdlen == 0)
//        return 0;
//
//    // If in the buffer there is a space, search a file, otherwise search a command
//    if ((p = strrchr(cmd2, ' ')) == NULL) {
//        p = cmd2;
//        fileName = (char*) temporaryFileCommands;
//    } else {
//        // p pointer refer to "strrchr(cmd2, ' ')"
//        p++;
//        fileName = (char*) temporaryFileEntryList;
//    }
//
//    // Complete the command by searching into selected file
//    return CliCompleteCommandSearchInFile(fileName, p);
}

void CLI::addCharAndUpdateConsole(uint8_t c) {

    int i;

    switch (c) {

        case ESCAPE_ARROW_LEFT:
            // Put the cursor to left; 0x1B 0x5B 0x44
            if (cmdi > 0) {
                cmdi--;
                this->CliPrintEscape(escape_arrow_left, 1);
            }
            break;

        case ESCAPE_ARROW_RIGHT:
            // Put the cursor to right; 0x1B 0x5B 0x43
            if (cmdi < cmdlen) {
                cmdi++;
                this->CliPrintEscape(escape_arrow_right, 1);
            }
            break;

        case ESCAPE_HOME:

            this->CliPrintEscape(escape_arrow_left, cmdi);
            cmdi = 0;
            break;

        case ESCAPE_END:

            this->CliPrintEscape(escape_arrow_right, cmdlen - cmdi);
            cmdi = cmdlen;
            break;

        case ESCAPE_DEL:

            if (cmdi < cmdlen) {
                for (i = cmdi; i < cmdlen; i++) {
                    cmd2[i] = cmd2[i + 1];
                }
                // Sposto a destra di uno e cancello
                this->CliPrintEscape(escape_arrow_right, 1);
                this->CliPrintBackspace();

                // Decrementa la lunghezza del buffer in accordo al DEL
                cmdlen--;

                // Stampa gli elementi che sono da i a len
                this->CliPrintFor(cmd2, cmdi, cmdlen);

                // Sposto a destra di uno e cancello
                this->CliPrintEscape(escape_arrow_right, 1);
                CliPrintBackspace();

                // Torna il curso in dietro di len - i posizioni
                this->CliPrintEscape(escape_arrow_left, cmdlen - cmdi);
            }
            break;

        case 0x7F:

            // Rimuovd un elmento dal buffer
            if (cmdi > 0) {
                cmdi--;
                for (i = cmdi; i < cmdlen; i++) {
                    cmd2[i] = cmd2[i + 1];
                }
                CliPrintBackspace();
                cmdlen--;

                // Stampa gli elementi che sono da i a len
                this->CliPrintFor(cmd2, cmdi, cmdlen);

                // Sposto a destra di uno e cancello
                this->CliPrintEscape(escape_arrow_right, 1);
                this->CliPrintBackspace();

                // Torna il curso in dietro di len - i posizioni
                this->CliPrintEscape(escape_arrow_left, cmdlen - cmdi);
            }

            break;

        case ESCAPE_ARROW_UP:

            if (lastCmd < nCmd) {
                lastCmd++;
                this->getLastCommandFromFile(lastCmd);
                if (config.console.echo)
                    printf("\r>%s", cmd2);
                this->CliPrintEscape(escape_clear_end_row, 1);
            }
            break;

        case ESCAPE_ARROW_DOWN:

            if (lastCmd > 1) {
                lastCmd--;
                this->getLastCommandFromFile(lastCmd);
                if (config.console.echo)
                    printf("\r>%s", cmd2);
                this->CliPrintEscape(escape_clear_end_row, 1);
            } else if (lastCmd == 1) {
                cmd2[0] = '\0';
                lastCmd = cmdi = cmdlen = 0;
                if (config.console.echo)
                    printf("\r>%s", cmd2);
                this->CliPrintEscape(escape_clear_end_row, 1);
            }
            break;

        default:
            // Other printable character
            if (cmdlen < CLI_MAX_BUF_SIZE && cmdi < CLI_MAX_BUF_SIZE) {

                // Sposta gli elementi all'interno del buffer
                if (cmdi < cmdlen) {
                    for (i = cmdlen; i > cmdi; i--)
                        cmd2[i] = cmd2[i - 1];
                }

                // Aggiunge il carattere ricevuto al buffer e lo stampa
                cmd2[cmdi++] = c;
                cmd2[++cmdlen] = '\0';
                if (config.console.echo)
                    putc(c);

                // Stampa gli elementi che sono da i a len
                this->CliPrintFor(cmd2, cmdi, cmdlen);

                // Torna il curso in dietro di len - i posizioni
                this->CliPrintEscape(escape_arrow_left, cmdlen - cmdi);
            }
            break;
    }
}

void CLI::CliClearCommand(void) {
    //cmd2[0] = '\0';
    cmdlen = cmdi = 0;
}

void CLI::CliReprintConsole(void) {
    this->CliPrintEscape(escape_arrow_left, cmdlen - cmdi);
    if (cmdlen)
        printf(">%s", cmd2);
    else
        putc('>');
}

bool CLI::CliAddStringAndUpdateConsole(char *str) {
    while (*str != '\0')
        this->addCharAndUpdateConsole(*str++);
}

void CLI::CliPrintEscape(const char *p, int i) {
    while (i--)
        printf((char*) p);
}

void CLI::CliPrintBackspace(void) {
    char c = 0x7F;
    putc(c);
}

void CLI::CliPrintFor(char *p, int i, int len) {
    // Stampa gli elementi che sono da i a len
    while (i < len)
        putc(p[i++]);
}

bool CLI::searchCommand(char *name) {

    int len;
    std::list<CommandBase>::iterator it;

    len = strlen(name);
    for (it = commandList.begin(); it != commandList.end(); it++)
        if (len == it->getCommandNameLength() && strcmp(name, it->getCommandName()) == 0) {
            cmd = &(*it);
            return true;
        }
    return false;
}


//bool CLI::CliCreateFileListOfCommands(void) {
//
//    FIL *fp;
//    FRESULT fres;
//    int i;
//    bool rtn;
//
//    // Allocate enough space for FIL structure
//    custom_malloc((void**) fp, sizeof (FIL));
//
//    rtn = true;
//    // Create a temporary file where will create a list of commands
//    if ((fres = f_open(fp, temporaryFileCommands, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
//        // Changes the properties of the temporary file to hide it
//        if ((fres = f_chmod(temporaryFileCommands, AM_HID, AM_HID)) == FR_OK) {
//            // Prints all commands in the file
//            for (i = 0; i < numberOfCommands; i++) {
//                f_printf(fp, "%s\n", commands[i].name);
//            }
//        } else {
//            verbosePrintf("Error: %s", string_rc(fres));
//            rtn = false;
//        }
//        // Close the temporary file
//        if ((fres = f_close(fp)) != FR_OK) {
//            // Unable to close the temporary file
//            verbosePrintf("Error: %s", string_rc(fres));
//            rtn = false;
//        }
//    } else {
//        // Unable to create temporary file.
//        verbosePrintf("Error: %s", string_rc(fres));
//        rtn = false;
//    }
//
//    custom_free((void**) &fp);
//
//    return rtn;
//}
//
//bool CLI::CliCreateFileListOfFilesEntry(void) {
//
//    FILINFO *finfo;
//    FIL *fp;
//    DIR *dir;
//    FRESULT fres;
//
//    // Allocate enough space for FILINFO, FIL, and DIR structures
//    custom_malloc((void**) finfo, sizeof (FILINFO));
//    custom_malloc((void**) fp, sizeof (FIL));
//    custom_malloc((void**) dir, sizeof (DIR));
//
//    // Create a temporary file where will create a list of files
//    if ((fres = f_open(fp, temporaryFileEntryList, FA_WRITE | FA_CREATE_ALWAYS)) == FR_OK) {
//        // Changes the properties of the temporary file to hide it
//        put_rc(f_chmod(temporaryFileEntryList, AM_HID, AM_HID));
//
//        // Flush the hidden property
//        put_rc(f_sync(fp));
//
//        // Get the current directory name
//        put_rc(f_getcwd(tmp, CLI_MAX_DIR_SIZE));
//
//        // Open the directory
//        if (put_rc(f_opendir(&dir, tmp))) {
//
//            while (TRUE) {
//                // Read a directory item
//                if (!put_rc(f_readdir(&dir, &finfo)))
//                    break;
//                // Break on end of dir
//                if (finfo.fname[0] == 0)
//                    break;
//                // Ignore dot entry
//                if (finfo.fname[0] == '.')
//                    continue;
//
//                if (!(finfo.fattrib & AM_HID)) {
//                    // It is a file
//                    f_printf(fp, "%s\n", GetFileNamePointer(&finfo));
//                }
//            }
//            put_rc(f_closedir(&dir));
//        }
//
//        // Try to close temporary file
//        if (!put_rc(f_close(fp))) {
//            // Unable to close the temporary file
//            return FALSE;
//        }
//        return TRUE;
//
//    } else {
//        // Unable to create temporary file.
//        verbosePrintf("Error: %s", string_rc(fres));
//        return FALSE;
//    }
//}

void CLI::getLastCommandFromFile(int pos) {

//    int read;
//
//    // Place the file pointer at the end of the file
//    put_rc(f_lseek(&fileLastCommands, f_size(&fileLastCommands) - (CLI_MAX_BUF_SIZE * pos)));
//
//    // read last command
//    put_rc(f_read(&fileLastCommands, cl.cmd, CLI_MAX_BUF_SIZE, &read));
//
//    // Clear all indicator
//    cl.cmdlen = cl.cmdi = strlen(cl.cmd);
}

void CLI::putLastCommandInFile(void) {

    //    int i;
    //    int writed;
    //    FRESULT fres;
    //
    //    // Place the file pointer at the end of the file
    //    if ((fres = f_lseek(&fileLastCommands, f_size(&fileLastCommands))) != FR_OK)
    //        verbosePrintf("Error: %s", string_rc(fres));
    //
    //    for (i = cmdlen; i < CLI_MAX_BUF_SIZE; i++)
    //        cmd2[i] = '\0';
    //
    //    // Write into the file last command
    //    if ((fres = f_write(&fileLastCommands, cmd2, CLI_MAX_BUF_SIZE, &writed)) != FR_OK)
    //        verbosePrintf("Error: %s", string_rc(fres));
    //
    //    // Syncronize the content of the file on the micro SD
    //    if ((fres = f_sync(&fileLastCommands)) != FR_OK)
    //        verbosePrintf("Error: %s", string_rc(fres));
    //    
    //    nCmd++;
    //    lastCmd = 0;
}

CLI::~CLI(void) {

}

