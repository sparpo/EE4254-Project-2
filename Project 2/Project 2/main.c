/*
 * Project 2.c
 *
 * Created: 28/03/2020 14:11:00
 * Author : Aaron Naylor, Joseph Corbett
 */ 


/* #Includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#define Brightness_Multiplier 25.5 // 255/10 = 25.5
#define mV_multiplier 4.88 // 0.00488 * 1000
#define temp_divider 2.0 //(5v/1023)=4.887mV = 5mV, every deg c is 10Mv voltage change therefore divide by 2
#define Light_Threshold 512 // threshold that if over LDR is bright

/* Initializing Voids */
void sendmsg (char *s);
void init_USART(void); 
void init_adc(void); 
void init_ports(void); 
void init_timer0(void); 
void init_timer1(void); 
void init_timer2(void);


/*message arrays*/
char msg1[] = {"Unrecognized command"}; // message  for unrecognized character 
char msg2[] = {"ADC set for temperature sensor"};  // message when setting temperature sensor
char msg3[] = {"ADC set for LDR"}; // message when setting LDR
char msg4[] = {"ADC set for potentiometer"}; // message when setting potentiometer
char msg5[] = {"Must set ADC for temperature by typing 'M' or 'm'"}; // warning for when incorrectly asking for temperature
char msg6[] = {"Must set ADC for LDR by typing 'N' or 'n'"}; // warning for when incorrectly asking for LDR result
char msg7[] = {"LDR = Bright"}; // LDR result 
char msg8[] = {"LDR = Dark"}; // LDR result 
char msg9[] = {"Continuous display already disabled"};

/* Initialized Variables */		
unsigned char qcntr = 0,sndcntr = 0;   /*indexes into the queue*/
unsigned char queue[100];       /*character queue*/
unsigned int adc_reading; // adc value saved here
volatile unsigned int new_adc_data; // flag to show new data

float adc_mV; // adc value in mV saved here
float temp; // temperature in Centigrade saved here
float OC; // OCR2A value saved here

enum adc{Volt,LDR,Temp} input;

unsigned int enContDisplay = 0; //enable continuous display


/* Main */
int main(void)
{
	/* Initialized Local Variables */		

	char ch;  /* character variable for received character*/
	char data[30]; // used to set of string in sprintf
	char str_temp[7]; // string written to user for temperature in /centigrade
	char str_adc_mV[9]; // string written to user for adc in mV

	int Brightness; // variable that user will enter to set brightness of LED
	
	/* Calling Initialized Registers */
	init_ports(); // initializes ports
	init_USART(); // initializes USART
	init_adc(); // initializes adc
	init_timer0(); // initializes timer0
	//init_timer1(); // initializes timer1
	init_timer2(); // initializes timer2

	sei(); /*global interrupt enable */

	while (1)
	{	
		if (UCSR0A & (1<<RXC0)) /*check for character received*/
		{
			ch = UDR0;    /*get character sent from PC*/
			switch (ch) { //character input
				
				/* Sets ADC to ADC2 */
				case 'M':
				case 'm':
					input = Temp;
					sendmsg(msg2);
				break;
				
				/* Sets ADC to ADC1 */
				case 'N':
				case 'n':
					input = LDR;
					sendmsg(msg3);
				break;
				
				/* Sets ADC to ADC0 */
				case 'P':
				case 'p':
					input = Volt;
					sendmsg(msg4);
				break;
				
				/* Report Temperature to user */
				case 'T':
				case 't':
					if (input == Temp) {
						temp = adc_reading/temp_divider; //(5v/1023)=4.887mV = 5mV, every deg c is 10Mv voltage change therefore divide by 2
						dtostrf(temp,6,2,str_temp); // Changes value from double to string
						sprintf(data,"LM35 Temperature = %s deg C",str_temp); //Report Temperature value
						sendmsg(data);
					} else {
						//Give warning
						sendmsg(msg5);
					}
				break;
				
				/* Report Bright/Dark to user */
				case 'L':
				case 'l':
					if (input == LDR) {
							if(adc_reading>Light_Threshold)
							{
								sendmsg(msg7); // Report Bright
							}
							else
							{
								sendmsg(msg8); // Report Dark
							}
					} else {
						//Give warning
						sendmsg(msg6);
					}
				break;
				
				/* Report ADC Value to user */
				case 'A':
				case 'a':
					sprintf(data, "ADC value = %d", adc_reading); //Report ADC value
					sendmsg(data);
				break;
				
				/* Report ADC Value in mV to user */
				case 'V':
				case 'v':
					adc_mV = (adc_reading*mV_multiplier); // Calculates ADC in mV
					dtostrf(adc_mV,8,2,str_adc_mV);  // Changes value from double to string
					sprintf(data, "ADC value = %s mV",str_adc_mV); //Report ADC value in mV
					sendmsg(data);	
				break;
				
				/* Report Values Continuously */
				case 'C':
				case 'c':
					enContDisplay = 1; //enable continuous adc display
				break;
				
				/* Stop Reporting Values Continuously */
				case 'E':
				case 'e':
					if(enContDisplay==0) {
						sendmsg(msg9);
					}
					enContDisplay = 0; //disable continuous adc display
				break;
				
				/* Report OCR2A Value */
				case 'S':
				case 's':
					sprintf(data, "OCR2A = %d", OCR2A); // Report OCR2A value
					sendmsg(data);
				break;
				
				/* Sets LED Brightness Level */
				case '0' ... '9':
					
					Brightness = ch - '0';
					OCR2A = Brightness * Brightness_Multiplier;
				break;
				
				default:
				sendmsg(msg1); /*send default message*/
			}
		}
	/*Continuous Loop */   
	
		if(new_adc_data) {
			if(enContDisplay) {
				switch(input){
					case Volt:
						adc_mV = (adc_reading*mV_multiplier);
						dtostrf(adc_mV,8,2,str_adc_mV);
						sprintf(data, "ADC value = %s mV",str_adc_mV); //Report ADC value in mV
						sendmsg(data);
					break;
					
					case LDR:
						if(adc_reading>Light_Threshold)
						{
							sendmsg(msg7);
						}
						else
						{
							sendmsg(msg8);
						}
					break;
					
					case Temp:
						temp = adc_reading/temp_divider; //(5v/1023)=4.887mV = 5mV, every deg c is 10Mv voltage change therefore divide by 2
						dtostrf(temp,6,2,str_temp); // Changes value from double to string
						sprintf(data,"LM35 Temperature = %s deg C",str_temp); //Report Temperature value
						sendmsg(data);
					break;
					
					default:
						temp = adc_reading/temp_divider; //(5v/1023)=4.887mV = 5mV, every deg c is 10Mv voltage change therefore divide by 2
						dtostrf(temp,6,2,str_temp); // Changes value from double to string
						sprintf(data,"LM35 Temperature = %s deg C",str_temp); //Report Temperature value
						sendmsg(data);
				}
			}
			new_adc_data=0;
		}
	}
	return 1;
	
}

/* Initializing ADc registers */
void init_adc() {
	ADMUX = (1<<6)|(1<<1); //sets voltage ref to Vcc and starts ADC2
	ADCSRA  = (1<<7)|(1<<6)|(1<<5)|(1<<3)|(7<<0); //enable adc, starts conversion, enable interrupt, sets prescalar 128
	ADCSRB = (1<<2);// sets timer0 overflow 
}

/* Initializing Ports */
void init_ports() {
	
	DDRB = (1<<3);		// Initialize turn bit 3 to output
	PORTB = 0;		// Initialize to all off	
}

/* Initializing USART registers */
void init_USART() {
	UCSR0B	= (1<<RXEN0) | (1<<TXEN0) | (1<<TXCIE0) | (0<<UCSZ02);  //enable receiver, transmitter, TX Complete and transmit interrupt and setting data to 8 bits
	UBRR0 = 10;  //baud rate = 90909
}

/* Initializing Timer0 registers */
void init_timer0() {
	
	TCCR0A = 0;
	TIMSK0 = 0;
	TCCR0B = (5<<0); // prescalar 1024
	TCNT0 = 6; // TCNT0 set to 6 so that will cause timer overflow after 16 ms

}

/* Initializing Timer1 registers */
void init_timer1() {
	
	TCCR1A = 0;
	TCCR1B = (1<<1); // prescalar 8 
	TIMSK1 = (1<<5) | (1<<0); //Input Capture set for falling edge with noise control turned OFF , Input Capture and Timer1 Overflow Interrupts enable

}

/* Initializing Timer2 registers */
void init_timer2() {
	
	TCCR2A = (1<<7)|(1<<0); // Clear OC2A on Compare Match when Upcounting , Phase Correct PWM Mode
	TCCR2B = (6<<0); // Phase Correct PWM Mode, prescalar 256
	OCR2A = 0; // turn off led
	
}

/* sendmsg function*/
void sendmsg (char *s)
{
	qcntr = 0;    /*preset indices*/
	sndcntr = 1;  /*set to one because first character already sent*/
	
	queue[qcntr++] = 0x0d;   /*put CRLF into the queue first*/
	queue[qcntr++] = 0x0a;
	while (*s)
	queue[qcntr++] = *s++;   /*put characters into queue*/
	
	UDR0 = queue[0];  /*send first character to start process*/
}

/********************************************************************************/
/* Interrupt Service Routines													*/
/********************************************************************************/

/*this interrupt occurs whenever the */
/*USART has completed sending a character*/

ISR(USART_TX_vect)
{
	/*send next character and increment index*/
	if (qcntr != sndcntr)
		UDR0 = queue[sndcntr++];
}

ISR (ADC_vect)//handles ADC interrupts
{
	
	//adc_reading = ADC;
	new_adc_data = 1;
	adc_reading = ADC;
	switch(input) {
		
		case Volt :
			ADMUX = (1<<6) | (0<<0); //adc0
		break;
		
		case LDR :
			ADMUX = (1<<6) | (1<<0); //adc1
		break;
		
		case Temp :
			ADMUX = (1<<6) | (1<<1) ; //adc2
		break;
		default:
			ADMUX = (1<<6) | (1<<1); //adc2
	} 
	TIFR0 = (1<<0); //clears Counter0 overflow
}
