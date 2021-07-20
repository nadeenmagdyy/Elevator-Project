/*
 * trial_final_code.c
 *
 * Created: 7/16/2021 8:54:12 PM
 * Author : HERO
 */ 


#define F_CPU 1000000UL
#include <stdio.h>
#include <avr/io.h>
#define _DELAY_BACKWARD_COMPATIBLE_
#include <util/delay.h>
#include <avr/io.h>
#define MOSI 5
#define SCK 7
#define SS 4
/*****************************************************************************/
#define GET_BIT(Var, PIN) ((Var>>PIN)&1)
#define Time 500
/*****************************************************************************/

unsigned char ref;
unsigned char ps=1;
unsigned char ef=0;

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define C0 4
#define C1 5
#define C2 6
#define C3 7

#define keypadPORT PORTD
#define keypadPIN PIND
#define keypadDDR DDRD

char keypad(void)
{
	unsigned char row,col;
	char key_num = -1;
	unsigned char i = 0;
	while (i<3)
	{
		for (col=0;col<3;col++)
		{
			PORTD = (~(0b00010000<<col));

			for (row=0;row<4;row++)
			{
				if (!(PIND & (1<<row)))
				{
					key_num = row*3+col+1;

					if (row==3 && col ==1)
					{
						key_num = 0;
					}
					return key_num;
				}
			}
		}
		i++;
	}
	return key_num;
}




void execute(unsigned char cmd, unsigned char data)
{
	PORTB &= ~(1<<SS);
	SPDR = cmd;
	while (!(SPSR & (1<<SPIF)));

	SPDR = data;
	while (!(SPSR & (1<<SPIF)));

	PORTB |=(1<<SS);
}

void door_open(void)
{
	if (ef == 0)
	{
		PORTC=(1<<PC3); //move motor anti clockwise to indicate open
		_delay_ms(Time);
		PORTC &=~(1<<PC3);
	}
	if (ef == 1)
	{
		PORTC=(1<<PC5);
		_delay_ms(Time);
		PORTC &=~(1<<PC5);
	}
	if (ef == 2)
	{
		PORTC=(1<<PC7);
		_delay_ms(Time);
		PORTC &=~(1<<PC7);
	}
	if (ef == 3)
	{
		PORTB|=(1<<PB1);
		_delay_ms(Time);
		PORTB &=~(1<<PB1);
	}
}

void close_open (unsigned char ps)
{
	if (ef == 0 && ps == 1)
	{
		PORTC=(1<<PC2); //move motor anti clockwise to indicate closed
		_delay_ms(Time);
		PORTC &=~(1<<PC2);
	}
	if (ef == 1 && ps == 1)
	{
		PORTC=(1<<PC4);
		_delay_ms(Time);
		PORTC &=~(1<<PC4);
	}
	if (ef == 2 && ps == 1)
	{
		PORTC=(1<<PC6);
		_delay_ms(Time);
		PORTC &=~(1<<PC6);
	}
	if (ef == 3 && ps == 1)
	{
		PORTB|=(1<<PB0);
		_delay_ms(Time);
		PORTB &=~(1<<PB0);
	}
}

void after_call(unsigned char x)
{

	//x=call;
	//y=ef;

	DDRB |= ((1<<MOSI) |(1<<SCK) |(1<<SS));
	SPCR= (1<<SPE) | (1<<MSTR) | (1<<SPR0);
	execute(0x09,0b00000001);
	execute(0x0B, 0x01);
	execute(0x0C, 0x01);
	ref = ef;
	//unsigned char rq_f=1;
	if(x>ef)  //clockwise
	{
		for(int q=0 ; q<(x-ef) ; q++)
		{
			PORTC=0x02;
			execute(0x01, ref);
			_delay_ms(Time);
			ref++;
		}
		PORTC = 0x03; //00000011
		execute(0x01, ref);
		ef=x;
		door_open();
		_delay_ms(Time);
		close_open(1);
	}
	if(x<ef)  //anti-clockwise
	{
		for(int q=0 ; q<(ef-x) ; q++)
		{
			PORTC=0x01;
			execute(0x01, ref);
			_delay_ms(Time);
			ref--;
		}
		PORTC= 0x03; //00000011
		execute(0x01, ref);
		ef=x;
		door_open();
		_delay_ms(Time);
		close_open(1);
	}
}
int main(void)
{
	char Temp = -1;
	char call = -1;
	DDRA=0x00;
	DDRB=(1<<PB0)|(1<<PB1);
	DDRC=0xFF;
	DDRD = 0b01110000;
	PORTA=0x00;
	PORTB=0x00;
	PORTC=0;
	PORTD = 0b00000000;
	while (1)
	{
		if((GET_BIT(PINA, 0)==1))
		{
			call=1;
			after_call(call);
		}
		if((GET_BIT(PINA, 1)==1))
		{
			call=2;
			after_call(call);
		}
		if((GET_BIT(PINA, 2)==1))
		{
			call=3;
			after_call(call);
		}
		if((GET_BIT(PINA, 4)==1))
		{
			call=0;
			after_call(call);
		}
		Temp = keypad();
		if (Temp<4 && Temp != -1)
		{
			call = Temp;
			after_call(call);
		}
		after_call(ef);
	}
}

