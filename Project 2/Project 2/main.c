/*
 * Project 2.c
 *
 * Created: 28/03/2020 14:11:00
 * Author : Aaron Naylor, Joseph Corbett
 */ 

#include <avr/io.h>

void init_adc() {

}

void init_ports() {

}

void init_usart(){
	UCSR0A	= 0x00;
	UCSR0B	= (1<<RXEN0) | (1<<TXEN0) | (1<<TXC0);  /*enable receiver, transmitter and transmit interrupt*/
	UBRR0	= 103;  /*baud rate = 9600*/
}

void init_timer0() {

}

void init_timer1() {
	
}

void init_timer2() {
	
}

int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
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


