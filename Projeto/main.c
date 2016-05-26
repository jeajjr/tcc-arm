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

char command_received = 0;
unsigned char samples_array[BUFFER_SIZE] = {0};
unsigned int current_sample_index = 0;
unsigned int current_frame_start_index = 0;
unsigned int continuousSamplingHoldOff = 1;
unsigned int hold_off_sleep_limit = 0;
unsigned int hold_off_counter = 0;

int LED2 = 0;
int limLED2 = 5;
int LED3 = 0;
int limLED3 = 50;
int PWM = 0;
BOOL sending_frame = FALSE;

typedef enum {
	CONTINUOUS,
	TRIGGERED,
	HOLD_OFF_SLEEP,
	HOLD_OFF_DETECT
} STATES;
STATES ctrl_current_state = CONTINUOUS;

void blinkLED2() {
	if (LED2)
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); LED2 = FALSE; }
	else
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); LED2 = TRUE; }
}

void blinkLED3() {
	if (LED3)
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); LED3 = FALSE; }
	else
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0); LED3 = TRUE; }
}



void blinkB7() {
	static char B7 = 0;

	if (B7)
			{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0); B7 = FALSE; }
		else
			{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0); B7 = TRUE; }
}

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

		parseCommand(&configs, command_received);

		UARTCharPut(UART0_BASE, command_received);
		UARTCharPut(UART1_BASE, command_received);
	}
}

void UART1IntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = UARTIntStatus(UART1_BASE, true); //get interrupt status
	UARTIntClear(UART1_BASE, ulStatus); //clear the asserted interrupts
	while(UARTCharsAvail(UART1_BASE)) //loop while there are chars
	{
		command_received = UARTCharGet(UART1_BASE);

		parseCommand(&configs, command_received);
	}
}

int i = 0;
void Timer0AIntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	if (ctrl_do_sample && !sending_frame)
		blinkB2();
	//else
		ctrl_do_sample = TRUE;
/*
	if (i++ == 1000)
		blinkLED3();
	if (i == 2000)
		i=0;
		*/
}

void Timer1AIntHandler(void)
{
	static int a = 0;
	// Clear the timer interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	PWM++;
	if (PWM == 4 || PWM == 6 || PWM == 11 || PWM == 20 || (PWM == 21 && (a++ % 2)))
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);  }
	else
		{ GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);  }

	if (PWM == 70)
		PWM = 0;
}

void Timer2AIntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	//blinkLED2();

	if (ctrl_current_state == CONTINUOUS)
		UARTPrint("USPOK");
}

/**
 * MAIN METHOD
 */
int main(void)
{
	initializeConfiguration(&configs);

	//0.00001 = período de 20us
	//0.001 = período de 2ms
	initializeHardware(&configs, 0.01);

	//test();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_7);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_7, 0);

	UARTPrint("USPOK");
	int i;

	while(0) {
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
		for (i=0;i<100;i++)
			ADCRead();
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);
		for (i=0;i<100000;i++);
	}

	while(1)
	{
		if (ctrl_do_sample)
		{
			ctrl_do_sample = FALSE;

			// continuous circular acquisition
			samples_array[current_sample_index] = ADCRead();

			unsigned char previousSample;

			switch(ctrl_current_state) {
			case CONTINUOUS:
				if (continuousSamplingHoldOff != 0)
					continuousSamplingHoldOff--;
				else {
					UARTPrintChar(samples_array[current_sample_index]);
					continuousSamplingHoldOff = getContinuousModeSamplingSpacing(&configs);
				}
				break;

			case TRIGGERED:
				// detect end of current frame
				if (current_sample_index == incrementIndexBy(current_frame_start_index, configs.num_samples_frame - 1))
				{
					sending_frame = TRUE;
					sendSamplesFrame(&configs, samples_array, current_frame_start_index);
					sending_frame = FALSE;

					hold_off_counter = calculateHoldOffTicks(&configs);
					hold_off_sleep_limit = calculateHoldOffSleepTicks(&configs);

					ctrl_current_state = HOLD_OFF_SLEEP;
				}
				break;

			case HOLD_OFF_SLEEP:
				if (hold_off_counter < hold_off_sleep_limit || hold_off_counter == 0)
					ctrl_current_state = HOLD_OFF_DETECT;
				else
					hold_off_counter--;
				break;

			case HOLD_OFF_DETECT:
				// Time's up, no trigger detected
				if (hold_off_counter == 0) {
					ctrl_current_state = CONTINUOUS;
					continuousSamplingHoldOff = getContinuousModeSamplingSpacing(&configs);
				}
				else {
					hold_off_counter--;
				}
				break;
			}

			//trigger detection
			if (configs.trigger_configuration != DISABLED &&
				(ctrl_current_state == CONTINUOUS || ctrl_current_state == HOLD_OFF_DETECT)) {
				previousSample = samples_array[decrementIndex(current_sample_index, configs.num_samples_frame)];
				if (
					(configs.trigger_configuration == RISE
							&& samples_array[current_sample_index] >= configs.current_trigger_level
							&& previousSample < configs.current_trigger_level)
					||
					(configs.trigger_configuration == FALL
							&& samples_array[current_sample_index] <= configs.current_trigger_level
							&& previousSample > configs.current_trigger_level)
					)
				{
					ctrl_current_state = TRIGGERED;
					current_frame_start_index = getFrameStart(&configs, current_sample_index);
				}
			}

			current_sample_index = incrementIndex(current_sample_index);
		}
	}
}
