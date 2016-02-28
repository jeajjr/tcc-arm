#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "math.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"

#define PI 3.141592

#ifdef DEBUG
void__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

int LED = 0;

void UART0IntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ulStatus); //clear the asserted interrupts
	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		char a = UARTCharGetNonBlocking(UART0_BASE);
		UARTCharPutNonBlocking(UART1_BASE, a);
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
			UARTCharPutNonBlocking(UART0_BASE, a);

			LED++;
			if (LED%2) //if (val == 0)
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 1<<3);
			else //if (val == 255)
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
		}
}

int main(void)
{
	unsigned long ulADC0Value[4];
	// clock
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

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

	// interruptions
	IntMasterEnable();
	IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
/*
	IntEnable(INT_UART1);
	UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
*/


	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 1<<3);
/*
	UARTCharPut(UART0_BASE, 'E');
	UARTCharPut(UART0_BASE, 'n');
	UARTCharPut(UART0_BASE, 't');
	UARTCharPut(UART0_BASE, 'e');
	UARTCharPut(UART0_BASE, 'r');
	UARTCharPut(UART0_BASE, ' ');
	UARTCharPut(UART0_BASE, 'T');
	UARTCharPut(UART0_BASE, 'e');
	UARTCharPut(UART0_BASE, 'x');
	UARTCharPut(UART0_BASE, 't');
	UARTCharPut(UART0_BASE, ':');
	UARTCharPut(UART0_BASE, ' ');
*/
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 1<<2);

	char val = 0;
	int sampleCount = 0;
	int limLED = 10000;

	while(1)
	{
//#ifdef NOIA
		ADCIntClear(ADC0_BASE, 1);
		ADCProcessorTrigger(ADC0_BASE, 1);
		while(!ADCIntStatus(ADC0_BASE, 1, false));
		ADCSequenceDataGet(ADC0_BASE, 1, ulADC0Value);
		unsigned int leitura = (ulADC0Value[0] + ulADC0Value[1] + ulADC0Value[2] + ulADC0Value[3])/4;
/*
		ADCSequenceDataGet(ADC0_BASE, 3, ulADC0Value);
		unsigned int leitura = ulADC0Value[0];
		/*
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 1<<1);
		UARTCharPut(UART0_BASE, (leitura>>4) & 0xFF);
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		*/
		//UARTCharPut(UART0_BASE, leitura & 0xFF);

		//SysCtlDelay(200000);
//		if (sampleCount++ == 10)
		{
			UARTCharPut(UART0_BASE, (leitura>>4) & 0xFF);
			sampleCount = 0;
		}
//		UARTCharPut(UART1_BASE, (leitura>>4) & 0xFF);

		if (LED < limLED/2)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 1<<3);
		else //if (val == 255)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

		if (LED++ == limLED) LED = 0;

//#endif
		/*
		 val = (char) ((sin(LED * 2.0 * PI / 256.0) + 1.0) * 128.0);

		UARTCharPut(UART0_BASE, val);

		LED++;
		if (LED%2) //if (val == 0)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 1<<3);
		else //if (val == 255)
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
			*/
	}
	/*
	while(1)
	{

		// Turn on the LED
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, LED);
		// Delay for a bit
		SysCtlDelay(2000000);
		// Cycle through Red, Green and Blue LEDs
		if (LED == 8) {LED = 2;} else {LED = LED*2;}

		int i = 0;

		for (i=0; i < 2000000/100; i++) {
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 1<<3);
			SysCtlDelay(2000000*5/100);
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
			SysCtlDelay(2000000*95/100);
		}

		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
		SysCtlDelay(2000000);
	}
	 */
}
