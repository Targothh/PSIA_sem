#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define PACKET_SIZE 1024

typedef struct {
    int id;
    char index;
    unsigned char data[1016];
} datagram_t ;

 
int main(int argc, char *argv[]){
    if(argc != 4 && argc != 5){
        printf("Invalid number of arguments!\n");
        exit(-1);
    }
    int socket_init;
    struct sockaddr_in sender_addr, reciver_addr;
    datagram_t datagram;
    unsigned char datagram_arr[PACKET_SIZE];
    datagram.id = 0;
    memset(datagram_arr, '\0', sizeof(datagram_arr));
    if ((socket_init = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("Cannot create socket\n");
        exit(-1);
    }
    if(strcmp(argv[1], "-r") == 0){ //reciver
        reciver_addr.sin_family = AF_INET;
        char name[32];
        reciver_addr.sin_port = htons(atoi(argv[3]));
        reciver_addr.sin_addr.s_addr = inet_addr(argv[2]);
        if(bind(socket_init, (struct sockaddr*)&reciver_addr, sizeof(reciver_addr)) < 0){
            printf("Couldn't bind to the port\n");
            exit(-1);
        }
        printf("Listening on port %s...\n", argv[3]);
        //init phase - sockaddr_in of client and size of data
        if (recv(socket_init, datagram_arr , sizeof(datagram_arr), 0) > 0){ //reciving from anyone
            memcpy(&datagram, datagram_arr, sizeof(datagram_arr));
            memcpy(&sender_addr, datagram.data, sizeof(sender_addr));
            memcpy(name, datagram.data + sizeof(sender_addr), 32);
        }
        printf("Recived PORT: %d Recived FILENAME: %s", ntohs(sender_addr.sin_port), name);
        FILE *fr;
        fr = fopen("OUTPUT1.svg", "wb");
        while(datagram.id == 0){
            recvfrom(socket_init, datagram_arr, sizeof(datagram_arr),0, (struct sockaddr *)&sender_addr,(unsigned int*) sizeof(sender_addr));
            memcpy(&datagram, datagram_arr, sizeof(datagram_arr));
            fwrite(datagram.data, sizeof(datagram.data)-datagram.id, 1, fr);
        }
        fclose(fr);

    } else if(strcmp(argv[1], "-s") == 0){ //sender
        /*nepotrebne*/
        sender_addr.sin_family = AF_INET;
        sender_addr.sin_port = htons(5005);
        sender_addr.sin_addr.s_addr = inet_addr("10.4.5.42");
        char name[32];
        strcpy(name, argv[4]);

        if(bind(socket_init, (struct sockaddr*)&sender_addr, sizeof(sender_addr)) < 0){
            printf("Couldn't bind to the port\n");
            exit(-1);
        }
        /*nepotrebne*/
        reciver_addr.sin_family = AF_INET;
        reciver_addr.sin_port = htons(atoi(argv[3]));
        reciver_addr.sin_addr.s_addr = inet_addr(argv[2]);
        memcpy(datagram.data, &sender_addr, sizeof(sender_addr)); 
        memcpy(datagram.data + sizeof(sender_addr), name, sizeof(name)); // jmeno souboru
        memcpy(datagram_arr, &datagram, sizeof(datagram));
        if(sendto(socket_init, datagram_arr, sizeof(datagram_arr), 0, (struct sockaddr*)&reciver_addr, sizeof(reciver_addr)) < 0){
            printf("Error while sending info\n");
            exit(-1);
        }
        printf("Initial packet sent sucessfuly\n");
        FILE *fw;
        if ((fw = fopen(name, "rb")) == NULL){
            printf("File not found!\n");
            exit(404);
        }
        int id;
        while(datagram.id == 0){
            if((id=fread(datagram.data, 1, sizeof(datagram.data), fw)) != sizeof(datagram.data)){
                datagram.id = sizeof(datagram.data) - id;
            }                                                                  
            printf("Packet sent ID: %d\nSIZE: %lu, ID:%d ",datagram.id, sizeof(datagram), id);
            memcpy(datagram_arr, &datagram, sizeof(datagram));
            sendto(socket_init, datagram_arr, sizeof(datagram_arr), 0, (struct sockaddr*)&reciver_addr, sizeof(reciver_addr));
            }
        fclose(fw);
    } else {
        printf("Invalid flag!\n");
        exit(-1);
    }
    close(socket_init);

}
