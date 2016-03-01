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
}
