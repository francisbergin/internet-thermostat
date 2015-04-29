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

WORD cnt0end=1500;
WORD relays_cnt0=1499;

float temp_difference;

// overall hvac system on or off
BYTE hvac_flag_sys=1;
BYTE hvac_flag_auto=1;
BYTE hvac_flag_heat_src=1;

// hvac components on or off
BYTE hvac_heat_flag=1, hvac_ac_flag=1, hvac_vent_flag=1;

BYTE hvac_heat=0, hvac_ac=0, hvac_vent=0;


void control_relays (void)
{
	if (adc0_valid==0)
	{
		//PORTC |= 0xc3;
		return;
	}

	if (hvac_flag_sys==0)
	{
		PORTC &= ~(1 << 0);
		PORTC &= ~(1 << 1);
		PORTC &= ~(1 << 6);
		PORTC &= ~(1 << 7);
		relays_cnt0=0;
		return;
	}

	if (rot_enc_change==1)
		return;


	temp_difference=adc0_temp-desired_temp;

	if (temp_difference<=-1)
	{
		relays_cnt0++;
		if (relays_cnt0==cnt0end)
		{
			relays_cnt0=0;
			hvac_heat=1;
			hvac_ac=0;
			hvac_vent=0;
		}
	}

	else if (temp_difference>=5)
	{
		relays_cnt0++;
		if (relays_cnt0==cnt0end)
		{
			relays_cnt0=0;
			hvac_heat=0;
			hvac_ac=1;
			hvac_vent=0;
		}
	}
	else
	{
		hvac_heat=0;
		hvac_ac=0;
		hvac_vent=0;
	}


	if (((hvac_heat_flag==1)&&(hvac_heat==1))||((hvac_heat_flag==1)&&(hvac_flag_auto==0)))
	{
		if (hvac_flag_heat_src==1)
		{
			PORTC &= ~(1 << 0);
			PORTC |= 1 << 1;
			PORTC &= ~(1 << 6);
			PORTC |= 1 << 7;
			return;
		}

		else if (hvac_flag_heat_src==2)
		{
			PORTC |= 1 << 0;
			PORTC &= ~(1 << 1);
			PORTC &= ~(1 << 6);
			PORTC &= ~(1 << 7);
			return;
		}
	}

	if (((hvac_ac_flag==1)&&(hvac_ac==1))||((hvac_ac_flag==1)&&(hvac_flag_auto==0)))
	{
		PORTC &= ~(1 << 0);
		PORTC &= ~(1 << 1);
		PORTC |= 1 << 6;
		PORTC |= 1 << 7;
		return;
	}

	if (((hvac_vent_flag==1)&&(hvac_vent==1))||((hvac_vent_flag==1)&&(hvac_flag_auto==0)))
	{
		PORTC &= ~(1 << 0);
		PORTC &= ~(1 << 1);
		PORTC &= ~(1 << 6);
		PORTC |= 1 << 7;
		return;
	}

	PORTC &= ~(1 << 0);
	PORTC &= ~(1 << 1);
	PORTC &= ~(1 << 6);
	PORTC &= ~(1 << 7);

}
