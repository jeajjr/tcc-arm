#include "osciloscopio.h"

void initializeConfiguration (CONFIG *configs) {
	configs->trigger_configuration = FALL;
	configs->current_trigger_level = 0x80;
	configs->trigger_sample_offset = 0;
	configs->current_time_scale = TIME_SCALE_100MS;
	configs->current_voltage_range = 0;
	updateNumSamplesFrame(configs);
	configs->hold_off_value = HOLD_OFF_MIN;
}

unsigned int incrementIndex(unsigned int index) {
	if (++index == BUFFER_SIZE)
		index = 0;

	return index;
}

unsigned int incrementIndexBy(unsigned int index, unsigned int addend) {
	index += addend;
	if (index >= BUFFER_SIZE)
		return index - BUFFER_SIZE;
	return index;
}

unsigned int decrementIndex(unsigned int value, unsigned int limit) {
	if (value != 0)
		return value - 1;

	return limit - 1;
}

unsigned int getFrameStart(CONFIG * configs, unsigned int current_index) {
	int a = current_index - configs->trigger_sample_offset;
	if (a < 0)
		return BUFFER_SIZE + current_index - configs->trigger_sample_offset;
	else
		return current_index - configs->trigger_sample_offset;
}

void parseCommand(CONFIG * configs, char command_received) {

	switch(command_received & MASK_COMMAND) {

	case SET_TRIGGER_RISE:
		configs->trigger_configuration = RISE;
		configs->current_trigger_level = (unsigned char) ((command_received & MASK_COMMAND_VALUE) * 256 / TRIGGER_LEVEL_100);
		break;

	case SET_TRIGGER_FALL:
		configs->trigger_configuration = FALL;
		configs->current_trigger_level = (unsigned char) ((command_received & MASK_COMMAND_VALUE) * 256 / TRIGGER_LEVEL_100);
		break;

	case SET_TRIGGER_OFF:
		configs->trigger_configuration = DISABLED;
		break;

	case SET_HOLD_OFF:
		configs->hold_off_value = (unsigned char) (command_received & MASK_COMMAND_VALUE);
		break;

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

void sendSamplesFrame(CONFIG *configs, unsigned char *samples_array, unsigned int current_frame_start_index)
{
	unsigned int i;
	//U S P B K [DATA | CHANNEL] [VOLTAGE_SCALE] [TIME_SCALE] [DATA_LENGTH_H] [DATA_LENGTH_L] [DATA...] 40 80 200

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

	for (i = 0; i < configs->num_samples_frame; i++) {
		UARTPrintChar(samples_array[current_frame_start_index]);
		current_frame_start_index = incrementIndex(current_frame_start_index);
	}

	UARTPrintChar(40);
	UARTPrintChar(80);
	UARTPrintChar(200);
		/*
	UARTPrintChar('O');
	UARTPrintChar('K');
	*/
}


static unsigned int NUM_SAMPLES_FRAME[] = {1, 2, 5, 10, 30, 60, 120, 256, 512, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024};
static unsigned int TIME_PERIOD[] = {500, 625, 500, 500, 417, 417, 417, 488, 488, 488, 1221, 2441, 4883, 12207, 24414, 48828, 122070};
static unsigned int HOLD_OFF_SLEEP_TICKS[] = {12500, 10000, 12499, 12499, 14996, 14993, 14985, 12768, 12736, 12672, 4992, 2432, 1152, 384, 128, 125, 0};
static unsigned int HOLD_OFF_TICKS[] = {99999, 79998, 99995, 99990, 119970, 119940, 119880, 102144, 101888, 101376, 39936, 19456, 9216, 3072, 1024, 1000, 500};


unsigned long getTimePeriod(CONFIG *configs)
{
	return TIME_PERIOD[configs->current_time_scale];
}

void updateNumSamplesFrame(CONFIG *configs)
{
	configs->num_samples_frame = NUM_SAMPLES_FRAME[configs->current_time_scale];
}

unsigned int calculateHoldOffSleepTicks(CONFIG *configs)
{
	return HOLD_OFF_SLEEP_TICKS[configs->current_time_scale] * configs->hold_off_value;
}

unsigned int calculateHoldOffTicks(CONFIG *configs)
{
	return HOLD_OFF_TICKS[configs->current_time_scale];
}

unsigned int getContinuousModeSamplingSpacing(CONFIG *configs)
{
	static unsigned int CONT_SAMPLING_SPACING[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 4, 1, 1, 1, 1, 1, 1, 1};
	return CONT_SAMPLING_SPACING[configs->current_time_scale];
}

unsigned char ADCRead() {
	unsigned long ulADC0Value[4];
	unsigned long sequencer = 3;
	ADCIntClear(ADC0_BASE, sequencer);
	ADCProcessorTrigger(ADC0_BASE, sequencer);
	while(!ADCIntStatus(ADC0_BASE, sequencer, false));
	ADCSequenceDataGet(ADC0_BASE, sequencer, ulADC0Value);
	/*
	unsigned int leitura = (ulADC0Value[0] + ulADC0Value[1] + ulADC0Value[2] + ulADC0Value[3])/4;
	return ((leitura>>4) & 0xFF);
	*/
	return ((ulADC0Value[0]>>4) & 0xFF);
}

void initializeHardware(CONFIG *configs, float halfPeriodOut) {
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
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_1MSPS);

	// ADC - sequencer 1
//	ADCSequenceDisable(ADC0_BASE, 1);
//	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
//	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, /*ADC_CTL_CH0);
//	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH0);
//	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH0);
//	ADCSequenceStepConfigure(ADC0_BASE, 1, 3,*/ ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
//	ADCSequenceEnable(ADC0_BASE, 1);


	// ADC - sequencer 3
	ADCSequenceDisable(ADC0_BASE, 3);
	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 3);

	// Timer 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
	TimerLoadSet(TIMER0_BASE, TIMER_A, getTimePeriod(configs));
	IntEnable(INT_TIMER0A);
	TimerEnable(TIMER0_BASE, TIMER_A);

	// Timer 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_32_BIT_PER);
	TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet() * halfPeriodOut));
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
}

void test()
{
	char blah[50];

	snprintf (blah, 50, "Welcome!"); UARTPrintln(blah);


	snprintf (blah, 50, "%d", SysCtlClockGet()); UARTPrintln(blah);
	unsigned long a, b, c = 10;
	while (c--) {
		a = (unsigned long) TimerValueGet(TIMER1_BASE, TIMER_A);
		TimerEnable(TIMER1_BASE, TIMER_A);
		ADCRead();
		TimerDisable(TIMER1_BASE, TIMER_A);
		b = (unsigned long) TimerValueGet(TIMER1_BASE, TIMER_A);
		snprintf (blah, 50, "%d", a-b); UARTPrintln(blah);
	}

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
	c = 400000;
	while (c--) {
		ADCRead();
	}

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

	while(1);
}
