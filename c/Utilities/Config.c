/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Config.c
 * Dependencies:    Config.h
 * Processor:       PIC32MX270F256B
 * Compiler:        Microchip XC32 v1.33 or higher
 * Company:         LP Systems
 * Author:          Luca Pascarella luca.pascarella@gmail.com
 * Web Site:        www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2013 LP Systems  All rights reserved.
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
 * File Description: Encoder and Decoder state finite machine
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release (1 September 2013, 16.00)
 *
 ********************************************************************/
#define __CONFIG_C
#ifdef __CONFIG_C

#include "Utilities/Config.h"
#include "Utilities/Logging.h"
#include "Utilities/GPIO.h"

const char config_msg[] = "Config.c";

const char file_ini[] = "conf.ini";
const char file_ini_dump[] = "conf.dmp";

configuration config;

//#define FULL_CONFIG

#ifdef FULL_CONFIG
const char conf[] = "# Embedded MP3 Player and Recorder\r\n\
# Author: Luca Pascarella\r\n\
# Contact: luca.pascarella@gmail.com\r\n\
# Web site author: www.lucapascarella.it\r\n\
# Web site producer: www.futuragroupsrl.it\r\n\
# Software version: v0.1\r\n\
#\r\n\
# +--+------------+------------------------------+\r\n\
# |  |  \\______/  |                    *****     |\r\n\
# |  |  Micro SD  |     ***            *****     |\r\n\
# |  |            |     ***            *****     |\r\n\
# |  |            | BLUE  RED                    |\r\n\
# |  +------------+    *  *                      |\r\n\
# |  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 |\r\n\
# |+---------------------------------------------+\r\n\
#    |  |  |  |  |  |  |  |  |  |  |  |  |  |  |\r\n\
#    V  G  M  S  S  A  A  R  T  U  U  R  L  R  L\r\n\
#    d  n  C  C  D  N  N  X  X  S  S  o  o  i  i\r\n\
#    d  d  L  L  A  0  1  /  /  B  B  u  u  n  n\r\n\
#          R  /  /  /  /  I  I  P  N  t  t\r\n\
#    +5V      I  I  I  I  O  O  /  /\r\n\
#   +3.3V     O  O  O  O  4  5  I  I\r\n\
#             0  1  2  3        O  O\r\n\
#                               6  7\r\n\
# 1) Vdd: Power in, it can be +5V or +3.3V. GPIO will be always +3.3V;\r\n\
# 2) Gnd: Ground reference;\r\n\
# 3) MCLR: Master Clear Reset input. This pin is an active-low hard reset;\r\n\
# 4) SCL/IO0: SCL I2C or GPIO0 pin;\r\n\
# 5) SDA/IO1: SDA I2C or GPIO1 pin;\r\n\
# 6) AN0/IO2: AN0 or GPIO2;\r\n\
# 7) AN1/IO3: AN1 or GPIO3;\r\n\
# 8) RX/IO4: UART RX or GPIO4;\r\n\
# 9) TX/IO5: UART TX or GPIO5;\r\n\
# 10) USB_P/IO6: USB_P or GPIO6;\r\n\
# 11) USN_N/IO7: USB_N or GPIO7;\r\n\
# 12) Rout: Right output;\r\n\
# 13) Lout: Left output;\r\n\
# 14) Rin: Right input;\r\n\
# 15) Lin: Left input.\r\n\
\r\n\
[Console]\r\n\
# Baudrate (bps): 2400, 4800, 9600, 19200, 38400, 56000, 57600 and <115200>.\r\n\
baudrate = 115200\r\n\
# Console:\r\n\
# <0> = CLI, Command Line Interpreter;\r\n\
# 1 = SCC, Single Character Commands;\r\n\
# 2 = SCC with Echo.\r\n\
console = 0\r\n\
# Console echo: 0 = off and <1> = on\r\n\
echo = 0\r\n\
# Console port: <0> = UART, 1 = USB serial emulator and 2 = I2C\r\n\
port = 0\r\n\
# Verbose: 0 = none, <1> = minimal and 2 = debug\r\n\
verbose = 1\r\n\
# Print boot info at startup: 0 = none, 1 = restricted or <2> = extended\r\n\
boot = 2\r\n\
# Log file: 0 = none, 1 = minimal, 2 = normal or <3> = debug\r\n\
log = 2\r\n\
\r\n\
[Play]\r\n\
# Playlist file name.\r\n\
playlist = play.pls\r\n\
# Repeat forever, 'stop -all' interrupts the executions: <0> = off or 1 = on.\r\n\
repeat = 0\r\n\
\r\n\
[Record]\r\n\
# Default recording name. If prog_over = 1 generates progressive files: Track(1).mp3, Track(2).mp3 etc.\r\n\
r_name = track.mp3\r\n\
# File already existing: 0 = overwrite or <1> = progressive.\r\n\
prog_over = 1\r\n\
# bitrate (kbit/s): 32, 40, 48, 56, 64, 80, 96, 112, <128>, 160, 192, 224, 256 and 320. Or quality 0 to 9 if Quality Mode is selected.\r\n\
bitrate = 128\r\n\
# samplerate (Hz): 8000, 11025, 12000, 16000, 22050, 24000, 32000, <44100> and 48000.\r\n\
samplerate = 44100\r\n\
# bitrate mode: 0 = Quality Mode, <1> = VBR, 2 = ABR or 3 = CBR.\r\n\
bitrate_mode = 1\r\n\
# Encoding gain: from 1x to 63x or 0 for automatic gain control.\r\n\
gain = 1\r\n\
# Maximum autogain amplification: from 1x to 63x.\r\n\
max_gain = 10\r\n\
# Format of encoding: <0> = MP3 or 1 = Ogg Vorbis.\r\n\
format = 0\r\n\
# ADC mode: 0 = Joint stereo (common AGC), 1 = Dual channel (separate AGC), 2 = Left, 3 = Right, 4 = Mono.\r\n\
adcMode = 0\r\n\
\r\n\
[Volume]\r\n\
# Initial volume attenuation from maximum in 1/2 dB steps (1 ... 255, 0 = mute). Example: 36 -> 36 * -0.5 = -18dB of attenuation.\r\n\
volume = 10\r\n\
# Volume balance (-100 ... 100, 0 = off).\r\n\
balance = 0\r\n\
# Treble Control in 1.5 dB steps (-8 ... 7, 0 = off).\r\n\
treble_db = 0\r\n\
# Lower limit frequency in 1000 Hz steps (1 ... 15).\r\n\
treble_freq = 1\r\n\
# Bass Enhancement in 1 dB steps (0 ... 15, 0 = off).\r\n\
bass_db = 0\r\n\
# Upper limit frequency in 10 Hz steps (2 ... 15).\r\n\
bass_freq = 2\r\n\
\r\n\
[GPIO]\r\n\
# GPIO can provide 6 mA source/sink current or can be enabled pull-up/down add-ons features.\r\n\
# Mode of operations associated with each pin:\r\n\
#\r\n\
# 0 = nothing;\r\n\
#\r\n\
# Input features:\r\n\
# 1 = starts the execution of the playlist;\r\n\
# 2 = stop the current track;\r\n\
# 3 = starts the playlist or stop the current track;\r\n\
# 4 = toggle pause if playing or recording;\r\n\
# 5 = starts the recording of a track (See: r_name);\r\n\
# 6 = starts and stop the recording of a track;\r\n\
# 7 = soft reset;\r\n\
#\r\n\
# Output signals when:\r\n\
# 20 = start play;\r\n\
# 21 = stop play;\r\n\
# 22 = pause play;\r\n\
# 23 = start or stop play;\r\n\
# 24 = start, pause or stop play;\r\n\
# 25 = file not found;\r\n\
# 26 = start recording;\r\n\
# 27 = stop recording;\r\n\
# 28 = pause recording;\r\n\
# 29 = start or stop recording;\r\n\
# 30 = start, pause or stop recording;\r\n\
# 31 = start play or recording;\r\n\
# 32 = stop play or recording;\r\n\
# 33 = pause play or recording;\r\n\
# 34 = start or stop play or recording;\r\n\
# 35 = start, pause or stop play or recording;\r\n\
# 36 = a command is correctly interpreted;\r\n\
# 37 = a command is not correctly interpreted;\r\n\
# 38 = micro SD is correctly initialized;\r\n\
#\r\n\
# Special functions:\r\n\
# 60 = enable I2C slave. Only for pin 4 (SCL) and 5 (SDA)\r\n\
# 70 = enable analog input. Only for pin 6 AN0/IO2 and 7 AN1/IO3 (Analog not yet implemented);\r\n\
# 80 = enable UART RxTx console. Only for pin 8 (RX) and 9 (TX)\r\n\
# 90 = enable USB Composite device: Serial emulator + Mass Storage. Only for pin 10 (USB_P) and 11 (USB_N)\r\n\
mode0 = 0\r\n\
mode1 = 0\r\n\
mode2 = 0\r\n\
mode3 = 0\r\n\
mode4 = 80\r\n\
mode5 = 80\r\n\
mode6 = 90\r\n\
mode7 = 90\r\n\
# Duration of changed state (Output mode). (10 ... 1000, 0 = toggle) (time in milliseconds).\r\n\
duration0 = 100\r\n\
duration1 = 100\r\n\
duration2 = 200\r\n\
duration3 = 200\r\n\
duration4 = 0\r\n\
duration5 = 0\r\n\
duration6 = 0\r\n\
duration7 = 0\r\n\
# Idle state: 0 = low or 1 = high.\r\n\
idle0 = 1\r\n\
idle1 = 1\r\n\
idle2 = 0\r\n\
idle3 = 0\r\n\
idle4 = 0\r\n\
idle5 = 0\r\n\
idle6 = 0\r\n\
idle7 = 0\r\n\
# Internal Pulled-up/down (Input mode). 0 = off, 1 = pulled-up or 2 = pulled-down.\r\n\
pull0 = 0\r\n\
pull1 = 0\r\n\
pull2 = 0\r\n\
pull3 = 0\r\n\
pull4 = 0\r\n\
pull5 = 0\r\n\
pull6 = 0\r\n\
pull7 = 0";
#else
const char conf[] = "";
#endif

const char c_console[] = "Console";
const char c_play[] = "Play";
const char c_record[] = "Record";
const char c_volume[] = "Volume";
const char c_gpio[] = "GPIO";

static int handler(void* user, const char* section, const char* name, const char* value) {

    configuration* pconfig = (configuration*) user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    // [Serial]
    if (MATCH(c_console, "baudrate")) {
        pconfig->console.baudrate = atoi(value);
    } else if (MATCH(c_console, "console")) {
        pconfig->console.console = atoi(value);
    } else if (MATCH(c_console, "echo")) {
        pconfig->console.echo = atoi(value);
    } else if (MATCH(c_console, "port")) {
        pconfig->console.port = atoi(value);
    } else if (MATCH(c_console, "verbose")) {
        pconfig->console.verbose = atoi(value);
    } else if (MATCH(c_console, "boot")) {
        pconfig->console.bootInfo = atoi(value);
    } else if (MATCH(c_console, "log")) {
        pconfig->console.log = atoi(value);
    } else

        // [Play]
        if (MATCH(c_play, "playlist")) {
        pconfig->play.playlist = strdup(value);
    } else if (MATCH(c_play, "repeat")) {
        pconfig->play.repeat = atoi(value);
    } else

        // [Record]
        if (MATCH(c_record, "r_name")) {
        pconfig->record.r_name = strdup(value);
    } else if (MATCH(c_record, "prog_over")) {
        pconfig->record.prog_over = atoi(value);
    } else if (MATCH(c_record, "bitrate")) {
        pconfig->record.bitrate = atoi(value);
    } else if (MATCH(c_record, "samplerate")) {
        pconfig->record.samplerate = atoi(value);
    } else if (MATCH(c_record, "bitrate_mode")) {
        pconfig->record.bitrate_mode = atoi(value);
    } else if (MATCH(c_record, "gain")) {
        pconfig->record.gain = atoi(value);
    } else if (MATCH(c_record, "max_gain")) {
        pconfig->record.max_gain = atoi(value);
    } else if (MATCH(c_record, "format")) {
        pconfig->record.format = atoi(value);
    } else if (MATCH(c_record, "adcMode")) {
        pconfig->record.adcMode = atoi(value);
    } else

        // [Volume]
        if (MATCH(c_volume, "volume")) {
        pconfig->volume.bits.left = pconfig->volume.bits.right = atoi(value);
    } else if (MATCH(c_volume, "balance")) {
        pconfig->volume.word.balance = atoi(value);
    } else if (MATCH(c_volume, "treble_db")) {
        pconfig->volume.bits.treble_db = atoi(value);
    } else if (MATCH(c_volume, "treble_freq")) {
        pconfig->volume.bits.treble_freq = atoi(value);
    } else if (MATCH(c_volume, "bass_db")) {
        pconfig->volume.bits.bass_db = atoi(value);
    } else if (MATCH(c_volume, "bass_freq")) {
        pconfig->volume.bits.bass_freq = atoi(value);
    } else

        // [GPIO]
        if (MATCH(c_gpio, "mode0")) {
        pconfig->gpio[GPIO_0_SCL].mode = atoi(value);
    } else if (MATCH(c_gpio, "mode1")) {
        pconfig->gpio[GPIO_1_SDA].mode = atoi(value);
    } else if (MATCH(c_gpio, "mode2")) {
        pconfig->gpio[GPIO_2_AN0].mode = atoi(value);
    } else if (MATCH(c_gpio, "mode3")) {
        pconfig->gpio[GPIO_3_AN1].mode = atoi(value);
    } else if (MATCH(c_gpio, "mode4")) {
        pconfig->gpio[GPIO_4_RX].mode = atoi(value);
    } else if (MATCH(c_gpio, "mode5")) {
        pconfig->gpio[GPIO_5_TX].mode = atoi(value);
    } else if (MATCH(c_gpio, "mode6")) {
        pconfig->gpio[GPIO_6_USB_P].mode = atoi(value);
    } else if (MATCH(c_gpio, "mode7")) {
        pconfig->gpio[GPIO_7_USB_N].mode = atoi(value);

    } else if (MATCH(c_gpio, "duration0")) {
        pconfig->gpio[GPIO_0_SCL].durationInMilliSecs = atoi(value);
    } else if (MATCH(c_gpio, "duration1")) {
        pconfig->gpio[GPIO_1_SDA].durationInMilliSecs = atoi(value);
    } else if (MATCH(c_gpio, "duration2")) {
        pconfig->gpio[GPIO_2_AN0].durationInMilliSecs = atoi(value);
    } else if (MATCH(c_gpio, "duration3")) {
        pconfig->gpio[GPIO_3_AN1].durationInMilliSecs = atoi(value);
    } else if (MATCH(c_gpio, "duration4")) {
        pconfig->gpio[GPIO_4_RX].durationInMilliSecs = atoi(value);
    } else if (MATCH(c_gpio, "duration5")) {
        pconfig->gpio[GPIO_5_TX].durationInMilliSecs = atoi(value);
    } else if (MATCH(c_gpio, "duration6")) {
        pconfig->gpio[GPIO_6_USB_P].durationInMilliSecs = atoi(value);
    } else if (MATCH(c_gpio, "duration7")) {
        pconfig->gpio[GPIO_7_USB_N].durationInMilliSecs = atoi(value);

    } else if (MATCH(c_gpio, "idle0")) {
        pconfig->gpio[GPIO_0_SCL].bits.idle = atoi(value);
    } else if (MATCH(c_gpio, "idle1")) {
        pconfig->gpio[GPIO_1_SDA].bits.idle = atoi(value);
    } else if (MATCH(c_gpio, "idle2")) {
        pconfig->gpio[GPIO_2_AN0].bits.idle = atoi(value);
    } else if (MATCH(c_gpio, "idle3")) {
        pconfig->gpio[GPIO_3_AN1].bits.idle = atoi(value);
    } else if (MATCH(c_gpio, "idle4")) {
        pconfig->gpio[GPIO_4_RX].bits.idle = atoi(value);
    } else if (MATCH(c_gpio, "idle5")) {
        pconfig->gpio[GPIO_5_TX].bits.idle = atoi(value);
    } else if (MATCH(c_gpio, "idle6")) {
        pconfig->gpio[GPIO_6_USB_P].bits.idle = atoi(value);
    } else if (MATCH(c_gpio, "idle7")) {
        pconfig->gpio[GPIO_7_USB_N].bits.idle = atoi(value);

    } else if (MATCH(c_gpio, "pull0")) {
        pconfig->gpio[GPIO_0_SCL].bits.pull = atoi(value);
    } else if (MATCH(c_gpio, "pull1")) {
        pconfig->gpio[GPIO_1_SDA].bits.pull = atoi(value);
    } else if (MATCH(c_gpio, "pull2")) {
        pconfig->gpio[GPIO_2_AN0].bits.pull = atoi(value);
    } else if (MATCH(c_gpio, "pull3")) {
        pconfig->gpio[GPIO_3_AN1].bits.pull = atoi(value);
    } else if (MATCH(c_gpio, "pull4")) {
        pconfig->gpio[GPIO_4_RX].bits.pull = atoi(value);
    } else if (MATCH(c_gpio, "pull5")) {
        pconfig->gpio[GPIO_5_TX].bits.pull = atoi(value);
    } else if (MATCH(c_gpio, "pull6")) {
        pconfig->gpio[GPIO_6_USB_P].bits.pull = atoi(value);
    } else if (MATCH(c_gpio, "pull7")) {
        pconfig->gpio[GPIO_7_USB_N].bits.pull = atoi(value);

    } else {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

const char playlist[] = "Play.pls";
const char bookmark[] = "Book.bmk";
const char record[] = "Track.mp3";

BOOL ConfigInit(void) {

    //BOOL exit = FALSE;
    int i;
    char *pIni, *pDmp;

    config.console.baudrate = 115200;
    config.console.console = 0;
    config.console.echo = 0;        // Console echo: 0 = off and 1 = on
    config.console.port = 0;
    config.console.verbose = 1;
    config.console.versionMajor = 0;
    config.console.versionMinor = 2;
    config.console.bootInfo = 1;
    config.console.log = 0;

    config.play.playlist = (char*) playlist;
    config.play.repeat = 0;

    config.record.r_name = (char*) record;
    config.record.prog_over = 1;
    config.record.bitrate = 128;
    config.record.samplerate = 44100;
    config.record.bitrate_mode = 1;
    config.record.gain = 1;
    config.record.max_gain = 10;
    config.record.format = 0;
    config.record.adcMode = 0;  // ADC mode: 0 = Joint stereo (common AGC), 1 = Dual channel (separate AGC), 2 = Left, 3 = Right, 4 = Mono

    config.volume.bits.left = config.volume.bits.right = 10;
    config.volume.word.balance = 0;
    config.volume.bits.treble_db = 0;
    config.volume.bits.treble_freq = 1;
    config.volume.bits.bass_db = 0;
    config.volume.bits.bass_freq = 2;

    // GPIO mode
    config.gpio[GPIO_0_SCL].mode = 0; // I2C
    config.gpio[GPIO_1_SDA].mode = 0; // I2C
    config.gpio[GPIO_2_AN0].mode = 0; // GPIO
    config.gpio[GPIO_3_AN1].mode = 0; // GPIO
    config.gpio[GPIO_4_RX].mode = GPIO_S_UART; // UART
    config.gpio[GPIO_5_TX].mode = GPIO_S_UART; // UART
    config.gpio[GPIO_6_USB_P].mode = GPIO_S_USB; // USB
    config.gpio[GPIO_7_USB_N].mode = GPIO_S_USB; // USB
    // GPIO duration
    config.gpio[GPIO_0_SCL].durationInMilliSecs = 100;
    config.gpio[GPIO_1_SDA].durationInMilliSecs = 100;
    config.gpio[GPIO_2_AN0].durationInMilliSecs = 200;
    config.gpio[GPIO_3_AN1].durationInMilliSecs = 200;
    config.gpio[GPIO_4_RX].durationInMilliSecs = 0;
    config.gpio[GPIO_5_TX].durationInMilliSecs = 0;
    config.gpio[GPIO_6_USB_P].durationInMilliSecs = 0;
    config.gpio[GPIO_7_USB_N].durationInMilliSecs = 0;
    // GPIO idle state
    config.gpio[GPIO_0_SCL].bits.idle = 1;
    config.gpio[GPIO_1_SDA].bits.idle = 1;
    config.gpio[GPIO_2_AN0].bits.idle = 0;
    config.gpio[GPIO_3_AN1].bits.idle = 0;
    config.gpio[GPIO_4_RX].bits.idle = 0;
    config.gpio[GPIO_5_TX].bits.idle = 0;
    config.gpio[GPIO_6_USB_P].bits.idle = 0;
    config.gpio[GPIO_7_USB_N].bits.idle = 0;
    // GPIO pull up/down state
    config.gpio[GPIO_0_SCL].bits.pull = 0;
    config.gpio[GPIO_1_SDA].bits.pull = 0;
    config.gpio[GPIO_2_AN0].bits.pull = 0;
    config.gpio[GPIO_3_AN1].bits.pull = 0;
    config.gpio[GPIO_4_RX].bits.pull = 0;
    config.gpio[GPIO_5_TX].bits.pull = 0;
    config.gpio[GPIO_6_USB_P].bits.pull = 0;
    config.gpio[GPIO_7_USB_N].bits.pull = 0;

    // Initialize pointer to dump file name for backup operation
    pIni = (char*) file_ini;
    pDmp = (char*) file_ini_dump;

    for (i = 0; i < 5; i++) {
        // Try to open 'ini' file
        writeToLogFile(LOG_NORMAL, config_msg, "Try to open '%s'", pIni);
        if (ini_parse(pIni, handler, &config) < 0) {
            writeToLogFile(LOG_NORMAL, config_msg, "Cannot load '%s'", pIni);
            if (pIni == (char*) file_ini) {
                // Try to open 'dump' file
                pIni = (char*) file_ini_dump;
                pDmp = (char*) file_ini;
            } else {
                // Generate 'ini' file
                pIni = (char*) file_ini;
                pDmp = (char*) file_ini_dump;
                if (ConfigGenerate()) {
                    writeToLogFile(LOG_DEBUG, config_msg, "Generated '%s'", pIni);
                } else {
                    writeToLogFile(LOG_NORMAL, config_msg, "Cannot generated '%s'", pIni);
                    return FALSE;
                }
            }
            // File opend successful
        } else {
            if (ConfigDumpIni(pIni, pDmp)) {
                writeToLogFile(LOG_DEBUG, config_msg, "Backup '%s' in '%s'", pIni, pDmp);
                return TRUE;
            } else {
                writeToLogFile(LOG_NORMAL, config_msg, "Cannot backup '%s' in '%s'", pIni, pDmp);
                return FALSE;
            }
        }
    }

    return FALSE;
}

BOOL ConfigGenerate(void) {

    extern FIL ftmp1;
    FIL *fp = &ftmp1;
    FRESULT fres;
    UINT writed;

    // Try to open in write mode the ini file
    if ((fres = f_open(fp, (char*) file_ini, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK)
        return FALSE;

    // Write the content of the ini file
    fres = f_write(fp, conf, sizeof (conf) - 1, &writed);
    //    fprintf(file, conf);

    // Close the ini file
    if (f_close(fp) != FR_OK)
        return FALSE;
    return TRUE;
}

BOOL ConfigDumpIni(char *pIni, char *pDmp) {

    extern FIL ftmp1, ftmp2;
    FIL *fini, *fdmp;
    FRESULT fres;
    char buf[128];
    int read, write;

    fini = &ftmp1;
    fdmp = &ftmp2;

    if ((fres = f_open(fini, pIni, FA_READ)) != FR_OK)
        return FALSE;

    if ((fres = f_open(fdmp, pDmp, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
        f_close(fini);
        return FALSE;
    }

    while (TRUE) {
        if ((fres = f_read(fini, buf, sizeof (buf), &read)) != FR_OK)
            break;
        if (read == 0)
            break;
        if ((fres = f_write(fdmp, buf, read, &write)) != FR_OK)
            break;
    }

    f_chmod(pDmp, AM_HID, AM_HID);

    if ((fres = f_close(fini)) == FR_OK) {
        if ((fres = f_close(fdmp)) == FR_OK)
            return TRUE;
    } else {
        f_close(fdmp);
    }

    return FALSE;
}


#endif
