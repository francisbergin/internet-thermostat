// Copyright (C) 2015 Francis Bergin

#ifndef TCP_H_
#define TCP_H_

#define TCP_HEADER_LEN 20
#define TCP_OPTION_LEN 4

#define TCP_FLAG_FIN_V 0x01
#define TCP_FLAG_SYN_V 0x02
#define TCP_FLAG_RST_V 0x04
#define TCP_FLAG_PSH_V 0x08
#define TCP_FLAG_ACK_V 0x10
#define TCP_FLAG_URG_V 0x20
#define TCP_FLAG_ECE_V 0x40
#define TCP_FLAG_CWR_V 0x80

#define TCP_SRC_PORT_H_P 0x22
#define TCP_SRC_PORT_L_P 0x23
#define TCP_DST_PORT_H_P 0x24
#define TCP_DST_PORT_L_P 0x25
#define TCP_SEQ_P 0x26    // the tcp seq number is 4 bytes 0x26-0x29
#define TCP_SEQACK_P 0x2A // 4 bytes
#define TCP_HEADER_LEN_P 0x2E
#define TCP_FLAGS_P 0x2F
#define TCP_WINDOWSIZE_H_P 0x30 // 2 bytes
#define TCP_WINDOWSIZE_L_P 0x31
#define TCP_CHECKSUM_H_P 0x32
#define TCP_CHECKSUM_L_P 0x33
#define TCP_URGENT_PTR_H_P 0x34 // 2 bytes
#define TCP_URGENT_PTR_L_P 0x35
#define TCP_OPTIONS_P 0x36
#define TCP_DATA_P 0x36

// function prototypes
extern WORD tcp_get_dlength(BYTE *rxtx_buffer);
extern BYTE tcp_get_hlength(BYTE *rxtx_buffer);
extern WORD tcp_puts_data(BYTE *rxtx_buffer, BYTE *data, WORD offset);
extern WORD tcp_puts_data_p(BYTE *rxtx_buffer, PGM_P data, WORD offset);

extern void tcp_send_packet(BYTE *rxtx_buffer, WORD_BYTES dest_port,
                            WORD_BYTES src_port, BYTE flags,
                            BYTE max_segment_size, BYTE clear_seqack,
                            WORD next_ack_num, WORD next_seq_num, WORD dlength,
                            BYTE *dest_mac, BYTE *dest_ip);

#endif // TCP_H_
