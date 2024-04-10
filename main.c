#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define PACKET_SIZE 1024


 
int main(int argc, char *argv[]){
    if(argc != 4){
        printf("Invalid number of arguments!\n");
        exit(-1);
    }
    int socket_init;
    struct sockaddr_in sender_addr, reciver_addr;
    char data[PACKET_SIZE];
    memset(data, '\0', sizeof(data));
    socklen_t reciver_lenght = sizeof(reciver_addr);
    if ((socket_init = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Cannot create socket\n");
        exit(-1);
    }
    if(strcmp(argv[1], "-r") == 0){ //reciver
        reciver_addr.sin_family = AF_INET;
        reciver_addr.sin_port = htons(atoi(argv[3]));
        reciver_addr.sin_addr.s_addr = inet_addr(argv[2]);
        if(bind(socket_init, (struct sockaddr*)&reciver_addr, sizeof(reciver_addr)) < 0){
            printf("Couldn't bind to the port\n");
            exit(-1);
        }
        printf("Listening on port %s...\n", argv[3]);
        //init phase - sockaddr_in of client and size of data
        if (recv(socket_init, &reciver_addr , sizeof(reciver_addr), 0) < 0){
            printf("Error while reciving sender's info\n");
            exit(-1);
        }
        

    } else if(strcmp(argv[1], "-s") == 0){ //sender
        /*nepotrebne*/
        sender_addr.sin_family = AF_INET;
        sender_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        sender_addr.sin_port = htons(5005);

        /*nepotrebne*/
        reciver_addr.sin_family = AF_INET;
        reciver_addr.sin_port = htons(atoi(argv[3]));
        reciver_addr.sin_addr.s_addr = inet_addr(argv[2]);
        if(sendto(socket_init, &sender_addr, sizeof(sender_addr), 0, (struct sockaddr*)&reciver_addr, &reciver_lenght) < 0){
            printf("Error while sending info\n");
            exit(-1);
        }
    } else {
        printf("Invalid flag!\n");
        exit(-1);
    }

}
