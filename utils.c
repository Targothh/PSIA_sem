#include "packet.h"


void setup_addr(struct sockaddr_in *addr, int port, char *ip){
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(ip);
}


int init_socket(void) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}


void bind_socket(int sockfd, struct sockaddr_in *addr) {
    if (bind(sockfd, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
        perror("Couldn't bind to the port");
        exit(EXIT_FAILURE);
    }
}

void compute_file_hash(const char* filename, unsigned char* final_hash) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "File not found!\n");
        exit(EXIT_NOT_FOUND);
    }

    SHA256_CTX hash;
    sha256_init(&hash);
    unsigned char data[DATA_SIZE];
    int size;
    
    while ((size = fread(data, 1, DATA_SIZE, file)) != 0) {
        sha256_update(&hash, data, size);
    }
    
    sha256_final(&hash, final_hash);
    fclose(file);
}

void print_sha256_hash(const unsigned char* hash) {
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

uint16_t acked_shift(window_t window) {
    for (int i = 0; i < WINDOW_SIZE; i++) {
        if (window.acks[i].index == -1) {
            return i;
        }
    }
    return WINDOW_SIZE;
}
