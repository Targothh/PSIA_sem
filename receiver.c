#include "packet.h"
#include "sha256.c"
int main(int argc, char *argv[]){
    int socket_recv = init_socket();
    struct sockaddr_in sender_data_addr, receiver_addr, sender_ack_addr;
    datagram_t datagram = {0};
    datagram.index = 0;
    datagram.free_space = 0;
    socklen_t sender_addr_len = sizeof(sender_data_addr),
                sender_ack_addr_len = sizeof(sender_ack_addr);
    uLong crc;
    FILE *fr;
    int expected_index = 0, received = -1;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = MAX_TIMEOUT;
    unsigned char received_hash[SHA256_BLOCK_SIZE];
    char *file_name = "OUTPUT1.jpg";
    fr = fopen(file_name, "wb");
    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS); 
    setup_addr(&sender_data_addr, NETDERPER_SENDER_DATA_PORT, SENDER_DATA_ADDRESS);
    setup_addr(&sender_ack_addr, NETDERPER_SENDER_ACK_PORT, SENDER_ACK_ADDRESS);
    bind_socket(socket_recv, &receiver_addr);
    
    setsockopt(socket_recv, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    printf("Listening on port %d...\n", RECEIVER_PORT);
    while(1){
        if(recvfrom(socket_recv, &datagram, sizeof(datagram), 0, (struct sockaddr *) &sender_data_addr, &sender_addr_len) < 0){            printf("\n");
            fprintf(stderr,"Error in receiving data tady\n");
        }
        crc = crc32(0L, Z_NULL, 0);     
        crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));
        if(crc == datagram.crc){
            fprintf(stderr,"Hash received\n");
            memcpy(received_hash, datagram.data, SHA256_BLOCK_SIZE);
            int received = 1;
            sendto(socket_recv, &received, sizeof(received), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);
            break;
        }
    }
    while(true){
        setup_addr(&sender_ack_addr, NETDERPER_SENDER_ACK_PORT, SENDER_ACK_ADDRESS);
        if(recvfrom(socket_recv, &datagram, sizeof(datagram), 0, (struct sockaddr *) &sender_data_addr, &sender_addr_len) < 0){
            fprintf(stderr,"Error in receiving data\n");
            continue;
        }
        received = datagram.index;
        crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));
        if((datagram.index == expected_index && crc == datagram.crc) || received < expected_index){  
            if (received == expected_index){
                fwrite(datagram.data, sizeof(datagram.data) - datagram.free_space, 1, fr);
                fprintf(stderr,"Received %d\n", received);
                sendto(socket_recv, &expected_index, sizeof(expected_index), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);
                expected_index++;                
                if (datagram.free_space != 0)
                    break;  
            }
            else
                sendto(socket_recv, &received, sizeof(received), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);
        } else {
            int nack = -1;
            sendto(socket_recv, &nack, sizeof(nack), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);
        }
    }
    fclose(fr);

    SHA256_CTX hash;
    sha256_init(&hash);
    int size;
    unsigned char data[DATA_SIZE];
    unsigned char final_hash[SHA256_BLOCK_SIZE];
    FILE *fhs;
    if ((fhs = fopen(file_name, "rb")) == NULL){
        fprintf(stderr,"File not found!\n");
        exit(EXIT_NOT_FOUND);
    }
    while ((size = fread (data, 1, DATA_SIZE, fhs)) != 0){
        sha256_update(&hash, data, size);
    }
    sha256_final(&hash, final_hash);
    fclose(fhs);
    printf("File hash:\n");
    for(int i = 0; i < SHA256_BLOCK_SIZE; i++) printf("%02x", final_hash[i]);
    printf("\nReceived hash:\n");
    for(int i = 0; i < SHA256_BLOCK_SIZE; i++) printf("%02x", received_hash[i]);
    close(socket_recv);
    return 0;
} 
