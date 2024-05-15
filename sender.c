#include "packet.h"

int main(int argc, char *argv[]){
    char name[32];
    strcpy(name, argv[1]);
    int socket_send = init_socket();
    struct sockaddr_in sender_addr, receiver_addr;
    datagram_t datagram;
    datagram.index = 0;
    datagram.free_space = 0;
    setup_addr(&sender_addr, SENDER_PORT, SENDER_ADDRESS);
    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS);
    bind_socket(socket_send, &sender_addr);
    FILE *fw;
    if ((fw = fopen(name, "rb")) == NULL){
        fprintf(stderr,"File not found!\n");
        exit(EXIT_NOT_FOUND);
    }
    struct timeval tv;
    tv.tv_sec = MAX_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(socket_send, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    int read_data; 
    uLong crc;
    int ack = -1;
    while(true){
        if (ack == datagram.index - 1){
            if((read_data=fread(datagram.data, 1, sizeof(datagram.data), fw)) != DATA_SIZE){
                datagram.free_space = sizeof(datagram.data) - read_data;
            } 
        }
        crc = crc32(0L, Z_NULL, 0);                                        
        datagram.crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));     
        if(sendto(socket_send, &datagram, sizeof(datagram), 0, (struct sockaddr *) &receiver_addr, sizeof(receiver_addr)) < 0){
            fprintf(stderr,"Failed to send data\n");
            continue;
        }


        if(recvfrom(socket_send, &ack, sizeof(ack), 0, NULL, NULL) >= 0 && ack == datagram.index){
            datagram.index++;
        } else {
            fprintf(stderr,"NACK or timeout, resending %d\n", datagram.index);
        }
        if (ack == datagram.index - 1 && datagram.free_space != 0)
            break;
    }

    fclose(fw);
    close(socket_send);
    return 0;
}
