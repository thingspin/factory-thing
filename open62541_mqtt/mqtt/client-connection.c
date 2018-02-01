/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

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

#include <stdio.h>
#include "client-config.h"

extern int beStop;
extern UAMQ_Configuration g_Configutation;

UA_StatusCode opcua_server_connect(UA_Client *client)
{
    UA_EndpointDescription* endpoints = NULL;

    printf(">> connect opc.ua server ... to %s\n", g_Configutation.uaServerAddress);

    size_t epsize = 0;
    UA_StatusCode retval = UA_Client_getEndpoints(client, g_Configutation.uaServerAddress, &epsize, &endpoints);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Array_delete(endpoints, epsize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        UA_Client_delete(client);
        return (int)retval;
    }

    printf("%i endpoints found\n", (int)epsize);
    for(size_t i = 0; i < epsize; i++) {
        printf("URL of endpoint %i is %.*s\n", (int)i,
               (int)endpoints[i].endpointUrl.length,
               endpoints[i].endpointUrl.data);
    }
    UA_Array_delete(endpoints, epsize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);

    retval = UA_Client_connect(client, g_Configutation.uaServerAddress);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return (int)retval;
    }

    return retval;
}