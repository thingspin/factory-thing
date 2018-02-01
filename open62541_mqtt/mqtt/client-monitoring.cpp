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
# include <stdio.h>

#endif

#include <ctime>
#include <iostream>
#include <sstream>
#include <iterator>
#include <map>
using namespace std;

#include "client-nodemap.h"
#include "client-nodeid.h"
#include "MQTTPacket.h"
#include "client-common.h"
#include "json.h"

extern int beStop;

extern map<int, Group>* gmap;
extern UAMQ_Configuration* g_config;

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include <vector>

int64_t epoch(void) {

    struct timespec tms;

    /* POSIX.1-2008 way */
    if (clock_gettime(CLOCK_REALTIME,&tms)) {
        return -1;
    }
    /* seconds, multiplied with 1 million */
    int64_t micros = tms.tv_sec * 1000000;
    /* Add full microseconds */
    micros += tms.tv_nsec/1000;
    /* round up if necessary */
    if (tms.tv_nsec % 1000 >= 500) {
        ++micros;
    }

    return micros;
}

static void callback(UA_UInt32 mid, UA_DataValue *data, void *context) {

    if(!data->hasValue) {
        return;
    }

    Node* d = (Node*)context;
    Group* p = d->parent;

    UA_Variant val = data->value;
    const UA_DataType* type = val.type;

    static char topic[64] = {0,};
    static char payload[256] = {0,};
    static char value[32] = {0,};

    json_object* jobj = json_object_new_object();

    switch(type->typeIndex) {
        case UA_TYPES_BOOLEAN : json_object_object_add(jobj, d->alias, json_object_new_boolean((*(UA_Boolean*)val.data))); sprintf(&value[0], "%s", (*(UA_Boolean*)val.data) == true ? "true" : "false"); break;
        case UA_TYPES_SBYTE : json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_SByte*)val.data))); sprintf(&value[0], "%d", (*(UA_SByte*)val.data)); break;
        case UA_TYPES_BYTE : json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_Byte*)val.data))); sprintf(&value[0], "%d", (*(UA_Byte*)val.data)); break;
        case UA_TYPES_INT16 : json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_Int16*)val.data))); sprintf(&value[0], "%d", (*(UA_Int16*)val.data)); break;
        case UA_TYPES_UINT16 : json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_UInt16*)val.data))); sprintf(&value[0], "%d", (*(UA_UInt16*)val.data)); break;
        case UA_TYPES_INT32 : json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_Int32*)val.data))); sprintf(&value[0], "%d", (*(UA_Int32*)val.data)); break;
        case UA_TYPES_UINT32 : json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_UInt32*)val.data))); sprintf(&value[0], "%d", (*(UA_UInt32*)val.data)); break;
        case UA_TYPES_INT64 : json_object_object_add(jobj, d->alias, json_object_new_int64((*(UA_Int64*)val.data))); sprintf(&value[0], "%ld", (*(UA_Int64*)val.data)); break;
        case UA_TYPES_UINT64 : json_object_object_add(jobj, d->alias, json_object_new_int64((*(UA_UInt64*)val.data))); sprintf(&value[0], "%lu", (*(UA_UInt64*)val.data)); break;
        case UA_TYPES_FLOAT : json_object_object_add(jobj, d->alias, json_object_new_double((*(UA_Float*)val.data))); sprintf(&value[0], "%f", (*(UA_Float*)val.data)); break;
        case UA_TYPES_DOUBLE : json_object_object_add(jobj, d->alias, json_object_new_double((*(UA_Double*)val.data))); sprintf(&value[0], "%f", (*(UA_Double*)val.data)); break;
        case UA_TYPES_STRING : {
            if(0 != (*(UA_String*)val.data).length ) {
            sprintf(&value[0], "%s", ((*(UA_String*)val.data).data));
            json_object_object_add(jobj, d->alias, json_object_new_string(value)); 
            sprintf(&value[0], "\"%s\"", ((*(UA_String*)val.data).data));
            }
        }
        break;
        default : {
            printf("not supported dataType : %s, typeIndex:%d\n", val.type->typeName, val.type->typeIndex);
        }
    }

    sprintf(topic, "%s/%s/%s/%s", g_config->topicBase, g_config->deviceID, p->topic, d->topic);
    
    bool bEmpty = true;
    json_object_object_foreach(jobj, key, v) {
        bEmpty = false;
    }

    if(!bEmpty) {
        int64_t t = epoch();
        json_object_object_add(jobj, "time", json_object_new_int64(t));
        sprintf(payload, "%s=%ld, %s=%s", "time", t, d->alias, value);

        const char* contents = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
        
        switch(getPayloadFormat(p->format)) {
            case enumJSON : {
                if(p->mqtt) mqtt_publish("event", topic, contents);
                //if(p->amqp) amqp_publish("event", topic, contents);
                if(p->tcp) tcp_publish("event", topic, contents);
            }
            break;
            case enumKeyVal : {
                if(p->mqtt) mqtt_publish("event", topic, payload);
                //if(p->amqp) amqp_publish("event", topic, payload);
                if(p->tcp) tcp_publish("event", topic, payload);
            }
            break;

            default: {

            }
            break;
        }        
    }

    json_object_put(jobj);
}

void monitor_start(UA_Client* client)
{
    if(!g_config->asycRequestSupported && (getMonitorMode(g_config->method) == enumPoll)) {
        cout << "\n[EVENT MODE] DISCARDED.\n";
        return;
    }

    UA_UInt32 subId = 0;
    UA_Client_Subscriptions_new(client, UA_SubscriptionSettings_standard, &subId);
    if(subId)
        printf("Create subscription succeeded, id %u\n", subId);

    printf("\n[EVENT MODE]\n");
	map<int, Group>::iterator i;
	for (i = gmap->begin(); i != gmap->end(); ++i) {
		Group* p = (Group*)&i->second;

        if(getMonitorMode(p->method) == enumEvent) {
            cout << "\t[" << i->first << "] name: \"" << p->name << "\", enable: " << p->enable << ", method: " << p->method << ", interval(us): " << p->intervalUSec << ", mqtt: " << p->mqtt << ", tcp: " << p->tcp << "\n";

            if(!p->enable) {
                continue;
            }
            map<int, Node>::iterator n;
            for (n = p->nodes.begin(); n != p->nodes.end(); ++n) {
                Node* d = (Node*)&n->second;
                d->parent = p;
                cout << "\t\t[" << n->first << "] id: " << d->id << ", topic: " <<  d->topic << ", alias: " << d->alias << "\n";

                UA_NodeIdType type = getUA_NodeID(d->id, &d->ua);

                UA_UInt32 monId = 0; 
                UA_Client_Subscriptions_addMonitoredItem(client, subId, d->ua, UA_ATTRIBUTEID_VALUE, &callback, d, &monId);
                if (!monId) {
                    switch(d->ua.identifierType) {
                        case UA_NODEIDTYPE_STRING : {
                            printf("Monitoring id %u for %s ==> FAILED.\n", subId, d->ua.identifier.string.data);
                        }
                        break;
                        case UA_NODEIDTYPE_NUMERIC : {
                            printf("Monitoring id %u for %d ==> FAILED.\n", subId, d->ua.identifier.numeric);
                        }
                        break;
                        default: {
                            printf("Monitoring FAILED.\n");
                            break;
                        }
                    }
                }
            }
        }
	}
}

void* opcua_poll_group(void* param)
{
    UA_Client* client = (UA_Client*)g_config->client;
    Group* p = (Group*)param;

    do {
        json_object* jobj = json_object_new_object();
        vector<char*> kvs;

        map<int, Node>::iterator n;
        for (n = p->nodes.begin(); n != p->nodes.end(); ++n) {
            Node* d = (Node*)&n->second;

            UA_Variant *val = UA_Variant_new();
            UA_StatusCode retval = UA_Client_readValueAttribute(client, d->ua, val);

            if(retval != UA_STATUSCODE_GOOD) {
                UA_Variant_delete(val);
                printf("read failed.\n");
                usleep(p->intervalUSec);

				UA_StatusCode state = UA_STATUSCODE_GOOD;

				do {
					state = opcua_server_connect(client);
					if (state == UA_STATUSCODE_GOOD) {
						state = opcua_server_browse(client);
						
					} else {
						sleep(2);
						printf("OPC UA Server connect failed.");
					}
				} while(state != UA_STATUSCODE_GOOD);
                
				continue;
            }

            // typedef struct {
            //     const UA_DataType *type;      /* The data type description */
            //     UA_VariantStorageType storageType;
            //     size_t arrayLength;           /* The number of elements in the data array */
            //     void *data;                   /* Points to the scalar or array data */
            //     size_t arrayDimensionsSize;   /* The number of dimensions */
            //     UA_UInt32 *arrayDimensions;   /* The length of each dimension */
            // } UA_Variant;
            // UA_Boolean isScalar = UA_Variant_isScalar(val);
            // if(!isScalar) {
            //     printf(" ---- NOT SCALAR ----\n");
            //     //arrayDimensionsSize
            //     printf("\t arrayLength : %d\n", (int)val->arrayLength);

            //     for(int i = 0; i < (int)val->arrayLength; i++) {
            //         UA_UInt32 as = val->arrayDimensions[i];
            //     }
            // }


            char value[512] = {0,};

            const UA_DataType* type = val->type;

            switch(type->typeIndex) {
                case UA_TYPES_BOOLEAN : {
                    json_object_object_add(jobj, d->alias, json_object_new_boolean((*(UA_Boolean*)val->data))); 
                    sprintf(&value[0], "%s", (*(UA_Boolean*)val->data) == true ? "true" : "false");
                }
                break;
                case UA_TYPES_SBYTE : {
                    json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_SByte*)val->data))); 
                    sprintf(&value[0], "%d", (*(UA_SByte*)val->data));
                }
                break;
                case UA_TYPES_BYTE : {
                    json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_Byte*)val->data))); 
                    sprintf(&value[0], "%d", (*(UA_Byte*)val->data));
                }
                break;
                case UA_TYPES_INT16 : {
                    json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_Int16*)val->data))); 
                    sprintf(&value[0], "%d", (*(UA_Int16*)val->data)); 
                }
                break;
                case UA_TYPES_UINT16 : {
                    json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_UInt16*)val->data))); 
                    sprintf(&value[0], "%d", (*(UA_UInt16*)val->data)); 
                }
                break;
                case UA_TYPES_INT32 : {
                    json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_Int32*)val->data))); 
                    sprintf(&value[0], "%d", (*(UA_Int32*)val->data)); 
                }
                break;
                case UA_TYPES_UINT32 : {
                    json_object_object_add(jobj, d->alias, json_object_new_int((*(UA_UInt32*)val->data))); 
                    sprintf(&value[0], "%d", (*(UA_UInt32*)val->data)); 
                }
                break;
                case UA_TYPES_INT64 : {
                    json_object_object_add(jobj, d->alias, json_object_new_int64((*(UA_Int64*)val->data))); 
                    sprintf(&value[0], "%ld", (*(UA_Int64*)val->data)); 
                }
                break;
                case UA_TYPES_UINT64 : {
                    json_object_object_add(jobj, d->alias, json_object_new_int64((*(UA_UInt64*)val->data))); 
                    sprintf(&value[0], "%ld", (*(UA_UInt64*)val->data)); 
                }
                break;
                case UA_TYPES_FLOAT : {
                    json_object_object_add(jobj, d->alias, json_object_new_double((*(UA_Float*)val->data))); 
                    sprintf(&value[0], "%f", (*(UA_Float*)val->data)); 
                }break;
                case UA_TYPES_DOUBLE : {
                    json_object_object_add(jobj, d->alias, json_object_new_double((*(UA_Double*)val->data))); 
                    sprintf(&value[0], "%f", (*(UA_Double*)val->data));
                }
                break;
                case UA_TYPES_STRING : {
                    if(0 != (*(UA_String*)val->data).length ) {
                        sprintf(&value[0], "%s", ((*(UA_String*)val->data).data));
                        json_object_object_add(jobj, d->alias, json_object_new_string(value));
                        sprintf(&value[0], "\"%s\"", ((*(UA_String*)val->data).data));
                    }
                }
                break;
                default : {
                    printf("not supported dataType : %s, typeIndex:%d\n", val->type->typeName, val->type->typeIndex);
                    UA_Variant_delete(val);
                    usleep(p->intervalUSec);
                    continue;
                }
            }
            UA_Variant_delete(val);

            static char skv[32] = {0,};
            sprintf(skv, "%s=%s", d->alias, value);
            kvs.push_back(strndup(skv, strlen(skv)));
        }

        static char topic[64] = {0,};
        sprintf(topic, "%s/%s/%s", g_config->topicBase, g_config->deviceID, p->topic);

        bool bEmpty = true;
        json_object_object_foreach(jobj, key, val) {
            bEmpty = false;
        }

        if(!bEmpty) {
            int64_t t = epoch();
            json_object_object_add(jobj, "time", json_object_new_int64(t));

            static char skv[32] = {0,};
            sprintf(skv, "%s=%lu", "time", t);
            kvs.push_back(strndup(skv, strlen(skv)));

            static char payload[512] = {0,};

            ostringstream ss;

            copy(kvs.begin(), kvs.end() - 1, ostream_iterator<char*>(ss, ", "));
            ss << kvs.back();

            const char* contents = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
            switch(getPayloadFormat(p->format)) {
                case enumJSON : {
                    if(p->mqtt) mqtt_publish("poll", topic, contents);
                    //if(p->amqp) amqp_publish("poll", topic, contents);
                    if(p->tcp)
                    {
                        char* contents2 = const_cast<char*>(json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN));
                        strcat(contents2,"\n");
                        tcp_publish("poll", topic, contents2);
                    }
                }
                break;
                case enumKeyVal : {
                    if(p->mqtt) mqtt_publish("poll", topic, ss.str().c_str());
                    //if(p->amqp) amqp_publish("poll", topic, ss.str().c_str());
                    if(p->tcp) tcp_publish("poll", topic, ss.str().c_str());
                }
                break;

                default: {
                }
                break;
            }  
        }

        json_object_put(jobj);
        for ( size_t i = 0; i < kvs.size(); i++)
        {
            delete kvs[i];
        }
        vector<char*>().swap(kvs);

        usleep(p->intervalUSec);

    } while (!beStop);

    return NULL;

}

void* opcua_poll(void* param)
{
    if(!g_config->asycRequestSupported && (getMonitorMode(g_config->method) == enumEvent)) {
        cout << "\n[POLL MODE] DISCARDED.\n";
        return NULL;
    }

    UA_Client* client = (UA_Client*)param;

    printf("\n[POLL MODE]\n");
    map<int, Group>::iterator i;
	for (i = gmap->begin(); i != gmap->end(); ++i) {
		Group* p = (Group*)&i->second;

        if(getMonitorMode(p->method) == enumPoll) {
            cout << "\t[" << i->first << "] name: \"" << p->name << "\", enable: " << p->enable << ", method: " << p->method << ", interval(us): " << p->intervalUSec << ", mqtt: " << p->mqtt << ", tcp: " << p->tcp << "\n";
            if(!p->enable) {
                continue;
            }

            map<int, Node>::iterator n;
            for (n = p->nodes.begin(); n != p->nodes.end(); ++n) {
                Node* d = (Node*)&n->second;
                d->parent = p;

                cout << "\t\t[" << n->first << "] id: " << d->id << ", topic: " <<  d->topic << ", alias: " << d->alias << "\n";

                UA_NodeIdType type = getUA_NodeID(d->id, &d->ua);
            }
        }
	}

    for (i = gmap->begin(); i != gmap->end(); ++i) {
        Group* p = (Group*)&i->second;

        if(getMonitorMode(p->method) == enumPoll) {
            if(!p->enable) {
                continue;
            }

            pthread_t tid = 0;
            void* s = NULL;
            int th = pthread_create(&tid, NULL, opcua_poll_group, (void*)p);               
        }
    }

    return NULL;
}