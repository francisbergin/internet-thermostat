// Copyright (C) 2015 Francis Bergin

#include "includes.h"

//*****************************************************************************
//
// Function : arp_generate_packet
// Description : generate arp packet
//
//*****************************************************************************
void arp_generate_packet(BYTE *rxtx_buffer, BYTE *dest_mac, BYTE *dest_ip) {
    unsigned char i;

    // setup hardware type to ethernet 0x0001
    rxtx_buffer[ARP_HARDWARE_TYPE_H_P] = ARP_HARDWARE_TYPE_H_V;
    rxtx_buffer[ARP_HARDWARE_TYPE_L_P] = ARP_HARDWARE_TYPE_L_V;

    // setup protocol type to ip 0x0800
    rxtx_buffer[ARP_PROTOCOL_H_P] = ARP_PROTOCOL_H_V;
    rxtx_buffer[ARP_PROTOCOL_L_P] = ARP_PROTOCOL_L_V;

    // setup hardware length to 0x06
    rxtx_buffer[ARP_HARDWARE_SIZE_P] = ARP_HARDWARE_SIZE_V;

    // setup protocol length to 0x04
    rxtx_buffer[ARP_PROTOCOL_SIZE_P] = ARP_PROTOCOL_SIZE_V;

    // setup arp destination and source mac address
    for (i = 0; i < sizeof(MAC_ADDR); i++) {
        rxtx_buffer[ARP_DST_MAC_P + i] = dest_mac[i];
        rxtx_buffer[ARP_SRC_MAC_P + i] = avr_mac.byte[i];
    }

    // setup arp destination and source ip address
    for (i = 0; i < sizeof(IP_ADDR); i++) {
        rxtx_buffer[ARP_DST_IP_P + i] = dest_ip[i];
        rxtx_buffer[ARP_SRC_IP_P + i] = avr_ip.byte[i];
    }
}

//*****************************************************************************
//
// Function : arp_send_request
// Description : send arp request packet (who is?) to network.
//
//*****************************************************************************
void arp_send_request(BYTE *rxtx_buffer, BYTE *dest_ip) {
    unsigned char i;
    MAC_ADDR dest_mac;

    // generate ethernet header
    for (i = 0; i < sizeof(MAC_ADDR); i++)
        dest_mac.byte[i] = 0xff;
    eth_generate_header(rxtx_buffer, (WORD_BYTES){ETH_TYPE_ARP_V},
                        (BYTE *)&dest_mac);

    // generate arp packet
    for (i = 0; i < sizeof(MAC_ADDR); i++)
        dest_mac.byte[i] = 0x00;

    // set arp opcode is request
    rxtx_buffer[ARP_OPCODE_H_P] = ARP_OPCODE_REQUEST_H_V;
    rxtx_buffer[ARP_OPCODE_L_P] = ARP_OPCODE_REQUEST_L_V;
    arp_generate_packet(rxtx_buffer, (BYTE *)&dest_mac, dest_ip);

    // send arp packet to network
    enc28j60_packet_send(rxtx_buffer, sizeof(ETH_HEADER) + sizeof(ARP_PACKET));
}

//*****************************************************************************
//
// Function : arp_packet_is_arp
// Description : check received packet, that packet is match with arp and avr ip
// or not?
//
//*****************************************************************************
BYTE arp_packet_is_arp(BYTE *rxtx_buffer, WORD_BYTES opcode) {
    BYTE i;

    // if packet type is not arp packet exit from function
    if (rxtx_buffer[ETH_TYPE_H_P] != ETH_TYPE_ARP_H_V ||
        rxtx_buffer[ETH_TYPE_L_P] != ETH_TYPE_ARP_L_V)
        return 0;
    // check arp request opcode
    if (rxtx_buffer[ARP_OPCODE_H_P] != opcode.byte.high ||
        rxtx_buffer[ARP_OPCODE_L_P] != opcode.byte.low)
        return 0;
    // if destination ip address in arp packet not match with avr ip address
    for (i = 0; i < sizeof(IP_ADDR); i++) {
        if (rxtx_buffer[ARP_DST_IP_P + i] != avr_ip.byte[i])
            return 0;
    }
    return 1;
}

//*****************************************************************************
//
// Function : arp_send_reply
// Description : Send reply if recieved packet is ARP and IP address is match
// with avr_ip
//
//*****************************************************************************
void arp_send_reply(BYTE *rxtx_buffer, BYTE *dest_mac) {
    // generate ethernet header
    eth_generate_header(rxtx_buffer, (WORD_BYTES){ETH_TYPE_ARP_V}, dest_mac);

    // change packet type to echo reply
    rxtx_buffer[ARP_OPCODE_H_P] = ARP_OPCODE_REPLY_H_V;
    rxtx_buffer[ARP_OPCODE_L_P] = ARP_OPCODE_REPLY_L_V;
    arp_generate_packet(rxtx_buffer, dest_mac, &rxtx_buffer[ARP_SRC_IP_P]);

    // send arp packet
    enc28j60_packet_send(rxtx_buffer, sizeof(ETH_HEADER) + sizeof(ARP_PACKET));
}

//*****************************************************************************
//
// Function : arp_who_is
// Description : send arp request to destination ip, and save destination mac to
// dest_mac.
// call this function to find the destination mac address before send other
// packet.
//
//*****************************************************************************
BYTE arp_who_is(BYTE *rxtx_buffer, BYTE *dest_mac, BYTE *dest_ip) {
    BYTE i;
    WORD dlength;

    // send arp request packet to network
    arp_send_request(rxtx_buffer, dest_ip);

    for (i = 0; i < 10; i++) {
        // Time out 10x10ms = 100ms
        _delay_ms(10);
        dlength = enc28j60_packet_receive(rxtx_buffer, MAX_RXTX_BUFFER);

        // destination ip address was found on network
        if (dlength) {
            if (arp_packet_is_arp(rxtx_buffer,
                                  (WORD_BYTES){ARP_OPCODE_REPLY_V})) {
                // copy destination mac address from arp reply packet to
                // destination mac address
                memcpy(dest_mac, &rxtx_buffer[ETH_SRC_MAC_P], sizeof(MAC_ADDR));
                return 1;
            }
        }
    }

    // destination ip was not found on network
    return 0;
}
