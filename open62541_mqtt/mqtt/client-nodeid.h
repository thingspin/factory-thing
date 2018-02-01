#ifndef OPCUA_MQTT_BRIDGE_NODEID_H_
#define OPCUA_MQTT_BRIDGE_NODEID_H_

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

UA_NodeIdType getUA_NodeID(char*, UA_NodeId*);


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OPCUA_MQTT_BRIDGE_NODEID_H_ */