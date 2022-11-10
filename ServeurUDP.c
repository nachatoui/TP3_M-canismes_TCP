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
// pour le wait 
#include<sys/wait.h>

#define BUFFSIZE 500
#define PORT 3000
#define SYN "SYN"
#define ACK "ACK"
#define SYN_ACK "SYN-ACK "
#define FIN "FIN"

int check(int exp, const char *msg);
char* Num_Sequence(int num_seq, char* char_num_seq);
int Creation_Socket (int port, struct sockaddr_in server_addr);

int main(void){
    int socket_desc, Sous_socket, num_client = 1;
    struct sockaddr_in server_addr, client_addr, ss_addr;
    char server_message[BUFFSIZE], client_message[BUFFSIZE];
    int client_struct_length = sizeof(client_addr);
    
    // Vide les buffers:
    memset(server_message, '\0', BUFFSIZE);
    memset(client_message, '\0', BUFFSIZE);
    
    // Creation socket UDP :
    socket_desc = Creation_Socket (PORT, server_addr);

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
        int Sous_socket = Creation_Socket (nvx_port, ss_addr);
        num_client += 1;  
        
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
            int FinTransmission = 0;
            char lecture[BUFFSIZE-6];
            pid_t childpid_envoieFich;

            // A faire sur 2 fork different pour pouvoir ecouter et envoyer au même moment 
            childpid_envoieFich = fork();
            if (childpid_envoieFich == -1) {
                perror("fork");
                exit(-1);
            } else if (childpid_envoieFich == 0) { 
                // dans le processus fils
                while ( ! feof(fp) ) { 
                    // Envoie du fichier 
                    memset(server_message, '\0', BUFFSIZE);
                    fread(lecture, 1, BUFFSIZE-6, fp);
                    Num_Sequence(num_seq, char_num_seq);
                    fflush(fp);
                    sprintf(server_message, "%s%s", char_num_seq, lecture);

                    sendto(Sous_socket, server_message, strlen(server_message), 0,
                        (struct sockaddr*)&client_addr, client_struct_length) ;
                    num_seq += 1;
                }
                fclose(fp);
                sendto(Sous_socket, FIN, strlen(FIN), 0,
                        (struct sockaddr*)&client_addr, client_struct_length);
                exit(-1);
            }
            
            printf("Message reçu de l'@IP: %s et du port: %i\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            pid_t childpid_receptionFich = fork();
            if (childpid_receptionFich == -1) {
                perror("fork");
                exit(-1);
            } else if (childpid_receptionFich == 0) { 
                while(FinTransmission == 0)
                {
                    // reception du ack
                    if(strncmp("FIN", client_message, 3) == 0)
                    { 
                        FinTransmission = 1;
                    } else {
                        memset(client_message, '\0', BUFFSIZE);
                        if (recvfrom(Sous_socket, client_message, BUFFSIZE, 0,
                            (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                            printf("Erreur lors de la reception\n");
                            return -1;
                        }
                        printf("%s\n", client_message);
                    }
                }
                exit(-1);
            }
            int statu1, statu2;
            // Attend la fin des process fils
            waitpid(childpid_envoieFich, &statu1, 0);
            waitpid(childpid_receptionFich, &statu2, 0);
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

int Creation_Socket (int port, struct sockaddr_in server_addr)
{      
    // Creation socket UDP :
    int descripteur_socket;
    check((descripteur_socket = socket(AF_INET, SOCK_DGRAM, 0)), 
        "Échec de la création du socket");
    printf("Socket créée avec succès !\n");
    
    // Fixe port & IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind aux port & @IP:
    check(bind(descripteur_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)), 
        "Bind Failed!"); 
    printf("Bind réussie\n");

    return (descripteur_socket);
}

