//********************************************************************************************
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
//********************************************************************************************

#include "includes.h"

// Global variables
MAC_ADDR avr_mac;
IP_ADDR avr_ip;

MAC_ADDR server_mac;
IP_ADDR server_ip;

// set AVR ip address - stored in eeprom
BYTE ee_avr_ip[4] EEMEM = { 192, 168, 2, 20 };


//*****************************************************************************************
//
// Function : server_process
// Description : Run web server and listen on port 80
//
//*****************************************************************************************
void server_process ( void )
{
	MAC_ADDR client_mac;
	IP_ADDR client_ip;
	// you can change rx,tx buffer size in includes.h
	BYTE rxtx_buffer[MAX_RXTX_BUFFER];
	WORD plen;
			
	if ( flag1.bits.syn_is_sent )
		return;
	
	// get new packet
	plen = enc28j60_packet_receive( (BYTE*)&rxtx_buffer, MAX_RXTX_BUFFER );
	
	//plen will be unequal to zero if there is a valid packet (without crc error)
	if(plen==0)
		return;

	// copy client mac address from buffer to client mac variable
	memcpy ( (BYTE*)&client_mac, &rxtx_buffer[ ETH_SRC_MAC_P ], sizeof(MAC_ADDR) );
	
	// check arp packet if match with avr ip let's send reply
	if ( arp_packet_is_arp( rxtx_buffer, (WORD_BYTES){ARP_OPCODE_REQUEST_V} ) )
	{
		arp_send_reply ( (BYTE*)&rxtx_buffer, (BYTE*)&client_mac );
		return;
	}

	// get client ip address
	memcpy ( (BYTE*)&client_ip, &rxtx_buffer[ IP_SRC_IP_P ], sizeof(IP_ADDR) );
	// check ip packet send to avr or not?
	if ( ip_packet_is_ip ( (BYTE*)&rxtx_buffer ) == 0 )
	{
		return;
	}

	// check packet if packet is icmp packet let's send icmp echo reply
	if ( icmp_send_reply ( (BYTE*)&rxtx_buffer, (BYTE*)&client_mac, (BYTE*)&client_ip ) )
	{
		return;
	}
	
	// start web server at port 80, see http.c
	http_webserver_process ( (BYTE*)rxtx_buffer, (BYTE*)&client_mac, (BYTE*)&client_ip );
}


//*****************************************************************************************
//
// Function : main
// Description : main program, 
//
//*****************************************************************************************
int main (void)
{
	// change your mac address here
	avr_mac.byte[0] = 0x00;
	avr_mac.byte[1] = 0x04;
	avr_mac.byte[2] = 0xa3;
	avr_mac.byte[3] = 0xb7;
	avr_mac.byte[4] = 0x73;
	avr_mac.byte[5] = 0x6f;

	// read avr and server ip from eeprom
	eeprom_read_block ( &avr_ip, ee_avr_ip, 4 );
	
	// setup clock for timer0 and timer 1
	TCCR1B = 0x05;
	TCCR0 = 0x05;

	// initial enc28j60
	enc28j60_init( (BYTE*)&avr_mac );

	// initialize adc
	adc0_init();
	
	// initialize I/O
	DDRA = DDRA | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
	DDRB = DDRB | (1 << 0) | (1 << 1);
	DDRC = DDRC | (1 << 0) | (1 << 1) | (1 << 7) | (1 << 8);	
	DDRD = DDRD | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
	
	// initialize interupt for rotary encoder
	DDRB &= ~(1 << 3);
	GICR |= (1<<INT2);
	MCUCSR |= (0<<ISC2);
	sei();
		
	// loop forever
	for(;;)
	{
		// server process response for arp, icmp, http
		server_process ();

		// read adc value and calculate temperature
		adc0_data ();
		
		// display temperature values on the display
		update_display ();
		
		// check rotary flag to change desired temp
		check_rotary ();
		
		// turn on or off relays for hvac
		control_relays ();
	}

	return 0;
}
