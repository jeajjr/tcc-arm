/*
 * osciloscopio.h
 *
 *  Created on: 29/02/2016
 *      Author: jeajjr
 */

#ifndef OSCILOSCOPIO_H_
#define OSCILOSCOPIO_H_

/*********************************
 *
 * Phone -> uC communication
 *
 *********************************/
#define COMMAND 0b10000000

// TRIGGER LEVEL
#define SET_TRIGGER_LEVEL 0b00000000
#define TRIGGER_LEVEL_OFF 0b00000000
#define TRIGGER_LEVEL_0 0b00000001
#define TRIGGER_LEVEL_100 0b00011111

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
#define NUM_SAMPLES_FRAME 1000


/*********************************
 *
 * uC -> Phone communication
 *
 *********************************/
#define DATA 0x20
#define CHANNEL_1 0x01
//#define CHANNEL_2 0x02

#endif /* OSCILOSCOPIO_H_ */