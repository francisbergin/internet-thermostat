//*****************************************************************************
//	Copyright (C) 2012 Francis Bergin
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

DWORD adc0_average;
DWORD adc0_sum;
WORD adc0_check;		// Does not need to be global
WORD x;

WORD temp_value1;
WORD temp_value2;
WORD temp_value3;

BYTE adc0_valid=0;		// to wait until first conversion in done

float adc0_temp;

float adc0_calc_slope = (200.0/921.6);
float adc0_calc_intercept = (550.0/9.0);

float adc0_heat_compensation=15.0;
float adc0_heat_comp_custom=5.0;

BYTE adc0_pwr_src=1;

void adc0_init ( void )
{
	ADCSRA = 0x80;
	ADMUX = 0x40;
}

WORD adc0_read (void)
{
	return adc0_average;
}


void adc0_data (void)
{

	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait until conversion complete
	while ( !(ADCSRA & (1<<ADIF)))
		;

	// CAUTION: READ ADCL BEFORE ADCH!!!
	adc0_check = ((ADCL) | ((ADCH)<<8));
	adc0_sum += ((ADCL) | ((ADCH)<<8));

	if (adc0_valid==0)
	{
		if(x==511)
		{
			adc0_valid=1;

			adc0_average = adc0_sum >> 9;
			adc0_temp = (adc0_average*adc0_calc_slope)-adc0_calc_intercept;	//equation according to AD22100 characteristics
			adc0_temp -= adc0_heat_compensation;					//To compensate for board heat
			adc0_sum = 0;
			x=0;
		}
		else
			x++;
	}

	if (adc0_valid==1)
	{
		if(x==4095)
		{
			adc0_average = adc0_sum >> 12;
			adc0_temp = (adc0_average*0.2172)-61.07;	//equation according to AD22100 characteristics
			adc0_temp -= adc0_heat_compensation;							//To compensate for board heat
			adc0_sum = 0;
			x=0;
		}
		else
			x++;
	}
}

WORD adc_read_temp1 ( void )
{
	temp_value1 = adc0_temp/10;
	return temp_value1;
}

WORD adc_read_temp2 ( void )
{
	WORD temp0, temp1;

	temp0 = adc0_temp/10;
	temp1 = temp0 * 10;
	temp_value2 = adc0_temp - temp1;

	return temp_value2;
}

WORD adc_read_temp3 ( void )
{
	WORD temp0, temp1;

	temp0 = adc0_temp;
	temp1 = temp0 * 10;
	temp_value3 = (adc0_temp*10) - temp1;

	return temp_value3;
}


WORD adc_read_compensation1 ( void )
{
	temp_value1 = adc0_heat_compensation/10;
	return temp_value1;
}

WORD adc_read_compensation2 ( void )
{
	WORD temp0, temp1;

	temp0 = adc0_heat_compensation/10;
	temp1 = temp0 * 10;
	temp_value2 = adc0_heat_compensation - temp1;

	return temp_value2;
}

WORD adc_read_compensation3 ( void )
{
	WORD temp0, temp1;

	temp0 = adc0_heat_compensation;
	temp1 = temp0 * 10;
	temp_value3 = (adc0_heat_compensation*10) - temp1;

	return temp_value3;
}
