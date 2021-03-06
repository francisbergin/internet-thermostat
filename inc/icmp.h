// Copyright (C) 2015 Francis Bergin

#ifndef ICMP_H_
#define ICMP_H_

#define ICMP_TYPE_ECHOREPLY_V 0
#define ICMP_TYPE_ECHOREQUEST_V 8
#define ICMP_PACKET_LEN 40

// icmp buffer position
#define ICMP_TYPE_P 0x22
#define ICMP_CODE_P 0x23
#define ICMP_CHECKSUM_H_P 0x24
#define ICMP_CHECKSUM_L_P 0x25
#define ICMP_IDENTIFIER_H_P 0x26
#define ICMP_IDENTIFIER_L_P 0x27
#define ICMP_SEQUENCE_H_P 0x28
#define ICMP_SEQUENCE_L_P 0x29
#define ICMP_DATA_P 0x2A

// function prototypes
extern BYTE icmp_send_reply(BYTE *rxtx_buffer, BYTE *dest_mac, BYTE *dest_ip);
extern void icmp_send_request(BYTE *rxtx_buffer, BYTE *dest_mac, BYTE *dest_ip);
extern BYTE icmp_ping(BYTE *rxtx_buffer, BYTE *dest_mac, BYTE *dest_ip);

#endif // ICMP_H_
