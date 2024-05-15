#include "packet.h"
int main(int argc, char *argv[]){
    int socket_recv = init_socket();
    struct sockaddr_in sender_addr, receiver_addr;
    datagram_t datagram;
    datagram.index = 0;
    datagram.free_space = 0;
    socklen_t sender_addr_len = sizeof(sender_addr);
    uLong crc;
    FILE *fr;
    int expected_index = 0;
    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS); 
    setup_addr(&sender_addr, SENDER_PORT, SENDER_ADDRESS);
    bind_socket(socket_recv, &receiver_addr);
    struct timeval tv;
    tv.tv_sec = MAX_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(socket_recv, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    printf("Listening on port %d...\n", RECEIVER_PORT);


    fr = fopen("OUTPUT1.jpg", "wb");

    while(true){
        if(recvfrom(socket_recv, &datagram, sizeof(datagram), 0, (struct sockaddr *) &sender_addr, &sender_addr_len) < 0){
            fprintf(stderr,"Error in receiving data\n");
            continue;
        }
        crc = crc32(0L, Z_NULL, 0);     
        crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));
        if((datagram.index == expected_index && crc == datagram.crc) || datagram.index < expected_index){  
            fwrite(datagram.data, sizeof(datagram.data) - datagram.free_space, 1, fr);
            sendto(socket_recv, &expected_index, sizeof(expected_index), 0, (struct sockaddr *) &sender_addr, sender_addr_len);
            expected_index++;
        } else {
            int nack = -1;
            sendto(socket_recv, &nack, sizeof(nack), 0, (struct sockaddr *) &sender_addr, sender_addr_len);
        }
        if (datagram.free_space != 0)
            break;
        }
    fclose(fr);
    close(socket_recv);
    return 0;
} 
