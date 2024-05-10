#include <zlib.h>
#define PACKET_SIZE 1024
#define SENDER_ADRESS "127.0.0.1"
#define RECIVER_ADRESS "127.0.0.1"
#define SENDER_PORT 5005
#define RECIVER_PORT 5002
#define MAX_TIMEOUT 2
#define ACK (last_index + 1)


typedef struct {
    int id;
    uLong crc;
    int index;
    unsigned char data[1000];
} datagram_t ;