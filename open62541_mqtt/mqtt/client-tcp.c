/*******************************************************************************
 * Copyright (c) 2017 MDS Technology Ltd.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    lonycell - initial implementation and/or initial documentation
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "MQTTPacket.h"
#include "client-config.h"
#include "client-trans-tcp.h"

static int sock = 0;

extern int beStop;
extern UAMQ_Configuration* g_config;

int tcp_connect(int argc, char *argv[])
{
	char* host = g_config->tcpBrockerIP;
	int port = g_config->tcpBrockerPORT;
	
	sock = tcp_open(host, port);
	if(sock < 0) {
		return sock;
	}

	return 0;
}

int tcp_publish(const char* mode, char* topic, const char* value) 
{
	printf("[tcp] publish (%s) %s\t%s ", mode, topic, value);

#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wcast-qual"

	int rc = tcp_sendPacketBuffer(sock, (unsigned char*)value, strlen(value));

	if(rc < 0) {
		printf(" ==> FAILED");
	}
	printf("\n");

	return rc;

#pragma GCC diagnostic pop 
}

int tcp_main(int argc, char *argv[])
{
	int rc = 0;

	if(!g_config->tcpEnable) {
		return 0;
	}

	printf("\n[tcp] start publisher connection.\n");

	do {
		rc = tcp_connect(argc, argv);
	} while(!beStop && rc < 0 );

	printf("[tcp] start publisher message loop.\n");
	while (!beStop)
	{
		sleep(5);
	}

exit:
	tcp_close(sock);

	return 0;
}

void* tcp_run(void* param)
{
	return (void*)tcp_main(0, 0);
}
