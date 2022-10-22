#include <stdio.h>
#include <stdlib.h>
// entête pour la socket
#include <sys/types.h>  
#include <sys/socket.h>
// manipulation des adresse sockaddr
#include <string.h>
#include <netinet/in.h>
// htons..
#include <arpa/inet.h>
// pour close
#include <unistd.h>
// https://www.educative.io/answers/how-to-implement-udp-sockets-in-c

int main(void){
    int socket_desc, Sous_socket;
    struct sockaddr_in server_addr, client_addr, sc_addr;
    char server_message[2000], client_message[2000];
    int client_struct_length = sizeof(client_addr);
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    // Create UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Connection
    char *SYN_ACK = "SYN-ACK 4000";

    recvfrom(socket_desc, client_message, sizeof(client_message), 0,
            (struct sockaddr*)&client_addr, &client_struct_length);
    
    if(strncmp("SYN", client_message, 3) == 0)
    {
        sendto(socket_desc, SYN_ACK, strlen(SYN_ACK), 0,
            (struct sockaddr*)&client_addr, client_struct_length);
    }
    
    recvfrom(socket_desc, client_message, sizeof(client_message), 0,
            (struct sockaddr*)&client_addr, &client_struct_length);

    if(strncmp("ACK", client_message, 3) == 0)
    {
        // CONNECTE - crée socket direct avec le client 

        // Create UDP socket:
        Sous_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(Sous_socket < 0){
            printf("Error while creating socket\n");
            return -1;
        }
        printf("Sous Socket created successfully\n");
        
        // Set port and IP:
        sc_addr.sin_family = AF_INET;
        sc_addr.sin_port = htons(4000);
        sc_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        // Bind to the set port and IP:
        if(bind(Sous_socket, (struct sockaddr*)&sc_addr, sizeof(sc_addr)) < 0){
            printf("Couldn't bind to the port\n");
            return -1;
        }
        printf("Done with binding\n");

        // envoie d'un fichier : 
        printf("Envoie du fichier...\n");
        FILE *fp = fopen("FichierTexte.txt","r");
        if(fp == NULL) {
            perror ("Error in opening file");
            exit(-1);
        }

        memset(server_message, '\0', sizeof(server_message));
        while ( ! feof(fp) ) {
            fread(server_message, 1, sizeof(server_message), fp);
            sendto(Sous_socket, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) ;
        }
        fclose(fp);
        printf("Fichier envoyé !\n");


        printf("Listening for incoming messages...\n");
        while(1)
        {
            memset(server_message, '\0', sizeof(server_message));
            memset(client_message, '\0', sizeof(client_message));

            // Receive client's message:
            if (recvfrom(Sous_socket, client_message, sizeof(client_message), 0,
                (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                printf("Couldn't receive\n");
                return -1;
            }
            printf("Received message from IP: %s and port: %i\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            printf("Msg from client: %s\n", client_message);
            
            // Respond to client:
            strcpy(server_message, client_message);
            
            if (sendto(Sous_socket, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                printf("Can't send\n");
                return -1;
            }
        }
        close(Sous_socket);
    }
    // Close the socket:
    close(socket_desc);
    return 0;
}
