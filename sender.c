#include "packet.h"


void setup_sockets(int* socket_data_sender, int* socket_ack_sender, struct sockaddr_in* sender_data_addr, struct sockaddr_in* sender_ack_addr, struct timeval* tv) {
    *socket_data_sender = init_socket();
    *socket_ack_sender = init_socket();
    setup_addr(sender_data_addr, SENDER_DATA_PORT, SENDER_DATA_ADDRESS);
    setup_addr(sender_ack_addr, SENDER_ACK_PORT, SENDER_ACK_ADDRESS);
    bind_socket(*socket_data_sender, sender_data_addr);
    bind_socket(*socket_ack_sender, sender_ack_addr);
    setsockopt(*socket_data_sender, SOL_SOCKET, SO_SNDTIMEO, (const char*)tv, sizeof(*tv));
    setsockopt(*socket_ack_sender, SOL_SOCKET, SO_RCVTIMEO, (const char*)tv, sizeof(*tv));
}

void read_data(FILE *fw, datagram_t *datagram) {
    int read_data;
    read_data = fread(datagram->data, 1, sizeof(datagram->data), fw);
    if (read_data != DATA_SIZE) {
        datagram->free_space = sizeof(datagram->data) - read_data;
    }
}

int send_data(int socket_data_sender, struct sockaddr_in receiver_addr, datagram_t datagram) {
    fprintf(stderr, "Sending %d\n", datagram.index);
    uLong crc = crc32(0L, Z_NULL, 0);
    datagram.crc = crc32(crc, (const Bytef*) datagram.data, (uInt)(sizeof(datagram.data)));
    if (sendto(socket_data_sender, &datagram, sizeof(datagram), 0, (struct sockaddr *) &receiver_addr, sizeof(receiver_addr)) < 0) {
        fprintf(stderr, "Failed to send data\n");
        return -1;
    }
    return datagram.index;
}

ack_t recv_ack(int socket_ack_sender, struct sockaddr_in receiver_addr, int index) {
    ack_t ack = {-1, crc32(0L, Z_NULL, 0)};
    socklen_t receiver_addr_len = sizeof(receiver_addr);
    if (recvfrom(socket_ack_sender, &ack, sizeof(ack), 0, (struct sockaddr *) &receiver_addr, &receiver_addr_len) >= 0 
        && ack.index < index + WINDOW_SIZE && ack.index >= index 
        && ack.crc == crc32(0L, (const Bytef*)&ack.index, sizeof(ack.index))) {
    } else {
        ack = (ack_t){-1, 0};
    }
    return ack;
}


void send_hash_and_wait_for_ack(int socket_data_sender, int socket_ack_sender, struct sockaddr_in receiver_addr, datagram_t* datagram) {
     ack_t ack = {-1, crc32(0L, Z_NULL, 0)};
    socklen_t receiver_addr_len = sizeof(receiver_addr);
    while (ack.index != 1 && ack.crc != crc32(0L, (const Bytef*)&ack.index , sizeof(ack.index ))) {
        send_data(socket_data_sender, receiver_addr, *datagram);
        fprintf(stderr, "Sending hash\n");
        recvfrom(socket_ack_sender, &ack, sizeof(ack), 0, (struct sockaddr *) &receiver_addr, &receiver_addr_len);
    }
    print_sha256_hash(datagram->data);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Filename argument missing!\n");
        return EXIT_FAILURE;
    }

    char* filename = argv[1];
    unsigned char final_hash[SHA256_BLOCK_SIZE];
    compute_file_hash(filename, final_hash);

    int socket_data_sender, socket_ack_sender;
    struct sockaddr_in sender_data_addr, sender_ack_addr, receiver_addr;
    struct timeval tv = {.tv_sec = 0, .tv_usec = MAX_TIMEOUT};

    setup_sockets(&socket_data_sender, &socket_ack_sender, &sender_data_addr, &sender_ack_addr, &tv);
    setup_addr(&receiver_addr, NETDERPER_RECEIVER_PORT, NETDERPER_RECEIVER_ADDRESS);

    datagram_t hash_datagram = {0};
    memcpy(hash_datagram.data, final_hash, SHA256_BLOCK_SIZE);
    
    send_hash_and_wait_for_ack(socket_data_sender, socket_ack_sender, receiver_addr, &hash_datagram);

    FILE *fw = fopen(filename, "rb");
    if (!fw) {
        fprintf(stderr, "File not found!\n");
        return EXIT_NOT_FOUND;
    }
    window_t window = {0};
    for (int i = 0; i < WINDOW_SIZE; i++) {
        window.acks[i] = (ack_t){-1, crc32(0L, Z_NULL, 0)};
    }

    ack_t ack = {-1, crc32(0L, Z_NULL, 0)};
    int read = -1;
    int eof = 0;
    while (true) {
        for( int i =0; i < WINDOW_SIZE; i++){
            if (read < window.index + i && !eof){
                read_data(fw, &window.datagrams[i]);
                if (window.datagrams[i].free_space == DATA_SIZE){
                    eof = read;
                    break;
                }
               read ++;
            }
        }
        for(int i = 0; i < WINDOW_SIZE; i++){
            if (eof && window.index + i >= eof)
                break;
            if (window.acks[i].index == -1){
                window.datagrams[i].index = window.index + i;
                if (window.datagrams[i].free_space != DATA_SIZE) {
                    send_data(socket_data_sender, receiver_addr, window.datagrams[i]);
                    if (window.datagrams[i].free_space != 0)
                        fprintf(stderr, "Sent %d\n", window.datagrams[i].free_space);
                }
            }
        }
        ack = recv_ack(socket_ack_sender, receiver_addr, window.index);
        if (ack.index <= window.index + WINDOW_SIZE && ack.index >= window.index){
            window.acks[ack.index - window.index] = ack; 
        }
        int shift = acked_shift(window);
        for (int i = 0; i < shift; i++) {
            if (i + shift < WINDOW_SIZE){
                window.datagrams[i] = window.datagrams[i + shift];
                window.acks[i] = window.acks[i + shift];
            }else{
                window.acks[i] = (ack_t){-1, crc32(0L, Z_NULL, 0)};
                window.datagrams[i] = (datagram_t){0};
            
            }
        }
        window.index += shift;
        for (int i = WINDOW_SIZE-shift; i < WINDOW_SIZE; i++) {
            window.acks[i] = (ack_t){-1, crc32(0L, Z_NULL, 0)};
            window.datagrams[i] = (datagram_t){0};
        }
        if (eof && window.index >= eof)
            break;
    }

    fclose(fw);
    close(socket_data_sender);
    close(socket_ack_sender);
    
    return 0;
}
