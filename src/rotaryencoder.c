// Copyright (C) 2015 Francis Bergin

#include "includes.h"

//global variables
BYTE rot_enc_flag;
BYTE rot_enc_change=0;

float desired_temp=20;

ISR (INT2_vect)		//int2 interrupt subroutine
{
	if (rot_enc_flag==0)
	{
		TCNT0 = 0;
		TCNT1 = 0;
		rot_enc_flag = 1;
		rot_enc_change = 1;
	}
}


void check_rotary (void)
{
	int input;

	if (rot_enc_flag==1)
	{
		if (TCNT0 > 16)
		{
			input = PINB;

			if ((input|0xFB)==0xFB)
			{
				if ((input|0xF7)==0xF7)
				{
					//PORTB ^= 0x01;
					desired_temp -= 0.1;
				}

				else if ((input|0xF7)==0xFF)
				{
					//PORTB ^= 0x02;
					desired_temp += 0.1;
				}

				rot_enc_flag = 0;
			}
		}
	}

	if(TCNT1 > 45000)
	{
		rot_enc_change=0;
		TCNT1 = 0;
	}
}


WORD desired_read_temp1 ( void )
{
	WORD temp_value1;
	temp_value1 = desired_temp/10;
	return temp_value1;
}

WORD desired_read_temp2 ( void )
{
	WORD temp_value2;
	WORD temp0, temp1;

	temp0 = desired_temp/10;
	temp1 = temp0 * 10;
	temp_value2 = desired_temp - temp1;

	return temp_value2;
}

WORD desired_read_temp3 ( void )
{
	WORD temp_value3;
	WORD temp0, temp1;

	temp0 = desired_temp;
	temp1 = temp0 * 10;
	temp_value3 = (desired_temp*10) - temp1;

	return temp_value3;
}
