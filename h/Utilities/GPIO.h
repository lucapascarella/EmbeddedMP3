
#ifndef __GPIO_H
#define __GPIO_H

#define CONFIG_VERSION          "v0.1"		// Config.h stack version

#include <string.h>
#include <stdlib.h>
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "FatFS/ff.h"

// Mode of operations associated with each pin:
#define GPIO_NOTHING                                0

// Input features
#define GPIO_I_STARTS_PLAYLIST                      1
#define GPIO_I_STOP                                 2
#define GPIO_I_STARTS_STOP                          3
#define GPIO_I_TOGGLE_PAUSE                         4
#define GPIO_I_STARTS_RECORDING                     5
#define GPIO_I_STARTS_STOP_RECORDING                6
#define GPIO_I_RESET                                7

// Output signals when
#define GPIO_O_STARTS_PLAY                          20
#define GPIO_O_STOP_PLAY                            21
#define GPIO_O_PAUSE_PLAY                           22
#define GPIO_O_STARTS_STOP_PLAY                     23
#define GPIO_O_STARTS_PAUSE_STOP_PLAY               24

#define GPIO_O_FILE_NOT_FOUND                       25

#define GPIO_O_STARTS_RECORDING                     26
#define GPIO_O_STOP_RECORDING                       27
#define GPIO_O_PAUSE_RECORDING                      28
#define GPIO_O_STARTS_STOP_RECORDING                29
#define GPIO_O_STARTS_PAUSE_STOP_RECORDING          30

#define GPIO_O_STARTS_PLAY_RECORDING                31
#define GPIO_O_STOP_PLAY_RECORDING                  32
#define GPIO_O_PAUSE_PLAY_RECORDING                 33
#define GPIO_O_STARTS_STOP_PLAY_RECORDING           34
#define GPIO_O_STARTS_PAUSE_STOP_PLAY_RECORDING     35

#define GPIO_O_COMMAND_OK                           36
#define GPIO_O_COMMAND_ERROR                        37
#define GPIO_O_MICRO_SD_OK                          38

// Gpio Special functions:
#define GPIO_S_I2C                                  60
#define GPIO_S_ANALOG                               70
#define GPIO_S_UART                                 80
#define GPIO_S_USB                                  90



#define GPIO_BIT_STARTS_PLAY                        1
#define GPIO_BIT_PAUSE_PLAY                         2
#define GPIO_BIT_STOP_PLAY                          3

#define GPIO_BIT_STARTS_REC                         4
#define GPIO_BIT_PAUSE_REC                          5
#define GPIO_BIT_STOP_REC                           6

#define GPIO_BIT_CMD_OK                             7
#define GPIO_BIT_CMD_ERR                            8

#define GPIO_BIT_FILE_NOT_FOUND                     9
#define GPIO_BIT_MICRO_SD                           10

#ifndef ENABLED
#define ENABLED                                     1
#endif

#ifndef DISABLED
#define DISABLED                                    0
#endif


void GPIOInit();

void GPIOOutputTaskHandler();
void GPIOInputTaskHandler();
void GpioUpdateOutputState(int state);

int Gpio(int, char **);
BOOL GpioGetInputState(BYTE);
BOOL GpioCheckSpecialFunction(BYTE, BYTE);
void GpioSetTris(BYTE, BYTE);
void GpioSetPullUp(BYTE gpio, BYTE state);
void GpioSetPullDown(BYTE gpio, BYTE state);
void GpioSetOutputState(BYTE gpio, BYTE state);
void GpioToggleState(BYTE gpio);
void GpioSetChangeNoticeState(BYTE gpio, BYTE state);

#endif // __GPIO_H
