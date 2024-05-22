#include "packet.h"
#include "sha256.c"

void read_data(FILE *fw, datagram_t *datagram){
    int read_data;
    if((read_data=fread(datagram->data, 1, sizeof(datagram->data), fw)) != DATA_SIZE){
        datagram->free_space = sizeof(datagram->data) - read_data;
    } 
}
int send_data( int socket_data_sender, struct sockaddr_in receiver_addr, datagram_t *datagram){
    uLong crc = crc32(0L, Z_NULL, 0);                                        
    datagram->crc = crc32(crc, (const Bytef*) datagram->data, (uInt)(sizeof(datagram->data)));     
    if(sendto(socket_data_sender, datagram, sizeof(*datagram), 0, (struct sockaddr *) &receiver_addr, sizeof(receiver_addr)) < 0){
        fprintf(stderr,"Failed to send data\n");
        return -1;
    }
    return datagram->index;
}

int recv_ack(int socket_ack_sender, struct sockaddr_in receiver_addr,  int sent, datagram_t* datagram){
    int ack = -1;
    socklen_t receiver_addr_len = sizeof(receiver_addr);
    if(recvfrom(socket_ack_sender, &ack, sizeof(ack), 0, (struct sockaddr *) &receiver_addr , &receiver_addr_len) >= 0 && ack == sent){
        datagram->index++;
    } else {
        fprintf(stderr,"NACK or timeout, resending %d, ack = %d\n", sent, ack);
    }
    return ack;
}

int main(int argc, char *argv[]){
    int ack = -1;
    int sent = -1;
    SHA256_CTX hash;
    sha256_init(&hash);
    unsigned char final_hash[SHA256_BLOCK_SIZE] = {0};
    unsigned char data[DATA_SIZE] = {0};
    char name[32];
    strcpy(name, argv[1]);
    int socket_data_sender = init_socket(),
        socket_ack_sender = init_socket();
    struct sockaddr_in sender_data_addr, receiver_addr, sender_ack_addr;
    int size;
    datagram_t datagram;
    datagram.index = 0;
    datagram.free_space = 0;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = MAX_TIMEOUT;
    setup_addr(&sender_data_addr, SENDER_DATA_PORT, SENDER_DATA_ADDRESS);
    setup_addr(&sender_ack_addr, SENDER_ACK_PORT, SENDER_ACK_ADDRESS);
    setup_addr(&receiver_addr, NETDERPER_RECEIVER_PORT, NETDERPER_RECEIVER_ADDRESS);

    bind_socket(socket_data_sender, &sender_data_addr);
    bind_socket(socket_ack_sender, &sender_ack_addr);
    setsockopt(socket_data_sender, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(socket_ack_sender, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    socklen_t receiver_addr_len = sizeof(receiver_addr);
    FILE *fhs;
    if ((fhs = fopen(name, "rb")) == NULL){
        fprintf(stderr,"File not found!\n");
        exit(EXIT_NOT_FOUND);
    }
    while ((size = fread (data, 1, DATA_SIZE, fhs)) != 0){
        sha256_update(&hash, data, size);
    }
    sha256_final(&hash, final_hash);
    for(int i = 0; i < SHA256_BLOCK_SIZE; i++) printf("%02x", final_hash[i]);
    memcpy(datagram.data,final_hash ,SHA256_BLOCK_SIZE);
    while(ack != 1){
        send_data(socket_data_sender, receiver_addr, &datagram);
        fprintf(stderr,"Sending hash\n");
        recvfrom(socket_ack_sender, &ack, sizeof(ack), 0, (struct sockaddr *) &receiver_addr , &receiver_addr_len);

    }
        for(int i = 0; i < SHA256_BLOCK_SIZE; i++) printf("%02x", datagram.data[i]);


    FILE *fw;
    if ((fw = fopen(name, "rb")) == NULL){
        fprintf(stderr,"File not found!\n");
        exit(EXIT_NOT_FOUND);
    }
    
    ack = -1;
    while(true){
        setup_addr(&receiver_addr, NETDERPER_RECEIVER_PORT, NETDERPER_RECEIVER_ADDRESS);
        if (ack == sent){
            read_data(fw, &datagram);
        }

        sent = send_data(socket_data_sender, receiver_addr, &datagram);
        fprintf(stderr,"Sent %d\n", sent);  
        ack = recv_ack(socket_ack_sender, receiver_addr, sent, &datagram);
        if (ack == sent && datagram.free_space != 0)
            break;
    }
    fclose(fw);
    
    
    close(socket_data_sender);
    close(socket_ack_sender);
    sha256_final(&hash, final_hash);
    fclose(fhs);
    return 0;
}