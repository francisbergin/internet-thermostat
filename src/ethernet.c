// Copyright (C) 2015 Francis Bergin

#include "includes.h"


//*****************************************************************************
//
// Function : eth_generate_header
// Description : generarete ethernet header, contain destination and source MAC address,
// ethernet type.
//
//*****************************************************************************
void eth_generate_header ( BYTE *rxtx_buffer, WORD_BYTES type, BYTE *dest_mac )
{
	BYTE i;
	//copy the destination mac from the source and fill my mac into src
	for ( i=0; i<sizeof(MAC_ADDR); i++)
	{
		rxtx_buffer[ ETH_DST_MAC_P + i ] = dest_mac[i];
		rxtx_buffer[ ETH_SRC_MAC_P + i ] = avr_mac.byte[i];
	}
	rxtx_buffer[ ETH_TYPE_H_P ] = type.byte.high;//HIGH(type);
	rxtx_buffer[ ETH_TYPE_L_P ] = type.byte.low;//LOW(type);
}


//*****************************************************************************
//
// Function : software_checksum
// Description :
// The Ip checksum is calculated over the ip header only starting
// with the header length field and a total length of 20 bytes
// unitl ip.dst
// You must set the IP checksum field to zero before you start
// the calculation.
// len for ip is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we
// use the ip.src and ip.dst fields of the real packet:
// The udp checksum calculation starts with the ip.src field
// Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
// In other words the len here is 8 + length over which you actually
// want to calculate the checksum.
// You must set the checksum field to zero before you start
// the calculation.
// len for udp is: 8 + 8 + data length
// len for tcp is: 4+4 + 20 + option len + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
//
//*****************************************************************************
WORD software_checksum(BYTE *rxtx_buffer, WORD len, DWORD sum)
{
	// build the sum of 16bit words
	while(len>1)
	{
		sum += 0xFFFF & (*rxtx_buffer<<8|*(rxtx_buffer+1));
		rxtx_buffer+=2;
		len-=2;
	}
	// if there is a byte left then add it (padded with zero)
	if (len)
	{
		sum += (0xFF & *rxtx_buffer)<<8;
	}
	// now calculate the sum over the bytes in the sum
	// until the result is only 16bit long
	while (sum>>16)
	{
		sum = (sum & 0xFFFF)+(sum >> 16);
	}
	// build 1's complement:
	return( (WORD) sum ^ 0xFFFF);
}
