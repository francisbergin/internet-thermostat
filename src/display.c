//*****************************************************************************
//	Copyright (C) 2015 Francis Bergin
//
//
//	This file is part of Internet Thermostat.
//
//	Internet Thermostat is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	Internet Thermostat is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with Internet Thermostat.  If not, see <http://www.gnu.org/licenses/>.
//
//*****************************************************************************

#include "includes.h"


void update_display ( void )
{
	WORD disp_value1, disp_value2, disp_value3;

	DDRA = DDRA | 0xf0;
	DDRD = DDRD | 0x70;


	if(adc0_valid==0)
	{
	//everything is on
		PORTD |= (1 << 4);
		PORTD |= (1 << 5);
		PORTD |= (1 << 6);
		PORTD |= (1 << 7);
		PORTA = 8*16;
		PORTA |= (1 << 2);
		PORTA |= (1 << 1);
		_delay_ms(1);
	}

	if(rot_enc_change==1)
	{
		disp_value1 = desired_read_temp1();
		disp_value2 = desired_read_temp2();
		disp_value3 = desired_read_temp3();

	//first digit
		PORTD |= (1 << 4);
		PORTD &= ~(1 << 5);
		PORTD &= ~(1 << 6);
		PORTD &= ~(1 << 7);
		PORTA = disp_value1*16;
		PORTA &= ~(1 << 2);
		_delay_ms(1);


	//second digit + decimal point
		PORTD &= ~(1 << 4);
		PORTD |= (1 << 5);
		PORTD &= ~(1 << 6);
		PORTD &= ~(1 << 7);
		PORTA = disp_value2*16;
		PORTA |= (1 << 2);
		_delay_ms(1);

	//third digit
		PORTD &= ~(1 << 4);
		PORTD &= ~(1 << 5);
		PORTD |= (1 << 6);
		PORTD &= ~(1 << 7);
		PORTA = disp_value3*16;
		PORTA &= ~(1 << 2);
		_delay_ms(1);
	}

	else
	{
		disp_value1 = adc_read_temp1();
		disp_value2 = adc_read_temp2();
		disp_value3 = adc_read_temp3();


	//first digit
		PORTD |= (1 << 4);
		PORTD &= ~(1 << 5);
		PORTD &= ~(1 << 6);
		PORTD &= ~(1 << 7);
		PORTA = disp_value1*16;
		PORTA &= ~(1 << 2);
		_delay_ms(1);

	//second digit + decimal point
		PORTD &= ~(1 << 4);
		PORTD |= (1 << 5);
		PORTD &= ~(1 << 6);
		PORTD &= ~(1 << 7);
		PORTA = disp_value2*16;
		PORTA |= (1 << 2);
		_delay_ms(1);

	//third digit
		PORTD &= ~(1 << 4);
		PORTD &= ~(1 << 5);
		PORTD |= (1 << 6);
		PORTD &= ~(1 << 7);
		PORTA = disp_value3*16;
		PORTA &= ~(1 << 2);
		_delay_ms(1);

	//c at the end
		PORTD &= ~(1 << 4);
		PORTD &= ~(1 << 5);
		PORTD &= ~(1 << 6);
		PORTD |= (1 << 7);
		PORTA = 10*16;
		PORTA &= ~(1 << 2);
		_delay_ms(1);

	//dot over second digit
		PORTD &= ~(1 << 4);
		PORTD &= ~(1 << 5);
		PORTD &= ~(1 << 6);
		PORTD &= ~(1 << 7);
		PORTA = 11*16;
		PORTA &= ~(1 << 2);
		PORTA |= (1 << 1);
		_delay_ms(1);

	}
}
