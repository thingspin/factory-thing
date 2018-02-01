#ifndef OPCUA_MQTT_BRIDGE_MAP_H_
#define OPCUA_MQTT_BRIDGE_MAP_H_

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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

#include <map>

struct Group;

typedef struct Node {
	char* id;
	char* topic;
	char* alias;
	UA_NodeId ua;
	Group* parent;
} Node;

typedef struct Group {
	char* name;
	char* method;
	char* topic;
	char* format;
	int intervalUSec;
	bool mqtt;
	bool amqp;
	bool tcp;
	bool enable;
	map<int, Node> nodes;
} Group;

enum enumMonitorMode { 
	enumEvent, 
	enumPoll
};

enumMonitorMode getMonitorMode(char*);


enum enumPayloadFormat { 
	enumJSON,
	enumKeyVal
};

enumPayloadFormat getPayloadFormat(char*);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OPCUA_MQTT_BRIDGE_MAP_H_ */