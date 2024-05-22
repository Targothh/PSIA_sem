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
#define DATA_SIZE 958
#define SENDER_DATA_ADDRESS "147.32.216.190" 
#define SENDER_ACK_ADDRESS "147.32.216.190" 
#define RECEIVER_ADDRESS "147.32.217.229" 
#define NETDERPER_RECEIVER_ADDRESS "147.32.216.190"
#define SENDER_DATA_PORT 5001
#define SENDER_ACK_PORT 15001
#define NETDERPER_RECEIVER_PORT 14000
//#define NETDERPER_RECEIVER_ADDRESS "147.32.217.229"
// #define NETDERPER_SENDER_DATA_PORT 5001
// #define NETDERPER_SENDER_ACK_PORT 15001
// #define NETDERPER_RECEIVER_PORT 15000
#define NETDERPER_SENDER_DATA_PORT 14000
#define NETDERPER_SENDER_ACK_PORT 14001
#define RECEIVER_PORT 15000
#define MAX_TIMEOUT 1
#define EXIT_NOT_FOUND 404
#include <stdint.h>

typedef struct {
    int16_t free_space;
    uLong crc;
    int16_t index;
    unsigned char data[DATA_SIZE];
} datagram_t ;

void setup_addr(struct sockaddr_in *addr, int port, char *ip);
void bind_socket(int sockfd, struct sockaddr_in *addr);
int init_socket(void);
#endif
