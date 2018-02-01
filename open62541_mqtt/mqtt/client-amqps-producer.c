/* vim:set ft=c ts=2 sw=2 sts=2 et cindent: */
/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by Alan Antonuk are Copyright (c) 2012-2013
 * Alan Antonuk. All Rights Reserved.
 *
 * Portions created by Mike Steinert are Copyright (c) 2012-2013
 * Mike Steinert. All Rights Reserved.
 *
 * Portions created by VMware are Copyright (c) 2007-2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Portions created by Tony Garnock-Jones are Copyright (c) 2009-2010
 * VMware, Inc. and Tony Garnock-Jones. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ***** END LICENSE BLOCK *****
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_ssl_socket.h>
#include <amqp_framing.h>

#include "utils.h"

#include "client-common.h"

extern UAMQ_Configuration* g_config;
extern int beStop;
static amqp_connection_state_t conn;
static bool bC = false;
#define SUMMARY_EVERY_US 1000000

static void send_batch(amqp_connection_state_t conn,
                       char const *queue_name,
                       int rate_limit,
                       int message_count)
{
  if(!g_config->amqpEnable) {
    return;
  }

  uint64_t start_time = now_microseconds();
  int i;
  int sent = 0;
  int previous_sent = 0;
  uint64_t previous_report_time = start_time;
  uint64_t next_summary_time = start_time + SUMMARY_EVERY_US;

  char message[256];
  amqp_bytes_t message_bytes;

  for (i = 0; i < (int)sizeof(message); i++) {
    message[i] = i & 0xff;
  }

  message_bytes.len = sizeof(message);
  message_bytes.bytes = message;

  for (i = 0; i < message_count; i++) {
    uint64_t now = now_microseconds();

    die_on_error(amqp_basic_publish(conn,
                                    1,
                                    amqp_cstring_bytes("amq.direct"),
                                    amqp_cstring_bytes(queue_name),
                                    0,
                                    0,
                                    NULL,
                                    message_bytes),
                 "Publishing");
    sent++;
    if (now > next_summary_time) {
      int countOverInterval = sent - previous_sent;
      double intervalRate = countOverInterval / ((now - previous_report_time) / 1000000.0);
      printf("%d ms: Sent %d - %d since last report (%d Hz)\n",
             (int)(now - start_time) / 1000, sent, countOverInterval, (int) intervalRate);

      previous_sent = sent;
      previous_report_time = now;
      next_summary_time += SUMMARY_EVERY_US;
    }

    while (((i * 1000000.0) / (now - start_time)) > rate_limit) {
      microsleep(2000);
      now = now_microseconds();
    }
  }

  {
    uint64_t stop_time = now_microseconds();
    int total_delta = (int)(stop_time - start_time);

    printf("PRODUCER - Message count: %d\n", message_count);
    printf("Total time, milliseconds: %d\n", total_delta / 1000);
    printf("Overall messages-per-second: %g\n", (message_count / (total_delta / 1000000.0)));
  }
}

int amqp_main(int argc, char const *const *argv)
{
  if(!g_config->amqpEnable) {
    return 0;
  }

  char const *hostname;
  int port, status;
  int rate_limit;
  int message_count;
  amqp_socket_t *socket;
  //to blogal --> amqp_connection_state_t conn;

  fprintf(stdout, "[amqp] start...\n");


  hostname = g_config->mqttBrockerIP;
  port = g_config->mqttBrockerPORT;

  rate_limit = g_config->uaPublishIntervalUsecs;
  message_count = 2;

  do {
    conn = amqp_new_connection();

    socket = amqp_ssl_socket_new(conn);
    if (!socket) {
      sleep(3);
      continue;
    }
  } while(!beStop && !socket);

  bC = false;

  amqp_ssl_socket_set_verify_peer(socket, 0);
  amqp_ssl_socket_set_verify_hostname(socket, 0);

  // if (argc > 5) {
  //   int nextarg = 6;
  //   status = amqp_ssl_socket_set_cacert(socket, argv[5]);
  //   if (status) {
  //     die("setting CA certificate");
  //   }
  //   if (argc > nextarg && !strcmp("verifypeer", argv[nextarg])) {
  //     amqp_ssl_socket_set_verify_peer(socket, 1);
  //     nextarg++;
  //   }
  //   if (argc > nextarg && !strcmp("verifyhostname", argv[nextarg])) {
  //     amqp_ssl_socket_set_verify_hostname(socket, 1);
  //     nextarg++;
  //   }
  //   if (argc > nextarg + 1) {
  //     status =
  //         amqp_ssl_socket_set_key(socket, argv[nextarg + 1], argv[nextarg]);
  //     if (status) {
  //       die("setting client cert");
  //     }
  //   }
  // }

  status = amqp_socket_open(socket, hostname, port);
  if (status) {
    die("opening SSL/TLS connection");
  }

  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest"),
                    "Logging in");

  amqp_channel_open(conn, 1);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

  //send_batch(conn, "test queue", rate_limit, message_count);
  bC = true;
  while(!beStop) {
      sleep(2);
  }

  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");

  fprintf(stdout, "[amqp] stopped.\n");

  return 0;
}

int amqp_publish(const char* mode, char* topic, const char* value) 
{
  if(!bC) return 0;

	printf("[amqp] publish (%s) %s\t%s\n", mode, topic, value);
  

  amqp_bytes_t message_bytes;

  message_bytes.len = sizeof(value);

#pragma GCC diagnostic push  // require GCC 4.6
#pragma GCC diagnostic ignored "-Wcast-qual"
	message_bytes.bytes = (char*)value;
#pragma GCC diagnostic pop 
  

  amqp_basic_publish(conn,
    1,
    amqp_cstring_bytes(""),
    amqp_cstring_bytes(""),
    0,
    0,
    NULL,
    message_bytes);

  return 0;
}

void* amqp_run(void* param)
{
	return (void*)amqp_main(0, 0);
}

