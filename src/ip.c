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

static WORD_BYTES ip_identfier=(WORD_BYTES){1};


//*****************************************************************************
//
// Function : ip_generate_packet
// Description : generate all ip header
//
//*****************************************************************************
void ip_generate_header ( BYTE *rxtx_buffer, WORD_BYTES total_length, BYTE protocol, BYTE *dest_ip )
{
	BYTE i;
	WORD_BYTES ck;

	// set ipv4 and header length
	rxtx_buffer[ IP_P ] = IP_V4_V | IP_HEADER_LENGTH_V;

	// set TOS to default 0x00
	rxtx_buffer[ IP_TOS_P ] = 0x00;

	// set total length
	rxtx_buffer [ IP_TOTLEN_H_P ] = total_length.byte.high;
	rxtx_buffer [ IP_TOTLEN_L_P ] = total_length.byte.low;

	// set packet identification
	rxtx_buffer [ IP_ID_H_P ] = ip_identfier.byte.high;
	rxtx_buffer [ IP_ID_L_P ] = ip_identfier.byte.low;
	ip_identfier.word++;

	// set fragment flags
	rxtx_buffer [ IP_FLAGS_H_P ] = 0x00;
	rxtx_buffer [ IP_FLAGS_L_P ] = 0x00;

	// set Time To Live
	rxtx_buffer [ IP_TTL_P ] = 128;

	// set ip packettype to tcp/udp/icmp...
	rxtx_buffer [ IP_PROTO_P ] = protocol;

	// set source and destination ip address
	for ( i=0; i<4; i++ )
	{
		rxtx_buffer[ IP_DST_IP_P + i ] = dest_ip[i];
		rxtx_buffer[ IP_SRC_IP_P + i ] = avr_ip.byte[ i ];
	}

	// clear the 2 byte checksum
	rxtx_buffer[ IP_CHECKSUM_H_P ] = 0;
	rxtx_buffer[ IP_CHECKSUM_L_P ] = 0;

	// fill checksum value
	// calculate the checksum:
	ck.word = software_checksum ( &rxtx_buffer[ IP_P ], sizeof(IP_HEADER), 0 );
	rxtx_buffer[ IP_CHECKSUM_H_P ] = ck.byte.high;
	rxtx_buffer[ IP_CHECKSUM_L_P ] = ck.byte.low;
}


//*****************************************************************************
//
// Function : ip_check_ip
// Description : Check incoming packet
//
//*****************************************************************************
BYTE ip_packet_is_ip ( BYTE *rxtx_buffer )
{
	unsigned char i;

	// if ethernet type is not ip
	if ( rxtx_buffer[ ETH_TYPE_H_P ] != ETH_TYPE_IP_H_V || rxtx_buffer[ ETH_TYPE_L_P ] != ETH_TYPE_IP_L_V)
		return 0;

	// if ip packet not send to avr
	for ( i=0; i<sizeof(IP_ADDR); i++ )
	{
		if ( rxtx_buffer[ IP_DST_IP_P + i ] != avr_ip.byte[i] )
			return 0;
	}

	// destination ip address match with avr ip address
	return 1;
}
