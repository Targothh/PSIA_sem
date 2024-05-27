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
        receive_data(socket_recv, &datagram, &sender_data_addr, &sender_addr_len);
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
        receive_data(socket_recv, &datagram, &sender_data_addr, &sender_addr_len);
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
    unsigned char final_hash[SHA256_BLOCK_SIZE];
    compute_file_hash(file_name, final_hash);
    printf("File hash:\n");
    print_sha256_hash(final_hash);
    printf("\nReceived hash:\n");
    print_sha256_hash(received_hash);
    close(socket_recv);
    return 0;
} 
