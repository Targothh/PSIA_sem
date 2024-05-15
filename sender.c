#include "packet.h"

int main(int argc, char *argv[]){
    char name[32];
    strcpy(name, argv[1]);
    int socket_data_sender = init_socket(),
        socket_ack_sender = init_socket();
    struct sockaddr_in sender_data_addr, receiver_addr, sender_ack_addr;
    datagram_t datagram;
    datagram.index = 0;
    datagram.free_space = 0;
    setup_addr(&sender_data_addr, SENDER_DATA_PORT, SENDER_DATA_ADDRESS);
    setup_addr(&sender_ack_addr, SENDER_ACK_PORT, SENDER_ACK_ADDRESS);
    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS);

    bind_socket(socket_data_sender, &sender_data_addr);
    bind_socket(socket_ack_sender, &sender_ack_addr);
    FILE *fw;
    if ((fw = fopen(name, "rb")) == NULL){
        fprintf(stderr,"File not found!\n");
        exit(EXIT_NOT_FOUND);
    }
    struct timeval tv;
    tv.tv_sec = MAX_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(socket_data_sender, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(socket_ack_sender, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
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
        if(sendto(socket_data_sender, &datagram, sizeof(datagram), 0, (struct sockaddr *) &receiver_addr, sizeof(receiver_addr)) < 0){
            fprintf(stderr,"Failed to send data\n");
            continue;
        }


        if(recvfrom(socket_ack_sender, &ack, sizeof(ack), 0, NULL, NULL) >= 0 && ack == datagram.index){
            datagram.index++;
        } else {
            fprintf(stderr,"NACK or timeout, resending %d\n", datagram.index);
        }
        if (ack == datagram.index - 1 && datagram.free_space != 0)
            break;
    }

    fclose(fw);
    close(socket_data_sender);
    close(socket_ack_sender);
    return 0;
}
