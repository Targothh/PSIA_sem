#include "packet.h"

int main(int argc, char *argv[]){
    int socket_init;
    struct sockaddr_in sender_addr, receiver_addr;
    datagram_t datagram;
    unsigned char datagram_arr[PACKET_SIZE];
    datagram.id = 0;
    // struct timeval timeout;
    //     timeout.tv_sec = MAX_TIMEOUT;
    //     timeout.tv_usec = 0;
    if ((socket_init = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Cannot create socket\n");
        exit(-1);
    }
    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADRESS);
    setup_addr(&sender_addr, SENDER_PORT, SENDER_ADRESS);
    if(bind(socket_init, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0){
            printf("Couldn't bind to the port\n");
            exit(-1);
        }
    printf("Listening on port %d...\n", RECEIVER_PORT);

    FILE *fr;
        fr = fopen("OUTPUT1.jpg", "wb");
        while(datagram.id == 0){
            recvfrom(socket_init, datagram_arr, sizeof(datagram_arr),0, (struct sockaddr *)&sender_addr,(unsigned int*) sizeof(sender_addr));
            memcpy(&datagram, datagram_arr, sizeof(datagram_arr));
            fwrite(datagram.data, sizeof(datagram.data)-datagram.id, 1, fr);
        }
    fclose(fr);
    close(socket_init);
    return 0;
}
