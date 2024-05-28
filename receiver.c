#include "packet.h"


int receive_data(int socket_recv, datagram_t *datagram, struct sockaddr_in *sender_data_addr, socklen_t *sender_addr_len) {
    if (recvfrom(socket_recv, datagram, sizeof(*datagram), 0, (struct sockaddr *)sender_data_addr, sender_addr_len) < 0) {
        fprintf(stderr, "Error in receiving data\n");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]){
    int socket_recv = init_socket();
    struct sockaddr_in sender_data_addr, receiver_addr, sender_ack_addr;
    datagram_t datagram = {0};
    datagram.index = 0;
    datagram.free_space = 0;
    socklen_t sender_addr_len = sizeof(sender_data_addr),
                sender_ack_addr_len = sizeof(sender_ack_addr);
    uLong crc;
    ack_t ack = {-1, crc32(0L, Z_NULL, 0)};
    FILE *fr;
    int received = -1;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = MAX_TIMEOUT;
    unsigned char received_hash[SHA256_BLOCK_SIZE];
    char *file_name = "OUTPUT1.log";
    fr = fopen(file_name, "wb");
    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS); 
    setup_addr(&sender_data_addr, NETDERPER_SENDER_DATA_PORT, SENDER_DATA_ADDRESS);
    setup_addr(&sender_ack_addr, NETDERPER_SENDER_ACK_PORT, SENDER_ACK_ADDRESS);
    bind_socket(socket_recv, &receiver_addr);
    
    setsockopt(socket_recv, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    printf("Listening on port %d...\n", RECEIVER_PORT);
    while(1){
        receive_data(socket_recv, &datagram, &sender_data_addr, &sender_addr_len);
        crc = crc32(0L, Z_NULL, 0);     
        crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));
        if(crc == datagram.crc){
            fprintf(stderr,"Hash received\n");
            memcpy(received_hash, datagram.data, SHA256_BLOCK_SIZE);
            ack.index = 1;
            ack.crc = crc32(0L, (const Bytef*) &ack.index, sizeof(ack.index));
            sendto(socket_recv, &ack, sizeof(ack), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);
            break;
        }
    }
    window_t window = {0};
    for (int i = 0; i < WINDOW_SIZE; i++) {
        window.acks[i].index = -1;
    }
    int eof = 0;
    while(true){
        receive_data(socket_recv, &datagram, &sender_data_addr, &sender_addr_len);
        received = datagram.index;
        crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));
        //fprintf(stderr,"Received %d expected %d\n", received, window.index);
        if(received < window.index + WINDOW_SIZE && crc == datagram.crc){  
            if ((received >= window.index)&& window.acks[received -  window.index].index == -1){
                window.datagrams[received -  window.index] = datagram;
                window.acks[received -  window.index].index = received;
                if (datagram.free_space != 0){
                    fprintf(stderr, "EOF\n");
                    eof = 1;
                }
                ack.index = received;
                ack.crc = crc32(0L, (const Bytef*) &ack.index, sizeof(ack.index));
                sendto(socket_recv, &ack, sizeof(ack), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);    
            }
            else{
                ack.index = received;
                ack.crc = crc32(0L, (const Bytef*) &ack.index, sizeof(ack.index));
                sendto(socket_recv, &ack, sizeof(ack), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);   
            }
        } else {
                ack.index = -1;
                ack.crc = crc32(0L, (const Bytef*) &ack.index, sizeof(ack.index));
                sendto(socket_recv, &ack, sizeof(ack), 0, (struct sockaddr *) &sender_ack_addr, sender_ack_addr_len);   
        }
        int shift = acked_shift(window);
        for (int i = 0; i < WINDOW_SIZE; i++) {
            if (i < shift){
                fprintf(stderr, "Writing %d\n", window.index + i);
                fwrite(window.datagrams[i].data, sizeof(window.datagrams[i].data) - window.datagrams[i].free_space, 1, fr);
                }
            if (shift && i + shift < WINDOW_SIZE){
                if(ack.index == -1)
                    fprintf(stderr, "Shifting %d from %d to %d| from %d to %d \n",shift , i + shift, i, window.index + i + shift, window.index + i);
                window.datagrams[i] = window.datagrams[i + shift];
                window.acks[i] = window.acks[i + shift];
                window.datagrams[i + shift] = (datagram_t){0};
                window.acks[i + shift] = (ack_t){-1, crc32(0L, Z_NULL, 0)};
            }else{
                window.acks[i] = (ack_t){-1, crc32(0L, Z_NULL, 0)};
                window.datagrams[i] = (datagram_t){0};
            }
        }
        for (int i = shift; i < WINDOW_SIZE; i++) {
            window.acks[i] = (ack_t){-1, crc32(0L, Z_NULL, 0)};
            window.datagrams[i] = (datagram_t){0};
        }

        window.index += shift;
        if(eof && window.acks[0].index == -1){
            break;
        }
    }
    fclose(fr);


    SHA256_CTX hash;
    sha256_init(&hash);
    unsigned char final_hash[SHA256_BLOCK_SIZE]= {0};
    compute_file_hash(file_name, final_hash);
    printf("File hash:\n");
    print_sha256_hash(final_hash);
    printf("Received hash:\n");
    print_sha256_hash(received_hash);
    
    close(socket_recv);
    return 0;
} 
