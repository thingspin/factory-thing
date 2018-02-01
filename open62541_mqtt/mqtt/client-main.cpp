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
#include <signal.h>

#include <iostream>
using namespace std;

#include "MQTTPacket.h"
#include "client-common.h"
#include "client-trans-tcp.h"

int beStop = 0;

extern UAMQ_Configuration* g_config;

void signal_finish(int sig)
{
	signal(SIGINT, NULL);
	beStop = 1;
}

void signal_stop(void)
{
	signal(SIGINT, signal_finish);
	signal(SIGTERM, signal_finish);
    signal(SIGPIPE, SIG_IGN);
}

int main(int argc, char *argv[]) {

    signal_stop();

	if(config(argc, argv) != (int)UA_STATUSCODE_GOOD) {
		return (int) UA_STATUSCODE_GOOD;
	}

    UA_Client *client = NULL;
    g_config->client = NULL;

    UA_StatusCode state = UA_STATUSCODE_GOOD;

    do {
        g_config->client = client = UA_Client_new(UA_ClientConfig_standard);

        state = opcua_server_connect(client);
        if(state == UA_STATUSCODE_GOOD) {
            state = opcua_server_browse(client);
        } else {
			sleep(2);
            printf("retry connect to opcua server.\n");
		}
    } while(!beStop && state != UA_STATUSCODE_GOOD);

    monitor_start(client);

    pthread_t tid1 = 0;
    void* s1 = NULL;
	int th1 = pthread_create(&tid1, NULL, mqtt_run, (void*)client);

    pthread_t tid2 = 0;
    void* s2 = NULL;
	int th2 = pthread_create(&tid2, NULL, tcp_run, (void*)client);

    pthread_t tid3 = 0;
    void* s3 = NULL;
	//int th3 = pthread_create(&tid3, NULL, amqp_run, (void*)client);

    pthread_t tid4 = 0;
    void* s4 = NULL;
	int th4 = pthread_create(&tid4, NULL, opcua_poll, (void*)client);

	while (!beStop)
	{
        usleep(g_config->uaPublishIntervalUsecs);

        if(!g_config->asycRequestSupported && (!strncmp(g_config->method, "poll", strlen(g_config->method)))) {
            sleep(2);
            continue;
        } else {
            UA_Client_Subscriptions_manuallySendPublishRequest(client);
        }
    }

    pthread_cancel(tid1);
    pthread_cancel(tid2);
    pthread_cancel(tid3);
	pthread_join(tid1, &s1);
	pthread_join(tid2, &s2);
    pthread_join(tid3, &s3);

    UA_Client_disconnect(client);
    UA_Client_delete(client);

    printf("stopped.\n");

    return (int) UA_STATUSCODE_GOOD;
}
