#include "packet.h"

int main(int argc, char *argv[]){
    char name[32];
    strcpy(name, argv[1]);
    int socket_init;
    struct sockaddr_in sender_addr, reciver_addr;
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
    setup_addr(&sender_addr, SENDER_PORT, SENDER_ADDRESS);
    setup_addr(&reciver_addr, RECEIVER_PORT, RECEIVER_ADDRESS);
    if(bind(socket_init, (struct sockaddr*)&sender_addr, sizeof(sender_addr)) < 0){
        printf("Couldn't bind to the port\n");
        exit(EXIT_FAILURE);
    }
    FILE *fw;
    if ((fw = fopen(name, "rb")) == NULL){
        printf("File not found!\n");
        exit(EXIT_NOT_FOUND);
    }
    int id;
    while(datagram.id == 0){
        if((id=fread(datagram.data, 1, sizeof(datagram.data), fw)) != sizeof(datagram.data)){
            datagram.id = sizeof(datagram.data) - id;
        }                                                                  
        printf("Packet sent ID: %d\nSIZE: %lu, ID:%d ",datagram.id, sizeof(datagram), id);
        memcpy(datagram_arr, &datagram, sizeof(datagram));
        sendto(socket_init, datagram_arr, sizeof(datagram_arr), 0, (struct sockaddr*)&reciver_addr, sizeof(reciver_addr));
        }
    fclose(fw);
    close(socket_init);
    return 0;
}
