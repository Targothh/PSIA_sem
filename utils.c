#include "packet.h"


void setup_addr(struct sockaddr_in *addr, int port, char *ip){
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(ip);
}
