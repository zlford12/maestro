#ifndef MAESTRO_SOCKET_HELPER_H
#define MAESTRO_SOCKET_HELPER_H
#include <stdint.h>

void SocketInit();
void SocketListen(char *message, int message_size);
void SendFrames(uint32_t *buff, int size);
void SocketClose();

#endif