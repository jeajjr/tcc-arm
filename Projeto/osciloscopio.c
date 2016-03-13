#include "osciloscopio.h"

unsigned long getTimePeriod(unsigned long current_time_scale)
{
	switch(current_time_scale)
	{
	case TIME_SCALE_10US:
		return (unsigned long) ((SysCtlClockGet() * 0.00001) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_50US:
		return (unsigned long) ((SysCtlClockGet() * 0.00005) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_100US:
		return (unsigned long) ((SysCtlClockGet() * 0.0001) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_200US:
		return (unsigned long) ((SysCtlClockGet() * 0.0002) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_500US:
		return (unsigned long) ((SysCtlClockGet() * 0.0005) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_1MS:
		return (unsigned long) ((SysCtlClockGet() * 0.001) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_5MS:
		return (unsigned long) ((SysCtlClockGet() * 0.005) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_10MS:
		return (unsigned long) ((SysCtlClockGet() * 0.01) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_50MS:
		return (unsigned long) ((SysCtlClockGet() * 0.05) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_100MS:
		return (unsigned long) ((SysCtlClockGet() * 0.1) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_200MS:
		return (unsigned long) ((SysCtlClockGet() * 0.2) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_500MS:
		return (unsigned long) ((SysCtlClockGet() * 0.5) / NUM_SAMPLES_FRAME);
	case TIME_SCALE_1S:
		return (unsigned long) ((SysCtlClockGet() * 1) / NUM_SAMPLES_FRAME);
	}

	return 0;
}

void sendSamplesFrame(unsigned char current_time_scale, unsigned char current_voltage_range, unsigned char *samples_array, unsigned int current_frame_start_index)
{
	unsigned int i;

	//U S P [DATA | CHANNEL] [VOLTAGE_SCALE] [TIME_SCALE] [DATA_LENGTH_H] [DATA_LENGTH_L] [DATA...] O K

#ifndef PRINTF
	UARTCharPut(UART0_BASE, 'U');
	UARTCharPut(UART0_BASE, 'S');
	UARTCharPut(UART0_BASE, 'P');
	UARTCharPut(UART0_BASE, DATA | CHANNEL_1);
	UARTCharPut(UART0_BASE, current_voltage_range);
	UARTCharPut(UART0_BASE, current_time_scale);
	UARTCharPut(UART0_BASE, (NUM_SAMPLES_FRAME >> 8) & 0xFF);
	UARTCharPut(UART0_BASE, NUM_SAMPLES_FRAME & 0xFF);

	for (i = current_frame_start_index; i < NUM_SAMPLES_FRAME; i++)
			UARTCharPut(UART0_BASE, samples_array[i]);
	for (i = 0; i < current_frame_start_index; i++)
			UARTCharPut(UART0_BASE, samples_array[i]);

	UARTCharPut(UART0_BASE, 'O');
	UARTCharPut(UART0_BASE, 'K');
#else
	UARTPrintln("sample");
	char buff[100];
	snprintf (buff, 100, "%d %d %d %d ", DATA | CHANNEL_1, current_voltage_range, current_time_scale, NUM_SAMPLES_FRAME);
	UARTPrint(buff);
	for (i = current_frame_start_index; i < NUM_SAMPLES_FRAME; i++) {
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

