/*********************************************************************
 *
 *  Commands
 *
 *********************************************************************
 * FileName:        Commands.c
 * Dependencies:    Commands.h
 *                  HardwareProfile.h
 *                  Compiler.h
 *                  GenericTypeDefs.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.33 or higher
 * Company:         LP Systems
 * Author:          Luca Pascarella www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 All rights reserved.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 * File Description:
 * This file contains the routines to do a command execution
 *
 * Change History:
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/

#include "FatFS/ff.h"
#include "Commands.h"
#include "CommandLineInterpreter.h"
#include "Utilities/printer.h"
#include "BootloaderInfo.h"
#include "Utilities/Config.h"
#include "Delay/Tick.h"
#include "FatFS/Diskio.h"
#include "Delay/Delay.h"
#include "Utilities/RTCC.h"

extern _command_line cl;
extern const _command commands[];
extern int number_of_commands;

DWORD AccSize; /* Work register for fs command */
WORD AccFiles, AccDirs;
//FILINFO Finfo;



int FileSystem(int argc, char **argv) {

    BYTE res, b1;
    FATFS *fs;
    long p1, p2;
    UINT s1;

    //    // Get the current directory name
    //    if (!put_rc(f_getcwd(cl.tmp, CLI_MAX_DIR_SIZE)))
    //	return;

    if (argc == 1) {
        res = f_getfree("0", (DWORD*) & p2, &fs);
        if (res) {
            put_rc(res);
            return;
        }
        printf("FAT type = %u\r\nBytes/Cluster = %lu\r\nNumber of FATs = %u\r\n"
                "Root DIR entries = %u\r\nSectors/FAT = %lu\r\nNumber of clusters = %lu\r\n"
                "FAT start (lba) = %lu\r\nDIR start (lba,clustor) = %lu\r\nData start (lba) = %lu\r\n",
                fs->fs_type, (DWORD) fs->csize * 512, fs->n_fats,
                fs->n_rootdir, fs->fsize, fs->n_fatent - 2,
                fs->fatbase, fs->dirbase, fs->database
                );
#if _USE_LABEL
        res = f_getlabel("0", (char*) cl.tmp, (DWORD*) & p1);
        if (res) {
            put_rc(res);
            return;
        }
        printf(cl.tmp[0] ? "Volume name is %s\r\n" : "No volume label\r\n", cl.tmp);
        printf("Volume S/N is %04X-%04X\r\n", (WORD) ((DWORD) p1 >> 16), (WORD) (p1 & 0xFFFF));
#endif
        printf("...");
        AccSize = AccFiles = AccDirs = 0;
        strcpy((char*) cl.tmp, "");
        res = scan_files((char*) cl.tmp);
        if (res) {
            put_rc(res);
            return;
        }
        printf("%u files, %lu bytes.\r\n%u folders.\r\n"
                "%lu KB total disk space.\r\n%lu KB available.\r\n",
                AccFiles, AccSize, AccDirs,
                (fs->n_fatent - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
                );

        p1 = 0;

        if (disk_ioctl((BYTE) p1, GET_SECTOR_COUNT, &p2) == RES_OK) {
            printf("Drive size: %lu sectors\r\n", p2);
        }
        if (disk_ioctl((BYTE) p1, GET_BLOCK_SIZE, &p2) == RES_OK) {
            printf("Erase block: %lu sectors\r\n", p2);
        }
        if (disk_ioctl((BYTE) p1, MMC_GET_TYPE, &b1) == RES_OK) {
            printf("Card type: %u\r\n", b1);
        }
        if (disk_ioctl((BYTE) p1, MMC_GET_CSD, cl.tmp) == RES_OK) {
            printf("CSD:\r\n");
            put_dump(cl.tmp, 0, 16);
            printf("\r\n");
        }
        if (disk_ioctl((BYTE) p1, MMC_GET_CID, cl.tmp) == RES_OK) {
            printf("CID:\r\n");
            put_dump(cl.tmp, 0, 16);
            printf("\r\n");
        }
        if (disk_ioctl((BYTE) p1, MMC_GET_OCR, cl.tmp) == RES_OK) {
            printf("OCR:\r\n");
            put_dump(cl.tmp, 0, 4);
            printf("\r\n");
        }
        if (disk_ioctl((BYTE) p1, MMC_GET_SDSTAT, cl.tmp) == RES_OK) {
            printf("SD Status:\r\n");
            for (s1 = 0; s1 < 64; s1 += 16) {
                put_dump(cl.tmp + s1, s1, 16);
                printf("\r\n");
            }
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Move(int argc, char **argv) {

    FRESULT fres;

    if (argc < 3) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 3) {
        // Check the existence of the old file name
        if ((fres = f_stat(argv[1], NULL)) != FR_OK) {

            if (fres == FR_NO_FILE)
                // File not found
                CliNoSuchFileOrDirectory();
            else
                // Print the error message
                put_rc(fres);
        } else {
            // Check the existence of the new file name
            if ((fres = f_stat(argv[2], NULL)) == FR_NO_FILE) {
                if (put_rc(f_rename(argv[1], argv[2]))) {
                    CliCreateFileListOfFilesEntry();
                } else {
                    CliErrorOccurred(argv[0]);
                }
            } else {
                // Some other errors on the new file
                if (fres == FR_OK)
                    CliFileOrDirectoryAlreadyExists();
                else
                    // Print the error message
                    put_rc(fres);
            }
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}





int Delete(int argc, char **argv) {

    FRESULT fres;

    if (argc < 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 2) {
        // Check the existence of the resource
        if ((fres = f_unlink(argv[1])) != FR_OK) {
            if (fres == FR_NO_FILE)
                // File not found
                CliNoSuchFileOrDirectory();
            else
                // Print the error message
                put_rc(fres);
        } else {
            CliCreateFileListOfFilesEntry();
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Clear(int argc, char **argv) {

    extern const char escape_clear_screen[];

    if (argc < 1) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 1) {
        CliPrintEscape(escape_clear_screen, 1);
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int ChangeDir(int argc, char **argv) {

    if (argc < 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 2) {
        if (!put_rc(f_chdir(argv[1])))
            CliErrorOccurred(argv[0]);
        CliCreateFileListOfFilesEntry();
    } else {

        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Mkdir(int argc, char **argv) {

    FRESULT fres;

    if (argc < 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 2) {
        if ((fres = f_mkdir(argv[1])) != FR_OK) {
            put_rc(fres);
            CliErrorOccurred(argv[0]);
        }
        CliCreateFileListOfFilesEntry();
    } else {

        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Copy(int argc, char **argv) {

    extern FIL ftmp1, ftmp2;
    FIL *fr, *fw;
    FRESULT fres;
    int read, write;
    DWORD time;
    double d;

    fr = &ftmp1;
    fw = &ftmp2;

    if (argc < 3) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 3) {

        if ((fres = f_open(fr, argv[1], FA_READ)) != FR_OK) {
            // Unable to create temporary file.
            CliNoSuchFileOrDirectory();

        } else {
            if ((fres = f_open(fw, argv[2], FA_WRITE | FA_CREATE_NEW)) == FR_OK) {
                time = TickGet();

                while ((fres = f_read(fr, cl.tmp, sizeof (cl.tmp), &read)) == FR_OK) {
                    if (read == 0)
                        break;
                    LED_RED_ON();
                    if ((fres = f_write(fw, cl.tmp, read, &write)) != FR_OK) {
                        break;
                        CliErrorOccurred("COPY");
                    }
                    LED_BLUE_ON();
                }
                time = (TickGet() - time) / TICKS_PER_SECOND;
                d = (double) f_size(fr) / 1024 / time;
                printf("Size: %dkByte, Time: %dsec, Th: %.1fkByte/sec\r\n", f_size(fr) / 1024, time, d);

                f_close(fw);
            }
            f_close(fr);
            CliCreateFileListOfFilesEntry();
        }
    } else {

        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}



int Reboot(int argc, char **argv) {

    extern char *MyScratchPad;
    DWORD *p;

    if (argc < 1) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 1) {

        printf("Reboot in progress...\r\n\r\n");
        DelayMs(100);

        p = (DWORD*) & MyScratchPad[0];
        *p = 0xAA996655;
        p = (DWORD*) & MyScratchPad[4];
        *p = 0x00000000;

        Reset();

    } else {

        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Debug(int argc, char **argv) {

    int i;
    extern char *MyScratchPad;
    DWORD *p;

    printf("For production use only. Normally you don't need to use this command.\r\n");

    if (argc == 2) {
        if (strcmp(argv[1], "0xAA996655") == 0) {
            printf("Entering in debug mode\r\n");

            const char *gpio0_on[] = {"gpio", "0", "36", "500", "1", "0"};
            const char *gpio1_on[] = {"gpio", "1", "36", "500", "1", "0"};
            const char *gpio2_on[] = {"gpio", "2", "36", "500", "1", "0"};
            const char *gpio3_on[] = {"gpio", "3", "36", "500", "1", "0"};
            const char *gpio4_on[] = {"gpio", "4", "36", "500", "1", "0"};
            const char *gpio5_on[] = {"gpio", "5", "36", "500", "1", "0"};
            const char *gpio6_on[] = {"gpio", "6", "36", "500", "1", "0"};
            const char *gpio7_on[] = {"gpio", "7", "36", "500", "1", "0"};

            const char *gpio0_off[] = {"gpio", "0", "36", "500", "0", "0"};
            const char *gpio1_off[] = {"gpio", "1", "36", "500", "0", "0"};
            const char *gpio2_off[] = {"gpio", "2", "36", "500", "0", "0"};
            const char *gpio3_off[] = {"gpio", "3", "36", "500", "0", "0"};
            const char *gpio4_off[] = {"gpio", "4", "36", "500", "0", "0"};
            const char *gpio5_off[] = {"gpio", "5", "36", "500", "0", "0"};
            const char *gpio6_off[] = {"gpio", "6", "36", "500", "0", "0"};
            const char *gpio7_off[] = {"gpio", "7", "36", "500", "0", "0"};

            for (i = 0; i < 3; i++) {

                Gpio(6, gpio0_on);
                Gpio(6, gpio1_on);
                Gpio(6, gpio2_on);
                Gpio(6, gpio3_on);
                Gpio(6, gpio4_on);
                Gpio(6, gpio5_on);
                Gpio(6, gpio6_on);
                Gpio(6, gpio7_on);

                DelayMs(500);

                Gpio(6, gpio0_off);
                Gpio(6, gpio1_off);
                Gpio(6, gpio2_off);
                Gpio(6, gpio3_off);
                Gpio(6, gpio4_off);
                Gpio(6, gpio5_off);
                Gpio(6, gpio6_off);
                Gpio(6, gpio7_off);
                DelayMs(500);
            }
            //printf("Exiting from debug mode\r\n");
            //DelayMs(100);
            Reset();
        }
    }
    return 0;
}

int Flash(int argc, char **argv) {

    extern char *MyScratchPad;
    DWORD *p;

    if (argc < 2) {
        CliTooFewArgumnets(argv[0]);
    } else if (argc == 2) {

        if (strcmp(argv[1], "0xAA996655") == 0) {
            printf("Entering in bootloader mode to flash new image.\r\n");
            printf("Send trigger signal to start firmware upgrade..\r\n\r\n");

            p = (DWORD*) & MyScratchPad[0];
            *p = 0xAA996655;
            p = (DWORD*) & MyScratchPad[4];
            *p = 0xA965B401;

            //	    if (argc == 3){
            //		*p = 0xA965B402;
            //		strncpy(&MyScratchPad[8], argv[2], 56);
            //	    }
            DelayMs(100);

            Reset();
        }
    } else {

        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

int Version(int argc, char **argv) {

    extern char *MyScratchPad;
    extern configuration config;

    if (argc < 2) {
        printf("%s\r\n", MyScratchPad);
        printf("Embedded MP3 Player v%d.%d\r\n", config.console.versionMajor, config.console.versionMinor);
    } else {

        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}


#define adc_ADC             0
#define bass_BASS           1
#define bookmark_BOOKMARK   2
#define cat_CAT             3
#define cd_CHANGE_DIR       4
#define cfg_CONFIG          5
#define clear_CLEAR_SCREEN  6
#define cp_COPY_FILE        7
#define debug_DEBUG         8
#define ear_EAR_SPEAKER     9
#define fat_FILE_SYSTEM     10
#define flash_FLASH         11
#define fplay_FAST_PLAY     12
#define gpio_GPIO           13
#define help_HELP           14
#define info_INFO           15
#define ls_LIST             16
#define mkdir_MAKE_DIR      17
#define mv_MOVE_RENAME      18
#define pause_PAUSE         19
#define play_PLAY           20
#define playlist_PLAYLIST   21
#define rate_RATE_TUNE      22
#define reboot_REBOOT       23
#define rec_RECORD          24
#define rtcc_RTCC           25
#define rm_REMOVE           26
#define rmdir_REMOVE_DIR    27
#define speed_SPEED_SHIFTER 28
#define stone_SEMITONE      29
#define stop_STOP           30
#define treble_TREBLE       31
#define verbose_VERBOSE     32
#define version_VERSION     33
#define vol_VOLUME          34

int Help(int argc, char **argv) {

    int i;

    if (argc == 1) {
        // List of all commands
        printf("For more information about a command, type: help [command]:\r\n");

        CliPrintListOfAllCommands();

    } else if (argc == 2) {

        //        printf("'%s' '%s' '%d'", commands[3].name, argv[1], strlen(commands[3].name));

        switch (CliSearchCommand(argv[1])) {

            case adc_ADC:
                // Command adc
                printf("Perform an ADC conversion.\r\n");
                CliUsageCommand("adc");
                printf("No options for this command\r\n");
                CliUsageExample("adc");
                break;

            case bass_BASS:
                // Command bass
                printf("Sets or returns the value of the low tones.\r\n");
                CliUsageCommand("bass [att, freq]");
                printf("att\tBass Enhancement in 1 dB steps (0 ... 15, 0 = off)\r\n");
                printf("freq\tUpper limit frequency in 10 Hz steps (2 ... 15)\r\n");
                CliUsageExample("bass 7 10");
                break;

            case bookmark_BOOKMARK:
                // Command bookmark
                printf("Bookmark [Command not yet implemented]\r\n");
                //                CliUsageCommand("bookmark [op1, op2]");
                //                printf("op1\tDescription 1\r\n");
                //                printf("op2\tDescription 2\r\n");
                //                CliUsageExample("bookmark op1 op2");
                break;

            case cat_CAT:
                // Command cat
                printf("Print the contents of a textual file.\r\n");
                CliUsageCommand("cat file [-bin]");
                printf("file\tName of textual file (.txt, .ini or .pls)\r\n");
                printf("-bin\tforce binary transfer\r\n");
                CliUsageExample("cat conf.ini");
                break;

            case cd_CHANGE_DIR:
                // Command cd
                printf("Change the working directory.\r\n");
                CliUsageCommand("cd dir");
                printf("dir\tnew working directory\r\n");
                printf("\tor '..' to return to the parent folder\r\n");
                printf("\tor '/' to return to the root folder\r\n");
                CliUsageExample("cd songs");
                break;

            case cfg_CONFIG:
                // Command cfg
                printf("Changes the console port configuration values.\r\n");
                CliUsageCommand("cfg [opt]");
                printf("opt\t0 = UART, 1 = USB and 2 = I2C\r\n");
                CliUsageExample("cfg 1");
                break;

            case clear_CLEAR_SCREEN:
                // Command clear
                printf("Shows file system status\r\n");
                CliUsageCommand("fat");
                printf("No options for this command\r\n");
                CliUsageExample("fat");
                break;

            case cp_COPY_FILE:
                // Command cp
                printf("Copies a file.\r\n");
                CliUsageCommand("cp src cpy");
                printf("src\toriginal file\r\n");
                printf("cpy\tthe copy of source file\r\n");
                CliUsageExample("cp old.mp3 new.mp3");
                break;

            case debug_DEBUG:
                // Command cp
                printf("For production use only. Normally you don't need to use this command.\r\n");
                break;

            case ear_EAR_SPEAKER:
                // Command ear
                printf("Sets or returns the value of EarSpeaker Spatial Processing.\r\n");
                CliUsageCommand("ear [n]");
                printf("n\t0: Best option when listening through loudspeakers  or if the audio to be played contains binaural preprocessing\r\n");
                printf("\t12000: Suited for listening to normal musical scores with headphones, very subtle\r\n");
                printf("\t38000: Suited for listening to normal musical scores with headphones, moves sound source further away than minimal\r\n");
                printf("\t50000: Suited for old or 'dry' recordings\r\n");
                CliUsageExample("ear 12000");
                break;

            case fat_FILE_SYSTEM:
                // Command fat
                printf("Shows file system status\r\n");
                CliUsageCommand("fat");
                printf("No options for this command\r\n");
                CliUsageExample("fat");
                break;

            case flash_FLASH:
                // Command flash
                printf("Flash a new firmware.\r\n");
                printf("Before you start the flash procedure, make sure that in the micro SD is present a new firmware exactly named 'image.hex'\r\n");
                CliUsageCommand("flash magic");
                printf("magic\tused to prevent an accidental write/erasure of the firmware.\r\n");
                printf("\tUses the string: 0xAA996655\r\n");
                //printf("name\tfile image to flash");
                CliUsageExample("flash 0xAA996655");
                break;

            case fplay_FAST_PLAY:
                // Command fplay Fast play
                printf("Sets or returns the incremental speed execution\r\n");
                CliUsageCommand("fplay [spd]");
                printf("spd\tx multiplier (1 = 1x ... 10 = 10x)\r\n");
                CliUsageExample("fplay 1");
                break;

            case gpio_GPIO:
                // Command gpio Returns the GPIO state
                printf("Sets or returns the GPIO state.\r\n");
                CliUsageCommand("gpio [n, m, d, i, p]");
                printf("n\tpin number (0 = GPIO0, 1 = GPIO1 ... 7 = GPIO7)\r\n");
                printf("m\tmode:\r\n");
                printf("\t0 = do nothing\r\n");

                printf("\tInput:\r\n");
                printf("\t1 = starts the execution of the playlist\r\n");
                printf("\t2 = stop the current track\r\n");
                printf("\t3 = starts the playlist or stop the current track\r\n");
                printf("\t4 = toggle pause if playing or recording\r\n");
                printf("\t5 = starts the recording of a track\r\n");
                printf("\t6 = starts and stop the recording of a track\r\n");
                printf("\t7 = soft reset\r\n");

                printf("\tOutput:\r\n");
                printf("\t20 = start play\r\n");
                printf("\t21 = stop play\r\n");
                printf("\t22 = pause play\r\n");
                printf("\t23 = start or stop play\r\n");
                printf("\t24 = start, pause or stop play\r\n");
                printf("\t25 = file not found\r\n");
                printf("\t26 = start recording\r\n");
                printf("\t27 = stop recording\r\n");
                printf("\t28 = pause recording\r\n");
                printf("\t29 = start or stop recording\r\n");
                printf("\t30 = start, pause or stop recording\r\n");
                printf("\t31 = start play or recording\r\n");
                printf("\t32 = stop play or recording\r\n");
                printf("\t33 = pause play or recording\r\n");
                printf("\t34 = start or stop play or recording\r\n");
                printf("\t35 = start, pause or stop play or recording\r\n");
                printf("\t36 = a command is correctly interpreted\r\n");
                printf("\t37 = a command is not correctly interpreted\r\n");
                printf("\t38 = micro SD is correctly initialized\r\n");
                printf("d\tduration in ms (10 ... 1000, 0 = toggle)\r\n");
                printf("i\tidle state. 0 = low or 1 = high\r\n");
                printf("p\tinternal Pulled-up/down (Input mode). 0 = off, 1 = pulled-up or 2 = pulled-down\r\n");

                CliUsageExample("gpio 2 1 100 1 1");
                break;

            case help_HELP:
                // Command Returns the help informations
                printf("Returns the help informations.\r\n");
                CliUsageCommand("help [cmd]");
                printf("cmd\thelp of 'cmd' commnad\r\n");
                CliUsageExample("help play");
                break;

            case info_INFO:
                // Command info
                printf("Prints the informations of the current track.\r\n");
                CliUsageCommand("info");
                printf("No options for this command\r\n");
                CliUsageExample("info");
                break;

            case ls_LIST:
                // Command ls
                printf("Return the contents of the working directory.\r\n");
                CliUsageCommand("ls [-la] [dir]");
                printf("l\tlong format\r\n");
                printf("a\tincludes hidden files\r\n");
                printf("dir\trelative path\r\n");
                CliUsageExample("ls -la");
                break;

            case mkdir_MAKE_DIR:
                // Command mkdir
                printf("Creates a new directory.\r\n");
                CliUsageCommand("mkdir dir");
                printf("dir\tnew directory name\r\n");
                CliUsageExample("mkdir song");
                break;

            case mv_MOVE_RENAME:
                // Command mv
                printf("Moves or renames a file or a directory\r\n");
                CliUsageCommand("mv file path");
                printf("file\tname of the origin file\r\n");
                printf("path\tname or path of the destination\r\n");
                CliUsageExample("mv 1.mp3 folder/1.mp3");
                break;

            case pause_PAUSE:
                // Command pause
                printf("Enters or exits from the pause, when a song is in playing or recording.\r\n");
                CliUsageCommand("pause [t]");
                printf("\twithout parameter toggle pause\r\n");
                printf("t\tenters in pause for 't' milliseconds\r\n");
                CliUsageExample("pause 1500");
                break;

            case play_PLAY:
                // Command play
                printf("Puts a track in execution of the following type: mp3 or ogg.\r\n");
                CliUsageCommand("play file");
                printf("file\tname of the track\r\n");
                CliUsageExample("play song.mp3");
                break;

            case playlist_PLAYLIST:
                // Command playlist
                printf("Play an ordered list of track named in the file\r\n");
                CliUsageCommand("playlist [list] [-r]");
                printf("list\tname of the playlist or conf value if not specified\r\n");
                printf("r\tRepeat forever, 'stop -all' interrupts the executions: <0> = off or 1 = on\r\n");
                CliUsageExample("playlist play.lst");
                break;

            case rate_RATE_TUNE:
                // Command rate
                printf("Rate finetune get or set samplerate finetune in +-1ppm steps\r\n");
                CliUsageCommand("rate [n]");
                printf("n\tfinetunes the samplerate in 1 ppm steps. (-1000000 ... 1000000, 0 = off)\r\n");
                CliUsageExample("rate 59500");
                break;

            case reboot_REBOOT:
                // Command reboot
                printf("Reboots the Embedded MP3 Player and Recorder\r\n");
                CliUsageCommand("reboot");
                printf("No options for this command\r\n");
                CliUsageExample("reboot");
                break;

            case rec_RECORD:
                // Command rec
                printf("Starts a recording in MP3 or Ogg encoding format\r\n");
                printf("Embedded MP3 supports all MP3 samplerates and bitrates, in stereo and mono, both with constant bit-rate (CBR) or variable bitrate (VBR).\r\n");
                printf("The Ogg Vorbis Encoder supports encoding in mono and stereo, with any samplerate between 1 and 48000 Hz, and with different quality settings. Ogg Vorbis is always encoded using variable bitrate (VBR).\r\n");
                CliUsageCommand("rec file [s, b, g, m, v, f, i]");
                printf("file\tname of the track (it will be ignored if i > 0)\r\n");
                printf("s\tsamplerate (Hz): 8000 ... 48000\r\n");
                printf("b\tbitrate (kbit/s): 32 ... 320\r\n");
                printf("g\tgain (0 ... 63, 0 = auto gain)\r\n");
                printf("m\tmax auto gain (0 ... 63)\r\n");
                printf("v\tmode 0 = Quality Mode, 1 = VBR, 2 = ABR or 3 = CBR\r\n");
                printf("f\tformat 0 = mp3 or 1 = ogg\r\n");
                printf("c\tChannels: 0 = Joint stereo (common AGC), 1 = Dual channel (separate AGC), 2 = Left, 3 = Right, 4 = Mono\r\n");
                printf("i\ttime intervals (sec): 0 = disabled or 15 ... 3600\r\n");
                CliUsageExample("rec rec.mp3 44100 160 0 5 1 0 0 0");
                break;

            case rtcc_RTCC:
                // Command rtcc
                printf("Sets or returns Real Time Clock and Calendar (RTCC)\r\n");
                CliUsageCommand("rtcc [DD MM YYYY hh mm ss]");
                printf("DD\tday\r\n");
                printf("MM\tmonth\r\n");
                printf("YYYY\tyear\r\n");
                printf("hh\thours\r\n");
                printf("mm\tminutes\r\n");
                printf("ss\tseconds\r\n");
                CliUsageExample("rtcc 18 06 2014 14 45 00");
                break;

            case rm_REMOVE:
                // Cimmand rm
                printf("Removes a file\r\n");
                CliUsageCommand("rm file");
                printf("file\tname of the file to remove\r\n");
                CliUsageExample("rm 1.mp3");
                break;

            case rmdir_REMOVE_DIR:
                // Command rmdir
                printf("Removes a directory\r\n");
                CliUsageCommand("rmdir dir [-not-empty]");
                printf("dir\tname of the directory to remove\r\n");
                printf("-not-empty\tremoves a not empty directory\r\n");
                CliUsageExample("rmdir recs -not-empty");
                break;

            case speed_SPEED_SHIFTER:
                // Command speed
                printf("Sets or returns the Speed Shifter value\r\n");
                printf("Speed Shifter allows the playback tempo to be changed without changing the playback pitch. Speed Shifter and EarSpeaker can not be used at the same time. Speed Shifter overrides EarSpeaker\r\n");
                CliUsageCommand("speed [spd]");
                printf("spd\tx multiplier (680 = 0.68x ... 1000 = 1.00x ... 1640 = 1.64x)\r\n");
                CliUsageExample("speed 1000");
                break;

            case stone_SEMITONE:
                // Command stone
                // Cambia la tonalità senza cambiate il ritmo
                printf("Sets or returns the Semitone value\r\n");
                printf("Stone changes the pitch without changing the tempo\r\n");
                CliUsageCommand("stone [n]");
                printf("n\tsemitones (-12 ... 12, 0 = off)\r\n");
                CliUsageExample("stone 2");
                break;

            case stop_STOP:
                // Command stop
                printf("Stops a playing or recording command\r\n");
                CliUsageCommand("stop [-all]");
                printf("-all\tStop the execution of all track in playlist\r\n");
                CliUsageExample("stop");
                break;

            case treble_TREBLE:
                // Command treble
                printf("Sets or returns the value of the high tones\r\n");
                CliUsageCommand("treble [att, freq]");
                printf("att\tTreble Enhancement in 1.5 dB steps (-8 ... 7, 0 = off)\r\n");
                printf("freq\tLower limit frequency in 1000 Hz steps (1 ... 15)\r\n");
                CliUsageExample("treble 7 10");
                break;

            case verbose_VERBOSE:
                // Command verbose
                printf("Sets or returns the verbose level\r\n");
                CliUsageCommand("verbose [lev]");
                printf("lev\tlevel 0 = Nothing, 1 = Minimal or 2 = Debug\r\n");
                CliUsageExample("verbose 2");
                break;

            case version_VERSION:
                // Command version
                printf("Returns the version informations\r\n");
                CliUsageCommand("version");
                printf("No options for this command\r\n");
                CliUsageExample("version");
                break;

            case vol_VOLUME:
                // Command vol
                printf("Sets or returns the value of the volume\r\n");
                CliUsageCommand("vol [att [bal]]");
                printf("att\tvolume attenuation in 0.5dB steps (255 = sound off ... 0 = max vol)\r\n");
                printf("bal\tchannel attenuation in percentage of volume (L = -100%% ... 0%% ... 100%% = R)\r\n");
                CliUsageExample("vol 10 0");

                break;

            default:
                CliCommandNotFound(argv[1]);
        }
    }
    return 0;
}

BOOL CheckFileExistence(char *p) {

    FRESULT fres;
    fres = f_stat(p, NULL);

    if (fres == FR_OK)
        return TRUE;

    return FALSE;
}

const char *ByteToFatAttributes(BYTE att) {

    static char str[10];
    str[0] = '\0';

    //#define	AM_RDO	0x01	/* Read only */
    //#define	AM_HID	0x02	/* Hidden */
    //#define	AM_SYS	0x04	/* System */
    //#define	AM_VOL	0x08	/* Volume label */
    //#define   AM_LFN	0x0F	/* LFN entry */
    //#define   AM_DIR	0x10	/* Directory */
    //#define   AM_ARC	0x20	/* Archive */
    //#define   AM_MASK	0x3F	/* Mask of defined bits */

    strcat(str, ((att & AM_HID) == AM_HID) ? "h" : "-");
    //    strcat(str, ((att & AM_SYS) == AM_SYS) ? "s" : "-");
    //    strcat(str, ((att & AM_VOL) == AM_VOL) ? "v" : "-");
    //    strcat(str, ((att & AM_LFN) == AM_LFN) ? "l" : "-");
    strcat(str, ((att & AM_DIR) == AM_DIR) ? "d" : "-");
    //    strcat(str, ((att & AM_ARC) == AM_ARC) ? "a" : "-");
    strcat(str, ((att & AM_RDO) == AM_RDO) ? "r-" : "rw");

    return str;
}

char *GetFileNamePointer(FILINFO * finfo) {

//    if (finfo->lfname == NULL)
//        return finfo->fname;
//    else if (*finfo->lfname == '\0')
//        return finfo->fname;
//    else
//        return finfo->lfname;
    return finfo->fname;
}

BOOL put_rc(FRESULT rc) {

    FRESULT i;
    const char *str =
            "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
            "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
            "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
            "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";

    if (rc != FR_OK) {
        for (i = 0; i != rc && *str; i++) {
            while (*str++);
        }
        verbosePrintf(VER_DBG, "FatFS Error, rc=%u FR_%s\n", (UINT) rc, str);
        return FALSE;
    }
    return TRUE;
}

BOOL checkFatAttributes(FILINFO *finfo, BYTE attributes) {
    /* File attribute bits for directory entry */

    //#define	AM_RDO	0x01	/* Read only */
    //#define	AM_HID	0x02	/* Hidden */
    //#define	AM_SYS	0x04	/* System */
    //#define	AM_VOL	0x08	/* Volume label */
    //#define       AM_LFN	0x0F	/* LFN entry */
    //#define       AM_DIR	0x10	/* Directory */
    //#define       AM_ARC	0x20	/* Archive */

    BYTE positive = finfo->fattrib & attributes;
    BYTE negative = finfo->fattrib & (~attributes);

    if (positive == attributes && negative == 0)
        return TRUE;

    return FALSE;
}

//FRESULT empty_directory(char* path) {
//
//    UINT i, j;
//    FRESULT fr;
//    DIR dir;
//    // This function is recursive
//    FILINFO fno;
//
//#if _USE_LFN
//    fno.lfname = 0; /* Eliminate LFN output */
//#endif
//    fr = f_opendir(&dir, path);
//    if (fr == FR_OK) {
//        for (i = 0; path[i]; i++);
//        path[i++] = '/';
//        for (;;) {
//            fr = f_readdir(&dir, &fno);
//            if (fr != FR_OK || !fno.fname[0]) break;
//            if (fno.fname[0] == '.') continue;
//            j = 0;
//            do
//                path[i + j] = fno.fname[j]; while (fno.fname[j++]);
//            if (fno.fattrib & AM_DIR) {
//                fr = empty_directory(path);
//                if (fr != FR_OK) break;
//            }
//            fr = f_unlink(path);
//
//            if (fr != FR_OK) break;
//        }
//        path[--i] = '\0';
//        f_closedir(&dir);
//    }
//
//    return fr;
//}

FRESULT scan_files(
        char* path /* Pointer to the working buffer with start path */
        ) {
    DIR dirs;
    FRESULT res;
    extern FILINFO finfo;
    int i;
    char *fn;

    res = f_opendir(&dirs, path);
    if (res == FR_OK) {
        i = strlen(path);
        while (((res = f_readdir(&dirs, &finfo)) == FR_OK) && finfo.fname[0]) {
            if (FF_FS_RPATH && finfo.fname[0] == '.') continue;
//#if _USE_LFN
//            fn = *finfo.lfname ? finfo.lfname : finfo.fname;
//#else
//            fn = Finfo.fname;
//#endif
            fn = finfo.fname;
            
            if (finfo.fattrib & AM_DIR) {
                AccDirs++;
                path[i] = '/';
                strcpy(path + i + 1, fn);
                res = scan_files(path);
                path[i] = '\0';
                if (res != FR_OK) break;
            } else {
                //				xprintf(PSTR("%s/%s\n"), path, fn);

                AccFiles++;
                AccSize += finfo.fsize;
            }
        }
    }

    return res;
}

void put_dump(const BYTE *buff, DWORD ofs, BYTE cnt) {
    BYTE i;

    char buf[256];

    sprintf(buf, "%08lX:", ofs);
    printf(buf);

    for (i = 0; i < cnt; i++) {
        //printf(" %02X", buff[i]);
        sprintf(buf, " %02X", buff[i]);
        printf(buf);
    }

    printf(" ");

    for (i = 0; i < cnt; i++)
        putc((buff[i] >= ' ' && buff[i] <= '~') ? buff[i] : '.');
}

void commandsTask(void) {
    extern int argc;
    extern char *argv[];
    extern int (*commandToCall)(int, char**);

    // Call a new command or continue to call the same command to complete a task
    if (commandToCall != NULL) {
        if (commandToCall(argc, argv) == 0) {
            // Command function return with 0 => command ended
            commandToCall = NULL;
            CliReprintConsole();
        }
    }
}