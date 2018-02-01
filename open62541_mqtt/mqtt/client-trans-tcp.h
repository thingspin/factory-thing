#ifndef OPCUA_TCP_TRANCE_H_
#define OPCUA_TCP_TRANCE_H_

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int tcp_open(char* host, int port);
int tcp_close(int sock);

int tcp_sendPacketBuffer(int sock, unsigned char* buf, int buflen);
int tcp_getdata(unsigned char* buf, int count);
int tcp_getdatanb(void *sck, unsigned char* buf, int count);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* OPCUA_TCP_TRANCE_H_ */