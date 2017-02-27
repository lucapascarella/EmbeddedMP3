/*********************************************************************
 *
 *  MP3 Encoder and Decoder Application Entry Point
 *
 *********************************************************************
 * FileName:        Record.c
 * Dependencies:    Record.h
 * Processor:       PIC32MX250F128B
 * Compiler:        Microchip XC32 v1.11a or higher
 * Company:         LP Systems
 * Author:	    Luca Pascarella luca.pascarella@gmail.com
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

#include "MP3/Record.h"
#include "MP3/MP3.h"
#include "MP3/VS1063.h"

#include "Utilities/printer.h"
#include "CommandLineInterpreter.h"
#include "Utilities/Config.h"
#include "Utilities/Utilities.h"
#include "Delay/Tick.h"
#include "Utilities/GPIO.h"
#include "FatFS/ff.h"
#include "MP3/Play.h"
#include "Commands.h"

static enum {
    MP3_RECORD_HOME = 0,

    MP3_RECORD_OPEN_FILE,
    MP3_RECORD_OPEN_NEXT_FILE,
    MP3_RECORD_OPENED_SUCCESSFUL,
    MP3_RECORD_OPENED_FAILED,

    MP3_RECORD_READ_BUFFER,
    MP3_RECORD_WRITE_BUFFER,

    MP3_RECORD_PAUSE_WAIT_ENTERING,
    MP3_RECORD_PAUSE_WAIT,
    MP3_RECORD_PAUSE_DELAY_ENTERING,
    MP3_RECORD_PAUSE_DELAY,
    MP3_RECORD_PAUSE_EXIT,

    MP3_RECORD_SEND_FINISH_RECORDING,
    MP3_RECORD_FINISH_RECORDING,
    MP3_RECORD_FINALIZE,

    MP3_RECORD_CLOSE_FILE,
    MP3_RECORD_CLOSED_SUCCESSFUL,
    MP3_RECORD_CLOSED_FAILED,

} mp3RecSM = MP3_RECORD_HOME;

_REC_PRESET recPreSet;
BOOL timeout;
// See ffconf.h for dimension of LFN
extern TCHAR Lfname[];
// See MP3.h for dimension of stream
extern char stream[];
DWORD tick_delay, tick_max;

int RecordTaskHandler() {

    extern FIL fstream;
    static FIL *fp = &fstream;
    FRESULT fres;
    static UINT read, write;
    char byte, *p, *b;

    switch (mp3RecSM) {

        case MP3_RECORD_HOME:
            // Do nothing, wait a record command
            Nop();
            break;



        case MP3_RECORD_OPEN_FILE:

            // Checks if incremental mode is enabled otherwise overrides file
            if (config.record.prog_over == 0) {
                verbosePrintf(VER_DBG, "Try to open or override: %s", Lfname);
                fres = f_open(fp, Lfname, FA_WRITE | FA_CREATE_ALWAYS);
            } else {
                verbosePrintf(VER_DBG, "Try to open: %s", Lfname);
                fres = f_open(fp, Lfname, FA_WRITE | FA_CREATE_NEW);
            }

            if (fres == FR_EXIST) {
                mp3RecSM = MP3_RECORD_OPEN_NEXT_FILE;
            } else if (fres == FR_OK) {
                mp3RecSM = MP3_RECORD_OPENED_SUCCESSFUL;
            } else {
                mp3RecSM = MP3_RECORD_OPENED_FAILED;
            }
            break;

        case MP3_RECORD_OPEN_NEXT_FILE:

            // Gets a copy of extension if it exist
            if ((p = strrchr(Lfname, '.')) == NULL) {
                p = &Lfname[strlen(Lfname) - 1];
                stream[0] = '\0';
            } else {
                // Copy the extension in temp buffer
                strcpy(stream, p);
            }
            // Check if there is already a incremental number to increase it, otherwise starts with zero
            if ((b = strrchr(Lfname, '(')) != NULL) {
                read = atoi(b + 1);
            } else {
                read = 0;
                b = p;
            }
            // Copy in Lfname the incremental number and previous extension
            sprintf(b, "(%d)%s", ++read, stream);

            mp3RecSM = MP3_RECORD_OPEN_FILE;
            break;


        case MP3_RECORD_OPENED_SUCCESSFUL:

            GpioUpdateOutputState(GPIO_BIT_STARTS_REC);

            verbosePrintf(VER_DBG, "File opend successful");

            // Reinitialize the VS1063 and load patch
            VLSI_SoftReset();

            // Send all the parameters and start recording
            VLSI_InitRecording(&recPreSet);

            write = read = 0;
            mp3RecSM = MP3_RECORD_READ_BUFFER;
            break;



        case MP3_RECORD_OPENED_FAILED:
            verbosePrintf(VER_MIN, "File: %s not found", Lfname);
            GpioUpdateOutputState(GPIO_BIT_FILE_NOT_FOUND);
            FlashLight(100, 10, FALSE);
            mp3RecSM = MP3_RECORD_HOME;
            break;



        case MP3_RECORD_READ_BUFFER:
            read = VLSIGetArray(stream, STREAM_BUF_SIZE_RECORD, 1024);
            if (read == 0)
                break;
            //            if (read != 0)
            //                mp3RecSM = MP3_RECORD_WRITE_BUFFER;
            //            break;



        case MP3_RECORD_WRITE_BUFFER:

            LED_BLUE_ON();
            put_rc(f_write(fp, stream, read, &write));
            LEDs_OFF();
            //	    if (read != write)
            //		while (1);
            mp3RecSM = MP3_RECORD_READ_BUFFER;
            break;



        case MP3_RECORD_PAUSE_WAIT_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_REC);
            // Sets the bit to enter in pause during recording
            VLSI_SetBitRecMode();
            // Read pending data and write to file
            read = VLSIGetArray(stream, STREAM_BUF_SIZE_RECORD, 1);
            put_rc(f_write(fp, stream, read, &write));

            mp3RecSM = MP3_RECORD_PAUSE_WAIT;
            break;

        case MP3_RECORD_PAUSE_WAIT:
            // Wait until pause is resent
            Toggle1Second();
            break;


        case MP3_RECORD_PAUSE_DELAY_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_REC);
            GpioUpdateOutputState(GPIO_BIT_PAUSE_REC);
            // Sets the bit to enter in pause during recording
            VLSI_SetBitRecMode();
            // Read pending data and write to file
            read = VLSIGetArray(stream, STREAM_BUF_SIZE_RECORD, 1);
            put_rc(f_write(fp, stream, read, &write));

            mp3RecSM = MP3_RECORD_PAUSE_DELAY;
            break;

        case MP3_RECORD_PAUSE_DELAY:
            if (TickGet() - tick_delay >= tick_max)
                mp3RecSM = MP3_RECORD_PAUSE_EXIT;
            Toggle1Second();
            break;

        case MP3_RECORD_PAUSE_EXIT:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_REC);
            VLSI_ClearBitRecMode();
            mp3RecSM = MP3_RECORD_WRITE_BUFFER;
            break;






        case MP3_RECORD_SEND_FINISH_RECORDING:

            GpioUpdateOutputState(GPIO_BIT_STOP_REC);
            // Send stop signal to encoder
            VLSI_SendFinishRecording();

            verbosePrintf(VER_DBG, "Send finish recording");
            mp3RecSM = MP3_RECORD_FINISH_RECORDING;
            break;



        case MP3_RECORD_FINISH_RECORDING:
            // Finish to plaing the current song until Stop indicator is cleared or have passed 1000ms

            //fwrite(stream, sizeof (char), read, fp);
            f_write(fp, stream, read, &write);
            read = VLSIGetArray(stream, STREAM_BUF_SIZE_RECORD, 0);


            //            if (read == 0 && VLSI_IsClearedSmCancel() && (TickGet() - tick_delay) >= TICK_SECOND / 4ul) {
            //                // Normal condition termination
            //                if (VLSI_GetEndFillByte(&byte)) {
            //                    // There was still a byte to be written, write it
            //                    fwrite(&byte, sizeof (char), 1, fp);
            //                }
            //            }
            if (read == 0 && VLSI_GetEndFillByte(&byte)) {
                // There was still a byte to be written, write it
                //fwrite(&byte, sizeof (char), 1, fp);
                f_write(fp, &byte, 1, &write);
            }

            // When all samples have been transmitted, SM_ENCODE bit of SCI_MODE will be
            // cleared by VS1063a, and SCI_HDAT1 and SCI_HDAT0 are cleared.
            if (VLSI_IsClearedSmEncode()) {
                mp3RecSM = MP3_RECORD_FINALIZE;
            }

            break;



        case MP3_RECORD_FINALIZE:

            verbosePrintf(VER_DBG, "Reset encoder");
            VLSI_SoftReset();
            mp3RecSM = MP3_RECORD_CLOSE_FILE;
            break;



        case MP3_RECORD_CLOSE_FILE:
            verbosePrintf(VER_DBG, "Try to close file");
            if (f_close(fp) != FR_OK)
                mp3RecSM = MP3_RECORD_CLOSED_FAILED;
            else
                mp3RecSM = MP3_RECORD_CLOSED_SUCCESSFUL;
            break;

        case MP3_RECORD_CLOSED_SUCCESSFUL:
            verbosePrintf(VER_DBG, "File closed successful");
            CliCreateFileListOfFilesEntry();
            // Goto idle state machine
            mp3RecSM = MP3_RECORD_HOME;
            break;



        case MP3_RECORD_CLOSED_FAILED:
            verbosePrintf(VER_DBG, "File: %s not closed", Lfname);
            FlashLight(100, 100, FALSE);
            // Goto idle state machine
            mp3RecSM = MP3_RECORD_HOME;
            break;


        default:
            verbosePrintf(VER_DBG, "Return to idle state");
            // Goto idle state machine
            mp3RecSM = MP3_RECORD_HOME;
            break;
    }

    return mp3RecSM;
}

void SetBitRate(long samplerate, int bitrate, int gain, int maxgain, BOOL input, char mode, char format, BOOL reservoir) {

    //BOOL reservoir = FALSE;
    char bitrateMultiplier;

    // Check samplerate
    switch (samplerate) {
        case SAMPLERATE_8000:
        case SAMPLERATE_11025:
        case SAMPLERATE_12000:
        case SAMPLERATE_16000:
        case SAMPLERATE_22050:
        case SAMPLERATE_24000:
        case SAMPLERATE_32000:
        case SAMPLERATE_44100:
        case SAMPLERATE_48000:
            break;
        default:
            samplerate = SAMPLERATE_44100;
    }
    recPreSet.sci_aictrl0.bits.samplerate = samplerate;

    // Check bitrate
    switch (bitrate) {
        case QUALITY_0:
        case QUALITY_1:
        case QUALITY_2:
        case QUALITY_3:
        case QUALITY_4:
        case QUALITY_5:
        case QUALITY_6:
        case QUALITY_7:
        case QUALITY_8:
        case QUALITY_9:

        case BITRATE_32:
        case BITRATE_40:
        case BITRATE_48:
        case BITRATE_56:
        case BITRATE_64:
        case BITRATE_80:
        case BITRATE_96:
        case BITRATE_112:
        case BITRATE_128:
        case BITRATE_160:
        case BITRATE_192:
        case BITRATE_224:
        case BITRATE_256:
        case BITRATE_320:
            break;
        default:
            if (mode == QUALITY_MODE)
                bitrate = QUALITY_5;
            else
                bitrate = BITRATE_128;
    }
    bitrateMultiplier = BITRATE_MULTIPLIER_X1000;


    if (gain > 63)
        gain = 63;
    recPreSet.sci_aictrl1.bits.gain = gain * 1024;

    if (maxgain > 63)
        maxgain = 63;
    recPreSet.sci_aictrl2.bits.maxAutogain = maxgain * 1024;

    // Selection of the mode operation
    switch (mode) {
        case QUALITY_MODE:
        case VBR_MODE:
        case ABR_MODE:
        case CBR_MODE:
            break;
        default:
            mode = VBR_MODE;
    }

    // Selection of the appropriate format. 0 = IMA ADPCM, 1 = PCM , 2 = G.711 u-law, 3 = G.711 A-law, 4 = G.722 ADPCM, 5 = Ogg Vorbis, 6 = MP3
    if (format == REC_OGG) {
        // 5 << 4 Ogg Format
        recPreSet.sci_aictrl3.bits.EncodingFormat = 0x5;
        // Ogg Vorbis: 1 = limited frame length
        //             0 = do not limit frame length
        reservoir = 0;

    } else {
        // 6 << 4 MP3 Format
        recPreSet.sci_aictrl3.bits.EncodingFormat = 0x6;
        // The so-called MP3 bit reservoir offers a way to more efficiently encode MP3 files. To make
        // streaming more resilient to transmission errors, encoder only makes bit reservoir references
        // one frame back.

        // For some streaming applications it may be beneficial to turn the bit reservoir off by setting bit
        // 10 of register SCI_WRAMADDR before activating encoding. This will make frames more selfcontained.
        // When using ABR/VBR/Quality encoding, turning bit reservoir off will increase the
        // bitrate approximately 4... 16 kbit/s. Turning bit reservoir off in CBR mode is strongly discouraged
        // as it will have a huge impact in quality and coding efficiency.
        if (mode == 3)
            // in CBR mode bit reservoir is always on (bit-reservoir == 0)
            reservoir = BIT_RESERVOIR_ON;
    }

    //recPreSet.SCI_WRAMADDR = mode << 14 | reservoir << 11 | bitrateMultiplier << 12 | bitrate;
    recPreSet.sci_wramaddr.bits.bitrateBase = bitrate;
    recPreSet.sci_wramaddr.bits.bitrateMultiplier = bitrateMultiplier;
    recPreSet.sci_wramaddr.bits.MP3bitReservoir_OggLimitedFrameLength = reservoir;
    recPreSet.sci_wramaddr.bits.bitrateMode = mode;

    if (input == INPUT_MIC)
        recPreSet.sci_mode.word = VLSI_VAL_MODE_ENCODE_MIC;
    else
        recPreSet.sci_mode.word = VLSI_VAL_MODE_ENCODE_LINE;

}

void Record(int argc, char **argv) {

    int samplerate, bitrate, gain, maxGain, mode, format;

    if (argc == 1) {
        // Copy in Lfname gloabal variable the name of the default file
        strncpy(Lfname, config.record.r_name, _MAX_LFN);
        // Struct recording information
        SetBitRate(config.record.samplerate, config.record.bitrate, config.record.gain, config.record.max_gain, INPUT_LINE, config.record.bitrate_mode, config.record.format, BIT_RESERVOIR_ON);
        // Put in recording mode
        mp3RecSM = MP3_RECORD_OPEN_FILE;
    } else if (argc == 2) {
        // Extract the name of the recording file
        strncpy(Lfname, argv[1], _MAX_LFN);
        // Struct recording information
        SetBitRate(config.record.samplerate, config.record.bitrate, config.record.gain, config.record.max_gain, INPUT_LINE, config.record.bitrate_mode, config.record.format, BIT_RESERVOIR_ON);
        // Put in recording mode
        mp3RecSM = MP3_RECORD_OPEN_FILE;
    } else if (argc == 8) {
        // Extract the name of the recording file
        strncpy(Lfname, argv[1], _MAX_LFN);
        // Extract the samplerate
        samplerate = atoimm(argv[2], SAMPLERATE_8000, SAMPLERATE_48000, SAMPLERATE_44100);
        // Extract the bitrate
        bitrate = atoimm(argv[3], QUALITY_0, BITRATE_320, BITRATE_128);
        // Extract the gain
        gain = atoimm(argv[4], GAIN_0, GAIN_63, GAIN_10);
        // Extract the max auto gain
        maxGain = atoimm(argv[5], GAIN_1, GAIN_63, GAIN_10);
        // Extract mode
        mode = atoimm(argv[6], QUALITY_MODE, CBR_MODE, VBR_MODE);
        // Extract format
        format = atoimm(argv[7], REC_MP3, REC_OGG, REC_MP3);

        // Struct recording information
        SetBitRate(samplerate, bitrate, gain, maxGain, INPUT_LINE, mode, format, BIT_RESERVOIR_ON);
        // Put in recording mode
        mp3RecSM = MP3_RECORD_OPEN_FILE;

    } else {
        CliTooManyArgumnets(argv[0]);
    }
}

BOOL PauseRecord(int argc, char **argv) {

    LONG delay;

    if (argc == 1) {

        if (mp3RecSM >= MP3_RECORD_READ_BUFFER && mp3RecSM <= MP3_RECORD_WRITE_BUFFER) {
            // Enter in pause
            mp3RecSM = MP3_RECORD_PAUSE_WAIT_ENTERING;
            printf("Pause: ON\r\n");
            return TRUE;
        } else if (mp3RecSM >= MP3_RECORD_PAUSE_WAIT && mp3RecSM <= MP3_RECORD_PAUSE_DELAY) {
            // Exit from pause
            mp3RecSM = MP3_RECORD_PAUSE_EXIT;
            printf("Pause: OFF\r\n");
            return TRUE;
        }

    } else if (argc == 2) {

        if (mp3RecSM >= MP3_RECORD_READ_BUFFER && mp3RecSM <= MP3_RECORD_WRITE_BUFFER) {
            delay = atoimm(argv[1], 0, 10000000, 1000);

            tick_delay = TickGet();
            tick_max = TICK_SECOND / 1000 * delay;
            mp3RecSM = MP3_RECORD_PAUSE_DELAY_ENTERING;

            // Aggiungere il corretto pause del record
            //VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
            //            mp3RecSM = MP3_RECORD_PAUSE_DELAY;
            //            GpioUpdateState(GPIO_BIT_PAUSE_REC);

            printf("Pause for %ld ms\r\n", delay);
            return TRUE;
        }

    } else {
        CliTooManyArgumnets(argv[0]);
    }

    return FALSE;
}

BOOL StopRecord(int argc, char **argv) {


    if (argc == 1) {
        if (mp3RecSM >= MP3_RECORD_OPEN_FILE && mp3RecSM <= MP3_RECORD_FINISH_RECORDING) {
            //VLSI_FinishRecording();
            //r_check = TRUE;
            tick_delay = TickGet();
            mp3RecSM = MP3_RECORD_SEND_FINISH_RECORDING;
        } else {
            return FALSE;
        }
    } else if (argc == 2) {

        // posso aggiungere un ritardo ricopiando la struttura di Pause
        // anche kill

    } else {
        CliTooManyArgumnets(argv[0]);
        return FALSE;
    }
    return TRUE;
}

BOOL InfoRecord(int argc, char **argv) {

    _REC_PRESET recPreSet;
    const char *str_bitrate_mode[] = {"Quality Mode", "VBR", "ABR", "CBR"};
    const char *str_encoding_format[] = {"IMA ADPCM", "PCM", "G.711 u-law", "G.711 A-law", "G.722 ADPCM", "Ogg Vorbis", "MP3"};

    if (argc == 1) {
        // Print the track name in play otherwise print no track in execution
        if (mp3RecSM > MP3_RECORD_OPENED_SUCCESSFUL && mp3RecSM <= MP3_RECORD_CLOSED_SUCCESSFUL) {
            // First update the _REC_PRESET registers
            VLSI_GetRecordingInfo(&recPreSet);
            // Then prints all updatetd info
            printf("Filename: %s;\r\n", Lfname);
            if (recPreSet.sci_wramaddr.bits.bitrateMode == 0)
                printf("%s: %d;\r\n", str_bitrate_mode[recPreSet.sci_wramaddr.bits.bitrateMode], recPreSet.sci_wramaddr.bits.bitrateBase);
            else
                printf("Bitrate mode: %s @ %dkbit/s;\r\n", str_bitrate_mode[recPreSet.sci_wramaddr.bits.bitrateMode], recPreSet.sci_wramaddr.bits.bitrateBase);
            printf("Samplerate: %dHz;\r\n", recPreSet.sci_aictrl0.bits.samplerate);
            printf("Gain: %dx;\r\n", recPreSet.sci_aictrl1.bits.gain / 1024);
            printf("Autogain: %dx;\r\n", recPreSet.sci_aictrl2.bits.maxAutogain / 1024);
            printf("Encoding format: %s;\r\n", str_encoding_format[recPreSet.sci_aictrl3.bits.EncodingFormat]);
            printf("Input: %s;\r\n", (recPreSet.sci_mode.bits.SM_LINE1 ? "Line" : "Mic"));
            if (recPreSet.sci_aictrl3.bits.EncodingFormat == 5) { // Ogg
                printf("Limited frame length: %s;\r\n", (recPreSet.sci_wramaddr.bits.MP3bitReservoir_OggLimitedFrameLength ? "Off" : "On"));
            } else if (recPreSet.sci_aictrl3.bits.EncodingFormat == 6) { // MP3
                printf("Bit reservoir: %s;\r\n", (recPreSet.sci_wramaddr.bits.MP3bitReservoir_OggLimitedFrameLength ? "Off" : "On"));
            }
            return TRUE;
        }
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return FALSE;
}