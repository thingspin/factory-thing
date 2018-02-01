#ifndef OPCUA_MQTT_BRIDGE_COMMON_H_
#define OPCUA_MQTT_BRIDGE_COMMON_H_

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


#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "pub.h"
#include "client-config.h"

UA_StatusCode opcua_server_connect(UA_Client *client);
UA_StatusCode opcua_server_browse(UA_Client *client);

void monitor_start(UA_Client* client);

int mqtt_publish(const char* mode, char* topic, const char* value);
int amqp_publish(const char* mode, char* topic, const char* value);
int tcp_publish(const char* mode, char* topic, const char* value);

void* mqtt_run(void* param);
void* tcp_run(void* param);
void* amqp_run(void* param);
void* opcua_poll(void* param);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OPCUA_MQTT_BRIDGE_COMMON_H_ */