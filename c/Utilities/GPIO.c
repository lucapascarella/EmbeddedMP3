

#include "Utilities/GPIO.h"
#include "Utilities/Config.h"
#include "HardwareProfile.h"
#include "CommandLineInterpreter.h"
#include "Delay/Tick.h"
#include "Utilities/printer.h"
#include "Utilities/Uart.h"
#include "Commands.h"
#include "MP3/Play.h"
#include "MP3/Record.h"


BOOL signalNewOutputState, signalNewInputState;
WORD state;

void GPIOInit() {

    int n;

    signalNewOutputState = signalNewInputState = FALSE;


    // Disable the analog functionalities
    ANSELBbits.ANSB2 = 0; // RB2 --> GPIO 1
    ANSELBbits.ANSB3 = 0; // RB3 --> GPIO 2

    for (n = 0; n < GPIO_NUMBERS; n++) {

        // Reset associated counter
        config.gpio[n].durationInTick = config.gpio[n].timeout = config.gpio[n].bits.timeOutEnabled = 0;
        // Reset associated state
        config.gpio[n].bits.state = 0;

        if (GpioCheckSpecialFunction(config.gpio[n].mode, n)) {
            // Initialize appropriate special function
            Nop();
        } else {
            // Always disable both pull Up and Down
            GpioSetPullUp(n, DISABLED);
            GpioSetPullDown(n, DISABLED);
            // Initialize to GPIO function
            if (config.gpio[n].mode == GPIO_NOTHING) {
                GpioSetTris(n, INPUT);
                GpioSetChangeNoticeState(n, DISABLED);
            } else if (config.gpio[n].mode >= GPIO_I_STARTS_PLAYLIST && config.gpio[n].mode <= GPIO_I_RESET) {
                // Configure as digital input
                GpioSetTris(n, INPUT);
                // Enable the pull-up/down
                if (config.gpio[n].bits.pull == 1)
                    GpioSetPullUp(n, ENABLED);
                else if (config.gpio[n].bits.pull == 2)
                    GpioSetPullDown(n, ENABLED);
                GpioSetChangeNoticeState(n, ENABLED);
            } else if (config.gpio[n].mode >= GPIO_O_STARTS_PLAY && config.gpio[n].mode <= GPIO_O_MICRO_SD_OK) {
                // Disable the pull
                // Configure as digital output
                GpioSetTris(n, OUTPUT);
                // Set the idle state
                GpioSetOutputState(n, config.gpio[n].bits.idle);
                // Disable change notice mapping
                GpioSetChangeNoticeState(n, DISABLED);
            } else {
                Nop();
            }
        }
    }

    mPORTARead();
    mPORTBRead();

    // Change notice control for port A and B
    // Enabled individual port group CN
    CNCONAbits.ON = 1;
    CNCONBbits.ON = 1;

    ConfigIntCNA(CHANGE_INT_PRI_2 | CHANGE_INT_ON);
    ConfigIntCNB(CHANGE_INT_PRI_2 | CHANGE_INT_ON);
}

void __ISR(_CHANGE_NOTICE_VECTOR, IPL2AUTO) ChangeNotice_Handler(void) {
    unsigned int tempA, tempB;

    // clear the mismatch condition
    tempA = mPORTARead();
    tempB = mPORTBRead();

    // clear the interrupt flag
    mCNAClearIntFlag();
    mCNBClearIntFlag();

    config.gpio[0].bits.state = tempB >> 8;
    config.gpio[1].bits.state = tempB >> 9;
    config.gpio[2].bits.state = tempA >> 0;
    config.gpio[3].bits.state = tempA >> 1;
    config.gpio[4].bits.state = tempB >> 1;
    config.gpio[5].bits.state = tempB >> 0;
    config.gpio[6].bits.state = tempB >> 10;
    config.gpio[7].bits.state = tempB >> 11;

    signalNewInputState = TRUE;

}

void GPIOInputTaskHandler() {

    BOOL update;
    int n;

    if (signalNewInputState) {

        for (n = 0; n < GPIO_NUMBERS; n++) {

            // Manage the Input event associated to each GPIO
            if (config.gpio[n].bits.idle != config.gpio[n].bits.state) {

                //if (config.gpio[n].bits.timeOutEnabled && TickGet() - config.gpio[n].timeout >= config.gpio[n].durationInTick) {
                if (TickGet() - config.gpio[n].timeout >= config.gpio[n].durationInTick) {

                    // Reset update condition for every cycle
                    update = FALSE;
                    switch (config.gpio[n].mode) {
                            // 1
                        case GPIO_I_STARTS_PLAYLIST:
                            // Inviare il corrispondente comando
                            update = TRUE;
                            Playlist(1, NULL);
                            break;

                            // 2
                        case GPIO_I_STOP:
                            // Send Stop command and hold the input handler for duration time
                            update = TRUE;
                            Stop(1, NULL);
                            break;

                            // 3
                        case GPIO_I_STARTS_STOP:
                            update = TRUE;
                            if (StopPlay(1, NULL) == FALSE)
                                Playlist(1, NULL);
                            break;

                            // 4
                        case GPIO_I_TOGGLE_PAUSE:
                            update = TRUE;
                            Pause(1, NULL);
                            break;

                            // 5
                        case GPIO_I_STARTS_RECORDING:
                            update = TRUE;
                            Record(1, NULL);
                            break;

                            // 6
                        case GPIO_I_STARTS_STOP_RECORDING:
                            update = TRUE;
                            if (StopRecord(1, NULL) == FALSE)
                                Record(1, NULL);
                            break;

                            // 7
                        case GPIO_I_RESET:
                            update = TRUE;
                            Reboot(1, NULL);
                            break;

                        default:
                            break;

                    }

                    if (update && config.gpio[n].durationInMilliSecs) {
                        config.gpio[n].timeout = TickGet();
                        config.gpio[n].durationInTick = TICK_SECOND / 1000 * config.gpio[n].durationInMilliSecs;
                    }
                }
            }
        }
        signalNewInputState = FALSE;
    }
}

void GPIOOutputTaskHandler() {

    BOOL update;
    int n;

    for (n = 0; n < GPIO_NUMBERS; n++) {
        if (config.gpio[n].bits.timeOutEnabled && TickGet() - config.gpio[n].timeout >= config.gpio[n].durationInTick) {
            GpioToggleState(n);
            config.gpio[n].bits.timeOutEnabled = FALSE;
        }
    }

    if (signalNewOutputState) {

        for (n = 0; n < GPIO_NUMBERS; n++) {
            // Reset update condition for every cycle
            update = FALSE;
            // Manage the Output event associated to each GPIO
            switch (config.gpio[n].mode) {
                    // 10
                case GPIO_O_STARTS_PLAY:
                    if (state == GPIO_BIT_STARTS_PLAY)
                        update = TRUE;
                    break;

                    // 11
                case GPIO_O_STOP_PLAY:
                    if (state == GPIO_BIT_STOP_PLAY)
                        update = TRUE;
                    break;

                    // 12
                case GPIO_O_PAUSE_PLAY:
                    if (state == GPIO_BIT_PAUSE_PLAY)
                        update = TRUE;
                    break;

                    // 13
                case GPIO_O_STARTS_STOP_PLAY:
                    if (state == GPIO_BIT_STARTS_PLAY || state == GPIO_BIT_STOP_PLAY)
                        update = TRUE;
                    break;

                    // 14
                case GPIO_O_STARTS_PAUSE_STOP_PLAY:
                    if (state == GPIO_BIT_STARTS_PLAY || state == GPIO_BIT_PAUSE_PLAY || state == GPIO_BIT_STOP_PLAY)
                        update = TRUE;
                    break;


                    // 15
                case GPIO_O_FILE_NOT_FOUND:
                    if (state == GPIO_BIT_FILE_NOT_FOUND)
                        update = TRUE;
                    break;


                    // 16
                case GPIO_O_STARTS_RECORDING:
                    if (state == GPIO_BIT_STARTS_REC)
                        update = TRUE;
                    break;

                    // 17
                case GPIO_O_STOP_RECORDING:
                    if (state == GPIO_BIT_STOP_REC)
                        update = TRUE;
                    break;

                    // 18
                case GPIO_O_PAUSE_RECORDING:
                    if (state == GPIO_BIT_PAUSE_REC)
                        update = TRUE;
                    break;

                    // 19
                case GPIO_O_STARTS_STOP_RECORDING:
                    if (state == GPIO_BIT_STARTS_REC || state == GPIO_BIT_STOP_REC)
                        update = TRUE;
                    break;

                    // 20
                case GPIO_O_STARTS_PAUSE_STOP_RECORDING:
                    if (state == GPIO_BIT_STARTS_REC || state == GPIO_BIT_PAUSE_REC || state == GPIO_BIT_STOP_REC)
                        update = TRUE;
                    break;


                    // 21
                case GPIO_O_STARTS_PLAY_RECORDING:
                    if (state == GPIO_BIT_STARTS_PLAY || state == GPIO_BIT_STARTS_REC)
                        update = TRUE;
                    break;

                    // 22
                case GPIO_O_STOP_PLAY_RECORDING:
                    if (state == GPIO_BIT_STOP_PLAY || state == GPIO_BIT_STOP_REC)
                        update = TRUE;
                    break;

                    // 23
                case GPIO_O_PAUSE_PLAY_RECORDING:
                    if (state == GPIO_BIT_PAUSE_PLAY || state == GPIO_BIT_PAUSE_REC)
                        update = TRUE;
                    break;

                    // 24
                case GPIO_O_STARTS_STOP_PLAY_RECORDING:
                    if (state == GPIO_BIT_STARTS_PLAY || state == GPIO_BIT_STOP_PLAY || state == GPIO_BIT_STARTS_REC || state == GPIO_BIT_STOP_REC)
                        update = TRUE;
                    break;

                    // 25
                case GPIO_O_STARTS_PAUSE_STOP_PLAY_RECORDING:
                    if (state == GPIO_BIT_STARTS_PLAY || state == GPIO_BIT_PAUSE_PLAY || state == GPIO_BIT_STOP_PLAY || state == GPIO_BIT_STARTS_REC || state == GPIO_BIT_PAUSE_REC || state == GPIO_BIT_STOP_REC)
                        update = TRUE;
                    break;


                    // 26
                case GPIO_O_COMMAND_OK:
                    if (state == GPIO_BIT_CMD_OK)
                        update = TRUE;
                    break;

                    // 27
                case GPIO_O_COMMAND_ERROR:
                    if (state == GPIO_BIT_CMD_ERR)
                        update = TRUE;
                    break;

                    // 28
                case GPIO_O_MICRO_SD_OK:
                    if (state == GPIO_BIT_MICRO_SD)
                        update = TRUE;
                    break;

                default:
                    break;

            }


            if (update) {
                if (config.gpio[n].durationInMilliSecs == 0) {
                    GpioToggleState(n);
                } else {
                    config.gpio[n].timeout = TickGet();
                    config.gpio[n].durationInTick = TICK_SECOND / 1000 * config.gpio[n].durationInMilliSecs;
                    config.gpio[n].bits.timeOutEnabled = TRUE;
                    if (config.gpio[n].bits.idle == 0) {
                        GpioSetOutputState(n, TRUE);
                    } else {
                        GpioSetOutputState(n, FALSE);
                    }
                }
            }
        }

        signalNewOutputState = FALSE;
    }
}

void GpioUpdateOutputState(int event) {

    signalNewOutputState = TRUE;
    state = event;
}

int Gpio(int argc, char **argv) {

    int pin, n, i;

    const char *pull[] = {"None", "Up", "Down"};
    const char str[] = "SCL I2C\0" "SDA I2C\0" "AN0\0" "AN1\0" "RX UART\0" "TX UART\0" "USB_P\0" "USB_N\0";
    const char *p;

    if (argc == 1) {

        for (n = 0; n < GPIO_NUMBERS; n++) {
            if (GpioCheckSpecialFunction(config.gpio[n].mode, n)) {
                p = str;
                for (i = 0; i != n && *p; i++) {
                    while (*p++);
                }
                printf("GPIO %d, Mode: %s\r\n", n, p);
            } else {
                p = pull[config.gpio[n].bits.pull];
                printf("GPIO %d, Mode: %d%, Duration: %d, Idle: %s, Pull: %s, State: %s\r\n", n, config.gpio[n].mode, config.gpio[n].durationInMilliSecs, config.gpio[n].bits.idle ? "High" : "Low", p, GpioGetInputState(n) ? "High" : "Low");
            }
        }

    } else if (argc < 6) {
        CliTooFewArgumnets(argv[1]);
    } else if (argc == 6) {

        pin = atoimm(argv[1], 0, 7, 8);
        if (pin != 8) {
            config.gpio[pin].mode = atoimm(argv[2], 0, 38, 0);
            config.gpio[pin].durationInMilliSecs = atoimm(argv[3], 0, 1000, 100);
            config.gpio[pin].bits.idle = atoimm(argv[4], 0, 1, 0);
            config.gpio[pin].bits.pull = atoimm(argv[5], 0, 2, 0);
            // Update the configuration
            GPIOInit();
        }
    } else {
        CliTooManyArgumnets(argv[1]);
    }
    return 0;
}

BOOL GpioGetInputState(BYTE gpio) {

    switch (gpio) {
        case GPIO_0_SCL:
            return GPIO_0_INPUT ? TRUE : FALSE;
        case GPIO_1_SDA:
            return GPIO_1_INPUT ? TRUE : FALSE;
        case GPIO_2_AN0:
            return GPIO_2_INPUT ? TRUE : FALSE;
        case GPIO_3_AN1:
            return GPIO_3_INPUT ? TRUE : FALSE;
        case GPIO_4_RX:
            return GPIO_4_INPUT ? TRUE : FALSE;
        case GPIO_5_TX:
            return GPIO_5_INPUT ? TRUE : FALSE;
        case GPIO_6_USB_P:
            return GPIO_6_INPUT ? TRUE : FALSE;
        case GPIO_7_USB_N:
            return GPIO_7_INPUT ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

BOOL GpioCheckSpecialFunction(BYTE mode, BYTE gpio) {

    switch (gpio) {
        case GPIO_0_SCL:
            return mode == GPIO_S_I2C ? TRUE : FALSE;
        case GPIO_1_SDA:
            return mode == GPIO_S_I2C ? TRUE : FALSE;
        case GPIO_2_AN0:
            return mode == GPIO_S_ANALOG ? TRUE : FALSE;
        case GPIO_3_AN1:
            return mode == GPIO_S_ANALOG ? TRUE : FALSE;
        case GPIO_4_RX:
            return mode == GPIO_S_UART ? TRUE : FALSE;
        case GPIO_5_TX:
            return mode == GPIO_S_UART ? TRUE : FALSE;
        case GPIO_6_USB_P:
            return mode == GPIO_S_USB ? TRUE : FALSE;
        case GPIO_7_USB_N:
            return mode == GPIO_S_USB ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

void GpioSetTris(BYTE gpio, BYTE state) {
    switch (gpio) {
        case GPIO_0_SCL:
            GPIO_0_TRIS = state;
            break;
        case GPIO_1_SDA:
            GPIO_1_TRIS = state;
            break;
        case GPIO_2_AN0:
            GPIO_2_TRIS = state;
            break;
        case GPIO_3_AN1:
            GPIO_3_TRIS = state;
            break;
        case GPIO_4_RX:
            GPIO_4_TRIS = state;
            break;
        case GPIO_5_TX:
            GPIO_5_TRIS = state;
            break;
        case GPIO_6_USB_P:
            GPIO_6_TRIS = state;
            break;
        case GPIO_7_USB_N:
            GPIO_7_TRIS = state;
            break;
    }
}

void GpioSetPullUp(BYTE gpio, BYTE state) {
    switch (gpio) {
        case GPIO_0_SCL:
            GPIO_0_PULL_UP = state;
            break;
        case GPIO_1_SDA:
            GPIO_1_PULL_UP = state;
            break;
        case GPIO_2_AN0:
            GPIO_2_PULL_UP = state;
            break;
        case GPIO_3_AN1:
            GPIO_3_PULL_UP = state;
            break;
        case GPIO_4_RX:
            GPIO_4_PULL_UP = state;
            break;
        case GPIO_5_TX:
            GPIO_5_PULL_UP = state;
            break;
        case GPIO_6_USB_P:
            GPIO_6_PULL_UP = state;
            break;
        case GPIO_7_USB_N:
            GPIO_7_PULL_UP = state;
            break;
    }
}

void GpioSetPullDown(BYTE gpio, BYTE state) {
    switch (gpio) {
        case GPIO_0_SCL:
            GPIO_0_PULL_DOWN = state;
            break;
        case GPIO_1_SDA:
            GPIO_1_PULL_DOWN = state;
            break;
        case GPIO_2_AN0:
            GPIO_2_PULL_DOWN = state;
            break;
        case GPIO_3_AN1:
            GPIO_3_PULL_DOWN = state;
            break;
        case GPIO_4_RX:
            GPIO_4_PULL_DOWN = state;
            break;
        case GPIO_5_TX:
            GPIO_5_PULL_DOWN = state;
            break;
        case GPIO_6_USB_P:
            GPIO_6_PULL_DOWN = state;
            break;
        case GPIO_7_USB_N:
            GPIO_7_PULL_DOWN = state;
            break;
    }
}

void GpioSetOutputState(BYTE gpio, BYTE state) {
    switch (gpio) {
        case GPIO_0_SCL:
            GPIO_0_OUTPUT = state;
            break;
        case GPIO_1_SDA:
            GPIO_1_OUTPUT = state;
            break;
        case GPIO_2_AN0:
            GPIO_2_OUTPUT = state;
            break;
        case GPIO_3_AN1:
            GPIO_3_OUTPUT = state;
            break;
        case GPIO_4_RX:
            GPIO_4_OUTPUT = state;
            break;
        case GPIO_5_TX:
            // TODO fix this workaround
            U2MODEbits.ON = 0;
            GPIO_5_OUTPUT = state;
            break;
        case GPIO_6_USB_P:
            GPIO_6_OUTPUT = state;
            break;
        case GPIO_7_USB_N:
            GPIO_7_OUTPUT = state;
            break;
    }
}

void GpioToggleState(BYTE gpio) {
    switch (gpio) {
        case GPIO_0_SCL:
            GPIO_0_TOGGLE();
            break;
        case GPIO_1_SDA:
            GPIO_1_TOGGLE();
            break;
        case GPIO_2_AN0:
            GPIO_2_TOGGLE();
            break;
        case GPIO_3_AN1:
            GPIO_3_TOGGLE();
            break;
        case GPIO_4_RX:
            GPIO_4_TOGGLE();
            break;
        case GPIO_5_TX:
            GPIO_5_TOGGLE();
            break;
        case GPIO_6_USB_P:
            GPIO_6_TOGGLE();
            break;
        case GPIO_7_USB_N:
            GPIO_7_TOGGLE();
            break;
    }
}

void GpioSetChangeNoticeState(BYTE gpio, BYTE state) {
    switch (gpio) {
        case GPIO_0_SCL:
            GPIO_0_CHANGE_NOTICE = state;
            break;
        case GPIO_1_SDA:
            GPIO_1_CHANGE_NOTICE = state;
            break;
        case GPIO_2_AN0:
            GPIO_2_CHANGE_NOTICE = state;
            break;
        case GPIO_3_AN1:
            GPIO_3_CHANGE_NOTICE = state;
            break;
        case GPIO_4_RX:
            GPIO_4_CHANGE_NOTICE = state;
            break;
        case GPIO_5_TX:
            GPIO_5_CHANGE_NOTICE = state;
            break;
        case GPIO_6_USB_P:
            GPIO_6_CHANGE_NOTICE = state;
            break;
        case GPIO_7_USB_N:
            GPIO_7_CHANGE_NOTICE = state;
            break;
    }
}