/*
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */
//to compile with single file releases:
// * single-threaded: gcc -std=c99 server.c open62541.c -o server
// * multi-threaded: gcc -std=c99 server.c open62541.c -o server -lurcu-cds -lurcu -lurcu-common -lpthread

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS //disable fopen deprication warning in msvs
#endif

#ifdef UA_NO_AMALGAMATION
# include <time.h>
# include "ua_types.h"
# include "ua_server.h"
# include "ua_config_standard.h"
# include "ua_network_tcp.h"
# include "ua_log_stdout.h"
#else
# include "open62541.h"
#endif

#include <signal.h>
#include <errno.h> // errno, EINTR
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
# include <io.h> //access
#else
# include <unistd.h> //access
#endif

#ifdef UA_ENABLE_MULTITHREADING
# ifdef UA_NO_AMALGAMATION
#  ifndef __USE_XOPEN2K
#   define __USE_XOPEN2K
#  endif
# endif
#include <pthread.h>
#endif

#include "lepton.h"

#include "MQTTPacket.h"
#include "transport.h"

#define BASETOPIC "opcua/mqtt"

extern int sock;

/* ======================================================================================================================= */
UA_LeptonItem leptonBoxes[4] = {0, };
UA_LeptonItem leptonSpots[4] = {0, };
 
UA_Boolean currentValid = false;

/* ======================================================================================================================= */
UA_StatusCode readLepton(void *handle, const UA_NodeId nodeId, UA_Boolean sourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value)
{
	printf("readLepton(%d, %d) %d\r\n", nodeId.namespaceIndex, nodeId.identifier.numeric, currentValid);
    
	if(range) {
        value->hasStatus = true;
        value->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        return UA_STATUSCODE_GOOD;
    }

	// unsigned short int devID = 0;
	// char objName[32];
	// unsigned short int objID = 0;
	// char valName[32];
	// char valS[32];

	UA_DateTime currentTime = UA_DateTime_now();
	
	switch(nodeId.identifier.numeric) {
		case NODEID_BOX1_MIN: {

			// objID = 1;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[0].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX1_MAX: {
			
			// objID = 1;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[0].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX1_AVG: {

			// objID = 1;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[0].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX1_VAL: {

			// objID = 1;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "valid");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[0].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;
		
		case NODEID_BOX2_MIN: {

			// objID = 2;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[1].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX2_MAX: {

			// objID = 2;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[1].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX2_AVG: {

			// objID = 2;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[1].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX2_VAL: {

			// objID = 2;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "valid");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);

			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[1].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;
		
		case NODEID_BOX3_MIN: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[2].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX3_MAX: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[2].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX3_AVG: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[2].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX3_VAL: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "vallid");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[2].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;
		
		case NODEID_BOX4_MIN: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[3].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX4_MAX: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[3].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX4_AVG: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[3].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_BOX4_VAL: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "box");
			// sprintf(&valName[0], "%s", "valid");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);
			UA_Variant_setScalarCopy(&value->value, &leptonBoxes[3].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;
	
		case NODEID_SPOT1_MIN: {
			// objID = 1;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[0].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT1_MAX: {
			// objID = 1;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[0].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT1_AVG: {
			// objID = 1;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[0].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT1_VAL: {
			// objID = 1;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "valid");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[0].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;
		
		case NODEID_SPOT2_MIN: {
			// objID = 2;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[1].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT2_MAX: {
			// objID = 2;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[1].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT2_AVG: {
			// objID = 2;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[1].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT2_VAL: {
			// objID = 2;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "val");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[1].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;
		
		case NODEID_SPOT3_MIN: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[2].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT3_MAX: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[2].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT3_AVG: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[2].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT3_VAL: {
			// objID = 3;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "valid");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[2].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;
		
		case NODEID_SPOT4_MIN: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "min");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].min);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[3].min, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT4_MAX: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "max");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].max);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[3].max, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT4_AVG: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "avg");
			// sprintf(&valS[0], "%f", leptonBoxes[objID-1].avg);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[3].avg, &UA_TYPES[UA_TYPES_FLOAT]);
		}; break;
		case NODEID_SPOT4_VAL: {
			// objID = 4;
			// sprintf(&objName[0], "%s", "spot");
			// sprintf(&valName[0], "%s", "val");
			// sprintf(&valS[0], "%d", leptonBoxes[objID-1].val);
			UA_Variant_setScalarCopy(&value->value, &leptonSpots[3].val, &UA_TYPES[UA_TYPES_BOOLEAN]);
		}; break;

		default : {
			return UA_STATUSCODE_GOOD;
		}
	}
	
	value->hasValue = true;
	if(sourceTimeStamp) {
		value->hasSourceTimestamp = true;
		value->sourceTimestamp = currentTime;
	}
	
	// int rc = 0;
	// MQTTString topicString = MQTTString_initializer;
	// char topic[256];

	// sprintf(&topic[0], "%s/%d/%s/%d/%s", BASETOPIC, devID, objName, objID, valName);
	// topicString.cstring = &topic[0];

	// unsigned char buf[200];
	// int buflen = sizeof(buf);
	
	// char payload[256] = {0,};
	// sprintf(&payload[0], "%s", valS);
	// int payloadlen = strlen(payload);
	// int len = 0;

	// len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
	// rc = transport_sendPacketBuffer(sock, buf, len);

	return UA_STATUSCODE_GOOD;
}
/* ======================================================================================================================= */