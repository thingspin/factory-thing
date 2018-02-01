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
#include "transport.h"
#include "client-config.h"

static int sock = 0;

extern int beStop;
extern UAMQ_Configuration* g_config;

int mqtt_connect(int argc, char *argv[])
{
	int rc = 0;
	int len = 0;

	unsigned char buf[128];
	int buflen = sizeof(buf);

	char* host = g_config->mqttBrockerIP;
	int port = g_config->mqttBrockerPORT;
	
	sock = transport_open(host, port);
	if(sock < 0) {
		printf("open mqtt trasport (hostname '%s' port %d) ==> failed.\n", host, port);
		return sock;
	} else {
		printf("open mqtt trasport (hostname '%s' port %d) ==> ok.\n", host, port);
	}

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.clientID.cstring = "public";
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	data.username.cstring = "";
	data.password.cstring = "";

	len = MQTTSerialize_connect(buf, buflen, &data);
	rc = transport_sendPacketBuffer(sock, buf, len);

	if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
	{
		unsigned char sessionPresent, connack_rc;

		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
		{
			printf("Unable to connect, return code %d\n", connack_rc);
			return -1;
		}
	}
	else {
		printf("mqtt connection info read failed.\n");
		return -1;
	}

	printf("mqtt brocker connected successfully.\n");

	return 0;
}

int mqtt_publish(const char* mode, char* topic, const char* value) 
{
	printf("[mqtt] publish (%s) %s\t%s \n", mode, topic, value);
	
	MQTTString topicString = MQTTString_initializer;
	
	unsigned char buf[200] = {0, };
	int buflen = sizeof(buf);
	int len = 0;

	topicString.cstring = topic;
#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wcast-qual"
	len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)value, strlen(value));
#pragma GCC diagnostic pop 
	int rc = transport_sendPacketBuffer(sock, buf, len);

	return rc;
}

int mqtt_main(int argc, char *argv[])
{
	int rc = 0;
	
	if(!g_config->mqttEnable) {
		return 0;
	}

	do {
		rc = mqtt_connect(argc, argv);
	} while(!beStop && rc < 0 );


	MQTTString topicString = MQTTString_initializer;
	unsigned char buf[256];
	int buflen = sizeof(buf);
	
	char* payload = "hello";
	int payloadlen = strlen(payload);
	int len = 0;

	topicString.cstring = "sensor";
	while (!beStop)
	{
		#if 0
		printf("1\n");
		if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBLISH)
		{
			printf("2\n");
			unsigned char dup;
			int qos;
			unsigned char retained;
			unsigned short msgid;
			int payloadlen_in;
			unsigned char* payload_in;
			int rc;
			MQTTString receivedTopic;

			printf("3\n");

			rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic, &payload_in, &payloadlen_in, buf, buflen);
			printf("message arrived %.*s\n", payloadlen_in, payload_in);
			usleep(500000);
		}
		#endif
		sleep(3);

		// printf("publish: %s --> %s\n", topicString.cstring, payload);
		// len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
		// rc = transport_sendPacketBuffer(sock, buf, len);
	}

	printf("disconnecting\n");
	len = MQTTSerialize_disconnect(buf, buflen);
	rc = transport_sendPacketBuffer(sock, buf, len);

exit:
	transport_close(sock);

	return 0;
}

void* mqtt_run(void* param)
{
	return (void*)mqtt_main(0, 0);
}
