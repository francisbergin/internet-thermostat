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

WORD dlen;

BYTE packet;
BYTE web_page=1;
BYTE post_data=0;

BYTE mac_filter=0;
BYTE mac_filter_address[3];
BYTE mac_filter_pass=0;


//*****************************************************************************
//
// Function : http_webserver_process
// Description : Initial connection to web server
//
//*****************************************************************************
void http_webserver_process ( BYTE *rxtx_buffer, BYTE *dest_mac, BYTE *dest_ip )
{
	WORD dlength, dest_port;
//	BYTE count_time_temp[3];
	BYTE generic_buf[64];


	dest_port = (rxtx_buffer[TCP_SRC_PORT_H_P]<<8)|rxtx_buffer[TCP_SRC_PORT_L_P];
	// tcp port 80 start for web server
	if ( rxtx_buffer [ IP_PROTO_P ] == IP_PROTO_TCP_V && rxtx_buffer[ TCP_DST_PORT_H_P ] == 0 && rxtx_buffer[ TCP_DST_PORT_L_P ] == 80 )
	{

		// received packet with flags "SYN", let's send "SYNACK"
		if ( (rxtx_buffer[ TCP_FLAGS_P ] & TCP_FLAG_SYN_V) )
		{
			tcp_send_packet (
				rxtx_buffer,
				(WORD_BYTES){dest_port},
				(WORD_BYTES){80},					// source port
				TCP_FLAG_SYN_V|TCP_FLAG_ACK_V,			// flag
				1,						// (bool)maximum segment size
				0,						// (bool)clear sequence ack number
				1,						// (bool)calculate new seq and seqack number
				0,						// tcp data length
				0,
				dest_mac,		// server mac address
				dest_ip );		// server ip address
			//flag1.bits.syn_is_received = 1;
			return;
		}

		if ( (rxtx_buffer [ TCP_FLAGS_P ] & TCP_FLAG_ACK_V) )
		{
			// get tcp data length
			dlength = tcp_get_dlength( rxtx_buffer );
			if ( dlength == 0 )
			{
				// finack, answer with ack
				if ( (rxtx_buffer[TCP_FLAGS_P] & TCP_FLAG_FIN_V) )
				{
//					tcp_send_ack ( rxtx_buffer, dest_mac, dest_ip );
					tcp_send_packet (
						rxtx_buffer,
						(WORD_BYTES){dest_port},
						(WORD_BYTES){80},						// source port
						TCP_FLAG_ACK_V,			// flag
						0,						// (bool)maximum segment size
						0,						// (bool)clear sequence ack number
						1,						// (bool)calculate new seq and seqack number
						0,						// tcp data length
						0,
						dest_mac,		// server mac address
						dest_ip );		// server ip address
				}
				return;
			}


//************************************************Verify Data on Receive Buffer***************************************************************


			if ((http_get_post ( rxtx_buffer, dlength, PSTR( "OST " ), generic_buf )) || (post_data==1))
			{

				if (mac_filter==0)
				{
					mac_filter_pass=1;
				}
				else if (mac_filter==1)
				{
					if ((rxtx_buffer[9]==mac_filter_address[0])&&(rxtx_buffer[10]==mac_filter_address[1])&&(rxtx_buffer[11]==mac_filter_address[2]))
					{
						mac_filter_pass=1;
					}
					else
						mac_filter_pass=0;
				}


				if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "3Fpage" ), generic_buf ) )
				{
					post_data=0;

					if (generic_buf[0]==49)
						web_page=1;
					else if (generic_buf[0]==50)
						web_page=2;
				}


				else if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "3Fmac" ), generic_buf ) )
				{
					post_data=0;

					if (mac_filter_pass==1)
					{
						if (generic_buf[0]==48)
							mac_filter = 0;
						else if (generic_buf[0]==49)
						{
							mac_filter = 1;

							mac_filter_address[0]=rxtx_buffer[9];
							mac_filter_address[1]=rxtx_buffer[10];
							mac_filter_address[2]=rxtx_buffer[11];
						}
					}
				}


				else if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "3Fsub" ), generic_buf ) )
				{
					post_data=0;

					if (mac_filter_pass==1)
					{
						if (generic_buf[0]==49)
							PORTB ^= 0x02;
						else if (generic_buf[0]==50)
							PORTB ^= 0x01;

						else if (generic_buf[0]==117)
						{
							desired_temp += 0.1;
							TCNT1 = 0;
							rot_enc_change=1;
						}

						else if (generic_buf[0]==100)
						{
							desired_temp -= 0.1;
							TCNT1 = 0;
							rot_enc_change=1;
						}
					}
				}


				else if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "3Fhvac" ), generic_buf ) )
				{
					post_data=0;

					if (mac_filter_pass==1)
					{
						if (generic_buf[0]==48)
							hvac_flag_sys ^= 0x01;

						else if (generic_buf[0]==49)
							hvac_heat_flag ^= 0x01;

						else if (generic_buf[0]==50)
							hvac_vent_flag ^= 0x01;

						else if (generic_buf[0]==51)
							hvac_ac_flag ^= 0x01;


						else if (generic_buf[0]==112)
							hvac_flag_heat_src = 1;

						else if (generic_buf[0]==115)
							hvac_flag_heat_src = 2;


						else if (generic_buf[0]==97)
						{
							hvac_flag_auto = 1;
							hvac_heat_flag = 1;
							hvac_vent_flag = 1;
							hvac_ac_flag = 1;
						}

						else if (generic_buf[0]==109)
						{
							hvac_flag_auto = 0;
							hvac_heat_flag = 0;
							hvac_vent_flag = 0;
							hvac_ac_flag = 0;
						}

						else if (generic_buf[0]==120)
							;
					}
				}


				else if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "3Fadcsrc" ), generic_buf ) )
				{
					post_data=0;

					if (mac_filter_pass==1)
					{
						if (generic_buf[0]==48)
						{
							adc0_pwr_src=0;
							adc0_heat_compensation=5.0;
						}

						if (generic_buf[0]==49)
						{
							adc0_pwr_src=1;
							adc0_heat_compensation=15.0;
						}

						else if (generic_buf[0]==50)
						{
							adc0_pwr_src=2;
							adc0_heat_compensation=17.5;
						}

						else if (generic_buf[0]==51)
						{
							adc0_pwr_src=3;
							adc0_heat_compensation=20.0;
						}

						//heat compensation up or down
						else if (generic_buf[0]==117)
						{
							adc0_pwr_src=0;
							adc0_heat_compensation += 0.1;
						}

						else if (generic_buf[0]==100)
						{
							adc0_pwr_src=0;
							adc0_heat_compensation -= 0.1;
						}
					}
				}


				else if (post_data==0)
				{
					post_data=1;
					//_delay_us(100);
					return;
				}
			}


//*********************************************Print Webpage************************************************************************

			// send ack before send data
			tcp_send_packet (
						rxtx_buffer,
						(WORD_BYTES){dest_port},
						(WORD_BYTES){80},						// source port
						TCP_FLAG_ACK_V,			// flag
						0,						// (bool)maximum segment size
						0,						// (bool)clear sequence ack number
						1,						// (bool)calculate new seq and seqack number
						0,
						0,						// tcp data length
						dest_mac,		// server mac address
						dest_ip );		// server ip address


			// send first part of web page
			packet=0;
			dlength = http_home( rxtx_buffer );

			tcp_send_packet (
						rxtx_buffer,
						(WORD_BYTES){dest_port},
						(WORD_BYTES){80},						// source port
						TCP_FLAG_ACK_V,
						0,						// (bool)maximum segment size
						0,						// (bool)clear sequence ack number
						0,						// (bool)calculate new seq and seqack number
						1,
						dlength,				// tcp data length
						dest_mac,		// server mac address
						dest_ip );		// server ip address


			// determine which page to send
			//_delay_us(500);
			packet=web_page;

			// send first page HOME
			if (packet==1)
			{
				dlength = http_home( rxtx_buffer );

				tcp_send_packet (
							rxtx_buffer,
							(WORD_BYTES){dest_port},
							(WORD_BYTES){80},						// source port
							TCP_FLAG_ACK_V | TCP_FLAG_PSH_V | TCP_FLAG_FIN_V,
							0,						// (bool)maximum segment size
							0,						// (bool)clear sequence ack number
							0,						// (bool)calculate new seq and seqack number
							1,
							dlength,				// tcp data length
							dest_mac,		// server mac address
							dest_ip );		// server ip address
			}

			// send second page SETTINGS
			else if(packet==2)
			{
				// first part of SETTINGS
				packet=2;
				dlength = http_home( rxtx_buffer );

				tcp_send_packet (
							rxtx_buffer,
							(WORD_BYTES){dest_port},
							(WORD_BYTES){80},						// source port
							TCP_FLAG_ACK_V,
							0,						// (bool)maximum segment size
							0,						// (bool)clear sequence ack number
							0,						// (bool)calculate new seq and seqack number
							1,
							dlength,				// tcp data length
							dest_mac,		// server mac address
							dest_ip );		// server ip address


				// second part of SETTINGS
				packet=3;
				dlength = http_home( rxtx_buffer );

				tcp_send_packet (
							rxtx_buffer,
							(WORD_BYTES){dest_port},
							(WORD_BYTES){80},						// source port
							TCP_FLAG_ACK_V | TCP_FLAG_PSH_V | TCP_FLAG_FIN_V,			// flag		TCP_FLAG_PSH_V |
							0,						// (bool)maximum segment size
							0,						// (bool)clear sequence ack number
							0,						// (bool)calculate new seq and seqack number
							1,
							dlength,				// tcp data length
							dest_mac,		// server mac address
							dest_ip );		// server ip address

			}

		}
	}
}


//*****************************************************************************
//
// Function : http_get_variable
// Description : Get http variable from GET method, example http://10.1.1.1/?pwd=123456
//		when you call http_get_variable with val_key="pwd", then function stored "123456"
//		to dest buffer.
//
//*****************************************************************************
BYTE http_get_post ( BYTE *rxtx_buffer, WORD dlength, PGM_P val_key, BYTE *dest )
{
	WORD data_p; //data_p_test;
	PGM_P key;
	BYTE match=0, temp;
	//WORD sent_value;
	//WORD clength;

	key = val_key;

	if ((rxtx_buffer[0x36]==0x50)&&(rxtx_buffer[0x39]==0x54))
		dlength+=60;

	// get data position
	data_p = tcp_get_hlength( rxtx_buffer ) + sizeof(ETH_HEADER) + sizeof(IP_HEADER);

	// Find '?' in rx buffer, if found '?' in rx buffer then let's find variable key (val_key)
	for ( ; data_p<dlength; data_p++ )
	{
		if ( rxtx_buffer [ data_p ] == 'P' )
			break;
	}
	// not found '?' in buffer
	if ( data_p == dlength )
		return 0;

	// find variable key in buffer
	for ( ; data_p<dlength; data_p++ )
	{
		temp = pgm_read_byte ( key );

		// end of variable keyword
		if ( rxtx_buffer [ data_p ] == '/' && match != 0 )
		{
			//data_p_test = data_p+3;
			//if ( rxtx_buffer [ data_p_test ] != 'H')
			//	return 0;

			if ( temp == '\0' )
			{
				//return match;
				data_p++;
				break;
			}
		}
		// variable keyword match with rx buffer
		if ( rxtx_buffer [ data_p ] == temp )
		{
			key++;
			match++;
		}
		else
		{
			// no match in rx buffer reset match and find again
			key = val_key;
			match = 0;
		}
	}

	return match;
}


//*****************************************************************************
//
// Function : http_get_variable
// Description : Get http variable from GET method, example http://10.1.1.1/?pwd=123456
//		when you call http_get_variable with val_key="pwd", then function stored "123456"
//		to dest buffer.
//
//*****************************************************************************
BYTE http_get_variable ( BYTE *rxtx_buffer, WORD dlength, PGM_P val_key, BYTE *dest )
{
	WORD data_p; //data_p_test;
	PGM_P key;
	BYTE match=0, temp;
	//WORD sent_value;
	//WORD clength;

	key = val_key;

	if ((rxtx_buffer[0x36]==0x50)&&(rxtx_buffer[0x39]==0x54))
		dlength+=60;

	// get data position
	data_p = tcp_get_hlength( rxtx_buffer ) + sizeof(ETH_HEADER) + sizeof(IP_HEADER);

	if (post_data==1)
	{
		data_p-=5;
		dlength+=(data_p+30);
	}

	// Find '?' in rx buffer, if found '?' in rx buffer then let's find variable key (val_key)
	for ( ; data_p<dlength; data_p++ )
	{
		if ( rxtx_buffer [ data_p ] == '%' )
			break;
	}
	// not found '?' in buffer
	if ( data_p == dlength )
		return 0;

	// find variable key in buffer
	for ( ; data_p<dlength; data_p++ )
	{
		temp = pgm_read_byte ( key );

		// end of variable keyword
		if ( rxtx_buffer [ data_p ] == '=' && match != 0 )
		{
			data_p++;
			break;
		}
		// variable keyword match with rx buffer
		if ( rxtx_buffer [ data_p ] == temp )
		{
			key++;
			match++;
		}
		else
		{
			// no match in rx buffer reset match and find again
			key = val_key;
			match = 0;
		}
	}

	// if found variable keyword, then store variable value in destination buffer ( dest )

	if ( match != 0 )
	{
		match = 0;

		for ( ;; )
		{	//if ( rxtx_buffer [ data_p ] == '\r' )
			//	return 0;

			// end of variable value break from loop
			if ( rxtx_buffer [ data_p ] == '%' )//|| rxtx_buffer [ data_p ] == ' ' )
			{
				dest [ match ] = '\0';
				break;
			}
			dest [ match ] = rxtx_buffer [ data_p ];

			//sent_value = (sent_value<<8) | rxtx_buffer [ data_p ];

			match++;
			data_p++;
		}
	}


	//return with variable value length
	//read_sent_value( sent_value );

	return match;
}


//*****************************************************************************
//
// Function : http_home
// Description : prepare the webpage by writing the data to the tcp send buffer
//
//*****************************************************************************

WORD http_home( BYTE *rxtx_buffer )
{
	//WORD dlen=0, adc0_value;
	WORD temp_value1, temp_value2, temp_value3;
	WORD temp_desired1, temp_desired2, temp_desired3;
	WORD temp_compensation1, temp_compensation2, temp_compensation3;
	//BYTE count_time_temp[3];
	BYTE generic_buf[64];
	int input;

	//***********************Get Temperature Values*************************************************

	temp_value1 = adc_read_temp1();
	temp_value2 = adc_read_temp2();
	temp_value3 = adc_read_temp3();

	temp_desired1 = desired_read_temp1();
	temp_desired2 = desired_read_temp2();
	temp_desired3 = desired_read_temp3();

	temp_compensation1 = adc_read_compensation1();
	temp_compensation2 = adc_read_compensation2();
	temp_compensation3 = adc_read_compensation3();

	//*********************************************************************************************

	// first packet to send. this is the general structure including the title and CSS definitions
	if (packet==0)
	{
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" ), 0 );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<link rel='icon'type='image/gif'href='http://www.large-icons.com/stock-icons/large-time/gauge-icon.gif'/>" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<title>Internet Thermostat</title>" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<meta name='viewport'content='width=device-width,initial-scale=1.0'/><style type='text/css'media='all'>body{font:0.8em arial,helvetica,sans-serif;max-width:600px;margin:0px auto;text-align:left;}#header ul{list-style:none;padding:0;margin:0;}" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "#header li{float:left;border:1px solid #bbb;border-bottom-width:0;margin:0;}#header a:hover{background:#ddf;}" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "#header a{text-decoration:none;display:block;background:#eee;padding:0.24em 1em;color:#00c;width:8em;text-align:center;}" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "#header #selected{border-color:black;}#content{border:1px solid black;clear:both;padding:0 1em;}" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "#header #selected a{position:relative;top:1px;background:white;color:black;font-weight:bold;}h1{margin:0;padding:0 0 1em 0;}" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "</style></head><body><div id='header'><h1>Internet Thermostat</h1><ul>" ), dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<form id='myForm1'method='post'><input type='hidden'name='?page'value='1?'></form>" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<form id='myForm2'method='post'><input type='hidden'name='?page'value='2?'></form>" ), dlen );

		if (web_page==1)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<li id='selected'><a href onclick=\"document.forms['myForm1'].submit();return false;\">HOME</a></li>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<li><a href onclick=\"document.forms['myForm2'].submit();return false;\">SETTINGS</a></li>" ), dlen );
		}

		if (web_page==2)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<li><a href onclick=\"document.forms['myForm1'].submit();return false;\">HOME</a></li>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<li id='selected'><a href onclick=\"document.forms['myForm2'].submit();return false;\">SETTINGS</a></li>" ), dlen );
		}

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "</ul></div><br>" ), dlen );

		return(dlen);
	}
	//******************************************************************************************************************************************************************************

	// this is the HOME page
	if (packet==1)
	{
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<div id='content'><br><p>" ), 0 );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<script type='text/javascript'src='https://www.google.com/jsapi'></script><script type='text/javascript'>google.load('visualization','1',{packages:['gauge']});google.setOnLoadCallback(drawChart);function drawChart()" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "{var data=new google.visualization.DataTable();data.addColumn('string','Label');data.addColumn('number','Value');data.addRows([['Actual'," ), dlen );

	//current
		print_decimal ( generic_buf, 1, temp_value1 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		print_decimal ( generic_buf, 1, temp_value2 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "." ), dlen );
		print_decimal ( generic_buf, 1, temp_value3 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "],['Setpoint'," ), dlen );

	//desired
		print_decimal ( generic_buf, 1, temp_desired1 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		print_decimal ( generic_buf, 1, temp_desired2 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "." ), dlen );
		print_decimal ( generic_buf, 1, temp_desired3 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );


		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "],]);" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "var options={height:600,width:550,min:-30,max:70,greenColor:'#ADD8E6',greenFrom:-30,greenTo:10,yellowFrom:30,yellowTo:50,redFrom:40,redTo:70,majorTicks:['-30','-10','10',30,50,70],minorTicks:20};" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "var chart=new google.visualization.Gauge(document.getElementById('chart_div'));chart.draw(data,options);}" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "</script><div id='chart_div'></div>" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'width='100%'size='3'/>" ), dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<b>System status</b><hr color='black'align='left'width='20%'size='1'/>" ), dlen );


		if (hvac_flag_sys==0)
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "HVAC System: OFF" ), dlen );
		else
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "HVAC System: ON" ), dlen );

		if (hvac_flag_auto==0)
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Mode: Manual" ), dlen );
		else
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Mode: Automatic" ), dlen );

		if (hvac_flag_heat_src==1)
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Heat Source: Primary" ), dlen );
		else if (hvac_flag_heat_src==2)
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Heat Source: Secondary" ), dlen );



		if (((hvac_heat_flag==1)&&(hvac_heat==1))||((hvac_heat_flag==1)&&(hvac_flag_auto==0)))
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><br>Heating system: ON" ), dlen );
		else
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><br>Heating system: OFF" ), dlen );

		if (((hvac_vent_flag==1)&&(hvac_vent==1))||((hvac_vent_flag==1)&&(hvac_flag_auto==0)))
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Ventilation system: ON" ), dlen );
		else
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Ventilation system: OFF" ), dlen );

		if (((hvac_ac_flag==1)&&(hvac_ac==1))||((hvac_ac_flag==1)&&(hvac_flag_auto==0)))
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Cooling system: ON" ), dlen );
		else
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Cooling system: OFF" ), dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'width='100%'size='3'/>" ), dlen );

		//*************************LEDs***************************************************************************

		//dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>" ), dlen );

		input = (PINB|0xFC)-252;

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<FORM method='post'>"), dlen );

		if (input==0)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?sub'value='1?'>LED1</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?sub'value='2?'>LED2</button></FORM>" ), dlen );
		}
		else if (input==1)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?sub'value='1?'>LED1</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:orange'type='submit'name='?sub'value='2?'>LED2</button></FORM>" ), dlen );
		}
		else if (input==2)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:orange'type='submit'name='?sub'value='1?'>LED1</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?sub'value='2?'>LED2</button></FORM>" ), dlen );
		}
		else if (input==3)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:orange'type='submit'name='?sub'value='1?'>LED1</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:orange'type='submit'name='?sub'value='2?'>LED2</button></FORM>" ), dlen );
		}

	//******************************************************************************************************************

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "</p></div></body></html>"), dlen );
		//dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<a href='./'>Reresh</a></body>" ), dlen );

		return(dlen);
	}
	//******************************************************************************************************************************************************************************

	//this is the first segment of the Settings page
	if (packet==2)
	{
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<div id='content'><br><p>" ), 0 );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Change general settings here<hr color='black'width='100%'size='3'/>" ), dlen );




		//display current temperature
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "Actual Temparature: " ), dlen );
		print_decimal ( generic_buf, 1, temp_value1 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );



		print_decimal ( generic_buf, 1, temp_value2 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );


		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "." ), dlen );
		print_decimal ( generic_buf, 1, temp_value3 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "&deg;C" ), dlen );





		//display desired temperature
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br>Setpoint Temperature: "), dlen );
		print_decimal ( generic_buf, 1, temp_desired1 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		print_decimal ( generic_buf, 1, temp_desired2 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "." ), dlen );

		print_decimal ( generic_buf, 1, temp_desired3 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "&deg;C" ), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'align='left'width='20%'size='1'/>" ), dlen );


		//temp up
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<FORM method='post'><button type='submit'name='?sub'value='u?'>&uarr;</button><br>" ), dlen );

		//temp down
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?sub'value='d?'>&darr;</button></FORM>"), dlen );
	//*********************************************************************************************************

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'width='100%'size='3'/>" ), dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<FORM method='post'>" ), dlen );

		if (hvac_flag_sys==0)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?hvac'value='0?'>X</button> HVAC System"), dlen );

			//auto/manual
			if (hvac_flag_auto==0)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='x?'>M</button> Manual"), dlen );

			if (hvac_flag_auto==1)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='x?'>A</button> Automatic"), dlen );

			//primary/secondary heat source
			if (hvac_flag_heat_src==1)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='x?'>P</button> Primary Heat Source"), dlen );

			if (hvac_flag_heat_src==2)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='x?'>S</button> Secondary Heat Source"), dlen );


			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'align='left'width='20%'size='1'/>"), dlen );

			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?hvac'value='x?'>x</button> Heating"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='x?'>x</button> Ventilation"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='x?'>x</button> Cooling"), dlen );
		}

		if (hvac_flag_sys==1)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?hvac'value='0?'>&#10003;</button> HVAC System"), dlen );

			//auto/manual
			if (hvac_flag_auto==0)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='a?'>M</button> Manual"), dlen );

			if (hvac_flag_auto==1)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='m?'>A</button> Automatic"), dlen );

			//primary/secondary heat source
			if (hvac_flag_heat_src==1)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='s?'>P</button> Primary Heat Source"), dlen );

			if (hvac_flag_heat_src==2)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='p?'>S</button> Secondary Heat Source"), dlen );


			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'align='left'width='20%'size='1'/>"), dlen );


			//heating
			if (hvac_heat_flag==0)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?hvac'value='1?'>x</button> Heating"), dlen );
			else
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?hvac'value='1?'>&#10003;</button> Heating"), dlen );

			//ventilation
			if (hvac_vent_flag==0)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='2?'>x</button> Ventilation"), dlen );
			else
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='2?'>&#10003;</button> Ventilation"), dlen );

			//air conditioning
			if (hvac_ac_flag==0)
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='3?'>x</button> Cooling"), dlen );
			else
				dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?hvac'value='3?'>&#10003;</button> Cooling"), dlen );
		}

		return(dlen);
	}
	//******************************************************************************************************************************************************************************

	//this is the second segment of the Settings page
	if (packet==3)
	{
		//temp compensation part
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "</FORM><hr color='black'width='100%'size='3'/>" ), 0 );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "Temperature Compensation: " ), dlen );

		print_decimal ( generic_buf, 1, temp_compensation1 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		print_decimal ( generic_buf, 1, temp_compensation2 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "." ), dlen );

		print_decimal ( generic_buf, 1, temp_compensation3 );
		generic_buf[ 1 ] = '\0';
		dlen = tcp_puts_data ( rxtx_buffer, (BYTE *)generic_buf, dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "&deg;C" ), dlen );



		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'align='left'width='20%'size='1'/>"), dlen );
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<FORM method='post'>" ), dlen );

		if (adc0_pwr_src==0)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:#00FFFF'type='submit'name='?adcsrc'value='0?'>Custom</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='1?'>12VDC</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='2?'>12VAC</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='3?'>24VAC</button>" ), dlen );
		}

		if (adc0_pwr_src==1)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='0?'>Custom</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:#00FFFF'type='submit'name='?adcsrc'value='1?'>12VDC</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='2?'>12VAC</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='3?'>24VAC</button>" ), dlen );
		}
		else if (adc0_pwr_src==2)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='0?'>Custom</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='1?'>12VDC</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:#00FFFF'type='submit'name='?adcsrc'value='2?'>12VAC</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='3?'>24VAC</button>" ), dlen );
		}
		else if (adc0_pwr_src==3)
		{
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='0?'>Custom</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='1?'>12VDC</button>"), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?adcsrc'value='2?'>12VAC</button>" ), dlen );
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button style='background-color:#00FFFF'type='submit'name='?adcsrc'value='3?'>24VAC</button>" ), dlen );
		}

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?adcsrc'value='u?'>&uarr;</button>" ), dlen );

		//temp down
		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<br><button type='submit'name='?adcsrc'value='d?'>&darr;</button>"), dlen );





		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<hr color='black'width='100%'size='3'/>" ), dlen );

		if (mac_filter==0)
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?mac'value='1?'>x</button>"), dlen );
		else
			dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<button type='submit'name='?mac'value='0?'>&#10003;</button>"), dlen );


		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( " MAC Filtering" ), dlen );

		dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "</FORM></p></div></body></html>"), dlen );
		//dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "<a href='./'>Reresh</a></body>" ), dlen );


		return(dlen);
	}

return(0);
}


//*****************************************************************************
//
// Function : print_decimal
// Description : Print decimal to buffer, up to 5 digits
//
//*****************************************************************************
BYTE * print_decimal ( BYTE *ptr, BYTE digit, WORD dec )
{
	if ( digit >= 5 )
		*ptr++ = ( (dec/10000) + '0' );
	if ( digit >= 4 )
		*ptr++ = ( ((dec%10000)/1000) + '0' );
	if ( digit >= 3 )
		*ptr++ = ( ((dec%1000)/100) + '0' );
	if ( digit >= 2 )
		*ptr++ = ( ((dec%100)/10) + '0' );
	*ptr++ = ( ((dec%10)) + '0' );

	return ptr;
}
