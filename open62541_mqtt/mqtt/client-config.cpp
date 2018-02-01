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

#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include <iostream>
#include <list>
#include <map>
using namespace std;

#include "MQTTPacket.h"
#include "pub.h"
#include "client-config.h"
#include "json.h"
#include "client-nodemap.h"

extern int beStop;

map<int, Group> m;
map<int, Group>* gmap = &m;

UAMQ_Configuration g_Configutation;
UAMQ_Configuration* g_config = &g_Configutation;

int load(char* fn);
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);

void uses(char* exe) 
{
	printf("OPC/UA MQTT Bridge Server (version 1.0), MDS Technology.\nuses: %s -c|--config 'filename.json'\n", exe);
}

int config(int argc, char *argv[]) 
{
	if (argc != 3) {
		uses(argv[0]);
		return -1;
	}

	if(strncmp(argv[1], "-c", strlen(argv[1]))) {
		if(strncmp(argv[1], "--config", strlen(argv[1]))) {
			uses(argv[0]);
			return -1; 
		}
	}

	strcpy(&g_Configutation.configFile[0], argv[2]);

	if(load(&g_Configutation.configFile[0]) != UA_STATUSCODE_GOOD) {
		printf("[error] the file (%s) has invalid json syntax or path is not valid.\n", &g_Configutation.configFile[0]);
		return -1;
	}

    return (int)UA_STATUSCODE_GOOD;
}

void travers(json_object *parent, json_object *r, enum json_type t)
{
	enum json_type type;

	switch(t) {
		case json_type_object: {

			json_object_object_foreach(r, key, val) {
				printf("key:%s > ", key);

				type = json_object_get_type(val);
				switch(type) {
					case json_type_null: printf("2 %s\n", "null"); break;
					case json_type_boolean: printf("2 %s=%d\n", "boolean", json_object_get_boolean(val)); break;
					case json_type_double: printf("2 %s=%lf\n", "double", json_object_get_double(val)); break;
					case json_type_int: printf("2 %s=%d\n", "int", json_object_get_int(val)); break;
					case json_type_object: travers(r, val, json_type_object); break;
					case json_type_array: travers(r, val, json_type_array); break;
					case json_type_string: printf("2 %s=%s\n", "string", json_object_get_string(val)); break;
					default :  printf("2 %s\n", "unknown"); break;
				}
			}
		}
		break;
		case json_type_array: {
			int l = json_object_array_length(r);

			for (int i = 0; i < l; i++) {
				json_object *val = json_object_array_get_idx(r, i);

				enum json_type type;
				type = json_object_get_type(val);
				switch(type) {
					case json_type_null: printf("3 %s\n", "null"); break;
					case json_type_boolean: printf("3 %s=%d\n", "boolean", json_object_get_boolean(val)); break;
					case json_type_double: printf("3 %s=%lf\n", "double", json_object_get_double(val)); break;
					case json_type_int: printf("3 %s=%d\n", "int", json_object_get_int(val)); break;
					case json_type_object: travers(r, val, json_type_object); break;
					case json_type_array: travers(r, val, json_type_array); break;
					case json_type_string: printf("3 %s=%s\n", "string", json_object_get_string(val)); break;
					default : printf("3 %s\n", "unknown"); break;
				}
			}
		}
		break;
		default: {
			return;
		}
	}
}

//===================================================================================================================================================================
void make_node_list(int group, int depth, json_object *parent, json_object *r)
{
	enum json_type type;

	printf("make node list >> [G:%d/D:%d]\n",  group, depth);

	int l = json_object_array_length(r);

	for (int i = 0; i < l; i++) {
		json_object *val = json_object_array_get_idx(r, i);

		enum json_type type;
		type = json_object_get_type(val);
		switch(type) {
			case json_type_null: printf("%d/%d %s\n", group, i, "null"); break;
			case json_type_boolean: printf("%d/%d %s=%d\n", group, i, "boolean", json_object_get_boolean(val)); break;
			case json_type_double: printf("%d/%d %s=%lf\n", group, i,  "double", json_object_get_double(val)); break;
			case json_type_int: printf("%d/%d %s=%d\n", group, i, "int",  json_object_get_int(val)); break;
			case json_type_string: printf("%d/%d %s=%s\n", group, i,"string",   json_object_get_string(val)); break;

			case json_type_object: {
				printf("%d/%d %s\n",  group, i, " ===> object");
				//make_group(group, depth + 1, i, r, val);
			}
			break;

			case json_type_array: {
				printf("%d/%d %s\n",  group, i, "===> array");
				//make_node_list(group, depth + 1, r, val);
			}
			break;
			
			default :  printf("%d/%d %s\n",  group, i, "unknown"); break;
		}
	}
}

int make_group(json_object *r, Group* G)
{
	enum json_type type;
	int field = 0;

	json_object_object_foreach(r, key, val) {

		if(!strncmp(key, "name", strlen(key))) {
			G->name = strndup(json_object_get_string(val), strlen(json_object_get_string(val)));
		} else if(!strncmp(key, "method", strlen(key))) {
			G->method = strndup(json_object_get_string(val), strlen(json_object_get_string(val)));
		} else if(!strncmp(key, "intervalUSec", strlen(key))) {
			G->intervalUSec = json_object_get_int(val);
		} else if(!strncmp(key, "topic", strlen(key))) {
			G->topic = strndup(json_object_get_string(val), strlen(json_object_get_string(val)));
		}  else if(!strncmp(key, "format", strlen(key))) {
			G->format = strndup(json_object_get_string(val), strlen(json_object_get_string(val)));
		} else if(!strncmp(key, "mqtt", strlen(key))) {
			G->mqtt = json_object_get_boolean(val);
		} else if(!strncmp(key, "amqp", strlen(key))) {
			G->amqp = json_object_get_boolean(val);
		} else if(!strncmp(key, "tcp", strlen(key))) {
			G->tcp = json_object_get_boolean(val);
		} else if(!strncmp(key, "enable", strlen(key))) {
			G->enable = json_object_get_boolean(val);
		} else if(!strncmp(key, "nodes", strlen(key))) {

			type = json_object_get_type(val);

			switch(type) {
				case json_type_array: {

					Node n;

					int l = json_object_array_length(val);

					for (int i = 0; i < l; i++) {
						json_object *node = json_object_array_get_idx(val, i);

						type = json_object_get_type(node);

						switch(type) {
							case json_type_object: {
								json_object_object_foreach(node, field, v) {
									
									if(!strncmp(field, "id", strlen(field))) {
										n.id = strndup(json_object_get_string(v), strlen(json_object_get_string(v)));
									} else if(!strncmp(field, "topic", strlen(field))) {
										n.topic = strndup(json_object_get_string(v), strlen(json_object_get_string(v)));
									} else if(!strncmp(field, "alias", strlen(field))) {
										if(strlen(json_object_get_string(v)) == 0) {
											n.alias = strndup(n.topic, strlen(n.topic));
										} else {
											n.alias = strndup(json_object_get_string(v), strlen(json_object_get_string(v)));
										}
									}
								}
							}
							break;

							default :  {
								printf("%d %s\n", i, " ===> other");
							}
							break;
						}
						cout << "\t[" << i << "] id: " << n.id << ", topic: " << n.topic << ", alias: " << n.alias << "\n";
						G->nodes.insert(pair<int, Node>(i, n));
					}
				}
				break;
				default : {
					printf("%s\n", " ===> other"); 
					break;
				}
			}
		} else {
			return -1;
		}
	}

	return 0;
}

int load(char* fn)
{
	char exe[256] = {0, };
	ssize_t count = readlink( "/proc/self/exe", exe, 256 );

	char *folder = NULL;
	char *slash = NULL;
	slash = strrchr(exe, '/'); 
	folder = strndup(exe, strlen(exe) - strlen(slash));
	
	strcpy(&g_Configutation.configFolder[0], folder);
	
	char configFn[256] = {0, };

	if(!strncmp(fn, "/", strlen("/"))) {
		strcpy(configFn, g_Configutation.configFile);
	} else {
		sprintf(configFn, "%s/%s", folder, fn);
		sprintf(g_Configutation.configFile, "%s", configFn);
	}

	free(folder);

	#define CHUNK 1024 * 1024
	char data[CHUNK] = {0,};
	FILE *file;

	file = fopen(configFn, "r");
	if (file) {
		size_t nread;
		while ((nread = fread(data, 1, sizeof data, file)) > 0) {
			//fwrite(data, 1, nread, stdout);

			//allowed first 1MB only.
			break;
		}

		if (ferror(file)) {
			/* deal with error */
		}
		fclose(file);
	} else {
		return -1;
	}

	json_object *jobj = json_tokener_parse(data);
	json_object *o = NULL;
	json_object *c = NULL;
	json_object *v = NULL;

	bool b = json_object_object_get_ex(jobj, "device-configuration", &o);
	if(b) {
		printf("[[[ %s ]]]\n", "device-configuration");
		if(!json_object_object_get_ex(o, "Device", &c)) {
			return -1;
		}

		if(!json_object_object_get_ex(c, "deviceID", &v)) {
			return -1;
		}
		strcpy(g_Configutation.deviceID, json_object_get_string(v));
	}

	b = json_object_object_get_ex(jobj, "server-configuration", &o);
	if(b) {
		printf("[[[ %s ]]]\n", "server-configuration");
		//travers(jobj, o, json_type_object);

		if(!json_object_object_get_ex(o, "opcuaServer", &c)) {
			return -1;
		}

		if(!json_object_object_get_ex(c, "EndpointURL", &v)) {
			return -1;
		}
		strcpy(g_Configutation.uaServerAddress, json_object_get_string(v));

		if(!json_object_object_get_ex(c, "publishIntervalUs", &v)) {
			return -1;
		}
		g_Configutation.uaPublishIntervalUsecs = json_object_get_int(v);

		if(!json_object_object_get_ex(c, "asycRequestSupported", &v)) {
			return -1;
		}
		g_Configutation.asycRequestSupported = json_object_get_boolean(v);

		if(!json_object_object_get_ex(c, "method", &v)) {
			return -1;
		}
		strcpy(g_Configutation.method, json_object_get_string(v));

		// MQTT =========
		if(!json_object_object_get_ex(o, "mqttBrocker", &c)) {
			return -1;
		}

		if(!json_object_object_get_ex(c, "enable", &v)) {
			return -1;
		}
		g_Configutation.mqttEnable = json_object_get_boolean(v);

		if(!json_object_object_get_ex(c, "ip", &v)) {
			return -1;
		}
		strcpy(g_Configutation.mqttBrockerIP, json_object_get_string(v));

		if(!json_object_object_get_ex(c, "port", &v)) {
			return -1;
		}
		g_Configutation.mqttBrockerPORT = json_object_get_int(v);

		if(!json_object_object_get_ex(c, "topicBase", &v)) {
			return -1;
		}
		strcpy(g_Configutation.topicBase, json_object_get_string(v));

		// AMQP Rabbit =========
		if(!json_object_object_get_ex(o, "amqpRabbit", &c)) {
			return -1;
		}

		if(!json_object_object_get_ex(c, "enable", &v)) {
			return -1;
		}
		g_Configutation.amqpEnable = json_object_get_boolean(v);

		if(!json_object_object_get_ex(c, "ip", &v)) {
			return -1;
		}
		strcpy(g_Configutation.amqpIP, json_object_get_string(v));

		if(!json_object_object_get_ex(c, "port", &v)) {
			return -1;
		}
		g_Configutation.amqpPORT = json_object_get_int(v);

		if(!json_object_object_get_ex(c, "topicBase", &v)) {
			return -1;
		}
		strcpy(g_Configutation.amqpTopicBase, json_object_get_string(v));

		// TCP ============
		if(!json_object_object_get_ex(o, "tcpSever", &c)) {
			return -1;
		}
		if(!json_object_object_get_ex(c, "ip", &v)) {
			return -1;
		}
		strcpy(g_Configutation.tcpBrockerIP, json_object_get_string(v));

		if(!json_object_object_get_ex(c, "port", &v)) {
			return -1;
		}
		g_Configutation.tcpBrockerPORT = json_object_get_int(v);
		
		if(!json_object_object_get_ex(c, "sampleIntervalUs", &v)) {
			return -1;
		}
		g_Configutation.tcpSampleIntervalUs = json_object_get_int(v);

		if(!json_object_object_get_ex(c, "singleshot", &v)) {
			return -1;
		}
		g_Configutation.singleshot = json_object_get_boolean(v);

		if(!json_object_object_get_ex(c, "enable", &v)) {
			return -1;
		}
		g_Configutation.tcpEnable = json_object_get_boolean(v);
	}

	b = json_object_object_get_ex(jobj, "node-map", &o);
	if(b) {
		printf("[[[ %s ]]]\n", "node-map");

		int l = json_object_array_length(o);

		for (int i = 0; i < l; i++) {
			json_object *n = json_object_array_get_idx(o, i);

			enum json_type type;
			type = json_object_get_type(n);
			switch(type) {
				case json_type_object: {
					printf("[[[ %s : RECORD GROUP(OBJ) #%d ]]]\n", "node-map", i);
					Group g;
					make_group(n, &g);
					m.insert(pair<int, Group>(i, g));
				}
				break;
				default : {
					printf("%s\n", "node-map should be contained object array only.");
					break;
				}
			}
		}
	}

	cout << "\n";

	map<int, Group>::iterator i;
	for (i = m.begin(); i != m.end(); ++i) {
		Group* p = (Group*)&i->second;
		cout << "[" << i->first << "] name: " << p->name << ", method: " << p->method << ", interval(us): " << p->intervalUSec << ", mqtt: " << p->mqtt << ", tcp: " << p->tcp << "\n";

		map<int, Node>::iterator n;
		for (n = p->nodes.begin(); n != p->nodes.end(); ++n) {
			Node* d = (Node*)&n->second;
			cout << "\t[" << n->first << "] id: " << d->id << ", topic: " <<  d->topic << ", alias: " << d->alias << "\n";
		}
	}

	return (int)UA_STATUSCODE_GOOD;
}

enumMonitorMode getMonitorMode(char* method)
{
	if(!strncmp(method, "event", strlen(method))) {
		return enumEvent;
	} else if(!strncmp(method, "poll", strlen(method))) {
		return enumPoll;
	} else {
		return enumPoll;
	}
}

enumPayloadFormat getPayloadFormat(char* format)
{
	if(!strncmp(format, "json", strlen(format))) {
		return enumJSON;
	} else if(!strncmp(format, "kv", strlen(format))) {
		return enumKeyVal;
	} else {
		return enumJSON;
	}	
}