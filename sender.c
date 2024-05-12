#include "packet.h"

int main(int argc, char *argv[]){
    char name[32];
    strcpy(name, argv[1]);
    int socket_send;
    struct sockaddr_in sender_addr, receiver_addr;
    datagram_t datagram;
    if ((socket_send = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Cannot create socket\n");
        exit(-1);
    }
    setup_addr(&sender_addr, SENDER_PORT, SENDER_ADDRESS);
    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS);
    if(bind(socket_send, (struct sockaddr*)&sender_addr, sizeof(sender_addr)) < 0){
        printf("Couldn't bind to the port\n");
        exit(EXIT_FAILURE);
    }
    FILE *fw;
    if ((fw = fopen(name, "rb")) == NULL){
        printf("File not found!\n");
        exit(EXIT_NOT_FOUND);
    }
    int read_data; 
    while(datagram.free_space == 0){
        if((read_data=fread(datagram.data, 1, sizeof(datagram.data), fw)) != DATA_SIZE){
            datagram.free_space = sizeof(datagram.data) - read_data;
        }                                                                  
        sendto(socket_send, (const void*)&datagram, sizeof(datagram), 0, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));
        datagram.index++;
        }
    fclose(fw);
    close(socket_send);
    return 0;
}
