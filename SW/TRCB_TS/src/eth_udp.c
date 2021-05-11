/*
 * Copyright (C) 2017 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

/** Connection handle for a UDP Server session */

#include "eth_udp.h"

#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"
#include "netif/xadapter.h"
#include "sleep.h"
#include "eth_que.h"


#define DEFAULT_IP_ADDRESS	"192.168.200.61"
#define DEFAULT_IP_MASK		"255.255.0.0"
#define DEFAULT_GW_ADDRESS	"192.168.1.1"
#define UDP_CONN_PORT 29000 /* server port to listen on/connect to */

uint8_t rxEthData[QUE_BUFFER_SIZE][1200];

extern struct netif server_netif;
static struct udp_pcb *pcb;

/* Report interval in ms */
#define REPORT_INTERVAL_TIME (INTERIM_REPORT_INTERVAL * 1000)

void print_app_header(void)
{
	xil_printf("UDP server listening on port %d\r\n",
			UDP_CONN_PORT);
}

// Global Variables for Ethernet handling
u16_t    			RemotePort = 29000;
struct ip4_addr  	RemoteAddr;
struct udp_pcb 		send_pcb;


extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;

struct netif server_netif;

void start_application(void);
void print_app_header(void);
void sendUDPEcho(void);

static void print_ip(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\r\n", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	print_ip("Board IP:       ", ip);
	print_ip("Netmask :       ", mask);
	print_ip("Gateway :       ", gw);
}

static void assign_default_ip(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	int err;

	xil_printf("Configuring default IP %s \r\n", DEFAULT_IP_ADDRESS);

	err = inet_aton(DEFAULT_IP_ADDRESS, ip);
	if (!err)
		xil_printf("Invalid default IP address: %d\r\n", err);

	err = inet_aton(DEFAULT_IP_MASK, mask);
	if (!err)
		xil_printf("Invalid default IP MASK: %d\r\n", err);

	err = inet_aton(DEFAULT_GW_ADDRESS, gw);
	if (!err)
		xil_printf("Invalid default gateway address: %d\r\n", err);
}

struct netif *netif;
void init_ETH_UDP(void)
{

	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] = {
		0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	netif = &server_netif;
	
	xil_printf("\r\n\r\n");
	xil_printf("-----lwIP RAW Mode UDP Server Application-----\r\n");


	/* initialize lwIP */
	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, NULL, NULL, NULL, mac_ethernet_address,
				PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\r\n");
		return;
	}
	netif_set_default(netif);


	/* specify that the network if is up */
	netif_set_up(netif);
	assign_default_ip(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
	print_ip_settings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));

	xil_printf("\r\n");

	/* print app header */
	print_app_header();

	/* start the application*/
	start_application();
	xil_printf("\r\n");
}

void loop_ETH_UDP(void)
{
	if (TcpFastTmrFlag) {
		tcp_fasttmr();
		TcpFastTmrFlag = 0;
	}
	if (TcpSlowTmrFlag) {
		tcp_slowtmr();
		TcpSlowTmrFlag = 0;
	}
	xemacif_input(netif);
}



struct pbuf * psend;
void send_UDPData(uint8_t *data, uint16_t len)
{
	psend = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_REF);
	psend->payload = data;
	psend->len = len;
	err_t udpsenderr = udp_sendto(&send_pcb, psend, &RemoteAddr, RemotePort);
	if (udpsenderr != ERR_OK){
		xil_printf("UDP Send failed with Error %d\n\r", udpsenderr);
	}
	pbuf_free(psend);
}

/* recv_callback: function that handles responding to UDP packets */
void recv_callback(void *arg, struct udp_pcb *upcb,
				   struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	if (!p)
	{
		udp_disconnect(upcb);
		return;
	}

	/* Assign the Remote IP:port from the callback on each first pulse */
	RemotePort = port;
	RemoteAddr = *addr;

	/* Keep track of the control block so we can send data back in the main while loop */
	send_pcb = *upcb;

	static int32_t index = 0;
	if (index == QUE_BUFFER_SIZE)
	{
		index = 0;
	}

	memcpy(rxEthData[index], p->payload, p->len);
	putEthQue(rxEthData[index], p->len);
	index++;

	/* free the received pbuf */
	pbuf_free(p);
	return;
}

void start_application(void)
{
	err_t err;

	/* Create Server PCB */
	pcb = udp_new();
	if (!pcb) {
		xil_printf("UDP server: Error creating PCB. Out of Memory\r\n");
		return;
	}

	err = udp_bind(pcb, IP_ADDR_ANY, UDP_CONN_PORT);
	if (err != ERR_OK) {
		xil_printf("UDP server: Unable to bind to port");
		xil_printf(" %d: err = %d\r\n", UDP_CONN_PORT, err);
		udp_remove(pcb);
		return;
	}

	/* specify callback to use for incoming connections */
	udp_recv(pcb, recv_callback, NULL);

	return;
}

void sendACK(uint8_t messageType)
{
	uint8_t bufferTemp[8] = {MESSAGE_START_CODE_7E, MESSAGE_START_CODE_7E, 0x04, 0x00, 0x01, VERSION_MINOR, 0x00, 0x00};

	bufferTemp[3] = messageType;
	bufferTemp[6] = bufferTemp[0] ^ bufferTemp[2] ^ bufferTemp[4];
	bufferTemp[7] = bufferTemp[1] ^ bufferTemp[3] ^ bufferTemp[5];

	send_UDPData(bufferTemp, 8);

#ifdef _DEBUG_
	xil_printf("\r\n Sned Ack : ");
	for (int i = 0; i < 8; i++)
		xil_printf("%x ", bufferTemp[i]);
	xil_printf("\r\n");
#endif
}

void sendNACK(uint8_t messageType)
{
	uint8_t bufferTemp[8] = {MESSAGE_START_CODE_7E, MESSAGE_START_CODE_7E, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00};

	bufferTemp[3] = messageType;
	bufferTemp[6] = bufferTemp[0] ^ bufferTemp[2] ^ bufferTemp[4];
	bufferTemp[7] = bufferTemp[1] ^ bufferTemp[3] ^ bufferTemp[5];
	send_UDPData(bufferTemp, 8);
}
