#ifndef OPCUA_MQTT_BRIDGE_CONFIG_H_
#define OPCUA_MQTT_BRIDGE_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif
#pragma once

#ifdef UA_NO_AMALGAMATION
# include "ua_types.h"
# include "ua_client.h"
# include "ua_client_highlevel.h"
# include "ua_nodeids.h"
# include "ua_network_tcp.h"
# include "ua_config_standard.h"
#else
# include "open62541.h"
# include <string.h>
# include <stdlib.h>
#endif

typedef struct {
	char configFile[128];
	char configFolder[128];

	char deviceID[64];
    char uaServerAddress[128];
	int uaPublishIntervalUsecs;
	bool asycRequestSupported;
	char method[32];

	bool mqttEnable;
	char mqttBrockerIP[128];
	int mqttBrockerPORT;
	char topicBase[32];
	
	bool tcpEnable;
	char tcpBrockerIP[128];
	int tcpBrockerPORT;
	int tcpSampleIntervalUs;
	bool singleshot;

	bool amqpEnable;
	char amqpIP[128];
	int amqpPORT;
	char amqpTopicBase[32];

	UA_Client* client;
} UAMQ_Configuration;


int config(int argc, char *argv[]);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OPCUA_MQTT_BRIDGE_CONFIG_H_ */