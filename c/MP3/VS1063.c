/*********************************************************************
 *
 *  VS1063 Low Level Subroutines
 *
 *********************************************************************
 * FileName:        VS1063.c
 * Dependencies:    VS1063.h
 * Processor:       PIC32MX270F256B
 * Compiler:        Microchip XC32 v1.32 or higher
 * Company:         LP Systems
 * Author:	    Luca Pascarella www.lucapascarella.it
 *
 * Software License Agreement
 *
 * Copyright (C) 2012-2014 LP Systems  All rights reserved.
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
 * File Description: Low level subroutine entry point to interact with VS1063
 * Change History: In progress
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 *
 ********************************************************************/



#include "MP3/VS1063.h"
#include "Delay/Delay.h"
#include "Utilities/Config.h"
#include "Utilities/Utilities.h"
#include "MP3/VS1063Patches.h"

#define DEBUG

//_VOLUME volume;
_MP3_INFO mp3_info;
_EXTRA_PAR extra_par;
_MP3_RECORD mp3_record;

/****************************************************************************
  Function:
    BOOL InitVLSI(void)

  Description:
    This function initializes the VS1063

  Precondition:
    SPI peripheral already configured and turned on

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
BOOL InitVLSI(void) {

    WORD read;
    SHORT count;

    // Set up SPI port pins
    MP3_XDCS_O = 1; // Make the Data CS pin high
    MP3_XCS_O = 1; // Make the Control CS pin high
    MP3_XRESET_O = 0;

    MP3_XRESET_TRIS = 0;
    MP3_DREQ_TRIS = 1;
    MP3_XDCS_TRIS = 0; // Make the Data CS pin as an output
    //MP3_XCS_ADC = 0; // Disable analog functionalities
    MP3_XCS_TRIS = 0; // Make the Control CS pin as an output
    MP3_SDI_TRIS = 1; // Make the DIN pin as an input
    MP3_SDO_TRIS = 0; // Make the DOUT pin as an output
    MP3_SCK_TRIS = 0; // Make the SCK pin as an output


    // Deassert RESET (active low)
    MP3_XRESET_O = 1;
    DelayMs(5);
    MP3_XRESET_O = 0;
    DelayMs(5);
    MP3_XRESET_O = 1;

    // Wait until DREQ is pulled high
    for (count = 0; MP3_DREQ_I == 0 && count < 100; count++)
	DelayMs(10);

    // If DREQ is not pulled high reboot the board
    if (count == 100)
	FlashLight(100, 50, TRUE);

    // A quick sanity check: write to two registers, then test if we
    // get the same results. Note that if you use a too high SPI
    // speed, the MSB is the most likely to fail when read again.
    VLSIWriteReg(VLSI_ADD_HDAT0, 0xABAD, SPI_BRG_1MHZ);
    VLSIWriteReg(VLSI_ADD_HDAT1, 0x1DEA, SPI_BRG_1MHZ);
    if (VLSIReadReg(VLSI_ADD_HDAT0, SPI_BRG_1MHZ) != 0xABAD || VLSIReadReg(VLSI_ADD_HDAT1, SPI_BRG_1MHZ) != 0x1DEA) {
	FlashLight(100, 50, TRUE);
    }

    // Write configuration MODE register in a loop to verify that the chip is
    // connected and running correctly
    count = 0;
    do {
	#if defined VLSI_SINE_TEST
	VLSIWriteReg(VLSI_ADD_MODE, 0x0822); //0822 for a SineTest
    } while ((read = VLSIReadReg(VLSI_ADD_MODE)) != 0x0822u && count++ < 10);
	#else
	VLSIWriteReg(VLSI_ADD_MODE, VLSI_VAL_MODE_DECODE, SPI_BRG_1MHZ); // 0x0802 for a normal operations
    } while ((read = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_1MHZ)) != VLSI_VAL_MODE_DECODE && count++ < 10);
    #endif

    if (count == 10)
	FlashLight(100, 50, TRUE);

    if (((read = VLSIReadReg(VLSI_ADD_STATUS, SPI_BRG_1MHZ)) & VLSI_STATUS_VER) != VER_VS1063)
	FlashLight(100, 50, TRUE);

    // Set the Clock reference
    VLSIWriteReg(VLSI_ADD_CLOCKF, VLSI_VAL_CLOCKF_12MHz288, SPI_BRG_1MHZ);
    DelayMs(2);
    while (MP3_DREQ_I == 0);

    VLSI_SoftReset();

    SCI_MODE sci_mode;
    SCI_STATUS sci_status;
    SCI_CLOCK sci_clock;

    sci_mode.word = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
    sci_status.word = VLSIReadReg(VLSI_ADD_STATUS, SPI_BRG_8MHZ);
    sci_clock.word = VLSIReadReg(VLSI_ADD_CLOCKF, SPI_BRG_8MHZ);

    return TRUE;
}

/****************************************************************************
  Function:
    static void VLSI_SoftReset()

  Description:
    This function do a Soft Reset

  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SoftReset(void) {

    SCI_MODE sci_mode;

    sci_mode.word = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
    sci_mode.bits.SM_RESET = TRUE;
    VLSIWriteReg(VLSI_ADD_MODE, sci_mode.word, SPI_BRG_12MHZ);
    DelayMs(1);
    VLSIWriteReg(VLSI_ADD_CLOCKF, VLSI_VAL_CLOCKF_12MHz288, SPI_BRG_12MHZ);

    DelayMs(1);
    while (MP3_DREQ_I == 0);

    VLSI_Patch();

    VLSI_SetBalance(min(config.volume.bits.left, config.volume.bits.right), config.volume.word.balance);
    VLSI_SetTrebleBoost(config.volume.bits.treble_db, config.volume.bits.treble_freq);
    VLSI_SetBassBoost(config.volume.bits.bass_db, config.volume.bits.bass_freq);
}

/****************************************************************************
  Function:
    static WORD VLSIReadReg(BYTE vAddress)

  Description:
    This function reads the content of a register from the VLSI codec

  Precondition:
    VLSI - must be correctly initialized

  Parameters:
    BYTE vAddress   - register address
    WORD brg	    - value of baud rate generator (for SPI peripheral)

  Returns:
    WORD value of the content of the read register

  Remarks:
    None
 ***************************************************************************/
WORD VLSIReadReg(BYTE vAddress, WORD brg) {

    volatile DWORD dummy;
    volatile WORD Spibrg;

    // Full speed 5MHz
    Spibrg = SPIBRG;
    SPIBRG = brg;

    // Switch in Mode32 to speed up the bytes exchanged
    SPICON1bits.MODE32 = 1;
    while (!MP3_DREQ_I);
    MP3_XCS_O = 0;
    MP3_SPIBUF = 0x0300FFFF | vAddress << 16;
    while (!SPISTATbits.SPIRBF);
    dummy = MP3_SPIBUF;
    MP3_XCS_O = 1;
    // Return to default Mode8
    SPICON1bits.MODE32 = 0;

    // Restore the content of the register SPIBRG
    SPIBRG = Spibrg;

    //return wValue;
    return dummy;
}

/****************************************************************************
  Function:
    static void VLSIWriteReg(BYTE vAddress, WORD wValue)

  Description:
    This function writes one register from the VLSI code

  Precondition:
    VLSI - must be correctly initialized

  Parameters:
    BYTE vAddress   - address of register
    WORD wValue     - value to write to the register
    WORD brg	    - value of baud rate generator (for SPI peripheral)

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSIWriteReg(BYTE vAddress, WORD wValue, WORD brg) {

    volatile DWORD dummy;
    volatile WORD Spibrg;

    // Full speed 10MHz
    Spibrg = SPIBRG;
    SPIBRG = brg;

    // Switch in Mode32 to speed up the bytes exchanged
    SPICON1bits.MODE32 = 1;
    while (!MP3_DREQ_I);
    MP3_XCS_O = 0;
    MP3_SPIBUF = 0x02000000 | vAddress << 16 | wValue;
    while (!SPISTATbits.SPIRBF);
    dummy = MP3_SPIBUF;
    MP3_XCS_O = 1;
    // Return to default Mode8
    SPICON1bits.MODE32 = 0;

    // Restore the content of the register SPIBRG
    SPIBRG = Spibrg;
}

/****************************************************************************
  Function:
    WORD VLSIPutArray(char *bfr, int len)

  Description:
    This function writes len (or less) bytes of data to VLSI buffer

  Precondition:
    VLSI - must be correctly initialized

  Parameters:
    char *bfr   - pinter of buffer
    int len		- max lenght

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
WORD VLSIPutArray(BYTE *bfr, int len) {

    int i;
    volatile BYTE tmp;
    BYTE ret;
    WORD Spibrg;

    // If DREQ is high, VS1063a can take at least 32 bytes of SDI data or one SCI command.
    // DREQ is turned low when the stream buffer is too full and for the duration of an SCI command.
    if (DREQ_IS_FULL)
	return 0;

    Spibrg = SPIBRG;
    SPIBRG = SPI_BRG_12MHZ;

    MP3_XDCS_O = 0;

    MP3_SPIBUF = bfr[0];
    i = 1;
    if (len != 1) {
	do {
	    tmp = bfr[i++];
	    while (!MP3_SPI_RBF);
	    ret = MP3_SPIBUF;
	    // If DREQ is high, VS1063a can take at least 32 bytes of SDI data or one SCI command.
	    // DREQ is turned low when the stream buffer is too full and for the duration of an SCI command.
	    if (DREQ_IS_FULL) {
		MP3_XDCS_O = 1;
		i--;
		SPIBRG = Spibrg;
		return i;
	    }
	    MP3_SPIBUF = tmp;
	} while (i < len);
    }

    // Attendo che finisce di trasferire l'ultimo byte del buffer
    while (!MP3_SPI_RBF);
    ret = MP3_SPIBUF;

    MP3_XDCS_O = 1;
    SPIBRG = Spibrg;

    return i;
}

//WORD VLSIPutArray2(CIRCULAR_BUFFER *bfr) {
//
//    int i, tmpInc;
//    volatile BYTE tmp;
//    BYTE ret;
//    WORD Spibrg;
//
//    // If DREQ is high, VS1063a can take at least 32 bytes of SDI data or one SCI command.
//    // DREQ is turned low when the stream buffer is too full and for the duration of an SCI command.
//    if (DREQ_IS_FULL)
//        return 0;
//
//#ifdef DEBUG
//    LED_B_ON();
//#endif
//
//    // Setta il valore corretto di SPIBRG
//    //    VLSILockSPI();
//
//    if (bfr->put == bfr->get)
//        return 0;
//
//    Spibrg = SPIBRG;
//    SPIBRG = SPICalutateBRG(GetPeripheralClock(), SPI_BRG_12MHZ);
//
//    MP3_XDCS_O = 0;
//
//    MP3_SPIBUF = bfr->buff[bfr->get];
//    bfr->get = (bfr->get + 1) % SER_BUF_SIZE;
//    i = 1;
//    if (bfr->put != bfr->get) {
//        do {
//            tmp = bfr->buff[bfr->get];
//            tmpInc = (bfr->get + 1) % SER_BUF_SIZE;
//            while (!MP3_SPI_RBF);
//            ret = MP3_SPIBUF;
//            // If DREQ is high, VS1063a can take at least 32 bytes of SDI data or one SCI command.
//            // DREQ is turned low when the stream buffer is too full and for the duration of an SCI command.
//            if (DREQ_IS_FULL) {
//                MP3_XDCS_O = 1;
//#ifdef DEBUG
//                LED_B_OFF();
//#endif
//                SPIBRG = Spibrg;
//                return i;
//            }
//            MP3_SPIBUF = tmp;
//            i++;
//            bfr->get = tmpInc;
//        } while (bfr->put != bfr->get);
//    }
//
//    // Attendo che finisce di trasferire l'ultimo byte del buffer
//    while (!MP3_SPI_RBF);
//    ret = MP3_SPIBUF;
//
//    MP3_XDCS_O = 1;
//    SPIBRG = Spibrg;
//
//#ifdef DEBUG
//    LED_B_OFF();
//#endif
//
//    return i;
//}
//

/****************************************************************************
  Function:
    WORD VLSIGetArray(BYTE *bfr, int len, int minLenght)

  Description:
    This function reads len or less (minLenght) bytes of data from VLSI buffer

  Precondition:
    VLSI - must be correctly initialized

  Parameters:
    char *bfr   - pinter of buffer
    int len		- max lenght

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
WORD VLSIGetArray(BYTE *bfr, int len, int minLenght) {

    int i, k;
    volatile DWORD dummy;
    WORD BytesToRead, Spibrg;
    WORD_VAL WordsToRead;

    if (DREQ_IS_FULL) {
	for (k = 0; k < 2; k++)
	    Nop();
	if (DREQ_IS_FULL)
	    return 0;
    }
    #ifdef DEBUG
    LED_RED_ON();
    #endif

    //    // Max SPI clock speed allowd with 12.288MHz * 4 is 7MHz
    //    WordsToRead.Val = VLSIReadReg(VLSI_ADD_HDAT1, SPI_BRG_5MHZ);

    Spibrg = SPIBRG;
    SPIBRG = SPI_BRG_8MHZ; // SPI 6.66MHz

    // Switch in Mode32 to speed up the bytes exchanged
    SPICON1bits.MODE32 = 1;
    MP3_XCS_O = 0;
    MP3_SPIBUF = 0x0309FFFF;
    while (!SPISTATbits.SPIRBF);
    dummy = MP3_SPIBUF;
    MP3_XCS_O = 1;
    WordsToRead.v[1] = (BYTE) (dummy >> 8);
    WordsToRead.v[0] = (BYTE) (dummy);
    //    SPICON1bits.MODE32 = 0;

    //    MP3_XCS_O = 0;
    //    MP3_SPIBUF = 0x03;
    //    while (!MP3_SPI_RBF);
    //    ret = MP3_SPIBUF;
    //    MP3_SPIBUF = VLSI_ADD_HDAT1;
    //    while (!MP3_SPI_RBF);
    //    ret = MP3_SPIBUF;
    //    MP3_SPIBUF = 0xFF;
    //    while (!MP3_SPI_RBF);
    //    WordsToRead.v[1] = MP3_SPIBUF;
    //    MP3_SPIBUF = 0xFF;
    //    while (!MP3_SPI_RBF);
    //    WordsToRead.v[0] = MP3_SPIBUF;
    //    MP3_XCS_O = 1;

    BytesToRead = WordsToRead.Val * 2;
    if (BytesToRead < minLenght)
	len = 0;
    else if (BytesToRead < len)
	len = BytesToRead;

    //    SPICON1bits.MODE32 = 1;
    for (i = 0; i < len; i++) {

	//	if (DREQ_IS_FULL) {
	//	    // Little busy wait necessary to wait the completion of the previous reading operation
	//	    for (k = 0; k < 2; k++)
	//		Nop();
	//	    // If after the busy wait the decoder is always busy exit and returns later to read next byte
	//	    if (DREQ_IS_FULL)
	//		break;
	//	}

	while (DREQ_IS_FULL);
	MP3_XCS_O = 0;
	MP3_SPIBUF = 0x0308FFFF;
	while (!SPISTATbits.SPIRBF);
	dummy = MP3_SPIBUF;
	MP3_XCS_O = 1;
	bfr[i++] = (BYTE) (dummy >> 8);
	bfr[i] = (BYTE) (dummy);


	//	MP3_XCS_O = 0;
	//	MP3_SPIBUF = 0x03;
	//	while (!MP3_SPI_RBF);
	//	ret = MP3_SPIBUF;
	//	MP3_SPIBUF = VLSI_ADD_HDAT0;
	//	while (!MP3_SPI_RBF);
	//	ret = MP3_SPIBUF;
	//	MP3_SPIBUF = 0xFF;
	//	while (!MP3_SPI_RBF);
	//	tmp = MP3_SPIBUF;
	//	MP3_SPIBUF = 0xFF;
	//	bfr[i] = tmp;
	//	i++;
	//	while (!MP3_SPI_RBF);
	//	bfr[i] = MP3_SPIBUF;
	//	MP3_XCS_O = 1;
    }
    // Return to default Mode8
    SPICON1bits.MODE32 = 0;

    SPIBRG = Spibrg;

    #ifdef DEBUG
    LEDs_OFF();
    #endif

    return i;
}

/****************************************************************************
  Function:
    static void VLSI_InitRecording()

  Description:
    This function prepare the encoder to start the new registration

  Precondition:
    VLSI - must be correctly initialized

  Parameters:
    recPreSet - A structure with SCI_MODE, SCI_AICTRL0, SCI_AICTRL1 sci_aictrl1,
    SCI_AICTRL2, SCI_AICTRL3 and SCI_WRAMADDR registers

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_InitRecording(_REC_PRESET *recPreSet) {

    // Register     Bits    Description
    // SCI_MODE     2,12,14 Start ENCODE mode, select MIC/LINE1
    // SCI_AICTRL0  15:0    Samplerate 8000. . . 48000 Hz (read at encoding startup)
    // SCI_AICTRL1  15:0    Encoding gain (1024 = 1) or 0 for automatic gain control
    // SCI_AICTRL2  15:0    Maximum autogain amplification (1024 = 1, 65535 = 64)
    // SCI_AICTRL3  15      codec mode (both encode and decode)
    //              14      AEC enable
    //              13      UART TX enable
    //              12      reserved, set to 0
    //              11      Pause enable
    //              10      No RIFF WAV header inserted (or expected in codec mode)
    //              8:9     reserved, set to 0
    //              7:4     Encoding format 0. . . 6
    //              3       Reserved, set to 0
    //              2:0     ADC mode 0. . . 4
    // SCI_WRAMADDR 15...0  Quality / bitrate selection for Ogg Vorbis and MP3

    // Send all the parameters
    VLSIWriteReg(VLSI_ADD_AICTRL0, recPreSet->sci_aictrl0.word, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_AICTRL1, recPreSet->sci_aictrl1.word, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_AICTRL2, recPreSet->sci_aictrl2.word, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_AICTRL3, recPreSet->sci_aictrl3.word, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAMADDR, recPreSet->sci_wramaddr.word, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_MODE, recPreSet->sci_mode.word, SPI_BRG_12MHZ);

    // Start the recording operation
    VLSIWriteReg(VLSI_ADD_AIADDR, 0x50, SPI_BRG_12MHZ);
}

void VLSI_GetRecordingInfo(_REC_PRESET *recPreSet) {

    recPreSet->sci_aictrl0.word = VLSIReadReg(VLSI_ADD_AICTRL0, SPI_BRG_8MHZ);
    recPreSet->sci_aictrl1.word = VLSIReadReg(VLSI_ADD_AICTRL1, SPI_BRG_8MHZ);
    recPreSet->sci_aictrl2.word = VLSIReadReg(VLSI_ADD_AICTRL2, SPI_BRG_8MHZ);
    recPreSet->sci_aictrl3.word = VLSIReadReg(VLSI_ADD_AICTRL3, SPI_BRG_8MHZ);
    recPreSet->sci_wramaddr.word = VLSIReadReg(VLSI_ADD_WRAMADDR, SPI_BRG_8MHZ);
    recPreSet->sci_mode.word = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
}

void VLSI_SetBitRecMode(void) {

    mp3_record.word.AICTRL3 = VLSIReadReg(VLSI_ADD_AICTRL3, SPI_BRG_8MHZ);
    mp3_record.bits.pause = 1;
    VLSIWriteReg(VLSI_ADD_AICTRL3, mp3_record.word.AICTRL3, SPI_BRG_12MHZ);
}

void VLSI_ClearBitRecMode(void) {

    mp3_record.word.AICTRL3 = VLSIReadReg(VLSI_ADD_AICTRL3, SPI_BRG_8MHZ);
    mp3_record.bits.pause = 0;
    VLSIWriteReg(VLSI_ADD_AICTRL3, mp3_record.word.AICTRL3, SPI_BRG_12MHZ);
}

WORD VLSI_GetPlayRecMode() {

    mp3_record.word.AICTRL3 = VLSIReadReg(VLSI_ADD_AICTRL3, SPI_BRG_8MHZ);
    return mp3_record.word.AICTRL3;
}

/****************************************************************************
  Function:
    void VLSI_FinishPlaying(void)

  Description:
    This function send the finish command to stop playing.
     1. Send an audio file to VS1063a.
     2. Read extra parameter value endFillByte (Chapter 10.11).
     3. Send at least 2052 bytes of endFillByte[7:0]. For FLAC you should send 12288 endFill- Bytes when ending a file.
     4. Set SCI_MODE bit SM_CANCEL.
     5. Send at least 32 bytes of endFillByte[7:0].
     6. Read SCI_MODE. If SM_CANCEL is still set, go to 5. If SM_CANCEL hasn?t cleared after sending 2048 bytes, do a software reset (this should be extremely rare).
     7. The song has now been successfully sent. HDAT0 and HDAT1 should now both contain 0 to indicate that no format is being decoded. Return to 1.
  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_FinishPlaying(void) {

    int i, send, count;
    WORD fill;
    BYTE buf[4];

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0xC0C0, SPI_BRG_8MHZ);
    fill = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_12MHZ);

    buf[0] = buf[1] = buf[2] = buf[3] = 0x00FF & fill;

    for (i = 0; i < 2052 / 4; i++) {
	send = 0;
	while ((send += VLSIPutArray(&buf[send], sizeof (buf) - send)) != sizeof (buf));
    }

    // Set SM_CANCEL bit of SCI_MODE
    fill = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
    VLSIWriteReg(VLSI_ADD_MODE, fill | VLSI_VAL_SM_CANCEL, SPI_BRG_12MHZ);

    count = 0;
    do {
	for (i = 0; i < 32 / 4; i++) {
	    send = 0;
	    while ((send += VLSIPutArray(&buf[send], sizeof (buf) - send)) != sizeof (buf));
	}

	fill = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
    } while (fill & VLSI_VAL_SM_CANCEL && count++ < 64);

    if (count >= 64)
	VLSI_SoftReset();

    if (VLSIReadReg(VLSI_ADD_HDAT0, SPI_BRG_8MHZ) != 0 || VLSIReadReg(VLSI_ADD_HDAT1, SPI_BRG_8MHZ) != 0)
	VLSI_SoftReset();
}

/****************************************************************************
  Function:
    void VLSI_SendFinishRecording(void)

  Description:
    This function send finish the command to stop recording.

  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SendFinishRecording(void) {

    WORD fill;

    // Set SM_CANCEL bit of SCI_MODE
    fill = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
    VLSIWriteReg(VLSI_ADD_MODE, fill | VLSI_VAL_SM_CANCEL, SPI_BRG_12MHZ);
}

//// 1. Send a portion of an audio file to VS1063a.
//// 2. Set SCI_MODE bit SM_CANCEL.
//// 3. Continue sending audio file, but check SM_CANCEL after every 32 bytes of data. If it is still set, goto 3. If SM_CANCEL doesn?t clear after 2048 bytes or one second, do a software reset (this should be extremely rare).
//// 4. When SM_CANCEL has cleared, read extra parameter value endFillByte (Chapter 10.11).
//// 5. Send 2052 bytes of endFillByte[7:0]. For FLAC you should send 12288 endFillBytes.
//// 6. HDAT0 and HDAT1 should now both contain 0 to indicate that no format is being decoded.
//// You can now send the next audio file.
//
//void VLSI_CancelPlaying() {
//
//    int i, send, count;
//    WORD fill;
//    BYTE buf;
//
//    // Set SM_CANCEL bit of SCI_MODE
//    fill = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_5MHZ);
//    VLSIWriteReg(VLSI_ADD_MODE, fill | VLSI_VAL_SM_CANCEL, SPI_BRG_5MHZ);
//}

/****************************************************************************
  Function:
    void VLSI_SetBassBoost(BYTE bass, BYTE gfreq)

  Description:
    This function sets the bass booster.

  Precondition:
    None

  Parameters:
    BYTE bass   - Bass Enhancement in 1 dB steps (0 ... 15, 0 = off)
    BYTE gfreq  - Upper limit frequency in 10 Hz steps (2 ... 15)

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SetBassBoost(BYTE bass, BYTE gfreq) {

    // Make sure values are in the allowed range
    if (bass > 15)
	bass = 15;
    if (bass < 0)
	bass = 0;

    if (gfreq > 15)
	gfreq = 15;
    if (gfreq < 2)
	gfreq = 2;

    config.volume.bits.bass_db = bass;
    config.volume.bits.bass_freq = gfreq;

    VLSIWriteReg(VLSI_ADD_BASS, config.volume.word.boost, SPI_BRG_12MHZ);
}

/****************************************************************************
  Function:
    void VLSI_SetTrebleBoost(CHAR treble, BYTE gfreq)

  Description:
    This function sets the treble booster.

  Precondition:
    None

  Parameters:
    CHAR bass   - Treble Control in 1.5 dB steps (-8 ... 7, 0 = off)
    BYTE gfreq  - Lower limit frequency in 1000 Hz steps (1 ... 15)

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SetTrebleBoost(CHAR treble, BYTE gfreq) {

    // Make sure values are in the allowed range
    if (treble > 7)
	treble = 7;
    if (treble < -8)
	treble = -8;

    if (gfreq > 15)
	gfreq = 15;
    if (gfreq < 1)
	gfreq = 1;

    config.volume.bits.treble_db = treble;
    config.volume.bits.treble_freq = gfreq;

    VLSIWriteReg(VLSI_ADD_BASS, config.volume.word.boost, SPI_BRG_12MHZ);
}

/****************************************************************************
  Function:
    void VLSI_SetVolume(SHORT left, SHORT right)

  Description:
    This function sets the volume for both the channels

  Precondition:
    None

  Parameters:
    SHORT left   - Left channel volume, (0 ... 255, -1 = leaves unchanged)
    SHORT right  - Right channel volume, (0 ... 255, -1 = leaves unchanged)

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SetVolume(SHORT left, SHORT right) {

    // Make sure values are in the allowed range
    if (left > 255)
	left = 255;
    if (left < 0)
	left = config.volume.bits.left;

    if (right > 255)
	right = 255;
    if (right < 0)
	right = config.volume.bits.right;

    config.volume.bits.left = left;
    config.volume.bits.right = right;

    VLSIWriteReg(VLSI_ADD_VOL, config.volume.word.volume, SPI_BRG_12MHZ);
}

/****************************************************************************
  Function:
    void VLSI_SetBalance(CHAR volume, CHAR balance)

  Description:
    This function sets the volume in relation of the balance value

  Precondition:
    None

  Parameters:
    BYTE volume  - volume value
    CHAR balance - percentage of attenuation (L = -100% ... 0% ... 100% = R)

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SetBalance(BYTE volume, CHAR balance) {

    LONG left, right;

    if (balance == 0) {
	right = left = volume;
    } else if (balance < 0) {
	left = (LONG) ((255 - volume) * abs(balance) / 100) + volume;
	right = volume;
    } else if (balance > 0) {
	right = (LONG) ((255 - volume) * abs(balance) / 100) + volume;
	left = volume;
    }

    VLSI_SetVolume(left, right);
}

/****************************************************************************
  Function:
    BYTE VLSI_GetBass(void)

  Description:
    This function gets the the Bass Enhancement value

  Precondition:
    None

  Parameters:
    None

  Returns:
    BYTE - return a value between the 0 and 15

  Remarks:
    None
 ***************************************************************************/
BYTE VLSI_GetBass(void) {
    return config.volume.bits.bass_db;
}

/****************************************************************************
  Function:
    BYTE VLSI_GetBassFreq(void)

  Description:
    This function gets the the Bass Frequencies value

  Precondition:
    None

  Parameters:
    None

  Returns:
    BYTE - return a value between the 2 and 15

  Remarks:
    None
 ***************************************************************************/
BYTE VLSI_GetBassFreq(void) {
    return config.volume.bits.bass_freq;
}

/****************************************************************************
  Function:
    BYTE VLSI_GetTreble(void)

  Description:
    This function gets the the Treble Enhancement value

  Precondition:
    None

  Parameters:
    None

  Returns:
    CHAR - return a value between the -8 and 7

  Remarks:
    None
 ***************************************************************************/
CHAR VLSI_GetTreble(void) {
    return config.volume.bits.treble_db;
}

/****************************************************************************
  Function:
    BYTE VLSI_GetTrbleFreq(void)

  Description:
    This function gets the the Bass Frequencies value

  Precondition:
    None

  Parameters:
    None

  Returns:
    BYTE - return a value between the 1 and 15

  Remarks:
    None
 ***************************************************************************/
BYTE VLSI_GetTrebleFreq(void) {
    return config.volume.bits.treble_freq;
}

/****************************************************************************
  Function:
    BYTE VLSI_GetLeft(void)

  Description:
    This function gets the laft channel value

  Precondition:
    None

  Parameters:
    None

  Returns:
    BYTE - return a value between the 0 and 255

  Remarks:
    None
 ***************************************************************************/
BYTE VLSI_GetLeft(void) {
    return config.volume.bits.left;
}

/****************************************************************************
  Function:
    BYTE VLSI_GetRight(void)

  Description:
    This function gets the right channel value

  Precondition:
    None

  Parameters:
    None

  Returns:
    BYTE - return a value between the 0 and 255

  Remarks:
    None
 ***************************************************************************/
BYTE VLSI_GetRight(void) {
    return config.volume.bits.right;
}

/****************************************************************************
  Function:
    BYTE VLSI_GetBalance(void)

  Description:
    This function gets percentage of volume attenuation

  Precondition:
    None

  Parameters:
    None

  Returns:
    CHAR - return a value between the 0 and 255

  Remarks:
    None
 ***************************************************************************/
CHAR VLSI_GetBalance(void) {

    LONG temp;

    if (config.volume.bits.right < config.volume.bits.left)
	temp = -(config.volume.bits.left - config.volume.bits.right) * 100 / (255 - config.volume.bits.right);
    else if (config.volume.bits.left < config.volume.bits.right)
	temp = (config.volume.bits.right - config.volume.bits.left) * 100 / (255 - config.volume.bits.left);
    else
	temp = 0;

    return temp;
}

/****************************************************************************
 *  Function:
 *    void VLSI_SetFastSpeed(WORD speed)
 *
 *  Description:
 *  This function configure speed execution
 *  playSpeed makes it possible to fast forward songs. Decoding of the bitstream is performed,
 *  but only each playSpeed frames are played. For example by writing 4 to playSpeed will play
 *  the song four times as fast as normal, if you are able to feed the data with that speed. Write 0
 *  or 1 to return to normal speed. SCI_DECODE_TIME will also count faster. All current codecs
 *  support the playSpeed configuration.

  Precondition:
    None

  Parameters:
    WORD - 0,1 = normal speed, 2 = double, 3 = three times etc.

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SetFastSpeed(WORD speed) {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e04, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, speed, SPI_BRG_12MHZ);
}

/****************************************************************************
 *  Function:
 *    void VLSI_GetFastSpeed(WORD speed)
 *
 *  Description:
 *  This function return current speed execution

  Precondition:
    None

  Parameters:
    WORD - 0,1 = normal speed, 2 = double, 3 = three times etc.

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
WORD VLSI_GetFastSpeed(void) {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e04, SPI_BRG_12MHZ);
    extra_par.word.playSpeed = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);
    return extra_par.word.playSpeed;
}

/****************************************************************************

  Function:
    void VLSI_SetRateTune(DWORD finetune)

  Description:
    This function configure finetune speed execution
    rateTune finetunes the samplerate in 1 ppm steps. This is useful in streaming applications
    where long-term buffer fullness is used to adjust the samplerate very accurately. Zero is normal
    speed, positive values speed up, negative values slow down. To calculate rateTune for a speed,
    use (x -1.0) * 1000000. For example 5.95% speedup (1.0595 - 1.0) * 1000000 = 59500.

  Precondition:
    None

  Parameters:
    DWORD - 0,1 = normal speed, 2 = double, 3 = three times etc.

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void VLSI_SetRateTune(DWORD finetune) {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e08, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, finetune >> 16, SPI_BRG_12MHZ);

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e07, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, finetune, SPI_BRG_12MHZ);
}

DWORD VLSI_GetRateTune(void) {

    volatile DWORD tmp;

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e08, SPI_BRG_12MHZ);
    tmp = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);
    tmp = tmp << 16;
    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e07, SPI_BRG_12MHZ);
    tmp |= VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);

    return tmp;
}

void VLSI_SetSpeedShifter(WORD speed) {

    WORD tmp;

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e09, SPI_BRG_12MHZ);
    tmp = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);
    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e09, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, tmp | 0x40, SPI_BRG_12MHZ);

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e1D, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, speed, SPI_BRG_12MHZ);
}

WORD VLSI_GetSpeedShifter() {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e1D, SPI_BRG_12MHZ);
    extra_par.word.speedShifter = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);
    return extra_par.word.speedShifter;
}

void VLSI_SetEarSpeaker(WORD ear) {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e1e, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, ear, SPI_BRG_12MHZ);
}

WORD VLSI_GetEarSpeaker(void) {

    volatile WORD tmp;

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e1e, SPI_BRG_12MHZ);
    tmp = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);

    return tmp;
}

void VLSI_SetBitPlayMode(BYTE mode) {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e09, SPI_BRG_12MHZ);
    extra_par.word.playMode = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);
    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e09, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, extra_par.word.playMode | mode, SPI_BRG_12MHZ);
}

void VLSI_ClearBitPlayMode(BYTE mode) {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e09, SPI_BRG_12MHZ);
    extra_par.word.playMode = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);
    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e09, SPI_BRG_12MHZ);
    VLSIWriteReg(VLSI_ADD_WRAM, extra_par.word.playMode & mode, SPI_BRG_12MHZ);
}

WORD VLSI_GetPlayMode() {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e09, SPI_BRG_12MHZ);
    extra_par.word.playMode = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);
    return extra_par.word.playMode;
}

BOOL VLSI_GetEndFillByte(char *byte) {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e06, SPI_BRG_12MHZ);
    extra_par.word.endFillByte = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);

    // If the most significant bit (bit 15) is set to 1, then the file is of an
    // odd length and bits 7:0 contain the last byte that still should be written
    // to the output file.
    if ((extra_par.word.endFillByte & 0x8000) != 0) {
	// bits 7:0 contain the last byte, return it to caller
	*byte = extra_par.word.endFillByte & 0x00FF;
	// Now write 0 to endFillByte
	VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e06, SPI_BRG_12MHZ);
	VLSIWriteReg(VLSI_ADD_WRAM, 0x0000, SPI_BRG_12MHZ);
	return TRUE;
    }
    return FALSE;
}

BOOL VLSI_IsClearedSmCancel() {

    WORD tmp;

    tmp = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
    if ((tmp & VLSI_VAL_SM_CANCEL) != 0)
	return FALSE;
    return TRUE;
}

BOOL VLSI_IsClearedSmEncode(void) {

    WORD tmp;

    tmp = VLSIReadReg(VLSI_ADD_MODE, SPI_BRG_8MHZ);
    if ((tmp & VLSI_BIT_SM_ENCODE) == 0)
	return TRUE;
    return FALSE;
}

void VLSI_GetHDAT(void) {

    mp3_info.word.HDAT0 = VLSIReadReg(VLSI_ADD_HDAT0, SPI_BRG_8MHZ);
    mp3_info.word.HDAT1 = VLSIReadReg(VLSI_ADD_HDAT1, SPI_BRG_8MHZ);
}

const char * VLSI_GetBitrateOthersFormat(WORD *bitrate) {

    const char *str_encoding_format[] = {"Ogg Vorbis", "Wav", "WMA", "mp4", "ACC", "AAC", "FLAC", "MP3"};

    if (mp3_info.word.HDAT1 == 0x4F67) { // "Og"
	*bitrate = mp3_info.word.HDAT0 * 8; // Ogg
	return str_encoding_format[0];
    } else if (mp3_info.word.HDAT1 == 0x7665) { // "ve"
	*bitrate = mp3_info.word.HDAT0 * 8; // Wav
	return str_encoding_format[1];
    } else if (mp3_info.word.HDAT1 == 0x574D) { // "WM"
	*bitrate = mp3_info.word.HDAT0 * 8; // WMA
	return str_encoding_format[2];
    } else if (mp3_info.word.HDAT1 == 0x4D34) { // "M4"
	*bitrate = mp3_info.word.HDAT0 * 8; // mp4
	return str_encoding_format[3];
    } else if (mp3_info.word.HDAT1 == 0x4154) { // "AT"
	*bitrate = mp3_info.word.HDAT0 * 8; // AAC ADTS
	return str_encoding_format[4];
    } else if (mp3_info.word.HDAT1 == 0x4144) { // "AD"
	*bitrate = mp3_info.word.HDAT0 * 8; // AAC ADIF
	return str_encoding_format[5];
    } else if (mp3_info.word.HDAT1 == 0x664c) { // "fL"
	*bitrate = mp3_info.word.HDAT0 * 32; // FLAC
	return str_encoding_format[6];
    } else {
	*bitrate = 0; // mp3
	return str_encoding_format[7];
    }
}

WORD VLSI_GetBitrateMP3Format(void) {

    const unsigned char bitrate_tab_1[16] = {0, 16, 24, 28, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 0};
    const unsigned char bitrate_tab_2[16] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 56, 64, 72, 80, 0};
    const unsigned char bitrate_tab_3[16] = {0, 16, 20, 24, 28, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 0};

    //    VLSI_GetHDAT();

    int bitare, layer, id;
    WORD rtn;

    bitare = mp3_info.bits.bitrate;
    layer = 4 - mp3_info.bits.layer;
    id = mp3_info.bits.ID;

    if (layer == 3) {
	if (id == 3)
	    rtn = bitrate_tab_3[bitare];
	else
	    rtn = bitrate_tab_2[bitare];
    } else if (layer == 2) {
	if (id == 3)
	    rtn = bitrate_tab_1[bitare];
	else
	    rtn = bitrate_tab_2[bitare];
    } else {
	rtn = 0;
    }

    // Return kbit/s
    return rtn * 2;
}

WORD VLSI_GetSamplerate(void) {

    const unsigned int samplerate_tab[4][4] = {11025, 11025, 22050, 44100,
	12000, 12000, 24000, 48000,
	8000, 8000, 16000, 32000,
	0, 0, 0, 0};

    int r, c;

    //    VLSI_GetHDAT();

    r = mp3_info.bits.samplerate;
    c = mp3_info.bits.ID;

    // Return Hz
    return samplerate_tab[r][c];
}

const char* VLSI_GetLayer(void) {

    const char *layer_tab[] = {"", "III", "II", "I"};
    //    BYTE layer = 0;

    //    VLSI_GetHDAT();

    //    if (mp3_info.bits.layer < 4)
    //        layer = 4 - mp3_info.bits.layer;
    //
    //    return layer;

    return layer_tab[mp3_info.bits.layer];
}

BOOL VLSI_GetCopyright() {
    //    VLSI_GetHDAT();
    return mp3_info.bits.copyright;
}

BOOL VLSI_GetOriginal() {
    //    VLSI_GetHDAT();
    return mp3_info.bits.original;
}

BOOL VLSI_GetPad() {
    //    VLSI_GetHDAT();
    return mp3_info.bits.pad;
}

BOOL VLSI_GetProtect() {
    //    VLSI_GetHDAT();
    return mp3_info.bits.protect_bit;
}

const char * VLSI_GetEmphasis() {
    const char *emphasis_tab[4] = {"none", "50/15 microsec", "reserved", "CCITT J.17"};
    //    VLSI_GetHDAT();
    return emphasis_tab[mp3_info.bits.emphasis];
}

const char * VLSI_GetMode() {
    const char *mode_tab[] = {"stereo", "joint stereo", "dual channel", "mono"};
    //    VLSI_GetHDAT();
    return mode_tab[mp3_info.bits.mode];
}

const char * VLSI_GetID() {
    const char *ID_tab[] = {"MPG 2.5 (1/4-rate)", "MPG 2.5 (1/4-rate)", "ISO 13818-3 MPG 2.0 (1/2-rate)", "ISO 11172-3 MPG 1.0"};
    //    VLSI_GetHDAT();
    return ID_tab[mp3_info.bits.ID];
}

void VLSI_GetExtra() {

    int i;

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0xC0C0, SPI_BRG_12MHZ);

    for (i = 0; i<sizeof (extra_par) / 2; i++)
	extra_par.w[i] = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);

    Nop();
}

WORD VLSI_GetBitRatePer100() {

    VLSIWriteReg(VLSI_ADD_WRAMADDR, 0x1e05, SPI_BRG_12MHZ);
    extra_par.word.bitRatePer100 = VLSIReadReg(VLSI_ADD_WRAM, SPI_BRG_8MHZ);

    // byte/s
    // byte/s * 8 / 100 = kbit/s
    return extra_par.word.bitRatePer100;
}

