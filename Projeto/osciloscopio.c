#include "osciloscopio.h"

void initializeConfiguration (CONFIG *configs) {
	configs->trigger_configuration = RISE;
	configs->current_trigger_level = 0x80;
	configs->trigger_sample_offset = INITIAL_TRIGGER_SAMPLES_OFFSET;
	configs->current_time_scale = TIME_SCALE_500MS;
	configs->current_voltage_range = 0;
	configs->num_samples_frame = MAX_SAMPLES_FRAME;
	configs->hold_off_value = calculateHoldOffTicks(configs);
}

unsigned int decrementIndex(unsigned int value, unsigned int limit) {
	if (value != 0)
		return value - 1;

	return limit - 1;
}

unsigned long getTimePeriod(CONFIG *configs)
{
	switch(configs->current_time_scale)
	{
	case TIME_SCALE_10US:
		return 500;
	case TIME_SCALE_50US:
		return 500;
	case TIME_SCALE_100US:
		return 500;
	case TIME_SCALE_200US:
		return 500;
	case TIME_SCALE_500US:
		return 500;
	case TIME_SCALE_1MS:
		return 500;
	case TIME_SCALE_5MS:
		return 500;
	case TIME_SCALE_10MS:
		return 500;
	case TIME_SCALE_50MS:
		return 2500;
	case TIME_SCALE_100MS:
		return 5000;
	case TIME_SCALE_200MS:
		return 10000;
	case TIME_SCALE_500MS:
		return 25000;
	case TIME_SCALE_1S:
		return 50000;
	}

	return 0;
}

void parseCommand(CONFIG * configs, char command_received) {
	if ((command_received & MASK_COMMAND) == COMMAND) {
		switch(command_received & MASK_SUB_COMMAND) {
		case SET_TRIGGER_LEVEL:
			if ((command_received & MASK_COMMAND_VALUE) == TRIGGER_LEVEL_OFF)
				configs->trigger_configuration = DISABLED;
			else {

				configs->trigger_configuration = RISE; //TODO
				configs->current_trigger_level = (unsigned char) ((command_received & MASK_COMMAND_VALUE) * 256 / TRIGGER_LEVEL_100);
			}
			break;
		/*
		case SET_VOLTAGE_RANGE:
			break;
		*/
		case SET_TIME_SCALE:
			if (configs->current_time_scale != (unsigned char) (command_received & MASK_COMMAND_VALUE)) {
				configs->current_time_scale = (command_received & MASK_COMMAND_VALUE);

				unsigned long newTimePeriod = getTimePeriod(configs);
				updateNumSamplesFrame(configs);

				TimerLoadSet(TIMER0_BASE, TIMER_A, newTimePeriod);
			}
			break;
		}
	}
}

void updateNumSamplesFrame(CONFIG *configs) {
	switch(configs->current_time_scale) {
	case TIME_SCALE_10US:
		configs->num_samples_frame = 1;
		break;
	case TIME_SCALE_50US:
		configs->num_samples_frame = 5;
		break;
	case TIME_SCALE_100US:
		configs->num_samples_frame = 10;
		break;
	case TIME_SCALE_200US:
		configs->num_samples_frame = 20;
		break;
	case TIME_SCALE_500US:
		configs->num_samples_frame = 50;
		break;
	case TIME_SCALE_1MS:
		configs->num_samples_frame = 100;
		break;
	case TIME_SCALE_5MS:
		configs->num_samples_frame = 500;
		break;
	case TIME_SCALE_10MS:
		configs->num_samples_frame = 1000;
		break;
	case TIME_SCALE_50MS:
		configs->num_samples_frame = 1000;
		break;
	case TIME_SCALE_100MS:
		configs->num_samples_frame = 1000;
		break;
	case TIME_SCALE_200MS:
		configs->num_samples_frame = 1000;
		break;
	case TIME_SCALE_500MS:
		configs->num_samples_frame = 1000;
		break;
	case TIME_SCALE_1S:
		configs->num_samples_frame = 1000;
		break;
	}
}

unsigned int calculateHoldOffTicks(CONFIG *configs) {
//	return HOLD_OFF_TIME_MS; //return configs->num_samples_frame * 5;

	switch(configs->current_time_scale)
	{
	case TIME_SCALE_10US:
		return 99999;
	case TIME_SCALE_50US:
		return 99995;
	case TIME_SCALE_100US:
		return 99990;
	case TIME_SCALE_200US:
		return 99980;
	case TIME_SCALE_500US:
		return 99950;
	case TIME_SCALE_1MS:
		return 99900;
	case TIME_SCALE_5MS:
		return 39800;
	case TIME_SCALE_10MS:
		return 99000;
	case TIME_SCALE_50MS:
		return 19000;
	case TIME_SCALE_100MS:
		return 9000;
	case TIME_SCALE_200MS:
		return 4000;
	case TIME_SCALE_500MS:
		return 1000;
	case TIME_SCALE_1S:
		return 1000;
	}

	return 0;
}

void sendSamplesFrame(CONFIG *configs, unsigned char *samples_array, unsigned int current_frame_start_index)
{
	unsigned int i;

	//U S P [DATA | CHANNEL] [VOLTAGE_SCALE] [TIME_SCALE] [DATA_LENGTH_H] [DATA_LENGTH_L] [DATA...] O K

#ifndef PRINTF
	UARTPrintChar('U');
	UARTPrintChar('S');
	UARTPrintChar('P');
	UARTPrintChar('B');
	UARTPrintChar('K');
	UARTPrintChar(DATA | CHANNEL_1);
	UARTPrintChar(configs->current_voltage_range);
	UARTPrintChar(configs->current_time_scale);
	UARTPrintChar((configs->num_samples_frame >> 8) & 0xFF);
	UARTPrintChar(configs->num_samples_frame & 0xFF);

	for (i = current_frame_start_index; i < configs->num_samples_frame; i++)
			UARTPrintChar(samples_array[i]);
	for (i = 0; i < current_frame_start_index; i++)
			UARTPrintChar(samples_array[i]);

	UARTPrintChar('O');
	UARTPrintChar('K');
#else
	UARTPrintln("sample");
	char buff[100];
	snprintf (buff, 100, "%d %d %d %d ", DATA | CHANNEL_1, configs->current_voltage_range, configs->current_time_scale, NUM_SAMPLES_FRAME);
	UARTPrint(buff);
	for (i = current_frame_start_index; i < configs->num_samples_frame; i++) {
		snprintf (buff, 100, "%d ", samples_array[i]);
		UARTPrint(buff);
	}

	for (i = 0; i < current_frame_start_index; i++) {
		snprintf (buff, 100, "%d ", samples_array[i]);
		UARTPrint(buff);
	}

	UARTPrintln("");
#endif
}

unsigned char ADCRead() {
	unsigned long ulADC0Value[4];
	ADCIntClear(ADC0_BASE, 1);
	ADCProcessorTrigger(ADC0_BASE, 1);
	while(!ADCIntStatus(ADC0_BASE, 1, false));
	ADCSequenceDataGet(ADC0_BASE, 1, ulADC0Value);
	unsigned int leitura = (ulADC0Value[0] + ulADC0Value[1] + ulADC0Value[2] + ulADC0Value[3])/4;
	return ((leitura>>4) & 0xFF);
}
unsigned int getFrameStart(CONFIG * configs, unsigned int current_index) {
	int a = current_index - configs->trigger_sample_offset;
	if (a < 0)
		return configs->num_samples_frame + current_index - configs->trigger_sample_offset;
	else
		return current_index - configs->trigger_sample_offset;
}

void UARTPrintChar(char a) {
	UARTCharPutNonBlocking(UART1_BASE, a);
	UARTCharPut(UART0_BASE, a);
}
void UARTPrint(char *string) {
	int i=0;
	while (string[i] != '\0')
		UARTPrintChar(string[i++]);
}

void UARTPrintln(char *string) {
	int i=0;
	while (string[i] != '\0')
		UARTPrintChar(string[i++]);
	UARTPrintChar('\n');
	UARTPrintChar('\r');
}

unsigned int getContinuousModeSamplingSpacing(CONFIG *configs) {
	switch(configs->current_time_scale)
	{
	case TIME_SCALE_10US:
		return 1;
	case TIME_SCALE_50US:
		return 5;
	case TIME_SCALE_100US:
		return 10;
	case TIME_SCALE_200US:
		return 20;
	case TIME_SCALE_500US:
		return 50;
	case TIME_SCALE_1MS:
		return 100;
	case TIME_SCALE_5MS:
		return 500;
	case TIME_SCALE_10MS:
		return 1000;
	case TIME_SCALE_50MS:
		return 1000;
	case TIME_SCALE_100MS:
		return 1000;
	case TIME_SCALE_200MS:
		return 1000;
	case TIME_SCALE_500MS:
		return 1000;
	case TIME_SCALE_1S:
		return 1000;
	}
	return 1000;
}

