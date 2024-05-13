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
