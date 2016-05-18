/*
 * osciloscopio.h
 *
 *  Created on: 29/02/2016
 *      Author: jeajjr
 */

#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "math.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include <string.h>
#include <stdio.h>
#include "utils/ustdlib.h"

#ifndef OSCILOSCOPIO_H_
#define OSCILOSCOPIO_H_

/*********************************
 *
 * GENERAL CONSTANTS AND DEFINITIONS
 *
 *********************************/

#define BOOL unsigned char
#define TRUE 1
#define FALSE 0
//#define PRINTF

typedef enum {DISABLED, RISE, FALL} TRIGGER_CONFIG;

typedef struct _CONFIG {
	TRIGGER_CONFIG trigger_configuration;
	unsigned char current_trigger_level;
	unsigned int trigger_sample_offset;
	unsigned char current_time_scale;
	unsigned char current_voltage_range;
	unsigned int hold_off_value;
	unsigned int num_samples_frame;
} CONFIG;

#define CONTINUOUS_MODE_SAMPLES_SEC 1000

#define BUFFER_SIZE 2048

/*********************************
 *
 * Phone -> uC communication
 *
 *********************************/
#define MASK_COMMAND 0b11100000
#define MASK_COMMAND_VALUE 0b00011111


/**************************************************
 *                  TRIGGER LEVEL
 *************************************************/

#define SET_TRIGGER_OFF  0b10000000
#define SET_TRIGGER_RISE 0b10100000
#define SET_TRIGGER_FALL 0b11000000

/* trigger offset:
	when 0, triggered sample will be at the start of the sent frame
	when NUM_SAMPLES_FRAME/2, triggered sample will be at the middle of the sent frame
	when NUM_SAMPLES_FRAME, triggered sample will be at the end of the sent frame
*/
#define INITIAL_TRIGGER_SAMPLES_OFFSET 400

#define TRIGGER_LEVEL_0 0b00000000
#define TRIGGER_LEVEL_100 0b00011111

/**************************************************
 *              HOLD OFF LENGTH
 *************************************************/
#define SET_HOLD_OFF 0b00100000

#define HOLD_OFF_MIN 0b00000001 // 1/8 of holdoff length
#define HOLD_OFF_MAX 0b00000111 // 7/8 of holdoff length

/**************************************************
 *                  TIME SCALE
 *************************************************/
// TIME SCALE
#define SET_TIME_SCALE 0b01000000

#define TIME_SCALE_10US  0b00000000
#define TIME_SCALE_20US  0b00000001
#define TIME_SCALE_50US  0b00000010
#define TIME_SCALE_100US 0b00000011
#define TIME_SCALE_200US 0b00000100
#define TIME_SCALE_500US 0b00000101
#define TIME_SCALE_1MS   0b00000110
#define TIME_SCALE_2MS   0b00000111
#define TIME_SCALE_5MS   0b00001000
#define TIME_SCALE_10MS  0b00001001
#define TIME_SCALE_20MS  0b00001010
#define TIME_SCALE_50MS  0b00001011
#define TIME_SCALE_100MS 0b00001100
#define TIME_SCALE_200MS 0b00001101
#define TIME_SCALE_500MS 0b00001110
#define TIME_SCALE_1S    0b00001111
#define TIME_SCALE_2S    0b00010000

// number of samples in a time frame (osciloscope screen)
#define HOLD_OFF_TIME_MS 1000
#define HOLD_OFF_ACTIVE_PERC 80

/*********************************
 *
 * uC -> Phone communication
 *
 *********************************/
#define DATA 0x20
#define CHANNEL_1 0x01
//#define CHANNEL_2 0x02

/*********************************
 *
 * METHODS PROTOTYPES
 *
 *********************************/
void initializeConfiguration (CONFIG *configs);

unsigned int incrementIndex(unsigned int index);
unsigned int decrementIndex(unsigned int value, unsigned int limit);
unsigned int incrementIndexBy(unsigned int index, unsigned int addend);

unsigned int getFrameStart(CONFIG * configs, unsigned int current_index);
void parseCommand(CONFIG * configs, char command_received);

unsigned char ADCRead();

void UARTPrintChar(char a);
void UARTPrint(char *string);
void UARTPrintln(char *string);
void sendSamplesFrame(CONFIG *configs, unsigned char *samples_array, unsigned int current_frame_start_index);

unsigned long getTimePeriod(CONFIG *configs);
void updateNumSamplesFrame(CONFIG *configs);
unsigned int calculateHoldOffSleepTicks(CONFIG *configs);
unsigned int calculateHoldOffTicks(CONFIG *configs);
unsigned int getContinuousModeSamplingSpacing(CONFIG *configs);
void initializeHardware(CONFIG *configs, float halfPeriodOut);

#endif /* OSCILOSCOPIO_H_ */



