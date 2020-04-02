/*
 * Project 2.c
 *
 * Created: 28/03/2020 14:11:00
 * Author : Aaron Naylor, Joseph Corbett
 */ 

void sendmsg (char *s);
void init_USART(void);
void init_adc(void);
void init_ports(void);
void init_timer0(void);
void init_timer1(void);
void init_timer2(void);

#include <avr/io.h>
#include <avr/interrupt.h>


/*message arrays*/
char msg1[] = {"Unrecognized command"};
char msg2[] = {"ADC set to read output of temperature sensor"};
char msg3[] = {"ADC set to read output of LDR"};
char msg4[] = {"ADC set to read output of potentiometer"};
	
unsigned char qcntr = 0,sndcntr = 0;   /*indexes into the queue*/
unsigned char queue[50];       /*character queue*/
unsigned int adc_reading; // adc value saved here
volatile unsigned int new_adc_data; // flag to show new data

	
enum adc{Volt,Bright,Temp} input;
	
unsigned int enContdisplay = 0; //enable continuous display

int main(void)
{
	char ch;  /* character variable for received character*/
	init_ports();
	init_USART();
	init_adc();
	init_timer0();
	init_timer1();
	init_timer2();


	sei(); /*global interrupt enable */

	while (1)
	{	
		if (UCSR0A & (1<<RXC0)) /*check for character received*/
		{
			ch = UDR0;    /*get character sent from PC*/
			switch (ch) { //character input
				//case 'M':
				case ('M'||'m'):
				input = Temp;
				sendmsg(msg2);
				break;
				
				//case 'N':
				case ('N'||'n'):
				input = Bright;
				sendmsg(msg3);
				break;
				
				//case 'P':
				case ('P'||'p'):
				input = Volt;
				sendmsg(msg4);
				break;
				
				//case 'T':
				case ('T'||'t'):
				if (input == Temp) {
					//Report temp in degrees
					sprintf();
				} else {
					//Give warning
					sendmsg(msg5);
				}
				break;
				
				//case 'L':
				case('L'||'l')'l':
				if (input == Bright) {
					//Report brightness in degrees
					sprintf();
					} else {
					//Give warning
					sendmsg(msg6);
				}
				break;
				
				//case 'A':
				case ('A'||'a'):
				//Report ADC value
				sprintf();
				break;
				
				//case 'V':
				case ('V'||'v'):
				//Report ADC value in mV
				sprintf();
				break;
				
				//case 'C':
				case ('C'||'c'):
					enContdisplay = 1; //enable continuous adc display
				break;
				
				//case 'E':
				case ('E'||'e'):
					enContdisplay = 0; //disable continuous adc display
				break;
				
				//case 'S':
				case ('S'||'s'):
				//report current value of OCR2A register
				sprintf();
				break;
				
				default:
				sendmsg(msg1); /*send default message*/
			}
		}
		
		if(new_adc_data == 1) {
			if(enContdisplay) {
				//send new adc data to usart
			}
			new_adc_data=0;
		}
	}
	return 1;
}
void init_adc() {
	ADMUX = (1<<6)|(1<<1); //sets voltage ref to Vcc and starts ADC2
	ADCSRA  = (1<<7)|(1<<6)|(1<<5)|(1<<3)|(7<<0); //enable adc, starts conversion, enable interrupt, sets prescalar 128
	ADCSRB = (1<<2);// sets timer0 overflow 

}

void init_ports() {
	
	DDRB = (1<<3);		// Initialize turn bit 3 to output
	PORTB = 0;		// Initialize to all off	
}

void init_USART() {
	
	UCSR0A	= (1<<RXC0) | (1<<TXC0); // enable RX and TX
	UCSR0B	= (1<<RXEN0) | (1<<TXEN0) | (1<<TXC0) | (1<<TXCIE0) | (0<<UCSZ02);  //enable receiver, transmitter, TX Complete and transmit interrupt and setting data to 8 bits
	UBRR0	= 103;  /*baud rate = 9600*/
	UCSR0C = (0b00000110); //setting data to 8 bits
	
}

void init_timer0() {
	
	TCCR0A = 0;
	TIMSK0 = 0;
	TCCR0B = (5<<0); // prescalar 1024
	TCNT0 = 6; // TCNT0 set to 6 so that will cause timer overflow after 16 ms

}

void init_timer1() {
	
	TCCR1A = 0;
	TCCR1B = (1<<1); // prescalar 8 
	TIMSK1 = (1<<5) | (1<<0); //Input Capture set for falling edge with noise control turned OFF , Input Capture and Timer1 Overflow Interrupts enable

}

void init_timer2() {
	
	TCCR0A = (1<<7)|(1<<0); // Clear OC2A on Compare Match when Upcounting , Phase Correct PWM Mode
	TCCR0B = (6<<0); // Phase Correct PWM Mode, prescalar 256
	OCR2A = 0; // turn off led
	
}

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
	
	adc_reading = ADC;
	new_adc_data = 1;
	switch(input) {
		
		case Volt :
			ADMUX = (1<<7); //adc0
		break;
		
		case Bright :
			ADMUX = (1<<7) | (1<<0); //adc1
		break;
		
		case Temp :
			ADMUX = (1<<7) | (1<<1); //adc2
		break;
		default:
			ADMUX = (1<<7) | (1<<1); //adc2
		break;
	}
	TIFR0 = TIFR0 & ~(1<<0); //clears Counter0 overflow
}