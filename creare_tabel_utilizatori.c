#include <stdlib.h>
#include <sqlite3.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("baza_de_date.db", &db);

    if ( rc != SQLITE_OK)
    {
        printf("Eroare la deschiderea bazei de date!\n");
        sqlite3_close(db);
        exit(1);
    }
    else
       printf("Database is opened!\n");

   char *sql = 
   "CREATE TABLE Utilizatori (id_Utilizator INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,nume_utilizator TEXT NOT NULL, parola TEXT NOT NULL, status TEXT, descriptor INTEGER);";

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        
        sqlite3_free(zErrMsg);        //If an error occurs then the last parameter points to the allocated error message.
        sqlite3_close(db);
        
        return 1;
    } 
    
    sqlite3_close(db);
    
    return 0;
}