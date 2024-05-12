#include "packet.h"

int main(int argc, char *argv[]){
    int socket_recv;
    struct sockaddr_in sender_addr, receiver_addr;
    datagram_t datagram;
    socklen_t sender_addr_len = sizeof(sender_addr);

    if ((socket_recv = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Cannot create socket\n");
        exit(-1);
    }

    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS); 
    setup_addr(&sender_addr, SENDER_PORT, SENDER_ADDRESS);

    if(bind(socket_recv, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0){
            printf("Couldn't bind to the port\n");
            exit(-1);
    }
    printf("Listening on port %d...\n", RECEIVER_PORT);

    uLong crc;
    FILE *fr;
    fr = fopen("OUTPUT1.jpg", "wb");
    while(datagram.free_space == 0){
        recvfrom(socket_recv, (void*)&datagram, sizeof(datagram), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));
        if (datagram.crc != crc){
            printf("CRC error\n");
            printf("Index: %d, CRC: %lu, expected CRC: %lu\n", datagram.index, datagram.crc, crc);
            exit(-1);
        }
        fwrite(datagram.data, sizeof(datagram.data) - datagram.free_space, 1, fr);
    }
    fclose(fr);
    close(socket_recv);
    return 0;
}
