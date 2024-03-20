#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>
#include <sqlite3.h>
#include <stdbool.h>

#define PORT 2908
extern int errno;
int cod_returnare;
sqlite3 *baza_de_date;
char *Mesaj_de_eroare = NULL;
int descriptor_utilizator;
int logged_users= 0;
char istoric_conv[2000];
char resultBuffer[1024] = ""; 
char id_mes[2000];

typedef struct thData{
	int idThread; 
	int cl; 
}thData;

struct UserData {
  int id;
  char status[10];
}userData={0,""};

///CALLBACK URI :
/*
int callback(int *id, int argc, char **argv, char **azColName){
  for(int i = 0;i < argc;i++){
    *id= atoi(argv[i]);
    printf("%s = %s\n",azColName[i],argv[i] ? argv[i] : "NULL");
  }
  return 0;
}*/
int callback(void *data, int argc, char **argv, char **azColName) {
    int *id=(int *)data;
    for(int i=0;i<argc; i++){
        *id = atoi(argv[i]);
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

int callback2(void *data, int argc, char **argv, char **azColName){
  struct UserData *userData=(struct UserData *)data;
  if(argc == 2){ 
    userData->id = atoi(argv[0]);
    strncpy(userData->status, argv[1], sizeof(userData->status) - 1);
  }
     return 0;
}
int callback4(void *NotUsed, int argc, char **argv, char **azColName){
    for(int i = 0;i < argc;i++){
        strncat(resultBuffer,argv[i] ? argv[i] : "NULL",sizeof(resultBuffer) -strlen(resultBuffer) -1);

        strncat(resultBuffer,"\n", sizeof(resultBuffer) -strlen(resultBuffer) - 1);
    }
      return 0;
}
int callback3(void*data, int argc, char **argv, char **azColName) 
{
    for(int i=0;i <argc;i++){
        if(strcmp(azColName[i], "nume_utilizator") == 0){
            strcat(resultBuffer,argv[i]);
            strcat(resultBuffer, ": ");
        }else if(strcmp(azColName[i],"id_Utilizator") == 0){
            strcat(resultBuffer, "[ ");
            strcat(resultBuffer, argv[i]);
            strcat(resultBuffer, " ]");
        }else if(strcmp(azColName[i],"text_Mesaj") == 0){
            strcat(resultBuffer, argv[i]);
            strcat(resultBuffer, " ");
        }else if(strcmp(azColName[i],"data_si_ora") == 0){
            strcat(resultBuffer, argv[i]);
            strcat(resultBuffer, "\n");
    }}
    strcat(resultBuffer,"\n");
    return 0;
};

int callback_count(void *data, int argc, char **argv, char **azColName) {
    int *count=(int *)data;
    if(argc == 1){ *count = atoi(argv[0]);   }
  return 0;
}

int callback_status(void *data, int argc, char **argv, char **azColName) {
    char* status=(char*)data;
    if(argc > 0 && argv[0] != NULL){
        strncpy(status, argv[0], 9);
        status[9] = '\0'; 
    }
  return 0;
}

int callback_users(void *NotUsed, int argc, char **argv, char **azColName) {  
    for(int i=0;i < argc;i++){  printf("%s\n", argv[i]);  }
    return 0;
}

int callback_users2(void *data, int argc, char **argv, char **azColName) {
    int *id_Conversatie=(int *)data;
    if(argc > 0){ *id_Conversatie = atoi(argv[0]); }
    return 0;
}
bool utilizator_gasit = false;
int callback_cautare_utilizator(void *nume_utilizator, int argc, char **argv, char **azColName) {
    for(int i=0;i < argc;i++){
        if(argv[i] != NULL){
            if(strcmp(argv[i], nume_utilizator) == 0){ utilizator_gasit = true; }
        }
    }
  return 0;
}
 
 int callback_descriptor(void *data, int argc, char **argv, char **azColName) 
{
    for(int i=0; i<argc; i++){ descriptor_utilizator=atoi(argv[i]);  }
    return 0;
}

///FUNCTII :
void deschide_baza_de_date()
{
  cod_returnare = sqlite3_open("baza_de_date.db", &baza_de_date);
  if (cod_returnare != SQLITE_OK)
  {
     fprintf(stderr, "[server :]Eroare la deschiderea bazei de date: %s\n", sqlite3_errmsg(baza_de_date));
     sqlite3_close(baza_de_date);
     exit(EXIT_FAILURE);
  }
}

void inchide_baza_de_date()
{
  sqlite3_close(baza_de_date);
}

void adaugare_utilizator_in_baza_de_date(char *nume_user, char *parola)
{
  deschide_baza_de_date();

  char status[] = "offline";
  char sql[150];
  /*char *parola = "1234";*/
  snprintf(sql, sizeof(sql), "INSERT INTO Utilizatori (nume_utilizator, parola, status, descriptor) VALUES('%s', '%s', '%s', '%d');", nume_user, parola, status, 0);
  cod_returnare = sqlite3_exec(baza_de_date, sql, 0, 0, &Mesaj_de_eroare);
  if (cod_returnare != SQLITE_OK)
  {
    printf("[server]Eroare : %s\n", sqlite3_errmsg(baza_de_date));
    sqlite3_close(baza_de_date);
    exit(EXIT_FAILURE);
  }

  inchide_baza_de_date();
}

int getID(char* username)
{
  deschide_baza_de_date();

 char sql[100];
 sprintf( sql, "SELECT id_Utilizator FROM Utilizatori WHERE nume_utilizator='%s'", username);
 int id = 0;
 int rc = sqlite3_exec(baza_de_date, sql, callback,(void *)&id, &Mesaj_de_eroare);
 if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", Mesaj_de_eroare, sql);
        sqlite3_free(Mesaj_de_eroare);
        sqlite3_close(baza_de_date);
        exit(1);
    } 

 inchide_baza_de_date();
 printf("ID-ul este %d\n", id);
 return id;
}

int islogged(int id)
{
  deschide_baza_de_date();
  char status[10]={0};
  char sql[100];
  sprintf( sql, "SELECT status from Utilizatori where id_Utilizator='%d'", id);

  int rc = sqlite3_exec(baza_de_date, sql, callback_status, &status, &Mesaj_de_eroare);  
  if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", Mesaj_de_eroare, sql);
        sqlite3_free(Mesaj_de_eroare);
        sqlite3_close(baza_de_date);
        exit(1);
    } 

  inchide_baza_de_date();
 
  if(strcmp(status ,"online") == 0 ) return 1;
  else if(strcmp(status ,"offline") == 0) return 0;
       else return -1;
}

int isInConv(int id_Desinatie, int id_Sursa)
{
  deschide_baza_de_date();
  char statusConv[10]={0};
  char sql[100];
  sprintf( sql, "SELECT status_Conv FROM Conversatii where id_utilizator1='%d' AND id_utilizator2='%d' ", id_Desinatie, id_Sursa);

  int rc = sqlite3_exec(baza_de_date, sql, callback_status, &statusConv, &Mesaj_de_eroare);  
  if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s \n %s \n ", Mesaj_de_eroare, sql);
        sqlite3_free(Mesaj_de_eroare);
        sqlite3_close(baza_de_date);
        exit(1);
    } 

  inchide_baza_de_date();
 
  if(strcmp(statusConv ,"online") == 0 ) return 1;
  else if(strcmp(statusConv ,"offline") == 0) return 0;
       else return -1;
}

bool verif_exist_useri(char *nume_user)
{
    deschide_baza_de_date();
    char sql[100];
    snprintf(sql, sizeof(sql), "SELECT nume_utilizator FROM Utilizatori WHERE nume_utilizator = '%s';", nume_user);

    utilizator_gasit = false;
    cod_returnare = sqlite3_exec(baza_de_date, sql, callback_cautare_utilizator, nume_user, &Mesaj_de_eroare);
    if(cod_returnare != SQLITE_OK)
    {
        printf("[server]Eroare : %s\n", sqlite3_errmsg(baza_de_date));
        sqlite3_close(baza_de_date);
        exit(EXIT_FAILURE);
    }

    if(utilizator_gasit == true) {
        printf("[server :]User ul %s a fost gasit in baza de date .\n", nume_user);
        return true;
    }else{
        printf("[server :]User ul NU %s a fost gasit in baza de date. \n", nume_user);
        return false;
    }

    inchide_baza_de_date();
}

void descriptor_search(char *username)
{
    deschide_baza_de_date();

    char sql[100];
    snprintf(sql, sizeof(sql), "SELECT descriptor FROM Utilizatori WHERE nume_utilizator = '%s';", username);

    cod_returnare = sqlite3_exec(baza_de_date, sql, callback_descriptor, 0, &Mesaj_de_eroare);
    if(cod_returnare != SQLITE_OK)
    {
        printf("[server]Eroare : %s\n", sqlite3_errmsg(baza_de_date));
        sqlite3_close(baza_de_date);
        exit(EXIT_FAILURE);
    }

     inchide_baza_de_date();
}

void adaugare_mesaj_in_baza_de_date (int id_Utilizator, int id_Destinatie, char *mesaj, char *status_Conv, int seen)
{
    deschide_baza_de_date();

    printf("Valoarea userData.id: %d\n", userData.id);
    printf("ID-ul utilizatorului curent : %d\n", id_Utilizator);
    printf("ID-ul cu cine vreau sa conversez: %d \n",id_Destinatie);
    
    int id_Conversatie = 0;
    // Cautam existenta conversatiei dintre cei 2
    char sql1[256];
    snprintf(sql1, sizeof(sql1), "SELECT id_Conversatie FROM Conversatii WHERE Id_utilizator1 ='%d' AND Id_utilizator2='%d'", id_Utilizator, id_Destinatie, id_Destinatie, id_Utilizator);

    int rc1 = sqlite3_exec(baza_de_date, sql1, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
    if (rc1 != SQLITE_OK) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
        sqlite3_free(Mesaj_de_eroare);
        return;
    }

    printf("Id_Conversatiei curente este: '%d'\n", id_Conversatie);
    // Dacă conversația nu există, o creează
    if (id_Conversatie == 0) {
            char sql2[256];
            snprintf(sql2, sizeof(sql2), "INSERT INTO Conversatii (Id_utilizator1, Id_utilizator2 , status_Conv) VALUES ('%d', '%d', '%s')", id_Utilizator, id_Destinatie, status_Conv);
            
            int rc2 = sqlite3_exec(baza_de_date, sql2, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
            if (rc2 != SQLITE_OK) {
                fprintf(stderr, "Failed to insert data\n");
                fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
                sqlite3_free(Mesaj_de_eroare);
                return;
            }
      
            char sql[256];
            snprintf(sql, sizeof(sql), "SELECT id_Conversatie FROM Conversatii WHERE Id_utilizator1 ='%d' AND Id_utilizator2='%d'", id_Utilizator, id_Destinatie);

            int rc = sqlite3_exec(baza_de_date, sql, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
            if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
            sqlite3_free(Mesaj_de_eroare);
            return;
             }
     }
        printf("Id_Conversatiei curente este:'%d'\n", id_Conversatie);
        
        // Inserarea mesajului în Mesajele
        if (id_Conversatie > 0) {
            char sql3[500];
            snprintf(sql3, sizeof(sql3), "INSERT INTO Mesajele (id_Conversatie, id_Utilizator,id_Destinatie, text_Mesaj,conv_Status, SEEN) VALUES ('%d', '%d','%d' ,'%s','%s', '%d')", id_Conversatie, id_Utilizator, id_Destinatie, mesaj, status_Conv, seen);

            int rc3 = sqlite3_exec(baza_de_date, sql3, callback_users, NULL, &Mesaj_de_eroare);
            if (rc3 != SQLITE_OK) {
                fprintf(stderr, "Failed to insert message\n");
                fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
                sqlite3_free(Mesaj_de_eroare);
                return;
            }
        }else{
            printf("\n Eroare la crearea conversatiei.\n");
        }
   
     inchide_baza_de_date();
}

///////////////////////////////////////////
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
  int nr,  sd , pid;	//mesajul primit de trimis la client 	//descriptorul de socket 
	int i=0;

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
    server.sin_family = AF_INET;	
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }
  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 6) == -1){
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii folosind thread-uri pentru fiecare client */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);
      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0){
	      perror ("[server]Eroare la accept().\n");
	      continue;
	    }
        /* s-a realizat conexiunea, se astepta mesajul */
	    td=(struct thData*)malloc(sizeof(struct thData));	
	    td->idThread=i++;
	    td->cl=client;  // cl descriptorul intors de accept

     ////CREAM THREAD PT FIECARE CLIENT
	    pthread_create(&th[i], NULL, &treat, td);	      			
	}   
};			

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
};

void raspunde(void *arg) {

    struct thData tdL;
    tdL = *((struct thData *)arg);
    int descriptor_sursa = tdL.cl;

    char message[1000]; // stocheaza mesajule primite de la client
    char raspuns[1000]="";//raspunsul pt client
    int read_msj, write_msj;   
    char nume_utilizator[100];//numele user pt fiecare client
    int logged =0;
    int logged_id_user =0;

    while(1){
  
     memset(message,0,strlen(message));
     if (read(tdL.cl, &message, sizeof(message)) <= 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la read() de la client.\n");
            
        }
     message[strlen(message)] = '\0';
     printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);
     fflush(stdout);

    char *command = strtok(message, ":|");
   
    if (strcmp(command, "register") == 0) {
      char *username = strtok(NULL, "|");
      char *password = strtok(NULL, "|");
      if (username && password)
      {
        ////////////CAZURI //////////////////
        deschide_baza_de_date();

        char sql[200];
        sprintf(sql, "SELECT id_Utilizator FROM Utilizatori WHERE nume_utilizator='%s'", username);
        int id1 = 0;
        int rc1 = sqlite3_exec(baza_de_date, sql, callback,(void *) &id1, &Mesaj_de_eroare);
        if (rc1 != SQLITE_OK)
        {
          fprintf(stderr, "Failed to select data\n");
          fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
          sqlite3_free(Mesaj_de_eroare);
          return;
        }

        sprintf(sql, "SELECT id_Utilizator FROM Utilizatori WHERE parola='%s'", password);
        int id2 = 0;
        int rc2 = sqlite3_exec(baza_de_date, sql, callback,(void *)&id2, &Mesaj_de_eroare);
        if (rc2 != SQLITE_OK)
        {
          fprintf(stderr, "Failed to select data\n");
          fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
          sqlite3_free(Mesaj_de_eroare);
          return;
        }

        if (id1 != 0 || id2 != 0)
        {
          if (id1 != 0 && id2 == 0)
          {
            printf("User deja existent, te rog sa iti cauti alt username!\n");
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "User deja existent, te rog sa iti cauti alt username!\n");
            strcat(raspuns, nume_utilizator);
            write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
            if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
          }
          else if (id1 == 0 && id2 != 0)
          {
            printf("Parola deja existent, te rog sa iti cauti alta parola!\n");
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "Parola deja existent, te rog sa iti cauti alta parola!\n");
            strcat(raspuns, nume_utilizator);
            write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
            if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
          }
          else
          {
            printf("User si parola deja existente, te rog sa iti cauti altele!\n");
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "User si parola deja existente, te rog sa iti cauti altele!\n");
            strcat(raspuns, nume_utilizator);
            write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
            if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
          }
        }
        else
        {
          // Aici introducem utilizatorul nou în baza de date
          printf("Inregistrare utilizator: %s, Parola: %s\n", username, password);
          adaugare_utilizator_in_baza_de_date(username, password);
          
          memset(raspuns, 0, sizeof(raspuns));
          strcat(raspuns, "Utilizator introdus in baza de date : ");
          strcat(raspuns, username);
          strcat(raspuns, " si parola: ");
          strcat(raspuns, password);
          write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
          if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
          }
        inchide_baza_de_date();
      }
    }else if(strcmp (command, "login") == 0){
      char temp[50]="";  
      printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);

      if (logged == 0)
      {
        char *username = strtok(NULL, "|");
        char *password = strtok(NULL, "|");

        if (username && password)
        {
          deschide_baza_de_date();
          char sql[200];
          sprintf(sql, "SELECT id_Utilizator,  status FROM Utilizatori WHERE nume_utilizator='%s' and parola='%s'", username, password);
          int rc = sqlite3_exec(baza_de_date, sql, callback2, &userData, &Mesaj_de_eroare);
          if (rc != SQLITE_OK)
          {
            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
            sqlite3_free(Mesaj_de_eroare);
            return;
          }

          if(userData.id == 0)
          {
            printf("User inexistent, trebuie sa va faceti cont mai intai!\n");
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "User inexistent, trebuie sa va faceti cont mai intai!");
            write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
            if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
            
          } else if (strcmp(userData.status, "online") == 0){

            printf("Utilizatorul este deja online.\n");
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "[server]Utilizatorul ");
            strcat(raspuns, username);
            strcat(raspuns," este deja online.\n");
            write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
            if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
          }else {
                 logged_users++;
                 logged = 1;
                 logged_id_user = userData.id;
                 memset(nume_utilizator,0,strlen(nume_utilizator));
                 strcpy(nume_utilizator, username);
                 char sql[200];
                 sprintf(sql, "UPDATE Utilizatori SET status = 'online', descriptor= '%d' WHERE nume_utilizator = '%s' and parola='%s'",descriptor_sursa, username, password);

                 int id = 0;
                 int rc = sqlite3_exec(baza_de_date, sql, callback,(void *)&id, &Mesaj_de_eroare);
                 if (rc != SQLITE_OK)
                 {
                   fprintf(stderr, "Failed to select data\n");
                   fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
                   sqlite3_free(Mesaj_de_eroare);
                   return;
                 }
                printf("S-a reusit logarea utilizatorului %s \n", username);
                memset(raspuns, 0, sizeof(raspuns));
                strcat(raspuns, "[server]S-a reusit logarea utilizatorului: ");
                strcat(raspuns, username);
                strcat(raspuns,"\n");

                //// NR DE MESAJE OFFLINE
                sprintf(sql, "SELECT COUNT(id_Mesaj) FROM Mesajele WHERE SEEN = %d AND id_Destinatie = %d", 0, logged_id_user);
 
                int count = 0;// contine nr de mesaje offline
                int rc1 = sqlite3_exec(baza_de_date, sql, callback_count, &count, &Mesaj_de_eroare);
                if (rc1 != SQLITE_OK) {
                   fprintf(stderr, "Failed to select data\n");
                   fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
                   sqlite3_free(Mesaj_de_eroare);
                   return;
                  }

                printf("Numarul de mesaje offline: %d\n", count);
                if(count > 0){
                     strcat(raspuns, "Ati primit ");        
                     sprintf(temp, "%d", count);
                     strcat(raspuns, temp);
                     strcat(raspuns, " mesaje cat timp ati fost deconectat de la aplicatie, pentru a le vedea utilizati comanda : offline_messeges");
                }else{
                     strcat(raspuns, "Nu ati primit niciun mesaj cat ati fost deconectat.\n");
                }
           
            write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
            if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
          }
          inchide_baza_de_date();
        }else{
          printf("Usernameul sau parola este NULL.");
        }
      }else{
        printf("User deja logat, te rog sa te deloghezi pentru a te loga cu alt cont!\n");
        memset(raspuns, 0, sizeof(raspuns));
        strcat(raspuns, "User deja logat, te rog sa te deloghezi pentru a te loga cu alt cont!");
        strcat(raspuns, nume_utilizator);
        write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
        if(write_msj == -1)
            {
                perror("[server]Eroare la write() catre client.\n");
                exit(EXIT_FAILURE);
            }
      }

    }else if(strcmp (command, "history") == 0){

        char *nume2 = strtok(NULL, "|");
        int id_Conversatie=0;
        if ( nume2 != NULL) {
            int numbe2 = getID(nume2); 
            printf("ID-ul destinatiei : %d", numbe2);

            deschide_baza_de_date();
            memset(istoric_conv, 0, strlen(istoric_conv));

            char sql1[200];
            sprintf(sql1," SELECT id_Conversatie FROM Conversatii WHERE  (Id_utilizator1 ='%d' AND Id_utilizator2='%d') OR (Id_utilizator1 ='%d' AND Id_utilizator2='%d')",logged_id_user , numbe2, numbe2, logged_id_user);
         
            int id1 = 0;
            int rc1 = sqlite3_exec(baza_de_date, sql1, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
            if (rc1 != SQLITE_OK)
            {
              fprintf(stderr, "Failed to select data\n");
              fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
              sqlite3_free(Mesaj_de_eroare);
              return;
            }
     
            ///id_Conversatie a fost reptionat
            memset(resultBuffer,0,strlen(resultBuffer));
            printf("ID CONVERSATIE: '%d'\n", id_Conversatie);
            printf("Mesajele din conversatia '%d' a utilizatorului '%d' cu '%d':",id_Conversatie, logged_id_user, numbe2);
            
            ///nume utilizator
            ///
            if(id_Conversatie>0){
                char sql2[200];
                sprintf(sql2," SELECT m.id_Utilizator, u.nume_utilizator, m.text_Mesaj, m.data_si_ora FROM Mesajele m JOIN Utilizatori u ON m.id_Utilizator = u.id_Utilizator WHERE m.id_Conversatie = '%d' AND m.SEEN = '%d'" ,id_Conversatie, 1);
         
                int id2 = 0;
                int rc2 = sqlite3_exec(baza_de_date, sql2, callback3, &resultBuffer, &Mesaj_de_eroare);
                if (rc2 != SQLITE_OK)
                {
                  fprintf(stderr, "Failed to select data\n");
                  fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
                  sqlite3_free(Mesaj_de_eroare);
                  return;
                }  


                printf("Conversatie existenta intre cele 2 id-uri.");
                memset(raspuns, 0, sizeof(raspuns));
                strcat(raspuns, "[server]Conversatie existenta intre: ");
                strcat(raspuns, nume_utilizator);
                strcat(raspuns," <=> ");
                strcat(raspuns, nume2);
                strcat(raspuns,"\n");
                strcat(raspuns, resultBuffer);


            inchide_baza_de_date();

            printf("\n [Thread %d] Mesajul a fost receptionat:s-a afisat istoricul.\n", tdL.idThread);
       
    }else{
            printf("Conversatie inexistenta intre cele 2 id-uri.");
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "[server]Conversatie inexistenta.\n");
    }

     write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
     if(write_msj == -1)
     {
       perror("[server]Eroare la write() catre client.\n");
       exit(EXIT_FAILURE);
     }
     }
    }else if(strcmp(message, "online_users")== 0 ){
       printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);
       printf("Sunt logati in aceasta aplicatie %d utilizatori.\n", logged_users);
       deschide_baza_de_date();       
       memset(resultBuffer, 0, sizeof(resultBuffer));
       char *sql = "SELECT nume_utilizator FROM Utilizatori WHERE status = 'online'";
       int rc = sqlite3_exec(baza_de_date, sql, callback4, (void *)resultBuffer, &Mesaj_de_eroare);
       if (rc != SQLITE_OK) {
          fprintf(stderr, "Failed to select data\n");
          fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
          sqlite3_free(Mesaj_de_eroare);
        } 
       printf("Rezultat online users : \n %s\n", resultBuffer);
       memset(raspuns, 0, sizeof(raspuns));
       strcat(raspuns, "[server]Trebuie sa fii conectat ca sa vezi mesajele.\n");
       strcat(raspuns, resultBuffer);  
    
       write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
       if(write_msj == -1)
         {
             perror("[server]Eroare la write() catre client.\n");
             exit(EXIT_FAILURE);
         }

      inchide_baza_de_date();

    }else if(strcmp(message, "offline_users") == 0){
       printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);
       
       deschide_baza_de_date();       
       memset(resultBuffer, 0, sizeof(resultBuffer));
       char *sql = "SELECT nume_utilizator FROM Utilizatori WHERE status = 'offline'";
       int rc = sqlite3_exec(baza_de_date, sql, callback4, (void *)resultBuffer, &Mesaj_de_eroare);
       if (rc != SQLITE_OK) {
          fprintf(stderr, "Failed to select data\n");
          fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
          sqlite3_free(Mesaj_de_eroare);
        } 
       printf("Rezultat online users : \n %s\n", resultBuffer);
       memset(raspuns, 0, sizeof(raspuns));
       strcat(raspuns, "[server]Trebuie sa fii conectat ca sa vezi mesajele.\n");
       strcat(raspuns, resultBuffer);  
    
       write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
       if(write_msj == -1)
         {
             perror("[server]Eroare la write() catre client.\n");
             exit(EXIT_FAILURE);
         }

      inchide_baza_de_date();

    }else if(strcmp(command, "send messege") == 0){

      char *nume_utilizator_destinatie = strtok(NULL, "|");
      char *mesaj = strtok(NULL, "|");
      memset(raspuns, 0, sizeof(raspuns));
      if(verif_exist_useri(nume_utilizator_destinatie) == true){

        printf("[server]Am primit mesajul conversatie: %s\n", mesaj);
        descriptor_search(nume_utilizator_destinatie);
        printf("[server]Clientul %s are descriptorul %d.\n", nume_utilizator_destinatie, descriptor_utilizator);
                        
        int  id_util= getID(nume_utilizator);
        int id_destinatie=getID(nume_utilizator_destinatie);
        int logatDESTINATIE = 0;
        logatDESTINATIE= islogged(id_destinatie);
        int logatCONV=0;
        logatCONV=isInConv(id_destinatie, logged_id_user);
        printf("eSTE IN CONV:%d",logatCONV);

        if(logatDESTINATIE == 1){
              if(logatCONV == 1){
              printf("Mesaje ONLINE: '%s' ",mesaj);
              adaugare_mesaj_in_baza_de_date(id_util,id_destinatie, mesaj,"online",1);

              strcat(raspuns, " [");
              strcat(raspuns, nume_utilizator);
              strcat(raspuns, "]");
              strcat(raspuns, mesaj);
              strcat(raspuns, "\nPentru a raspunde: 'send messege'\n");

              write_msj = write(descriptor_utilizator, &raspuns, strlen(raspuns));
              if(write_msj == -1){
                   perror("[server]Eroare la write catre client.\n");
                   exit(EXIT_FAILURE);
                  }
              }else if(logatCONV == 0){
                 printf("Mesaje OFFLINE: '%s'", mesaj);
              adaugare_mesaj_in_baza_de_date(id_util,id_destinatie, mesaj,"offline", 0);
            
              strcat(raspuns, "\nMesaj trimis, dar utilizator nu e in conversatie.\n");
              write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
              if(write_msj == -1){
                 perror("[server]Eroare la write catre client.\n");
                 exit(EXIT_FAILURE);
                        }
        }
        }else if(logatDESTINATIE == 0){
              printf("Mesaje OFFLINE: '%s'", mesaj);
              adaugare_mesaj_in_baza_de_date(id_util,id_destinatie, mesaj,"offline", 0);
             
              strcat(raspuns, "\nMesaj trimis, dar utilizator offline.\n");
              write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
              if(write_msj == -1){
                 perror("[server]Eroare la write catre client.\n");
                 exit(EXIT_FAILURE);
                        }
        }
      } 
                   

    }else if(strcmp (message, "offline_messeges") == 0){
    printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);
    memset(resultBuffer, 0, sizeof(resultBuffer));
        
    if(logged_id_user>0){

        deschide_baza_de_date();
        printf("Mesajele utilizatorului '%d' cat timp a fost offline:\n", logged_id_user);

        char sql2[200];
        sprintf(sql2, "SELECT m.id_Utilizator, u.nume_utilizator, m.text_Mesaj, m.data_si_ora FROM Mesajele m JOIN Utilizatori u ON m.id_Utilizator = u.id_Utilizator WHERE m.id_Destinatie = '%d' AND m.SEEN = '%d' ", logged_id_user, 0);

        int rc = sqlite3_exec(baza_de_date, sql2, callback3, (void *)resultBuffer, &Mesaj_de_eroare);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
            sqlite3_free(Mesaj_de_eroare);
            inchide_baza_de_date();
            return;  
        } 
  
      char sql[200];
      sprintf(sql, "UPDATE Mesajele SET SEEN = 1 WHERE id_Destinatie = %d AND SEEN = 0", logged_id_user);
      int rc2 = sqlite3_exec(baza_de_date, sql, callback, NULL, &Mesaj_de_eroare);
      if (rc2 != SQLITE_OK) {
        fprintf(stderr, "Failed to update data\n");
        fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
        sqlite3_free(Mesaj_de_eroare);
        inchide_baza_de_date();
        return;  
      }
      printf("result buffer : \n %s\n", resultBuffer);
      memset(raspuns, 0, sizeof(raspuns));
      strcat(raspuns, "[server]Mesajele OFFLINE: \n");
      strcat(raspuns, resultBuffer);

    }else{
       printf("Trebuie sa fii conectat ca sa vezi mesajele.\n");  
       memset(raspuns, 0, sizeof(raspuns));
       strcat(raspuns, "[server]Trebuie sa fii conectat ca sa vezi mesajele.\n");
            
      }
    inchide_baza_de_date();

    write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
    if(write_msj == -1) {
       perror("[server]Eroare la write() catre client.\n");
       exit(EXIT_FAILURE);
    }

    printf("\n[Thread %d] Mesajul a fost receptionat: S-AU AFISAT MESAJELE OFFLINE.\n", tdL.idThread);
     
    }else if(strcmp (message, "reply") == 0){
       
      char temp[50]=""; 
      printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);
      
      memset(id_mes, 0, sizeof(id_mes));
      memset(raspuns, 0, sizeof(raspuns));
      read_msj = read(tdL.cl, &id_mes, sizeof(id_mes));
      if(read_msj == -1)
      {
        perror("[server]Eroare la read() de la client.\n");
        exit(EXIT_FAILURE);
      }
      id_mes[strlen(id_mes)] = '\0';

       int id_mesaj=atoi(id_mes);
       printf("ID ul mesajului: %d",id_mesaj);
       deschide_baza_de_date();
       printf("logged_id_user : %d\n", logged_id_user);

       if(logged_id_user > 0) {

         char sql[256];
         sprintf(sql, "SELECT m.id_Utilizator, u.nume_utilizator, m.text_Mesaj, m.data_si_ora FROM Mesajele m JOIN Utilizatori u ON m.id_Utilizator = u.id_Utilizator WHERE m.id_Mesaj='%d' AND m.id_Destinatie = '%d' AND m.SEEN = '%d'",id_mesaj, logged_id_user, 1);

         int rc = sqlite3_exec(baza_de_date, sql, callback3, (void *)resultBuffer, &Mesaj_de_eroare);
         if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to execute query\n");
            fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare);
            sqlite3_free(Mesaj_de_eroare);
         } 
         printf("Rezultatul interogării: %s\n", resultBuffer);

        if(strlen(resultBuffer) == 0) {
           printf("Nu s-a gasit mesajul cu id-ul '%d'.\n", id_mesaj);
           strcat(raspuns, "Nu s-a gasit mesajul cu id-ul");
           sprintf(temp, "%d", id_mesaj);
           strcat(raspuns, temp);
        } else {
            printf("S-a dat reply la mesajul cu id-ul '%d' :.\n", id_mesaj);
            
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "S-a dat reply la mesajul cu id-ul ");         
            sprintf(temp, "%d", id_mesaj);
            strcat(raspuns, temp);
            strcat(raspuns," ,pentru a raspunde folositi comanda 'send messege' \n");
            strcat(raspuns, resultBuffer);
        } 
      }else{
       printf("Trebuie sa fii conectat ca sa vezi mesajele.\n");  
       memset(raspuns, 0, sizeof(raspuns));
       strcat(raspuns, "[server]Trebuie sa fii conectat ca sa vezi mesajele.\n");  
      }
    inchide_baza_de_date();

    write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
    if(write_msj == -1){
        perror("[server]Eroare la write() catre client.\n");
        exit(EXIT_FAILURE);
    }

    }else if(strcmp(command,"sterge_mesaj") == 0 ){
      printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);

      memset(id_mes, 0, sizeof(id_mes));
      memset(raspuns, 0, sizeof(raspuns));
      read_msj = read(tdL.cl, &id_mes, sizeof(id_mes));
      if(read_msj == -1){
         perror("[server]Eroare la read() de la client.\n");
         exit(EXIT_FAILURE);
      }
      id_mes[strlen(id_mes)] = '\0';

      printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);
      int id_mesaj = atoi(id_mes);
      deschide_baza_de_date();
      
      printf("logged_id_user : %d", logged_id_user);
      if(logged_id_user>0){
          char sql[256];
          snprintf(sql, sizeof(sql), "DELETE FROM Mesajele where  id_Mesaj = '%d' AND (id_Utilizator ='%d' OR id_Destinatie = '%d') ;", id_mesaj, logged_id_user, logged_id_user);

          int rc = sqlite3_exec(baza_de_date, sql, NULL, NULL, &Mesaj_de_eroare);
          if (rc != SQLITE_OK)
          {
            fprintf(stderr, "Failed to delete user\n");
            fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare);
            sqlite3_free(Mesaj_de_eroare);
          }else{
             printf("Mesajul '%d' a fost sters cu succes.\n", id_mesaj);
             memset(raspuns, 0, strlen(raspuns));
             strcat(raspuns, "Mesajul a fost sters cu succes.");
          }
      }else{ 
           printf("Nu putem accesa mesajele dumneavoastra daca nu sunteti logat!\n");
           memset(raspuns, 0, strlen(raspuns));
           strcat(raspuns, "Nu putem accesa mesajele dumneavoastra daca nu sunteti logat!");
      }

      inchide_baza_de_date();
      write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
      if(write_msj == -1) {
          perror("Eroare la write() catre client.\n");
          exit(EXIT_FAILURE);
      } 

    }else if(strcmp(message, "sterge_cont") == 0){
    
      if(logged_id_user>0){
          logged_users--;
          logged=0;
          printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);
      
          deschide_baza_de_date();
          
          char sql[256];
          snprintf(sql, sizeof(sql), "DELETE FROM Utilizatori WHERE id_Utilizator = '%d';", logged_id_user);

          int rc = sqlite3_exec(baza_de_date, sql, NULL, NULL, &Mesaj_de_eroare);
          if (rc != SQLITE_OK){
              fprintf(stderr, "Failed to delete user\n");
              fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare);
              sqlite3_free(Mesaj_de_eroare);
          }else{
             printf("[server :]Utilizatorul '%d' a fost sters cu succes.\n", logged_id_user);
             memset(raspuns, 0, sizeof(raspuns));
              strcat(raspuns, "Contul a fost sters cu succes!\n");
          }
      }else{
          memset(raspuns, 0, strlen(raspuns));
          strcat(raspuns, "Nu este nimeni logat, nu putem sterge cont!\n");
           }

      inchide_baza_de_date();
      write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
      if(write_msj == -1){
          perror("Eroare la write() catre client.\n");
          exit(EXIT_FAILURE);
      } 
      
    }else if(strcmp (message, "logout") == 0){

      if (logged_users == 0){
           printf("[Thread %d] Mesajul a fost receptionat, dar nu avem pe nimeni de dealocat.\n", tdL.idThread);
           fflush(stdout);
           memset(raspuns, 0, sizeof(raspuns));
           strcat(raspuns, "Mesajul a fost receptionat, dar nu avem pe nimeni de dealocat.\n");
      }else{
        logged_users--;
        logged = 0;

        deschide_baza_de_date();

        char sql[200];
        sprintf(sql, "UPDATE Utilizatori SET status = 'offline', descriptor='%d' WHERE id_Utilizator = '%d'",0, logged_id_user);

        int id = 0;
        int rc = sqlite3_exec(baza_de_date, sql, callback2, &id, &Mesaj_de_eroare);
        if (rc != SQLITE_OK)
        {
          fprintf(stderr, "Failed to select data\n");
          fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
          sqlite3_free(Mesaj_de_eroare);
          return;
        }
        
        logged_id_user=0;
        memset(raspuns, 0, sizeof(raspuns));
        strcat(raspuns, "S-a dealocat utilizatorul.");
        printf("[Thread %d] Mesajul a fost receptionat: s-a dealocat utilizatorul.\n", tdL.idThread);
        fflush(stdout);
      }

      inchide_baza_de_date();
      write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
      if(write_msj == -1){
           perror("Eroare la write() catre client.\n");
           exit(EXIT_FAILURE);
      } 
    }else if(strcmp (message, "exit") == 0){
      printf("[Thread %d] Mesajul a fost receptionat: %s\n", tdL.idThread, message);

      logged_users--;
      logged = 0;
      
      deschide_baza_de_date();
      
      char sql[200];
      sprintf(sql, "UPDATE Utilizatori SET status = 'offline', descriptor='%d' WHERE id_Utilizator = '%d'",0, logged_id_user);

      int id = 0;
      int rc = sqlite3_exec(baza_de_date, sql, callback2, &id, &Mesaj_de_eroare);
      if (rc != SQLITE_OK){
          fprintf(stderr, "Failed to select data\n");
          fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
          sqlite3_free(Mesaj_de_eroare);
          return;
      }
      
      logged_id_user=0;
      printf("[Thread %d] Mesajul a fost receptionat: a iesit utilizatorul din aplicatie.\n", tdL.idThread);
      fflush(stdout);
      inchide_baza_de_date();

      return;

    }else if(strcmp(command, "iesire conversatie") == 0){
      char *nume2 = strtok(NULL, "|");
      int id_Conversatie=0;
        if ( nume2 != NULL) {
            int numbe2 = getID(nume2); 
            printf("ID-ul destinatiei : %d", numbe2);
              deschide_baza_de_date();
            // Cautam existenta conversatiei dintre cei 2
          char sql1[256];
          snprintf(sql1, sizeof(sql1), "SELECT id_Conversatie FROM Conversatii WHERE Id_utilizator1 ='%d' AND Id_utilizator2='%d'", logged_id_user, numbe2, numbe2, logged_id_user);

         int rc1 = sqlite3_exec(baza_de_date, sql1, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
         if (rc1 != SQLITE_OK) {
              fprintf(stderr, "Failed to select data\n");
              fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
              sqlite3_free(Mesaj_de_eroare);
              return;
           }

         printf("Id_Conversatiei din care vreti sa iesiti este: '%d'\n", id_Conversatie);
         if(id_Conversatie>0){
          char sql[256];
          snprintf(sql, sizeof(sql), "UPDATE Conversatii SET status_Conv = 'offline' WHERE id_Conversatie ='%d'", id_Conversatie);
          int rc = sqlite3_exec(baza_de_date, sql, callback, 0, &Mesaj_de_eroare);  
          if (rc != SQLITE_OK ) {
            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s \n %s \n ", Mesaj_de_eroare, sql);
            sqlite3_free(Mesaj_de_eroare);
            sqlite3_close(baza_de_date);
            exit(1);
           } 
         
       printf("Am iesit din conversatie.\n");  
       memset(raspuns, 0, sizeof(raspuns));
       strcat(raspuns, "Am iesit din conversatie.\n");
         }else{
         printf("Nu exista conversatie.\n");  
         memset(raspuns, 0, sizeof(raspuns));
         strcat(raspuns, "Nu exista conversatie.\n");
         }
        }
      
       inchide_baza_de_date();

       write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
       if(write_msj == -1) {
          perror("[server]Eroare la write() catre client.\n");
          exit(EXIT_FAILURE);
       }
    }else if(strcmp(command,"intrare conversatie") == 0){
        char *nume2 = strtok(NULL, "|");
        int id_Conversatie=0;
        if( nume2 != NULL) {
            int numbe2 = getID(nume2); 
            printf("ID-ul destinatiei : %d", numbe2);
              deschide_baza_de_date();
            // Cautam existenta conversatiei dintre cei 2
          char sql1[256];
          snprintf(sql1, sizeof(sql1), "SELECT id_Conversatie FROM Conversatii WHERE Id_utilizator1 ='%d' AND Id_utilizator2='%d'", logged_id_user, numbe2);

         int rc1 = sqlite3_exec(baza_de_date, sql1, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
         if (rc1 != SQLITE_OK) {
              fprintf(stderr, "Failed to select data\n");
              fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
              sqlite3_free(Mesaj_de_eroare);
              return;
           }

         printf("Id_Conversatiei din care vreti sa intrati este: '%d'\n", id_Conversatie);
         if(id_Conversatie>0){
          char sql[256];
          snprintf(sql, sizeof(sql), "UPDATE Conversatii SET status_Conv = 'online' WHERE id_Conversatie ='%d'", id_Conversatie);
          int rc = sqlite3_exec(baza_de_date, sql, callback, 0, &Mesaj_de_eroare);  
          if (rc != SQLITE_OK ) {
            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s \n %s \n ", Mesaj_de_eroare, sql);
            sqlite3_free(Mesaj_de_eroare);
            sqlite3_close(baza_de_date);
            exit(1);
           } 
         
            printf("Am intrat in conversatie.\n");  
            memset(raspuns, 0, sizeof(raspuns));
            strcat(raspuns, "Ai intrat in conversatie.");
         }else if(id_Conversatie == 0) {
          ///CREEZ CONVERSATIA daca nu exista
            char sql2[256];
            snprintf(sql2, sizeof(sql2), "INSERT INTO Conversatii (Id_utilizator1, Id_utilizator2 , status_Conv) VALUES ('%d', '%d', '%s')", logged_id_user, numbe2, "online");
            
            int rc2 = sqlite3_exec(baza_de_date, sql2, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
            if (rc2 != SQLITE_OK) {
                fprintf(stderr, "Failed to insert data\n");
                fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
                sqlite3_free(Mesaj_de_eroare);
                return;
            }
      
            char sql[256];
            snprintf(sql, sizeof(sql), "SELECT id_Conversatie FROM Conversatii WHERE Id_utilizator1 ='%d' AND Id_utilizator2='%d'", logged_id_user, numbe2);

            int rc = sqlite3_exec(baza_de_date, sql, callback_users2, &id_Conversatie, &Mesaj_de_eroare);
            if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to select data\n");
            fprintf(stderr, "SQL error: %s\n", Mesaj_de_eroare ? Mesaj_de_eroare : "Unknown error");
            sqlite3_free(Mesaj_de_eroare);
            return;
             }
     
        printf("Id_Conversatiei curente este:'%d'\n", id_Conversatie);
        
         printf("Nu exista conversatie, deci vom crea.\n");  
         memset(raspuns, 0, sizeof(raspuns));
         strcat(raspuns, "Nu exista conversatie, deci se va crea.\n");
         }
        }
      
       inchide_baza_de_date();

       write_msj = write(tdL.cl, &raspuns, strlen(raspuns));
       if(write_msj == -1) {
          perror("[server]Eroare la write() catre client.\n");
          exit(EXIT_FAILURE);
       }
    }else{
         printf("[Thread %d] Mesajul a fost receptionat %s :comanda nu a fost gasita, incercati alta.\n", tdL.idThread, message);
    }

    memset(message, 0, sizeof(message)); 
    memset(resultBuffer, 0, sizeof(resultBuffer));
    memset(raspuns,0,sizeof(raspuns));
  }  
}

 
