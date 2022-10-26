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

#define BUFFSIZE 2000
#define PORT 3000

int main(void){
    int socket_desc, Sous_socket, num_client = 1;
    struct sockaddr_in server_addr, client_addr, ss_addr;
    char server_message[BUFFSIZE], client_message[BUFFSIZE];
    int client_struct_length = sizeof(client_addr);
    
    // Vide les buffers:
    memset(server_message, '\0', BUFFSIZE);
    memset(client_message, '\0', BUFFSIZE);
    
    // Creation socket UDP :
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Erreur lors de la création de la socket\n");
        return -1;
    }
    printf("Socket créee avec succès \n");
    
    // Fixe port & IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind aux port & @IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Erreur lors du bind\n");
        return -1;
    }
    printf("Bind réussie\n");

    // Three-way handshake avec un client:
    int nvx_port = PORT + num_client;
    char char_nvx_port[5]; 
    sprintf(char_nvx_port, "%d", nvx_port);

    char *SYN_ACK = "SYN-ACK ";  
    char buffer[30];
    strcat(buffer, SYN_ACK);
    strcat(buffer, char_nvx_port);
    printf("syn ack message : %s\n", buffer);

    recvfrom(socket_desc, client_message, BUFFSIZE, 0,
            (struct sockaddr*)&client_addr, &client_struct_length);
    if(strncmp("SYN", client_message, 3) == 0)
    {
        sendto(socket_desc, buffer, strlen(buffer), 0,
            (struct sockaddr*)&client_addr, client_struct_length);
    }
    
    recvfrom(socket_desc, client_message, BUFFSIZE, 0,
            (struct sockaddr*)&client_addr, &client_struct_length);
    if(strncmp("ACK", client_message, 3) == 0)
    {
        // Protocole connecté !
        printf("Bien connecté ! \n");

        // Creation socket UDP directe avec le client:
        Sous_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(Sous_socket < 0){
            printf("Erreur lors de la création de la socket\n");
            return -1;
        }
        printf("Sous Socket créee avec succès ! \n");
        
        // Fixe port & @IP:
        ss_addr.sin_family = AF_INET;
        ss_addr.sin_port = htons(nvx_port);
        num_client += 1;  
        ss_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        // Bind to the set port and IP:
        if(bind(Sous_socket, (struct sockaddr*)&ss_addr, sizeof(ss_addr)) < 0){
            printf("Erreur lors du bind\n");
            return -1;
        }
        printf("Done with binding\n");

        // Envoie d'un fichier: 
        printf("Envoie du fichier...\n");
        FILE *fp = fopen("FichierTexte.txt","r");
        if(fp == NULL) {
            perror ("Error in opening file");
            exit(-1);
        }
        memset(server_message, '\0', BUFFSIZE);
        while ( ! feof(fp) ) {
            fread(server_message, 1, BUFFSIZE, fp);
            sendto(Sous_socket, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) ;
        }
        fclose(fp);
        printf("Fichier envoyé !\n");

        printf("Listening for incoming messages...\n");
        while(1)
        {
            memset(server_message, '\0', BUFFSIZE);
            memset(client_message, '\0', BUFFSIZE);

            // Reception du message du client:
            if (recvfrom(Sous_socket, client_message, BUFFSIZE, 0,
                (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                printf("Erreur lors de la reception\n");
                return -1;
            }
            printf("Message reçu de l'@IP: %s et du port: %i\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            printf("Message du client: %s\n", client_message);
            
            // Reponse du serveur:
            strcpy(server_message, client_message);
            if (sendto(Sous_socket, server_message, strlen(server_message), 0,
                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                printf("Envoie impossible\n");
                return -1;
            }
        }
        close(Sous_socket);

    } else {
        printf("erreur Threeway handshake");
    }
    
    close(socket_desc);
    return 0;
}
