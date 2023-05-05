
#include <avr/io.h>
//#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define dtrconst 25
#define min_coil_switch_time 3250

#define SPD 25

//#define F_CPU 16000000L


// #define FSTP 0x00
// #define HSTP 0x01
// #define LDIR 0x0
// #define RDIR
// #define PTB0
// #define PTC0
// #define PTD0
// #define PTD3




uint16_t steps1 = 1;
uint16_t steps2 = 1;
uint16_t steps3 = 1;


uint16_t stepper(uint16_t steps, uint8_t MDP, uint16_t current_step)
{
	void full_step(uint8_t stp)
	{
		switch(MDP & 0xf0)
		{
			case 0x10:
			switch(stp)
			{
				case 0:		PORTB = 0x01;	break;
				case 1:		PORTB = 0x01;	break;
				case 2:		PORTB = 0x02;	break;
				case 3:		PORTB = 0x02;	break;
				case 4:		PORTB = 0x04;	break;
				case 5:		PORTB = 0x04;	break;
				case 6:		PORTB = 0x08;	break;
				case 7:		PORTB = 0x08;	break;
			}
			break;
			case 0x20:
			switch(stp)
			{
				case 0:		PORTC = 0x01;	break;
				case 1:		PORTC = 0x01;	break;
				case 2:		PORTC = 0x02;	break;
				case 3:		PORTC = 0x02;	break;
				case 4:		PORTC = 0x04;	break;
				case 5:		PORTC = 0x04;	break;
				case 6:		PORTC = 0x08;	break;
				case 7:		PORTC = 0x08;	break;
			}
			break;
			case 0x40:
			switch(stp)
			{
				case 0:		PORTD = 0x08;	break;
				case 1:		PORTD = 0x08;	break;
				case 2:		PORTD = 0x20;	break;
				case 3:		PORTD = 0x20;	break;
				case 4:		PORTD = 0x40;	break;
				case 5:		PORTD = 0x40;	break;
				case 6:		PORTD = 0x80;	break;
				case 7:		PORTD = 0x80; 	break;
			}
			break;
		}
	}
	void half_step(uint8_t stp)
	{
		switch(MDP & 0xf0)
		{
			case 0x10:
			switch(stp)
			{
				case 0:		PORTB = 0x01;	break;
				case 1:		PORTB = 0x03;	break;
				case 2:		PORTB = 0x02;	break;
				case 3:		PORTB = 0x06;	break;
				case 4:		PORTB = 0x04;	break;
				case 5:		PORTB = 0x0c;	break;
				case 6:		PORTB = 0x08;	break;
				case 7:		PORTB = 0x09;	break;
			}
			break;
			case 0x20:
			switch(stp)
			{
				case 0:		PORTC = 0x01;	break;
				case 1:		PORTC = 0x03;	break;
				case 2:		PORTC = 0x02;	break;
				case 3:		PORTC = 0x06;	break;
				case 4:		PORTC = 0x04;	break;
				case 5:		PORTC = 0x0c;	break;
				case 6:		PORTC = 0x08;	break;
				case 7:		PORTC = 0x09;	break;
			}
			break;
			case 0x40:
			switch(stp)
			{
				case 0:		PORTD = 0x08;	break;
				case 1:		PORTD = 0x28;	break;
				case 2:		PORTD = 0x20;	break;
				case 3:		PORTD = 0x60;	break;
				case 4:		PORTD = 0x40;	break;
				case 5:		PORTD = 0xc0;	break;
				case 6:		PORTD = 0x80;	break;
				case 7:		PORTD = 0x88; 	break;
			}
			break;
		}
	}
	
	if ((MDP & 0x03) == 0x00)
	{
		full_step(current_step % 8);
		current_step++;
		if (current_step > steps*2) return 0;
		return current_step;
	}
	else if ((MDP & 0x03) == 0x01)
	{
		full_step((steps - current_step) % 8);
		current_step++;
		if (current_step > steps*2) return 0;
		return current_step;
	}
	else if ((MDP & 0x0c) == 0x04)
	{
		half_step(current_step % 8);
		current_step++;
		if (current_step > steps*2) return 0;
		return current_step;
	}
	else if ((MDP & 0x0c) == 0x08)
	{
		half_step((steps - current_step) % 8);
		current_step++;
		if (current_step > steps*2) return 0;
		return current_step;
	}
	else return 0;
}

void USART_Transmit(uint8_t data)
{
	
	// Wait for empty transmit buffer 
	while (!( UCSR0A & (1<<UDRE0)))
	;
	// Put data into buffer, sends the data 
	UDR0 = data;
}

void setup(void)
{
	DDRD = 0xff;
	DDRC = 0x0f;
	DDRB = 0x0f;
	
	//Transmitter on
	UCSR0B |= (1<<TXEN0);
	//Sync on, 1bit stop, 8bit frame
	UCSR0C |= (1<<UMSEL00) | (1<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00);
	//Baudrate
	UBRR0H = 0x00;
	UBRR0H |= (SPD>>8) & 0x0f;
	UBRR0L |= SPD & 0xff;

	//Timer1 Compare setup with prescaler 1/8
	TCCR1B |= (1<<CS11) | (1<<WGM12); //| (1<<CS10) | (1<<WGM13);
	//Compare Match Interrupt Enable (OCIE1A)
	TIMSK1 |= (1<<OCIE1A);
	//Compare Value
	OCR1A = min_coil_switch_time; // wartoœæ z dOOpska (powinno byæ 2000 dla 1ms[ale nie jest])
	
	sei();
}

int main(void)
{
	//Peripherals Setup
	setup();

	while(1)
	{
		
	}
	
}

ISR(TIMER1_COMPA_vect)
{
	if (steps1 != 0) steps1 = stepper(2048, 0x18, steps1); 
	
	if (steps2 != 0) steps2 = stepper(2048, 0x20, steps2); 
	
	if (steps3 != 0) steps3 = stepper(2048, 0x44, steps3); 
}

/*for (int i=0; i<10; i++) {asm("nop");}*/
