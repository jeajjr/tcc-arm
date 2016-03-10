#include "osciloscopio.h"

#define PI 3.141592

#ifdef DEBUG
void__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

/**
 * GLOBAL VARIABLES
 */
unsigned char current_trigger_level = 0x80;
unsigned char current_time_scale = TIME_SCALE_1S;
unsigned char current_voltage_range = 0;

unsigned int current_sample_index = 0;
unsigned int current_frame_start_index = 0;
unsigned char samples_array[NUM_SAMPLES_FRAME];

unsigned char ctrl_do_sample = FALSE;
unsigned char ctrl_trigger_detected = FALSE;

int LED2 = 0;
int limLED2 = 5;
int LED3 = 0;
int limLED3 = 50;

/**
 * INTERRUPT HANDLERS
 */
void UART0IntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ulStatus); //clear the asserted interrupts
	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		char a = UARTCharGetNonBlocking(UART0_BASE);
	}
}

void UART1IntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = UARTIntStatus(UART1_BASE, true); //get interrupt status
	UARTIntClear(UART1_BASE, ulStatus); //clear the asserted interrupts
	while(UARTCharsAvail(UART1_BASE)) //loop while there are chars
	{
		char a = UARTCharGetNonBlocking(UART1_BASE);
	}
}

void Timer0AIntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	ctrl_do_sample = TRUE;

	// LED3 blinking logic
	if (LED3)
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); LED3 = FALSE; }
	else
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); LED3 = TRUE; }
}

/**
 * MAIN METHOD
 */
int main(void)
{
	// clock
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

	// UART 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200*2,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	// UART 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinConfigure(GPIO_PC4_U1RX);
	GPIOPinConfigure(GPIO_PC5_U1TX);
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	// ADC
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_1MSPS);//SYSCTL_ADCSPEED_500KSPS

	// ADC - sequencer 1
	ADCSequenceDisable(ADC0_BASE, 1);
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);

	// ADC - sequencer 3
/*
	ADCSequenceDisable(ADC0_BASE, 3);
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 3);
*/

	// Timer 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
	TimerLoadSet(TIMER0_BASE, TIMER_A, getTimePeriod(current_time_scale)/NUM_SAMPLES_FRAME - 1);
	IntEnable(INT_TIMER0A);
	TimerEnable(TIMER0_BASE, TIMER_A);

	// interruptions
	IntMasterEnable();
	IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
/*
	IntEnable(INT_UART1);
	UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
*/
	char blah[50];
	int i;
	for (i=0; i<NUM_SAMPLES_FRAME; i++)
		samples_array[i] = 0;

	UARTPrintln("starting ...");
	while(1)
	{
#define ENABLE
#ifdef ENABLE
		if (ctrl_do_sample)
		{
			ctrl_do_sample = FALSE;

			// continuous circuilar acquisition
			samples_array[current_sample_index] = ADCRead();

			if (samples_array[current_sample_index] > current_trigger_level)
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
			else
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

			snprintf (blah, 50, "current_frame_start_index: %d", current_frame_start_index); UARTPrintln(blah);
			snprintf (blah, 50, "samples_array[%d]: %d", current_sample_index, samples_array[current_sample_index]); UARTPrintln(blah);

			if (!ctrl_trigger_detected)
			{
				//trigger detection
				if (samples_array[current_sample_index] > current_trigger_level
						/*
					&& samples_array[current_sample_index >= TRIGGER_SAMPLES_OFFSET ?
						current_sample_index - TRIGGER_SAMPLES_OFFSET :
						NUM_SAMPLES_FRAME - TRIGGER_SAMPLES_OFFSET + current_sample_index] < current_trigger_level
						*/
				)
				{
					ctrl_trigger_detected = TRUE;
					UARTPrintln("trigger on");
					current_frame_start_index = current_sample_index;
				}
			}
			else
			{
				// detect end of current frame
				if ((current_frame_start_index == 0 && current_sample_index == NUM_SAMPLES_FRAME - 1)
					|| (current_sample_index == current_frame_start_index - 1))
				{
					ctrl_trigger_detected = FALSE;
					UARTPrintln("trigger off");
					sendSamplesFrame(current_time_scale, current_voltage_range, samples_array, current_frame_start_index);
				}
			}

			current_sample_index++;
			if (current_sample_index == NUM_SAMPLES_FRAME)
				current_sample_index = 0;
		}
#endif

		//UARTCharPut(UART0_BASE, ADCRead());
/*
		// LED3 blinking logic
		if (LED3 < limLED3/2) GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
		else GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
		if (LED3++ == limLED3) LED3 = 0;
		*/
	}
}
