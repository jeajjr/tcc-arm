#include "osciloscopio.h"

unsigned long getTimePeriod(unsigned long current_time_scale)
{
	switch(current_time_scale)
	{
	case TIME_SCALE_10US:
		return (unsigned long) (SysCtlClockGet() * 0.00001);
	case TIME_SCALE_50US:
		return (unsigned long) (SysCtlClockGet() * 0.00005);
	case TIME_SCALE_100US:
		return (unsigned long) (SysCtlClockGet() * 0.0001);
	case TIME_SCALE_200US:
		return (unsigned long) (SysCtlClockGet() * 0.0002);
	case TIME_SCALE_500US:
		return (unsigned long) (SysCtlClockGet() * 0.0005);
	case TIME_SCALE_1MS:
		return (unsigned long) (SysCtlClockGet() * 0.001);
	case TIME_SCALE_5MS:
		return (unsigned long) (SysCtlClockGet() * 0.005);
	case TIME_SCALE_10MS:
		return (unsigned long) (SysCtlClockGet() * 0.01);
	case TIME_SCALE_50MS:
		return (unsigned long) (SysCtlClockGet() * 0.05);
	case TIME_SCALE_100MS:
		return (unsigned long) (SysCtlClockGet() * 0.1);
	case TIME_SCALE_200MS:
		return (unsigned long) (SysCtlClockGet() * 0.2);
	case TIME_SCALE_500MS:
		return (unsigned long) (SysCtlClockGet() * 0.5);
	case TIME_SCALE_1S:
		return (unsigned long) (SysCtlClockGet() * 1);
	}

	return 0;
}

void sendSamplesFrame(unsigned char current_time_scale, unsigned char current_voltage_range, unsigned char *samples_array)
{
	int i;

	//U S P [DATA | CHANNEL] [VOLTAGE_SCALE] [TIME_SCALE] [DATA_LENGTH_H] [DATA_LENGTH_L] [DATA...] O K
	UARTCharPut(UART0_BASE, 'U');
	UARTCharPut(UART0_BASE, 'S');
	UARTCharPut(UART0_BASE, 'P');
	UARTCharPut(UART0_BASE, DATA | CHANNEL_1);
	UARTCharPut(UART0_BASE, current_voltage_range);
	UARTCharPut(UART0_BASE, current_time_scale);
	UARTCharPut(UART0_BASE, (NUM_SAMPLES_FRAME >> 8) & 0xFF);
	UARTCharPut(UART0_BASE, NUM_SAMPLES_FRAME & 0xFF);

	for (i = 0; i < NUM_SAMPLES_FRAME; i++)
		UARTCharPut(UART0_BASE, samples_array[i]);

	UARTCharPut(UART0_BASE, 'O');
	UARTCharPut(UART0_BASE, 'K');
}



