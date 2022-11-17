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

#define BUFFSIZE 500
#define PORT 3000
#define SYN "SYN"
#define ACK "ACK"
#define SYN_ACK "SYN-ACK"
#define FIN "FIN"

void RemoveChar(char *str);
int check(int exp, const char *msg);
int extract(char *chaine, char *sousChaine);

int main(void){
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[BUFFSIZE], client_message[BUFFSIZE];
    int server_struct_length = sizeof(server_addr);
    
    // Clean buffers:
    memset(server_message, '\0', BUFFSIZE);
    memset(client_message, '\0', BUFFSIZE);
    
    // Create socket:
    check((socket_desc = socket(AF_INET, SOCK_DGRAM, 0)), 
        "Échec de la création du socket");
    printf("Socket créee avec succès \n");
    
    // Fixe port & IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Three-way handshake avec le serveur:
    sendto(socket_desc, SYN, strlen(SYN), 0,
            (struct sockaddr*)&server_addr, server_struct_length);

    recvfrom(socket_desc, server_message, sizeof(server_message), 0,
            (struct sockaddr*)&server_addr, &server_struct_length);
    if(strncmp("SYN-ACK", server_message, 3) == 0)
    {
        int nvx_port = ((int)server_message[8]-48)*1000 + ((int)server_message[9]-48)*100+ ((int)server_message[10]-48)*10 + ((int)server_message[11]-48);
        printf("nouveau port : %d\n", nvx_port);

        // Creation socket UDP directe avec le client:
        server_addr.sin_port = htons(nvx_port);

        sendto(socket_desc, ACK, strlen(ACK), 0,
            (struct sockaddr*)&server_addr, server_struct_length);
        
        // Protocole connecté !
        printf("Bien connecté ! \n");

        // reception d'un fichier : https://gist.github.com/XBachirX/865b00ba7a7c86b4fc2d7443b2c4f238 
        // strcpy : https://www.programiz.com/c-programming/library-function/string.h/strcpy 
        // strcat : https://koor.fr/C/cstring/strcat.wp 
        printf("Reception du fichier...\n");

        char num_seq[7];
        int FinTransmission = 0;
        if (remove("FichierTexteReçu.txt") == 0){
            printf(" Le fichier a été supprimé avec succès.\n");
        }    
        else
        {    
            printf("Impossible de supprimer le fichier\n");
            perror("Erreur");    
        }
        FILE *fp = fopen ("FichierTexteReçu.txt", "a"); 
        if(fp == NULL) {
            perror ("Erreur lors de l'ouverture du fichier");
            exit(-1);
        }
        char buffer_ACK[10];
        while(FinTransmission == 0)
        {
            memset(server_message, '\0', BUFFSIZE);
            recvfrom(socket_desc, server_message, sizeof(server_message), 0,
                    (struct sockaddr*)&server_addr, &server_struct_length);
            if(strncmp("FIN", server_message, 3) == 0)
            { 
                FinTransmission = 1;
            } else {
                memset(num_seq, '\0', 7);
                strncpy (num_seq,server_message,6);

                RemoveChar(server_message);
                fwrite (server_message, 1, BUFFSIZE-6, fp); 
                
                printf("reception N° %s\n", num_seq);

                memset(buffer_ACK, '\0', 10);
                sprintf(buffer_ACK, "%s%s", ACK, num_seq);
                if(sendto(socket_desc, buffer_ACK, strlen(buffer_ACK), 0,
                        (struct sockaddr*)&server_addr, server_struct_length)<0){
                        printf("Envoie impossible\n");
                        return -1;
                }
            } 
        } 
        sendto(socket_desc, FIN, strlen(FIN), 0,
                    (struct sockaddr*)&server_addr, server_struct_length);
        fclose(fp);
    } else {
        printf("erreur Threeway handshake \n");
    }

    close(socket_desc);
    return 0;
}

void RemoveChar(char *str){
    int x = 0;
    while(str[x] != '\0'){
        str[x] = str[x+6];
        x++;
    }
}

int extract(char *chaine, char *sousChaine)
{
  int i;
  for( i = 0; i < 6; i++){
    sousChaine[i] = chaine[i];
  }  
  return 0;  
} 

int check(int exp, const char *msg){
    if (exp == (-1) ) {
        perror(msg);
        exit(1);
    }
    return exp;
}

