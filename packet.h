#ifndef PACKET_H
#define PACKET_H
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <zlib.h>
#include <errno.h>
#include <stdbool.h>
#define PACKET_SIZE 1024
#define DATA_SIZE 1000
#define SENDER_ADRESS "127.0.0.1"
#define RECEIVER_ADRESS "127.0.0.1"
#define SENDER_PORT 5005
#define RECEIVER_PORT 5002
#define MAX_TIMEOUT 2
#define ACK (last_index + 1)
#define EXIT_NOT_FOUND 404


typedef struct {
    int id;
    uLong crc;
    int index;
    unsigned char data[DATA_SIZE];
} datagram_t ;

void setup_addr(struct sockaddr_in *addr, int port, char *ip);

#endif
