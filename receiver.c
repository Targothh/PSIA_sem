#include "packet.h"

int receive_data(int socket_recv, datagram_t *datagram, struct sockaddr_in *sender_data_addr, socklen_t *sender_addr_len) {
    ssize_t bytes_received;
    size_t total_bytes_received = 0;
    while (total_bytes_received < sizeof(*datagram)) {
        bytes_received = recvfrom(socket_recv, ((char*)datagram) + total_bytes_received, sizeof(*datagram) - total_bytes_received, 0, (struct sockaddr *)sender_data_addr, sender_addr_len);
        if (bytes_received < 0) {
            fprintf(stderr, "Error in receiving data\n");
            return -1;
        }
        total_bytes_received += bytes_received;
    }
    return 0;
}

void send_ack(int socket_recv, int value, struct sockaddr_in sender_ack_addr, socklen_t sender_ack_addr_len) {
    ack_t ack;
    ack.index = value;
    ack.crc = crc32(0L, (const Bytef*)&ack.index, sizeof(ack.index));
    if (sendto(socket_recv, &ack, sizeof(ack), 0, (struct sockaddr*)&sender_ack_addr, sender_ack_addr_len) < 0) {
        perror("Failed to send ACK");
    }
    //fprintf(stderr, "Sent ack %d\n", ack.index);
}

int main(int argc, char *argv[]) {
    int socket_recv = init_socket();
    struct sockaddr_in sender_data_addr, receiver_addr, sender_ack_addr;
    datagram_t datagram = {0};
    datagram.index = 0;
    datagram.free_space = 0;
    socklen_t sender_addr_len = sizeof(sender_data_addr), sender_ack_addr_len = sizeof(sender_ack_addr);
    uLong crc;
    FILE *fr = NULL;
    int received = -1;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = MAX_TIMEOUT;
    unsigned char received_hash[SHA256_BLOCK_SIZE];
    char *file_name = "OUTPUT1.jpg";

    setup_addr(&receiver_addr, RECEIVER_PORT, RECEIVER_ADDRESS);
    setup_addr(&sender_data_addr, NETDERPER_SENDER_DATA_PORT, SENDER_DATA_ADDRESS);
    setup_addr(&sender_ack_addr, NETDERPER_SENDER_ACK_PORT, SENDER_ACK_ADDRESS);
    bind_socket(socket_recv, &receiver_addr);

    setsockopt(socket_recv, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    printf("Listening on port %d...\n", RECEIVER_PORT);
    
    while (1) {
        if (receive_data(socket_recv, &datagram, &sender_data_addr, &sender_addr_len) < 0) {
            fprintf(stderr, "Error receiving initial data\n");
            continue; // Or handle the error as needed
        }
        
        crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, (const Bytef*)datagram.data, (uInt)(sizeof(datagram.data)));
        if (crc == datagram.crc) {
            fprintf(stderr, "Hash received\n");
            memcpy(received_hash, datagram.data, SHA256_BLOCK_SIZE);
            send_ack(socket_recv, 1, sender_ack_addr, sender_ack_addr_len);
            break;
        }
    }

    window_t window = {0};
    for (int i = 0; i < WINDOW_SIZE; i++) {
        window.acks[i].index = -1;
    }

    fr = fopen(file_name, "wb");
    if (!fr) {
        perror("Failed to open file");
        close(socket_recv);
        exit(EXIT_FAILURE);
    }

    int eof = 0;
    while (true) {
        if (receive_data(socket_recv, &datagram, &sender_data_addr, &sender_addr_len) < 0) {
            fprintf(stderr, "Error receiving data\n");
            continue; 
        }
        //fprintf(stderr, "Received %d expected %d\n", datagram.index, window.index);
        received = datagram.index;
        crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, (const Bytef*)datagram.data, (uInt)(sizeof(datagram.data)));

        if (received < window.index + WINDOW_SIZE && crc == datagram.crc) {
            int k = received - window.index;
            if (received >= window.index && window.acks[k].index == -1) {
                window.datagrams[k] = datagram;
                window.acks[k].index = received;
                if (datagram.free_space != 0) {
                    fprintf(stderr, "EOF\n");
                    eof = 1;
                }
                send_ack(socket_recv, received, sender_ack_addr, sender_ack_addr_len);
            } else {
                send_ack(socket_recv, received, sender_ack_addr, sender_ack_addr_len);
            }
        } else {
            send_ack(socket_recv, -1, sender_ack_addr, sender_ack_addr_len);
        }

        int shift = acked_shift(window);
        if (shift) {
            for (int i = 0; i < WINDOW_SIZE; i++) {
                if (i < shift) {
                    fprintf(stderr, "Writing %d\n", window.datagrams[i].index);
                    fwrite(window.datagrams[i].data, 1, DATA_SIZE - window.datagrams[i].free_space, fr);
                     fprintf(stderr, " %d < %d = %d\n", i + shift , WINDOW_SIZE,i + shift < WINDOW_SIZE);
                }
                if (i + shift < WINDOW_SIZE) {
                    printf("shifted %d index %d from %d to %d\n",shift , window.index, window.index + i + shift, window.index + i);
                    printf("from %d to %d, with value %d and free space %d\n", i + shift, i,  window.acks[i + shift].index, window.datagrams[i + shift].free_space);
                    
                    window.datagrams[i] = window.datagrams[i + shift];
                    window.acks[i] = window.acks[i + shift];
                    window.datagrams[i + shift] = (datagram_t){0};
                    window.acks[i + shift].index = -1;
                } else {
                    window.acks[i].index = -1;
                    window.datagrams[i] = (datagram_t){0};
                }
            }
            window.index += shift;
        }
        if (eof && window.index > eof) {
            break;
        }
    }

    fclose(fr);

    SHA256_CTX hash;
    sha256_init(&hash);
    unsigned char final_hash[SHA256_BLOCK_SIZE] = {0};
    compute_file_hash(file_name, final_hash);
    printf("File hash:\n");
    print_sha256_hash(final_hash);
    printf("Received hash:\n");
    print_sha256_hash(received_hash);

    close(socket_recv);
    return 0;
}
