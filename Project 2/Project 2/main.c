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

unsigned char qcntr = 0,sndcntr = 0;   /*indexes into the que*/
unsigned char queue[50];       /*character queue*/
unsigned int adc_reading; // adc value saved here

/*message arrays*/
char msg1[] = {"That was an a or an A."};
char msg2[] = {"That was a b, not an a."};
char msg3[] = {"That was neither b nor a."};
	
enum adc{ADC0,ADC1,ADC2} input;

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
			switch (ch)
			{
				case 'A':
				case 'a':
				sendmsg(msg1); /*send first message*/
				break;
				case 'b':
				sendmsg(msg2); /*send second message*/
				break;
				default:
				sendmsg(msg3); /*send default message*/
			}
		}
	}
	return 1;
}
void init_adc() {
	ADMUX = 0b01000010; //sets voltage ref
	ADCSRA  = 0b11101111; //enable adc, starts conversion, enable interrupt, sets prescalar 128
	ADCSRB = (1<<2);// sets free running mode

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
	TIMSK0 = 0;	TCCR0B = (0b00000101); // prescalar 1024	TCNT0 = 6; // TCNT0 set to 6 so that will cause timer overflow after 16 ms
}

void init_timer1() {
	
	TCCR1A = 0;
	TCCR1B = (0b00000010); // prescalar 8 	TIMSK1 = (0b00100001); //Input Capture set for falling edge with noise control turned OFF , Input Capture and Timer1 Overflow Interrupts enable
}

void init_timer2() {
	
	TCCR0A = (0b10000001); // Clear OC2A on Compare Match when Upcounting , Phase Correct PWM Mode	TCCR0B = (0b00000110); // Phase Correct PWM Mode, prescalr 256
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
}