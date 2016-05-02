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
CONFIG configs;

BOOL ctrl_do_sample = FALSE;
BOOL ctrl_trigger_detected = FALSE;
BOOL crtl_hold_off = FALSE;
BOOL ctrl_parse_command = FALSE;

char command_received;
unsigned char samples_array[MAX_SAMPLES_FRAME] = {0};
unsigned int current_sample_index = 0;
unsigned int current_frame_start_index = 0;
unsigned int continuousSamplingHoldOff = 1;

int LED2 = 0;
int limLED2 = 5;
int LED3 = 0;
int limLED3 = 50;
int PWM = 0;
int counter = 0;


typedef enum {
	CONTINUOUS,
	TRIGGERED,
	HOLD_OFF_SLEEP,
	HOLD_OFF_DETECT

} STATES;
STATES ctrl_current_state = CONTINUOUS;

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
		command_received = UARTCharGetNonBlocking(UART0_BASE);

		UARTCharPut(UART0_BASE, command_received);
		UARTCharPut(UART1_BASE, command_received);

		ctrl_parse_command = TRUE;

		if (LED2)
			{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); LED2 = FALSE; }
		else
			{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); LED2 = TRUE; }


	}


}

void UART1IntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = UARTIntStatus(UART1_BASE, true); //get interrupt status
	UARTIntClear(UART1_BASE, ulStatus); //clear the asserted interrupts
	while(UARTCharsAvail(UART1_BASE)) //loop while there are chars
	{
		command_received = UARTCharGetNonBlocking(UART1_BASE);

		ctrl_parse_command = TRUE;

		if (LED2)
			{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); LED2 = FALSE; }
		else
			{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); LED2 = TRUE; }
	}
}

void Timer0AIntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	ctrl_do_sample = TRUE;
/*
	// LED3 blinking logic
	if (LED3)
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); LED3 = FALSE; }
	else
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); LED3 = TRUE; }

*/
}

void Timer1AIntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	PWM++;
	if (PWM == 4 || PWM == 6)
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);  }
	else
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);  }

	if (PWM == 10)
		PWM = 0;
}

void Timer2AIntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	if (LED3)
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); LED3 = FALSE; }
	else
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); LED3 = TRUE; }

	if (ctrl_current_state == CONTINUOUS)
		UARTPrint("USPOK");
}

/**
 * MAIN METHOD
 */
int main(void)
{
	initializeConfiguration(&configs);

	// clock
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, 0);

	// UART 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 460800,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	// UART 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinConfigure(GPIO_PC4_U1RX);
	GPIOPinConfigure(GPIO_PC5_U1TX);
	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 460800,
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
	TimerLoadSet(TIMER0_BASE, TIMER_A, getTimePeriod(&configs));
	IntEnable(INT_TIMER0A);
	TimerEnable(TIMER0_BASE, TIMER_A);

	// Timer 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_32_BIT_PER);
	TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet() * 0.00053)); //0.00001 = período de 20us
																	  //0.001 = período de 2ms
	IntEnable(INT_TIMER1A);
	TimerEnable(TIMER1_BASE, TIMER_A);

	// Timer 2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(TIMER2_BASE, TIMER_CFG_32_BIT_PER);
	TimerLoadSet(TIMER2_BASE, TIMER_A, (SysCtlClockGet() * 1)); //1Hz
	IntEnable(INT_TIMER2A);
	TimerEnable(TIMER2_BASE, TIMER_A);

	// interruptions
	IntMasterEnable();
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	IntEnable(INT_UART1);
	UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);

	char blah[50];
	int i;
	for (i=0; i < MAX_SAMPLES_FRAME; i++) {
		samples_array[i] = 0;
	}

//	snprintf (blah, 50, "Welcome!"); UARTPrintln(blah);

/*
 	 UARTPrintln("starting ...");
	snprintf (blah, 50, "%d", (int) getTimePeriod(&configs)); UARTPrintln(blah);

	TimerEnable(TIMER1_BASE, TIMER_A);
	ADCRead();
	TimerDisable(TIMER1_BASE, TIMER_A);
	snprintf (blah, 50, "%d", (int) TimerValueGet(TIMER1_BASE, TIMER_A)); UARTPrintln(blah);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
	while(1);
*/

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);

	UARTPrint("USPOK");

	while(1)
	{
#define ENABLE
#ifdef ENABLE
		if (ctrl_do_sample)
		{
			ctrl_do_sample = FALSE;

			// continuous circular acquisition
			samples_array[current_sample_index] = ADCRead();

			switch(ctrl_current_state) {
			case CONTINUOUS:
				continuousSamplingHoldOff--;
				if (continuousSamplingHoldOff == 0) {
					UARTPrintChar(samples_array[current_sample_index]);
					continuousSamplingHoldOff = getContinuousModeSamplingSpacing(&configs);
				}

				//trigger detection
				if (configs.ctrl_trigger_enabled
					&& samples_array[current_sample_index] > configs.current_trigger_level)
				{
					ctrl_current_state = TRIGGERED;
					current_frame_start_index = getFrameStart(&configs, current_sample_index);
				}
				break;
			case TRIGGERED:

				// detect end of current frame
				if (
					((current_frame_start_index == 0 && current_sample_index == configs.num_samples_frame - 1)
					|| (current_sample_index == current_frame_start_index - 1))
					)
				{
					ctrl_current_state = HOLD_OFF_SLEEP;
					configs.hold_off_value = calculateHoldOffTicks(&configs);
					GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
					sendSamplesFrame(&configs, samples_array, current_frame_start_index);
				}

				break;
			case HOLD_OFF_SLEEP:
				configs.hold_off_value--;
				int limit = calculateHoldOffTicks(&configs) * HOLD_OFF_ACTIVE_PERC / 100;
				if (configs.hold_off_value < limit)
					ctrl_current_state = HOLD_OFF_DETECT;

				break;
			case HOLD_OFF_DETECT:
				configs.hold_off_value--;

				// Time's up, no trigger detected
				if (configs.hold_off_value == 0) {
					ctrl_current_state = CONTINUOUS;
					break;
				}

				//trigger detection
				if (configs.ctrl_trigger_enabled
					&& samples_array[current_sample_index] > configs.current_trigger_level)
				{
					ctrl_current_state = TRIGGERED;
					current_frame_start_index = getFrameStart(&configs, current_sample_index);
				}
				break;
			}

			current_sample_index++;
			if (current_sample_index == configs.num_samples_frame)
				current_sample_index = 0;
		}
#endif

		if(ctrl_parse_command) {
			ctrl_parse_command = FALSE;

			parseCommand(&configs, command_received);
		}

		//UARTCharPut(UART0_BASE, ADCRead());
/*
		// LED3 blinking logic
		if (LED3 < limLED3/2) GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
		else GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
		if (LED3++ == limLED3) LED3 = 0;
		*/
	}
}
