#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

extern int errno;

int port;
int logat = 0;

void *receveMSJ(void * arg);
pthread_t listen_for_msj;


//// AFISEAZA RASP PRIMIT DE LA SERVER FIIND ATENT NON-STOP LA CE TRIMITE
void *receveMSJ(void *arg)
{
    int server_descriptor = *((int *)arg);
   // free(arg);
    char raspuns[2000];
    char mesaj[2000];
    int read_msj, write_msj;

    while(1)
    {
        memset(raspuns, 0, sizeof(raspuns));
        read_msj = read(server_descriptor, &raspuns, sizeof(raspuns));
        if(read_msj == -1){
            perror("[client]Eroare la write() de la client.\n");
            exit(EXIT_FAILURE);
        }
        raspuns[strlen(raspuns)] = '\0';
     
        if(strcmp(raspuns, "S-a dealocat utilizatorul.") == 0)
        {
            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        } else if(strcmp(raspuns, "Contul a fost sters cu succes!") == 0){

            printf("[server :]%s\n", raspuns);
            fflush(stdout);       

        } else if(strcmp(raspuns, "Mesajul a fost sters cu succes.") == 0) {

            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        } else if(strcmp(raspuns, "Trebuie sa fii conectat ca sa stergi mesaje.") == 0){

            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        } else if(strcmp(raspuns, "Utilizatorul a parsit aplicatia.") == 0){

            printf("[server :]Utilizatorul a parasit aplicati.");
            fflush(stdout);
            
        } else if(strcmp(raspuns, "User deja existent, te rog sa iti cauti alt username!") == 0) {

            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        } else if(strcmp(raspuns, "Parola deja existent, te rog sa iti cauti alta parola!") == 0) {

            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        } else if(strcmp(raspuns, "User inexistent, trebuie sa va faceti cont mai intai!") == 0) {

            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        }else if(strcmp(raspuns, "User deja logat, te rog sa te deloghezi pentru a te loga cu alt cont!") == 0) {

            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        }else if(strcmp(raspuns, "Nu putem accesa mesajele dumneavoastra daca nu sunteti logat!") == 0) {

            printf("[server :]%s\n", raspuns);
            fflush(stdout);

        }else if(strcmp(raspuns,"Ai intrat din conversatie.") == 0){
            
             printf("[server :]%s\n", raspuns);
             fflush(stdout);

        }else {
            printf("\nMesaj: %s\n", raspuns);
            fflush(stdout);
        }
    }
}
void help()
{
  printf("Salut!\n Bine ai venit la ***OfflineMessenger***!\n");
  printf("Ai afisat setul de comenzi:\n");
  printf("'register' <username> <parola>\n");
  printf("'login' <username> <parola>\n");
  printf("arata 'online_users' or 'offline_users'");
  printf("'history'\n");
  printf("'sterge_cont' cu <username>\n");
  printf("'send messege' la <username> (while online or offline)\n");
  printf("arata 'offline_messeges' \n");
  printf("'reply' <messege number> \n"); 
  printf("'iesire conversatie'\n");
  printf("'logout'\n");
  printf("'exit'\n");
  printf("Scrie 'help' ca sa primesti manualul de comenzi.\n");
}

int main(int argc, char *argv[]) {
    help();
    int sd;
    struct sockaddr_in server;
    char message[100];
    char raspuns[1000] = "";
    int cod_read, cod_write;   
    int logat = 0;
    char id_mes[2000];

     ///////VERIFICA DACA AM DAT DESTULE ARGUMENTE
    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

//////// CREAM CANALUL DE COMUNICATIE SOCKET
    port = atoi(argv[2]);
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return errno;
    }

////////INSERAM IN STRUCTUL SPECIFIC DATELE DIN APELUL PROGRAMULUI
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

/////////CONECTAREA LA IP SI PORT LA CARE SE GASESTE SERVER UL 
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Eroare la connect().\n");
        return errno;
    }
////////////////////////////////////////////
////////INCEPE COMUNICAREA EFECTIVA CU SERVER :
//////// MAI INTAI VOM AVEA PARTEA DE SEND() => CE TRIMITEM LA SERVER
///////APOI INTR UN THREAD VOM AVEAM CE RASPUNSURI PRINDE DE LA SERVER PT A FII MEREU PE FAZA (SI IN CAZUL COMUNICARII INTRE CLIENTI)
///////////////////////////////////////////
          int *sd_ptr = malloc(sizeof(int));
          *sd_ptr = sd;      

 while(1){
    sleep(2);
    printf("\n[client]Introduceti comanda dorita: ");
    fflush(stdout);
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0'; 

     cod_write = write(sd, &message, strlen(message));    
        if(cod_write == -1)
        {
            perror("[client]Eroare la write() catre server.\n");
            exit(EXIT_FAILURE);
        }

    if(strcmp( message, "Quit.")== 0 ){
        printf("> %s\n", "Quit.");
        break;
   }else{
    
    printf("[client]Am citit: %s\n", message); 
    pthread_create(&listen_for_msj,NULL,&receveMSJ,(void *)sd_ptr);
    pthread_detach(listen_for_msj);

    if(strcmp (message, "register" ) == 0 ){
        printf("REGISTER: \n");
        char username[30];
        char password[30];
        char toSend[100];

        printf("Introdu nume utilizator: ");
        fgets(username, 30, stdin);
        username[strlen(username) - 1] = '\0';

        printf("Introduceti parola: ");
        fgets(password, 30, stdin);
        password[strlen(password) - 1] = '\0';  
    
        snprintf(toSend, sizeof(toSend), "register:%s|%s", username, password);
        
        if (write(sd, toSend, strlen(toSend)) <= 0) {
            perror("[client]Eroare la send() spre server.\n");
            break;}

    }else if( strcmp(message, "login") == 0){
        logat=1;
        printf("LOGIN: \n");
        char username[30];
        char password[30];
        char toSend[100];

        printf("Introdu nume utilizator: ");
        fgets(username, 30, stdin);
        username[strlen(username) - 1] = '\0';  

        printf("Introduceti parola: ");
        fgets(password, 30, stdin);
        password[strlen(password) - 1] = '\0';  
        snprintf(toSend, sizeof(toSend), "login:%s|%s", username, password);
        
        if (write(sd, toSend, strlen(toSend)) <= 0) {
            perror("[client]Eroare la send() spre server.\n");
            break;}    

    }else if(strcmp(message, "history")== 0){
        printf("HISTORY: \n");
        char nume2[30];
        char toSend[100];

        printf("Introduceti cu cine vreti sa vedeti istoricul: ");
        fgets(nume2, 30, stdin);
        nume2[strlen(nume2) - 1] = '\0';  
        snprintf(toSend, sizeof(toSend), "history:%s|", nume2);
       
        if (write(sd, toSend, strlen(toSend)) <= 0) {
            perror("[client]Eroare la send() spre server.\n");
            break;}
    }else if(strcmp(message,"online_users")== 0){
        printf("ONLINE USERS: \n");

    }else if(strcmp(message,"offline_users")== 0){
        printf("OFFLINE USERS: \n");
    }else if(strcmp(message, "sterge_mesaj") == 0){
        printf("STERGE MESAJ: \n");
        memset(id_mes, 0, sizeof(id_mes));
        printf("[client]Introduceti id-ul mesajului dorit pentru a stergere: ");
        fflush(stdout);
        fgets(id_mes, sizeof(id_mes), stdin);
        id_mes[strlen(id_mes)-1] = '\0';

                cod_write = write(sd, &id_mes, strlen(id_mes));    
                if(cod_write == -1)
                {
                    perror("[client]Eroare la write() catre server.\n");
                    exit(EXIT_FAILURE);
                }
            
    }else if(strcmp(message, "sterge_cont")== 0){
        printf("STERGE CONT: \n");
    }else if(strcmp(message, "send messege") == 0)
        {
            if(logat == 1)
            {
                 char user_conv[30];
                 char messageToSend[200];
                 char toSend[300];

        printf("Introduceti numele utilizatorului cu care vb : ");
        fgets(user_conv, 30, stdin);
        user_conv[strlen(user_conv) - 1] = '\0'; 

        printf("Introduceti mesajul: ");
        fgets(messageToSend, 200, stdin);
        messageToSend[strlen(messageToSend) - 1] = '\0';  
        snprintf(toSend, sizeof(toSend), "send messege:%s|%s", user_conv, messageToSend);
        
          if (write(sd, toSend, strlen(toSend)) <= 0) {
              perror("[client]Eroare la send() spre server.\n");
              break;}
             }
      else
            {
                printf("[client]Nu sunteti conectat la aplicatie.\n");
            }
    }else if(strcmp(message,"reply") == 0){
        printf("REPLY: \n");
        memset(id_mes, 0, sizeof(id_mes));
        printf("[client]Introduceti id-ul mesajului dorit pentru a raspunde: ");
        fflush(stdout);
        fgets(id_mes, sizeof(id_mes), stdin);
        id_mes[strlen(id_mes)-1] = '\0';

                cod_write = write(sd, &id_mes, strlen(id_mes));    
                if(cod_write == -1)
                {
                    perror("[client]Eroare la write() catre server.\n");
                    exit(EXIT_FAILURE);
                }
    }else if(strcmp(message,"offline_messeges") == 0 ){
        printf("OFFLINE MESSEGES: \n");
    }else if(strcmp(message,"logout") == 0){
        printf("LOGOUT: \n");
        
    }else if(strcmp(message,"exit") == 0){
        printf("EXIT. \n");
    }else if(strcmp(message,"help") == 0){
       printf("HELP:\n");
       help();
    }else if(strcmp(message, "iesire conversatie") == 0){
        printf("IESIRE CONVERSATIE \n");
        char nume2[30];
        char toSend[100];

        printf("[client]Introduceti nume_user conversatiei din care iesiti: ");
        fgets(nume2, 30, stdin);
        nume2[strlen(nume2) - 1] = '\0';  
        snprintf(toSend, sizeof(toSend), "iesire conversatie:%s|", nume2);
       
        if (write(sd, toSend, strlen(toSend)) <= 0) {
            perror("[client]Eroare la send() spre server.\n");
            break;}
    }else if(strcmp(message,"intrare conversatie") == 0){
        char nume2[30];
        char toSend[100];

        printf("[client]Introduceti nume_user conversatiei din care intrati: ");
        fgets(nume2, 30, stdin);
        nume2[strlen(nume2) - 1] = '\0';  
        snprintf(toSend, sizeof(toSend), "intrare conversatie:%s|", nume2);
       
        if (write(sd, toSend, strlen(toSend)) <= 0) {
            perror("[client]Eroare la send() spre server.\n");
            break;}
    }else{
        printf("[client]Am citit: %s\n", message);
    }
    memset(message,0,sizeof(message));
   }   
}
close(sd);
return 0;
}
