#include "packet.h"

int main(int argc, char *argv[]){
    int socket_init;
    struct sockaddr_in sender_addr, receiver_addr;
    datagram_t datagram;
    unsigned char datagram_arr[PACKET_SIZE];
    datagram.id = 0;
    socklen_t sender_addr_len = sizeof(sender_addr);

    if ((socket_init = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Cannot create socket\n");
        exit(-1);
    }

    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS); 
    setup_addr(&sender_addr, SENDER_PORT, SENDER_ADDRESS);

    if(bind(socket_init, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0){
            printf("Couldn't bind to the port\n");
            exit(-1);
        }
    printf("Listening on port %d...\n", RECEIVER_PORT);

    FILE *fr;
    fr = fopen("OUTPUT1.jpg", "wb");
    while(datagram.id == 0){
        recvfrom(socket_init, datagram_arr, sizeof(datagram_arr), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        memcpy(&datagram, datagram_arr, sizeof(datagram_arr));
        fwrite(datagram.data, sizeof(datagram.data) - datagram.id, 1, fr);
    }
    fclose(fr);
    close(socket_init);
    return 0;
}
