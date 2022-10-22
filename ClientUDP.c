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
int main(void){
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];
    int server_struct_length = sizeof(server_addr);
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    // Create socket:
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
    
    char *SYN = "SYN";
    char *ACK = "ACK";

    sendto(socket_desc, SYN, strlen(SYN), 0,
            (struct sockaddr*)&server_addr, server_struct_length);

    recvfrom(socket_desc, server_message, sizeof(server_message), 0,
            (struct sockaddr*)&server_addr, &server_struct_length);

    
    if(strncmp("SYN-ACK", server_message, 3) == 0)
    {
        int nvx_port = ((int)server_message[8]-48)*1000 + ((int)server_message[9]-48)*100+ ((int)server_message[10]-48)*10 + ((int)server_message[11]-48);
        printf("port : %d\n", nvx_port);

        sendto(socket_desc, ACK, strlen(ACK), 0,
            (struct sockaddr*)&server_addr, server_struct_length);
        
        
        // CONNECTE - rejoint socket direct entre moi et le serveur 
        server_addr.sin_port = htons(nvx_port);

        // reception d'un fichier : 
        printf("Reception du fichier...\n");
        memset(server_message, '\0', sizeof(server_message));
        recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                (struct sockaddr*)&server_addr, &server_struct_length);
        
        FILE *fp = fopen ("FichierTexteRecu.txt", "w"); 
        if(fp == NULL) {
            perror ("Error in opening file");
            exit(-1);
        }

        fwrite (server_message, 1, sizeof(server_message), fp); 
        fclose(fp);
        printf("Fichier envoyé !\n");

        while (1)
        {
            memset(server_message, '\0', sizeof(server_message));
            memset(client_message, '\0', sizeof(client_message));

            // Get input from the user:
            printf("Enter message: ");
            gets(client_message);
            
            // Send the message to server:
            if(sendto(socket_desc, client_message, strlen(client_message), 0,
                (struct sockaddr*)&server_addr, server_struct_length) < 0){
                printf("Unable to send message\n");
                return -1;
            }
            
            // Receive the server's response:
            if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                (struct sockaddr*)&server_addr, &server_struct_length) < 0){
                printf("Error while receiving server's msg\n");
                return -1;
            }
            
            printf("Server's response: %s\n", server_message);
        }
    }
    // Close the socket:
    close(socket_desc);
    
    return 0;
}

