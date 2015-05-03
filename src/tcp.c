// Copyright (C) 2015 Francis Bergin

#include "includes.h"

static BYTE seqnum=0xa; // my initial tcp sequence number
//static DWORD_BYTES tcp_sequence_number;


BYTE prev_seq_value[8];

WORD prev_seq_temp=0;
BYTE prev_seq_mod=0;
BYTE cntx=0, y=0;

DWORD prev_seq_total=0;
DWORD powx;
BYTE cnty=0;

WORD prev_dlength;
WORD prev_dlength0;
WORD prev_dlength2;

BYTE prev_cnt=0;

DWORD next_seq_number;
DWORD next_seq_temp=0;
BYTE next_seq_mod=0;
BYTE next_seq_add=0;



BYTE prev_seq_number[4];


//*****************************************************************************
//
// Function : tcp_get_dlength
// Description : claculate tcp received data length
//
//*****************************************************************************
WORD tcp_get_dlength ( BYTE *rxtx_buffer )
{
	int dlength, hlength;

	dlength = ( rxtx_buffer[ IP_TOTLEN_H_P ] <<8 ) | ( rxtx_buffer[ IP_TOTLEN_L_P ] );
	dlength -= sizeof(IP_HEADER);
	hlength = (rxtx_buffer[ TCP_HEADER_LEN_P ]>>4) * 4; // generate len in bytes;
	dlength -= hlength;

	if ( dlength <= 0 )
		dlength=0;

	return ((WORD)dlength);
}


//*****************************************************************************
//
// Function : tcp_get_hlength
// Description : claculate tcp received header length
//
//*****************************************************************************
BYTE tcp_get_hlength ( BYTE *rxtx_buffer )
{
	return ((rxtx_buffer[ TCP_HEADER_LEN_P ]>>4) * 4); // generate len in bytes;
}


//*****************************************************************************
//
// Function : tcp_puts_data_p
// Description : puts data from program memory to tx buffer
//
//*****************************************************************************
WORD tcp_puts_data_p ( BYTE *rxtx_buffer, PGM_P data, WORD offset )
{
	BYTE ch;

	while( (ch = pgm_read_byte(data++)) )
	{
		rxtx_buffer[ TCP_DATA_P + offset ] = ch;
		offset++;
	}

	return offset;
}


//*****************************************************************************
//
// Function : tcp_puts_data
// Description : puts data from RAM to tx buffer
//
//*****************************************************************************
WORD tcp_puts_data ( BYTE *rxtx_buffer, BYTE *data, WORD offset )
{
	while( *data )
	{
		rxtx_buffer[ TCP_DATA_P + offset ] = *data++;
		offset++;
	}

	return offset;
}


//*****************************************************************************
//
// Function : tcp_send_packet
// Description : send tcp packet to network.
//
//*****************************************************************************
void tcp_send_packet (
	BYTE *rxtx_buffer,
	WORD_BYTES dest_port,
	WORD_BYTES src_port,
	BYTE flags,
	BYTE max_segment_size,
	BYTE clear_seqack,
	WORD next_ack_num,
	WORD next_seq_num,
	WORD dlength,
	BYTE *dest_mac,
	BYTE *dest_ip )
{
	BYTE i, tseq;
	WORD_BYTES ck;

	// generate ethernet header
	eth_generate_header ( rxtx_buffer, (WORD_BYTES){ETH_TYPE_IP_V}, dest_mac );

	// sequence numbers:
	// add the rel ack num to SEQACK
	if ( next_ack_num )
	{
		for( i=4; i>0; i-- )
		{
			next_ack_num = rxtx_buffer [ TCP_SEQ_P + i - 1] + next_ack_num;
			tseq = rxtx_buffer [ TCP_SEQACK_P + i - 1];
			rxtx_buffer [ TCP_SEQACK_P + i - 1] = 0xff & next_ack_num;

			// copy the acknum sent to us into the sequence number
			rxtx_buffer[ TCP_SEQ_P + i - 1 ] = tseq;
			prev_seq_number[i] = 0xff & next_ack_num;

			next_ack_num >>= 8;
		}
	}

	if ( next_seq_num )
	{
		if (packet==0)
		{
			prev_dlength0=dlength;
		}

		else if (packet==1)
		{
			prev_dlength=prev_dlength0;
		}

		else if (packet==2)
		{
			prev_dlength2=dlength;
			prev_dlength=prev_dlength0;
		}

		else if (packet==3)
		{
			prev_dlength=prev_dlength2;
		}


		if ((packet==1)||(packet==2)||(packet==3))
		{
			prev_cnt++;
			y=7;
			for (cntx=0; cntx<=3; cntx++)
			{
				//if (rxtx_buffer [TCP_SEQ_P + cntx ]!=0)
				//{
					prev_seq_temp = rxtx_buffer [TCP_SEQ_P + cntx ];
					prev_seq_mod = prev_seq_temp % 16;
					prev_seq_value [y-1] = prev_seq_mod;

					prev_seq_temp = prev_seq_temp / 16;
					prev_seq_mod = prev_seq_temp % 16;
					prev_seq_value [y] = prev_seq_mod;

					y-=2;
				//}
				//else
				//{
				//	y-=2;
				//}
			}

			powx = 16;
			prev_seq_total = 0;
			for (cnty=0; cnty<=7; cnty++)
			{
				if(cnty==0)
					prev_seq_total += prev_seq_value [cnty];

				else
				{
					prev_seq_total += prev_seq_value [cnty] * powx;
					powx *= 16;
				}
			}

			next_seq_number = prev_dlength + prev_seq_total;




			//prev_cnt=0;

			y=7;
			next_seq_temp = next_seq_number;

			for (cntx=4; cntx>0; cntx--)
			{
				next_seq_add=0;

				next_seq_mod = next_seq_temp % 16;
				next_seq_add = next_seq_mod;
				next_seq_temp = next_seq_temp / 16;

				next_seq_mod = next_seq_temp % 16;
				next_seq_add += (next_seq_mod*16);
				next_seq_temp = next_seq_temp / 16;

				rxtx_buffer [TCP_SEQ_P + cntx -1 ] = next_seq_add;
			}

			//prev_cnt=0;
		}
	}

	// initial tcp sequence number
	// setup maximum segment size
	// require to setup first packet is receive or transmit only
	if ( max_segment_size )
	{
		// initial sequence number
		rxtx_buffer[ TCP_SEQ_P + 0 ] = 0;
		rxtx_buffer[ TCP_SEQ_P + 1 ] = 0;
		rxtx_buffer[ TCP_SEQ_P + 2 ] = seqnum;
		rxtx_buffer[ TCP_SEQ_P + 3 ] = 0;
		seqnum += 2;

		// setup maximum segment size
		rxtx_buffer[ TCP_OPTIONS_P + 0 ] = 2;
		rxtx_buffer[ TCP_OPTIONS_P + 1 ] = 4;
		rxtx_buffer[ TCP_OPTIONS_P + 2 ] = HIGH(1408);
		rxtx_buffer[ TCP_OPTIONS_P + 3 ] = LOW(1408);
		// setup tcp header length 24 bytes: 6*32/8 = 24
		rxtx_buffer[ TCP_HEADER_LEN_P ] = 0x60;
		dlength += 4;
	}
	else
	{
		// no options: 20 bytes: 5*32/8 = 20
		rxtx_buffer[ TCP_HEADER_LEN_P ] = 0x50;
	}

	// generate ip header and checksum
	ip_generate_header ( rxtx_buffer, (WORD_BYTES){(sizeof(IP_HEADER) + sizeof(TCP_HEADER) + dlength)}, IP_PROTO_TCP_V, dest_ip );

	// clear sequence ack number before send tcp SYN packet
	if ( clear_seqack )
	{
		rxtx_buffer[ TCP_SEQACK_P + 0 ] = 0;
		rxtx_buffer[ TCP_SEQACK_P + 1 ] = 0;
		rxtx_buffer[ TCP_SEQACK_P + 2 ] = 0;
		rxtx_buffer[ TCP_SEQACK_P + 3 ] = 0;
	}

	// setup tcp flags
	rxtx_buffer [ TCP_FLAGS_P ] = flags;

	// setup destination port
	rxtx_buffer [ TCP_DST_PORT_H_P ] = dest_port.byte.high;
	rxtx_buffer [ TCP_DST_PORT_L_P ] = dest_port.byte.low;

	// setup source port
	rxtx_buffer [ TCP_SRC_PORT_H_P ] = src_port.byte.high;
	rxtx_buffer [ TCP_SRC_PORT_L_P ] = src_port.byte.low;

	// setup maximum windows size
	rxtx_buffer [ TCP_WINDOWSIZE_H_P ] = HIGH((MAX_RX_BUFFER-sizeof(IP_HEADER)-sizeof(ETH_HEADER)));
	rxtx_buffer [ TCP_WINDOWSIZE_L_P ] = LOW((MAX_RX_BUFFER-sizeof(IP_HEADER)-sizeof(ETH_HEADER)));

	// setup urgend pointer (not used -> 0)
	rxtx_buffer[ TCP_URGENT_PTR_H_P ] = 0;
	rxtx_buffer[ TCP_URGENT_PTR_L_P ] = 0;

	// clear old checksum and calculate new checksum
	rxtx_buffer[ TCP_CHECKSUM_H_P ] = 0;
	rxtx_buffer[ TCP_CHECKSUM_L_P ] = 0;
	// This is computed as the 16-bit one's complement of the one's complement
	// sum of a pseudo header of information from the
	// IP header, the TCP header, and the data, padded
	// as needed with zero bytes at the end to make a multiple of two bytes.
	// The pseudo header contains the following fields:
	//
	// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	// +00+01+02+03+04+05+06+07+08+09+10+11+12+13+14+15+16+17+18+19+20+21+22+23+24+25+26+27+28+29+30+31+
	// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	// +                                       Source IP address                                       +
	// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	// +                                     Destination IP address                                    +
	// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	// +           0           +      IP Protocol      +                    Total length               +
	// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	ck.word = software_checksum( &rxtx_buffer[IP_SRC_IP_P], sizeof(TCP_HEADER)+dlength+8, IP_PROTO_TCP_V + sizeof(TCP_HEADER) + dlength );
	rxtx_buffer[ TCP_CHECKSUM_H_P ] = ck.byte.high;
	rxtx_buffer[ TCP_CHECKSUM_L_P ] = ck.byte.low;

	// send packet to ethernet media
	enc28j60_packet_send ( rxtx_buffer, sizeof(ETH_HEADER)+sizeof(IP_HEADER)+sizeof(TCP_HEADER)+dlength );
}
