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

#include <xc.h>

#include "MP3/Record.h"
#include "MP3/MP3.h"
#include "MP3/VS1063.h"

#include "Utilities/printer.h"
#include "CommandLineInterpreter.h"
#include "Utilities/Config.h"
#include "Utilities/Utilities.h"
#include "Delay/Tick.h"
#include "Utilities/GPIO.h"
#include "MP3/Play.h"
#include "Commands.h"
#include "Delay/Delay.h"
#include "MP3/MP3Frame.h"
#include "Utilities/RTCC.h"


_REC_PRESET recPreSet;
//BOOL timeout;
// See ffconf.h for dimension of LFN
extern TCHAR Lfname[];
// See MP3.h for dimension of stream
extern char stream[];



REC_CONFIG rec;

//BOOL dmaTXBusyFlag;

//void __ISR(_DMA2_VECTOR, IPL5AUTO) DmaHandler2(void) {
//    int evFlags; // event flags when getting the interrupt
//
//    INTClearFlag(INT_SOURCE_DMA(DMA_CHANNEL2)); // release the interrupt in the INT controller, we're servicing int
//
//    evFlags = DmaChnGetEvFlags(DMA_CHANNEL2); // get the event flags
//
//    if (evFlags & DMA_EV_BLOCK_DONE) { // just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
//        dmaTXBusyFlag = FALSE;
//        DmaChnClrEvFlags(DMA_CHANNEL2, DMA_EV_BLOCK_DONE);
//    }
////    if (evFlags & DMA_EV_CELL_DONE) { // just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
////        DmaChnClrEvFlags(DMA_CHANNEL2, DMA_EV_CELL_DONE);
////    }
//    
//}

int RecordTaskHandler() {

    extern int (*commandToCall)(int, char**);
    extern int argc;
    extern char *argv[];
    extern FIL fstream, ftmp2;
    //extern TCHAR Lfname[_MAX_LFN + 1];
    BYTE index;
    DWORD nextFrame, frameLength;
    UINT written, size;
    WORD i, j;
    char byte, *p, *b;
    double duration;

    FAT_TIME fat_time;
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;

    switch (rec.sm) {

        case SM_REC_HOME:
            // Do nothing, wait a record command
            break;


        case SM_REC_OPEN_FILE:

            // Configure the DMA Channel 3 to Recording purpose
            DmaChnOpen(REC_DMA_CHANNEL, DMA_CHN_PRI3, DMA_OPEN_DEFAULT);
            // Clear flags indicators
            //REC_DMA_CLR_BTC();
            REC_DMA_SET_BTC();

            //DmaChnSetEventControl(REC_DMA_CHANNEL, DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(_DMA2_IRQ));

            //            // enable the transfer done interrupt: all the characters transferred
            //            DmaChnSetEvEnableFlags(REC_DMA_CHANNEL, DMA_EV_BLOCK_DONE);
            //            // enable system wide multi vectored interrupts
            //            INTSetVectorPriority(INT_VECTOR_DMA(REC_DMA_CHANNEL), INT_PRIORITY_LEVEL_5); // set INT controller priority
            //            INTSetVectorSubPriority(INT_VECTOR_DMA(REC_DMA_CHANNEL), INT_SUB_PRIORITY_LEVEL_3); // set INT controller sub-priority
            //            // enable the chn interrupt in the INT controller
            //            INTEnable(INT_SOURCE_DMA(REC_DMA_CHANNEL), INT_ENABLED);
            //            // Reset busy flag indicator
            //            dmaTXBusyFlag = FALSE;

            //            LED_RED_ON()
            //            for (i = STREAM_BUF_SIZE / 2; i < STREAM_BUF_SIZE; i++)
            //                stream[i] = rand();
            //            LEDs_OFF();

            //            DelayMs(10);
            //            // Test velovità DMA
            //            LED_RED_ON()
            //            for (j = 0, i = STREAM_BUF_SIZE / 2; i < STREAM_BUF_SIZE; i++, j++)
            //                stream[j] = stream[i];
            //            LEDs_OFF();
            //            DelayMs(10);
            //
            //            for (i = 0, j = STREAM_BUF_SIZE / 2; i < STREAM_BUF_SIZE / 2; i++, j++)
            //                if (stream[i] != stream[j])
            //                    while (1);


            //            Nop();
            //            DelayMs(10);
            //            LED_RED_ON();
            //            dmaTXBusyFlag = TRUE;
            //            int size2 = STREAM_BUF_SIZE / 2;
            //            DmaChnSetTxfer(REC_DMA_CHANNEL, &stream[STREAM_BUF_SIZE / 2], &stream[0], size2, size2, size2);
            //            DmaChnStartTxfer(REC_DMA_CHANNEL, DMA_WAIT_NOT, 0);
            //            //            for (i = 0; dmaTXBusyFlag; i++)
            //            //                stream[i] = 0x00;
            //            //while (dmaTXBusyFlag);
            //            while (!DCH2INTbits.CHBCIF);
            //            DmaChnClrEvFlags(DMA_CHANNEL2, DMA_EV_CELL_DONE);
            //            LEDs_OFF();
            //            DelayMs(10);
            //            Nop();
            //
            //            for (i = 0, j = STREAM_BUF_SIZE / 2; i < STREAM_BUF_SIZE / 2; i++, j++)
            //                if (stream[i] != stream[j])
            //                    while (1);

            // Save the global file ponter in a local pointer
            rec.fp[0] = &fstream;
            if (rec.intervalRec) {
                rec.sm = SM_REC_OPENED_SUCCESSFUL;
                // Do samething for the second file pointer
                rec.fp[1] = &ftmp2;
                // Get date and time
                rtccGetDateAndTime(&rec.year, &rec.mon, &rec.day, &rec.hour, &rec.mins, &rec.sec);
                sprintf(Lfname, "/%04d%02d%02d", rec.year, rec.mon, rec.day);
                rec.fres = f_mkdir(Lfname);
                //                rec.fno.fdate = fat_time.word.data;
                //                rec.fno.ftime = fat_time.word.time;
                //                rec.fres = f_utime(Lfname, &rec.fno);
                if (rec.fres == FR_OK || rec.fres == FR_EXIST) {
                    sprintf(Lfname, "/%04d%02d%02d/%02d%02d%02d.mp3", rec.year, rec.mon, rec.day, rec.hour, rec.mins, rec.sec);
                    rec.fres = f_open(rec.fp[0], Lfname, FA_WRITE | FA_CREATE_ALWAYS);
                    rtccIncDateAndTime(&rec.year, &rec.mon, &rec.day, &rec.hour, &rec.mins, &rec.sec, rec.intervalRec);
                    sprintf(Lfname, "/%04d%02d%02d/%02d%02d%02d.mp3", rec.year, rec.mon, rec.day, rec.hour, rec.mins, rec.sec);
                    rec.fres |= f_open(rec.fp[1], Lfname, FA_WRITE | FA_CREATE_ALWAYS);
                    if (rec.fres != FR_OK)
                        rec.sm = SM_REC_OPENED_FAILED;
                } else {
                    rec.sm = SM_REC_OPENED_FAILED;
                }
            } else {
                // Time intervals is disabled
                // Checks if incremental mode is enabled otherwise overrides file
                if (config.record.prog_over == 0) {
                    verbosePrintf(VER_DBG, "Try to open or override: %s", Lfname);
                    rec.fres = f_open(rec.fp[0], Lfname, FA_WRITE | FA_CREATE_ALWAYS);
                } else {
                    verbosePrintf(VER_DBG, "Try to open: %s", Lfname);
                    rec.fres = f_open(rec.fp[0], Lfname, FA_WRITE | FA_CREATE_NEW);
                }

                if (rec.fres == FR_EXIST) {
                    rec.sm = SM_REC_OPEN_NEXT_FILE;
                } else if (rec.fres == FR_OK) {
                    rec.sm = SM_REC_OPENED_SUCCESSFUL;
                } else {
                    rec.sm = SM_REC_OPENED_FAILED;
                }
            }
            break;

        case SM_REC_OPEN_NEXT_FILE:

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
                rec.read = atoi(b + 1);
            } else {
                rec.read = 0;
                b = p;
            }
            // Copy in Lfname the incremental number and previous extension
            sprintf(b, "(%d)%s", ++rec.read, stream);

            rec.sm = SM_REC_OPEN_FILE;
            break;


        case SM_REC_OPENED_SUCCESSFUL:

            GpioUpdateOutputState(GPIO_BIT_STARTS_REC);

            verbosePrintf(VER_DBG, "File opened successfully");

            // Reinitialize the VS1063 and load patch
            VLSI_SoftReset();

            // Send all the parameters and start recording
            VLSI_InitRecording(&recPreSet);

            rec.toWrite = rec.read = 0;
            rec.alt = rec.fpIndex = 0;
            rec.sm = SM_REC_READ_BUFFER;

            rec.longDuration = 0;
            rec.frameCount = rec.fileLength = 0;
            rec.smSub = SM_REC_SUB_DEFAULT;
            break;


        case SM_REC_OPENED_FAILED:
            verbosePrintf(VER_MIN, "Cannot open: %s", Lfname);
            GpioUpdateOutputState(GPIO_BIT_FILE_NOT_FOUND);
            FlashLight(100, 10, FALSE);
            rec.sm = SM_REC_HOME;
            break;



        case SM_REC_READ_BUFFER:
            // Check the DMA busy flag
            if (REC_DMA_WORKING())
                break;

            // The absolute theoretical maximum frame size is 2881 bytes: MPEG 2.5 Layer II, 8000 Hz @ 160 kbps, with a padding slot.
            if (rec.read >= 4u && CheckFrameSyncBufferHead(stream) == FALSE) {
                for (i = 0; i < rec.read; i++)
                    if (0xFF == (BYTE) stream[i])
                        if (rec.read - i >= 4u && CheckFrameSyncBufferHead(&stream[i]) > 0)
                            break;
                // Clear Block transfer complete interrupt flag
                REC_DMA_CLR_BTC();
                // Set the transfer source and dest addresses, source and dest size and cell size
                size = rec.read - i;
                DmaChnSetTxfer(REC_DMA_CHANNEL, &stream[i], &stream[0], size, size, size);
                DmaChnStartTxfer(REC_DMA_CHANNEL, DMA_WAIT_NOT, 0);
                rec.read = size; // There are an error reset the buffer
                break;
            }

            // The encoding data buffer is 3712 16-bit words => 7424 bytes
            rec.read += VLSIGetArray(&stream[rec.read], STREAM_BUF_SIZE_RECORD - rec.read, 4);


            if (rec.intervalRec) {
                if (rec.longDuration >= rec.intervalRec) {
                    rec.smSub = SM_REC_SUB_CLOSE;
                    rec.alt++;
                    rec.fpIndex = rec.alt % 2;
                    rec.longDuration = 0;
                }
                if (rec.read == 0) {
                    switch (rec.smSub) {
                        case SM_REC_SUB_CLOSE:
                            index = (rec.alt + 1) % 2;
                            rec.fres = f_close(rec.fp[index]);
                            if (rec.fres != FR_OK) {
                                rec.sm = SM_REC_PUT_ERROR;
                                break;
                            }
                            rec.smSub++;
                            break;

                        case SM_REC_SUB_FS_STAT:
                            rec.smSub = SM_REC_SUB_MKDIR;
                            rec.fres = f_getfree("0", &fre_clust, &fs);
                            if (rec.fres != FR_OK) {
                                rec.sm = SM_REC_PUT_ERROR;
                                break;
                            }
                            // Get total sectors and free sectors size in Kbyte
                            tot_sect = (fs->n_fatent - 2) * fs->csize / 2;
                            fre_sect = fre_clust * fs->csize / 2;
                            // Check the free space in kByte
                            if (fre_sect < 1000 * 1024) {
                                rec.smSub = SM_REC_SUB_OPEN_DIR;
                            }
                            break;

                        case SM_REC_SUB_OPEN_DIR:
                            if ((rec.fres = f_opendir(&rec.dir, "/")) == FR_OK) {
                                rec.smSub++;
                                //rec.fno.lfname = Lfname;
                                //rec.fno.lfsize = sizeof (Lfname);
                                rec.ffind = FALSE;
                            } else {
                                rec.smSub = SM_REC_PUT_ERROR;
                            }
                            break;

                        case SM_REC_SUB_READ_DIR:
                            if ((rec.fres = f_readdir(&rec.dir, &rec.fno)) == FR_OK) {
                                if (!rec.fno.fname[0]) {
                                    if (rec.ffind)
                                        rec.smSub = SM_REC_SUB_DELETE_DIR;
                                    else
                                        // Current directory is empty
                                        rec.smSub = SM_REC_SUB_CLOSE_DIR;
                                } else {
                                    // Check dircetory entry
                                    if (rec.fno.fattrib & AM_DIR) {
                                        if (rec.ffind == FALSE) {
                                            strcpy(rec.lfname, GetFileNamePointer(&rec.fno));
                                            rec.fat_time.word.data = rec.fno.fdate;
                                            rec.fat_time.word.time = rec.fno.ftime;
                                            rec.ffind = TRUE;
                                        } else {
                                            fat_time.word.data = rec.fno.fdate;
                                            fat_time.word.time = rec.fno.ftime;
                                            if (fat_time.val < rec.fat_time.val) {
                                                strcpy(rec.lfname, GetFileNamePointer(&rec.fno));
                                                fat_time.word.data = rec.fno.fdate;
                                                fat_time.word.time = rec.fno.ftime;
                                            }
                                        }
                                    }
                                }
                            } else {
                                rec.sm = SM_REC_PUT_ERROR;
                            }
                            break;
                        case SM_REC_SUB_DELETE_DIR:
                            // Inviare il comando per la cancellazione di una directory
                            argv[0] = "rec";
                            argv[1] = rec.lfname;
                            argv[2] = "-not-empty"; //(char*) notEmpty;
                            argc = 3;
                            commandToCall = DeleteDir;
                            rec.smSub = SM_REC_SUB_CLOSE_DIR;
                            break;

                        case SM_REC_SUB_CLOSE_DIR:
                            f_closedir(&rec.dir);
                            //rec.smSub = SM_REC_SUB_FS_STAT;
                            rec.smSub = SM_REC_SUB_MKDIR;
                            break;

                        case SM_REC_SUB_MKDIR:
                            rtccGetDateAndTime(&rec.year, &rec.mon, &rec.day, &rec.hour, &rec.mins, &rec.sec);
                            rtccIncDateAndTime(&rec.year, &rec.mon, &rec.day, &rec.hour, &rec.mins, &rec.sec, rec.intervalRec);
                            sprintf(Lfname, "/%04d%02d%02d", rec.year, rec.mon, rec.day);
                            rec.fres = f_mkdir(Lfname);
                            if (rec.fres == FR_OK || rec.fres == FR_EXIST) {
                                rec.smSub++;
                            } else {
                                rec.sm = SM_REC_OPENED_FAILED;
                                break;
                            }
                            break;

                        case SM_REC_SUB_OPEN_FILE:
                            //rtccGetTime(&hour, &mins, &sec);
                            sprintf(Lfname, "/%04d%02d%02d/%02d%02d%02d.mp3", rec.year, rec.mon, rec.day, rec.hour, rec.mins, rec.sec);
                            index = (rec.alt + 1) % 2;
                            rec.fres = f_open(rec.fp[index], Lfname, FA_WRITE | FA_CREATE_ALWAYS);
                            if (rec.fres != FR_OK) {
                                rec.sm = SM_REC_OPENED_FAILED;
                                break;
                            }
                            rec.smSub++;
                            break;
                    }
                    break;
                } else {
                    if (rec.read >= 4u && CheckFrameSyncBufferHead(stream) == FALSE) {
                        Nop();
                    }
                    rec.toWrite = nextFrame = 0;
                    while ((rec.read - nextFrame) >= 4u && (frameLength = CalcFrameSize(&stream[nextFrame], &duration)) > 0u) {
                        // framLength represents the next frame to be written
                        nextFrame += frameLength;
                        if (nextFrame <= rec.read) {
                            rec.toWrite += frameLength;
                            rec.frameCount++;
                            rec.longDuration += duration;
                            rec.sm = SM_REC_WRITE_BUFFER;
                            if (rec.longDuration >= rec.intervalRec)
                                break;
                        } else {
                            // We have only the first bytes of the next frame
                            break;
                        }
                    }
                    if (rec.toWrite == 0)
                        break;
                }
            } else {
                // Recording intervals disabled
                if (rec.read == 0)
                    break;
                rec.toWrite = rec.read;
            }
            //rec.sm++;

        case SM_REC_WRITE_BUFFER:

            LED_BLUE_ON();
            rec.fres = f_write(rec.fp[rec.fpIndex], stream, rec.toWrite, &written);
            if (rec.fres != FR_OK) {
                rec.sm = SM_REC_PUT_ERROR;
                break;
            }
            // Reposition to beginning of the buffer the byte is not yet copied into the micro SD card
            size = rec.read - written;
            if (size > 0) {
                // Clear Block transfer complete interrupt flag
                REC_DMA_CLR_BTC();
                // Set the transfer source and dest addresses, source and dest size and cell size
                DmaChnSetTxfer(REC_DMA_CHANNEL, &stream[written], &stream[0], size, size, size);
                DmaChnStartTxfer(REC_DMA_CHANNEL, DMA_WAIT_NOT, 0);
            }
            rec.toWrite = size;
            rec.read = size;
            rec.fileLength += written;
            rec.sm = SM_REC_READ_BUFFER;
            LEDs_OFF();
            break;




        case SM_REC_PAUSE_WAIT_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_REC);
            // Sets the bit to enter in pause during recording
            VLSI_SetBitRecMode();
            // Read pending data and write to file
            rec.read += VLSIGetArray(stream, STREAM_BUF_SIZE_RECORD, 4);
            //put_rc(f_write(rec.fp[rec.fpIndex], stream, rec.read, &rec.written));
            //rec.read = rec.read - rec.written;

            rec.sm = SM_REC_PAUSE_WAIT;
            break;

        case SM_REC_PAUSE_WAIT:
            // Wait until pause is resent
            Toggle1Second();
            break;


        case SM_REC_PAUSE_DELAY_ENTERING:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_REC);
            // Sets the bit to enter in pause during recording
            VLSI_SetBitRecMode();
            // Read pending data and write to file
            rec.read += VLSIGetArray(stream, STREAM_BUF_SIZE_RECORD, 4);
            //put_rc(f_write(rec.fp[rec.fpIndex], stream, rec.read, &rec.written));
            //rec.read = rec.read - rec.written;

            rec.sm = SM_REC_PAUSE_DELAY;
            break;

        case SM_REC_PAUSE_DELAY:
            if (TickGet() - rec.tick_delay >= rec.tick_max)
                rec.sm = SM_REC_PAUSE_EXIT;
            Toggle1Second();
            break;

        case SM_REC_PAUSE_EXIT:
            GpioUpdateOutputState(GPIO_BIT_PAUSE_REC);
            VLSI_ClearBitRecMode();
            rec.sm = SM_REC_READ_BUFFER;
            break;






        case SM_REC_SEND_FINISH_RECORDING:

            GpioUpdateOutputState(GPIO_BIT_STOP_REC);
            // Send stop signal to encoder
            VLSI_SendFinishRecording();

            verbosePrintf(VER_DBG, "Send finish recording");
            rec.sm = SM_REC_FINISH_RECORDING;
            break;



        case SM_REC_FINISH_RECORDING:
            // Finish to plaing the current song until Stop indicator is cleared or have passed 1000ms

            //fwrite(stream, sizeof (char), rec.read, fp);
            rec.fres = f_write(rec.fp[rec.fpIndex], stream, rec.read, &written);
            rec.read = VLSIGetArray(stream, STREAM_BUF_SIZE_RECORD, 0);


            //            if (rec.read == 0 && VLSI_IsClearedSmCancel() && (TickGet() - rec.tick_delay) >= TICK_SECOND / 4ul) {
            //                // Normal condition termination
            //                if (VLSI_GetEndFillByte(&byte)) {
            //                    // There was still a byte to be written, write it
            //                    fwrite(&byte, sizeof (char), 1, fp);
            //                }
            //            }
            if (rec.read == 0 && VLSI_GetEndFillByte(&byte)) {
                // There was still a byte to be written, write it
                //fwrite(&byte, sizeof (char), 1, fp);
                rec.fres = f_write(rec.fp[rec.fpIndex], &byte, 1, &written);
            }

            // When all samples have been transmitted, SM_ENCODE bit of SCI_MODE will be
            // cleared by VS1063a, and SCI_HDAT1 and SCI_HDAT0 are cleared.
            if (VLSI_IsClearedSmEncode()) {
                rec.sm = SM_REC_FINALIZE;
            }

            break;



        case SM_REC_FINALIZE:
            verbosePrintf(VER_DBG, "Reset encoder");
            VLSI_SoftReset();
            rec.sm = SM_REC_CLOSE_FILE;
            break;



        case SM_REC_CLOSE_FILE:
            verbosePrintf(VER_DBG, "Try to close file");
            if (rec.intervalRec) {
                if (f_close(rec.fp[0]) != FR_OK || f_close(rec.fp[1]) != FR_OK) {
                    rec.sm = SM_REC_CLOSED_FAILED;
                } else {
                    sprintf(Lfname, "/%04d%02d%02d/%02d%02d%02d.mp3", rec.year, rec.mon, rec.day, rec.hour, rec.mins, rec.sec);
                    rec.fres = f_unlink(Lfname);
                    if (rec.fres == FR_OK)
                        rec.sm = SM_REC_CLOSED_SUCCESSFUL;
                    else
                        rec.sm = SM_REC_CLOSED_FAILED;
                }
            } else {
                if (f_close(rec.fp[0]) != FR_OK)
                    rec.sm = SM_REC_CLOSED_FAILED;
                else
                    rec.sm = SM_REC_CLOSED_SUCCESSFUL;
            }
            break;

        case SM_REC_CLOSED_SUCCESSFUL:
            verbosePrintf(VER_DBG, "File closed successful");
            CliCreateFileListOfFilesEntry();
            // Goto idle state machine
            rec.sm = SM_REC_HOME;
            break;

        case SM_REC_CLOSED_FAILED:
            verbosePrintf(VER_DBG, "File: %s not closed", Lfname);
            FlashLight(100, 100, FALSE);
            //            rec.sm = SM_REC_PUT_ERROR;
            //            break;

        case SM_REC_PUT_ERROR:
            put_rc(rec.fres);
            rec.sm = SM_REC_HOME;
            break;


        default:
            verbosePrintf(VER_DBG, "Return to idle state");
            // Goto idle state machine
            rec.sm = SM_REC_HOME;

            break;
    }

    return rec.sm;
}

void TestRecInternalGet(void) {
    if (rec.sm == SM_REC_WRITE_BUFFER || rec.sm == SM_REC_READ_BUFFER) {
        // The encoding data buffer is 3712 16-bit words => 7424 bytes
        rec.read += VLSIGetArray(&stream[rec.read], STREAM_BUF_SIZE_RECORD - rec.read, 64);
    }
}

void SetBitRate(long samplerate, int bitrate, int gain, int maxgain, BOOL input, char mode, char format, char adc, BOOL reservoir) {

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

    // SCI_AICTRL3 bits 0 to 2 select the ADC mode and implicitly the number of channels. 0 = joint
    // stereo (common AGC), 1 = dual channel (separate AGC), 2 = left channel, 3 = right channel, 4 = mono downmix.
    switch (adc) {
        case JOINT_STEREO:
        case DUAL_CHANNEL:
        case RIGHT_CHANNEL:
        case LEFT_CHANNEL:
        case MONO:
            break;
        default:
            adc = JOINT_STEREO;
    }
    recPreSet.sci_aictrl3.bits.ADC_mode = adc;

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

int Record(int argc, char **argv) {

    int samplerate, bitrate, gain, maxGain, mode, format, adc;

    if (rec.sm != SM_REC_HOME) {
        verbosePrintf(VER_MIN, "Recording already started\r\n");
        return 0;
    }

    if (argc == 1) {
        // Copy in Lfname gloabal variable the name of the default file
        strncpy(Lfname, config.record.r_name, _MAX_LFN);
        // Struct recording information
        SetBitRate(config.record.samplerate, config.record.bitrate, config.record.gain, config.record.max_gain, INPUT_LINE, config.record.bitrate_mode, config.record.format, config.record.adcMode, BIT_RESERVOIR_ON);
        // Disable intervals recording
        rec.intervalRec = 0;
        // Put in recording mode
        rec.sm = SM_REC_OPEN_FILE;
        config.record.prog_over = 2;
    } else if (argc == 2) {
        // Extract the name of the recording file
        strncpy(Lfname, argv[1], _MAX_LFN);
        // Struct recording information
        SetBitRate(config.record.samplerate, config.record.bitrate, config.record.gain, config.record.max_gain, INPUT_LINE, config.record.bitrate_mode, config.record.format, config.record.adcMode, BIT_RESERVOIR_ON);
        // Disable intervals recording
        rec.intervalRec = 0;
        // Put in recording mode
        rec.sm = SM_REC_OPEN_FILE;
    } else if (argc == 10) {
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
        // Extract ADC mode
        adc = atoimm(argv[8], JOINT_STEREO, MONO, JOINT_STEREO);
        // Enable recording intervals
        rec.intervalRec = atoimm(argv[9], MIN_DISABLED_REC, MAX_INT_REC, MID_INT_REC);
        // Struct recording information
        SetBitRate(samplerate, bitrate, gain, maxGain, INPUT_LINE, mode, format, adc, BIT_RESERVOIR_ON);
        // Put in recording mode
        rec.sm = SM_REC_OPEN_FILE;
    } else {
        CliTooManyArgumnets(argv[0]);
    }
    return 0;
}

BOOL PauseRecord(int argc, char **argv) {

    LONG delay;

    if (argc == 1) {

        if (rec.sm >= SM_REC_READ_BUFFER && rec.sm <= SM_REC_WRITE_BUFFER) {
            // Enter in pause
            rec.sm = SM_REC_PAUSE_WAIT_ENTERING;
            printf("Pause: ON\r\n");
            return TRUE;
        } else if (rec.sm >= SM_REC_PAUSE_WAIT && rec.sm <= SM_REC_PAUSE_DELAY) {
            // Exit from pause
            rec.sm = SM_REC_PAUSE_EXIT;
            printf("Pause: OFF\r\n");
            return TRUE;
        }

    } else if (argc == 2) {

        if (rec.sm >= SM_REC_READ_BUFFER && rec.sm <= SM_REC_WRITE_BUFFER) {
            delay = atoimm(argv[1], 0, 10000000, 1000);

            rec.tick_delay = TickGet();
            rec.tick_max = TICK_SECOND / 1000 * delay;
            rec.sm = SM_REC_PAUSE_DELAY_ENTERING;

            // Aggiungere il corretto pause del record
            //VLSI_SetBitPlayMode(PLAYMODE_PAUSE_ON);
            //            rec.sm = MP3_RECORD_PAUSE_DELAY;
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
        if (rec.sm >= SM_REC_OPEN_FILE && rec.sm <= SM_REC_FINISH_RECORDING) {
            //VLSI_FinishRecording();
            //r_check = TRUE;
            rec.tick_delay = TickGet();
            rec.sm = SM_REC_SEND_FINISH_RECORDING;
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
        if (rec.sm > SM_REC_OPENED_SUCCESSFUL && rec.sm <= SM_REC_CLOSED_SUCCESSFUL) {
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