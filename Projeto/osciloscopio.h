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

typedef struct _CONFIG {
	BOOL ctrl_trigger_enabled;
	unsigned char current_trigger_level;
	unsigned int trigger_sample_offset;
	unsigned char current_time_scale;
	unsigned char current_voltage_range;
	unsigned int hold_off_initial_value;
	unsigned int hold_off_current_value;
	unsigned int num_samples_frame;
} CONFIG;

#define MAX_SAMPLES_FRAME 1000

/*********************************
 *
 * Phone -> uC communication
 *
 *********************************/
#define MASK_COMMAND 0b10000000
#define MASK_SUB_COMMAND 0b01100000
#define MASK_COMMAND_VALUE 0b00011111

#define COMMAND 0b10000000

// TRIGGER LEVEL
#define SET_TRIGGER_LEVEL 0b00000000
/* trigger offset:
	when 0, triggered sample will be at the start of the sent frame
	when NUM_SAMPLES_FRAME/2, triggered sample will be at the middle of the sent frame
	when NUM_SAMPLES_FRAME, triggered sample will be at the end of the sent frame
*/
#define INITIAL_TRIGGER_SAMPLES_OFFSET 50

#define TRIGGER_LEVEL_0 0b00000000
#define TRIGGER_LEVEL_100 0b00011110
#define TRIGGER_LEVEL_OFF 0b00011111

// VOLTAGE RANGE
	/*
#define SET_VOLTAGE_RANGE 0b00100000
	*/

// TIME SCALE
#define SET_TIME_SCALE 0b01000000

#define TIME_SCALE_10US 0b00000000
#define TIME_SCALE_50US 0b00000001
#define TIME_SCALE_100US 0b00000010
#define TIME_SCALE_200US 0b00000011
#define TIME_SCALE_500US 0b00000100
#define TIME_SCALE_1MS 0b00000101
#define TIME_SCALE_5MS 0b00000110
#define TIME_SCALE_10MS 0b00000111
#define TIME_SCALE_50MS 0b00001000
#define TIME_SCALE_100MS 0b00001001
#define TIME_SCALE_200MS 0b00001010
#define TIME_SCALE_500MS 0b00001011
#define TIME_SCALE_1S 0b00001100

// number of samples in a time frame (osciloscope screen)
#define HOLD_OFF_START_VALUE 0
#define SET_HOLD_OFF 0b01100000

#define HOLD_OFF_0 0b00000000
#define HOLD_OFF_100 0b00011110
#define HOLD_OFF_OFF 0b00011111

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
unsigned long getTimePeriod(CONFIG *configs);
void parseCommand(CONFIG * configs, char command_received);
void sendSamplesFrame(CONFIG *configs, unsigned char *samples_array, unsigned int current_frame_start_index);
unsigned char ADCRead();
unsigned int getFrameStart(CONFIG *configs, unsigned int current_index);
void UARTPrint(char *string);
void UARTPrintln(char *string);

#endif /* OSCILOSCOPIO_H_ */



