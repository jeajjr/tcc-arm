#include "osciloscopio.h"

void initializeConfiguration (CONFIG *configs) {
	configs->ctrl_trigger_enabled = FALSE;
	configs->current_trigger_level = 0x80;
	configs->trigger_sample_offset = INITIAL_TRIGGER_SAMPLES_OFFSET;
	configs->current_time_scale = TIME_SCALE_10MS;
	configs->current_voltage_range = 0;
	configs->hold_off_value = HOLD_OFF_START_VALUE;
	configs->num_samples_frame = 1000;
}

unsigned long getTimePeriod(CONFIG *configs)
{
	switch(configs->current_time_scale)
	{
	case TIME_SCALE_10US:
		return (unsigned long) ((SysCtlClockGet() * 0.00001) / configs->num_samples_frame);
	case TIME_SCALE_50US:
		return (unsigned long) ((SysCtlClockGet() * 0.00005) / configs->num_samples_frame);
	case TIME_SCALE_100US:
		return (unsigned long) ((SysCtlClockGet() * 0.0001) / configs->num_samples_frame);
	case TIME_SCALE_200US:
		return (unsigned long) ((SysCtlClockGet() * 0.0002) / configs->num_samples_frame);
	case TIME_SCALE_500US:
		return (unsigned long) ((SysCtlClockGet() * 0.0005) / configs->num_samples_frame);
	case TIME_SCALE_1MS:
		return (unsigned long) ((SysCtlClockGet() * 0.001) / configs->num_samples_frame);
	case TIME_SCALE_5MS:
		return (unsigned long) ((SysCtlClockGet() * 0.005) / configs->num_samples_frame);
	case TIME_SCALE_10MS:
		return (unsigned long) ((SysCtlClockGet() * 0.01) / configs->num_samples_frame);
	case TIME_SCALE_50MS:
		return (unsigned long) ((SysCtlClockGet() * 0.05) / configs->num_samples_frame);
	case TIME_SCALE_100MS:
		return (unsigned long) ((SysCtlClockGet() * 0.1) / configs->num_samples_frame);
	case TIME_SCALE_200MS:
		return (unsigned long) ((SysCtlClockGet() * 0.2) / configs->num_samples_frame);
	case TIME_SCALE_500MS:
		return (unsigned long) ((SysCtlClockGet() * 0.5) / configs->num_samples_frame);
	case TIME_SCALE_1S:
		return (unsigned long) ((SysCtlClockGet() * 1) / configs->num_samples_frame);
	}

	return 0;
}

void parseCommand(CONFIG * configs, char command_received) {
	if ((command_received & MASK_COMMAND) == COMMAND) {
		switch(command_received & MASK_SUB_COMMAND) {
		case SET_TRIGGER_LEVEL:
			if ((command_received & MASK_COMMAND_VALUE) == TRIGGER_LEVEL_OFF)
				configs->ctrl_trigger_enabled = FALSE;
			else {
				configs->ctrl_trigger_enabled = TRUE;
				configs->current_trigger_level = (unsigned char) ((command_received & MASK_COMMAND_VALUE) * 256 / TRIGGER_LEVEL_100);
			}
			break;
		/*
		case SET_VOLTAGE_RANGE:
			break;
		*/
		case SET_TIME_SCALE:
			configs->current_time_scale = (command_received & MASK_COMMAND_VALUE);
			TimerLoadSet(TIMER0_BASE, TIMER_A, getTimePeriod(configs));
			break;
		}
	}
}

void sendSamplesFrame(CONFIG *configs, unsigned char *samples_array, unsigned int current_frame_start_index)
{
	unsigned int i;

	//U S P [DATA | CHANNEL] [VOLTAGE_SCALE] [TIME_SCALE] [DATA_LENGTH_H] [DATA_LENGTH_L] [DATA...] O K

#ifndef PRINTF
	UARTCharPut(UART0_BASE, 'U');
	UARTCharPut(UART0_BASE, 'S');
	UARTCharPut(UART0_BASE, 'P');
	UARTCharPut(UART0_BASE, DATA | CHANNEL_1);
	UARTCharPut(UART0_BASE, configs->current_voltage_range);
	UARTCharPut(UART0_BASE, configs->current_time_scale);
	UARTCharPut(UART0_BASE, (configs->num_samples_frame >> 8) & 0xFF);
	UARTCharPut(UART0_BASE, configs->num_samples_frame & 0xFF);

	for (i = current_frame_start_index; i < configs->num_samples_frame; i++)
			UARTCharPut(UART0_BASE, samples_array[i]);
	for (i = 0; i < current_frame_start_index; i++)
			UARTCharPut(UART0_BASE, samples_array[i]);

	UARTCharPut(UART0_BASE, 'O');
	UARTCharPut(UART0_BASE, 'K');
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

void UARTPrint(char *string) {
	int i=0;
	while (string[i] != '\0')
		UARTCharPut(UART0_BASE, string[i++]);
}

void UARTPrintln(char *string) {
	int i=0;
	while (string[i] != '\0')
		UARTCharPut(UART0_BASE, string[i++]);
	UARTCharPut(UART0_BASE, '\n');
	UARTCharPut(UART0_BASE, '\r');
}

