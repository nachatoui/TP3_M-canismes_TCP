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

#define BUFFSIZE 500
#define PORT 3000
#define SYN "SYN"
#define ACK "ACK"
#define SYN_ACK "SYN-ACK "
#define FIN "FIN"

int check(int exp, const char *msg);
char* Num_Sequence(int num_seq, char* char_num_seq);

int main(void){
    int socket_desc, Sous_socket, num_client = 1;
    struct sockaddr_in server_addr, client_addr, ss_addr;
    char server_message[BUFFSIZE], client_message[BUFFSIZE];
    int client_struct_length = sizeof(client_addr);
    
    // Vide les buffers:
    memset(server_message, '\0', BUFFSIZE);
    memset(client_message, '\0', BUFFSIZE);
    
    // Creation socket UDP :
    check((socket_desc = socket(AF_INET, SOCK_DGRAM, 0)), 
        "Échec de la création du socket");
    printf("Socket créée avec succès !\n");
    
    // Fixe port & IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind aux port & @IP:
    check(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)), 
        "Bind Failed!"); 
    printf("Bind réussie\n");

    // Three-way handshake avec un client:
    int nvx_port = PORT + num_client;

    char buffer_SYN_ACK[13];
    memset(buffer_SYN_ACK, '\0', 13);    

    sprintf(buffer_SYN_ACK, "%s%d", SYN_ACK, nvx_port);
    printf("syn ack message : %s\n", buffer_SYN_ACK);

    recvfrom(socket_desc, client_message, BUFFSIZE, 0,
            (struct sockaddr*)&client_addr, &client_struct_length);
    if(strncmp("SYN", client_message, 3) == 0)
    {
        sendto(socket_desc, buffer_SYN_ACK, strlen(buffer_SYN_ACK), 0,
            (struct sockaddr*)&client_addr, client_struct_length);
    
        // Creation socket UDP directe avec le client:
        check((Sous_socket = socket(AF_INET, SOCK_DGRAM, 0)), 
        "Échec de la création du socket");
        printf("Sous Socket créee avec succès ! \n");
        
        // Fixe port & @IP:
        ss_addr.sin_family = AF_INET;
        ss_addr.sin_port = htons(nvx_port);
        num_client += 1;  
        ss_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        // Bind to the set port and IP:
        check(bind(Sous_socket, (struct sockaddr*)&ss_addr, sizeof(ss_addr)), 
        "Bind Failed!"); 
        printf("Bind réussie\n");
        
        recvfrom(Sous_socket, client_message, BUFFSIZE, 0,
                (struct sockaddr*)&client_addr, &client_struct_length);
        if(strncmp("ACK", client_message, 3) == 0)
        {
            // Protocole connecté !
            printf("Bien connecté ! \n");

            // Envoie d'un fichier: 
            printf("Envoie du fichier...\n");
            FILE *fp = fopen("FichierTexte.txt","r");
            if(fp == NULL) {
                perror ("Error in opening file");
                exit(-1);
            }
            memset(server_message, '\0', BUFFSIZE);
            int num_seq = 1;
            char char_num_seq[6]; 
            // A faire sur 2 fork different pour pouvoir ecouter et envoyer au même moment
            while ( ! feof(fp) ) {
                fread(server_message, 1, BUFFSIZE-6, fp);
                Num_Sequence(num_seq, char_num_seq);
                fflush(fp);
                sprintf(server_message, "%s%s", char_num_seq, server_message);

                sendto(Sous_socket, server_message, strlen(server_message), 0,
                    (struct sockaddr*)&client_addr, client_struct_length) ;
                printf("Num sequence : %d \n", num_seq);
                num_seq += 1;
            }
            fclose(fp);
            sendto(Sous_socket, FIN, strlen(FIN), 0,
                    (struct sockaddr*)&client_addr, client_struct_length);
            printf("Fichier envoyé !\n");
            printf("Message reçu de l'@IP: %s et du port: %i\n",
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            while(1)
            {
                // reception du ack
                memset(client_message, '\0', BUFFSIZE);
                if (recvfrom(Sous_socket, client_message, BUFFSIZE, 0,
                    (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                    printf("Erreur lors de la reception\n");
                    return -1;
                }
                printf("Message du client: %s\n", client_message);
            }
            close(Sous_socket);
        }
    } else {
        printf("erreur Threeway handshake\n");
    }
    close(socket_desc);
    return 0;
}

int check(int exp, const char *msg){
    if (exp == (-1) ) {
        perror(msg);
        exit(1);
    }
    return exp;
}

char* Num_Sequence(int num_seq, char* char_num_seq){
    if(num_seq > 999999){
        return NULL;
    }
    char k[6];
    sprintf(k, "%d",num_seq);
    int len_k = strlen(k);
    char ki [6] ="000000";
    sprintf(ki+6-len_k,"%d",num_seq);
    sprintf(char_num_seq,"%s",ki);
    return char_num_seq;
}
