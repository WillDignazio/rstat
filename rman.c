/*
 * Will Dignazio
 * Running Stats (rstat) 2013
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <errno.h>

#include "rstat.h"

int create_database(const char *db_path, sqlite3 **sqconn) {
    printf("Creating %s\n", db_path);

    // Best way to check if a file exists that I know of.
    FILE *fp;
    if( (fp = fopen(db_path, "r")) == NULL) {
        if(errno != ENOENT) {
            fprintf(stderr, "error: GNU errno %d\n", errno);
        }
    } else {
        fprintf(stderr, "error: file exists at given path.\n");
        return RSTAT_FAIL;
    }

    printf("Creating handler.\n");
    /* Open connection with intention to create */
    int v2err = sqlite3_open_v2(db_path, sqconn,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    switch(v2err) {
        case SQLITE_OK:
            printf("Create OK.\n");
            /* Create table SQL Statement */
            char *runner_table_sql =
                "CREATE TABLE IF NOT EXISTS runners("
                        "uid INTEGER PRIMARY KEY,"
                        "username TEXT NOT NULL,"
                        "gender TEXT NOT NULL,"
                        "height REAL NOT NULL,"
                        "weight REAL NOT NULL)";
            /* Execute statement */
            switch( sqlite3_exec(*sqconn, runner_table_sql, 0, 0, 0) ) {
                case SQLITE_OK:
                    printf("Successfully created runner table.\n");
                    break;
                default:
                    fprintf(stderr, "%s\n", sqlite3_errmsg(*sqconn));
                    return RSTAT_FAIL;
            }

            char *run_table_sql =
                "CREATE TABLE IF NOT EXISTS runs("
                        "location TEXT NOT NULL,"
                        "time REAL NOT NULL,"
                        "distance REAL NOT NULL,"
                        "temperature REAL NOT NULL)";

            switch( sqlite3_exec(*sqconn, run_table_sql, 0, 0, 0) )  {
                case SQLITE_OK:
                    printf("Successfully create runs table.\n");
                    break;
                default:
                    fprintf(stderr, "%s\n", sqlite3_errmsg(*sqconn));
                    return RSTAT_FAIL;
            }
            break;
        default:
            fprintf(stderr, "error: %s\n", sqlite3_errmsg(*sqconn));
            return RSTAT_FAIL;
    }
    // Build initial user
    return RSTAT_SUCCESS;
}

int put_runner(runner_t *runner, sqlite3 **sqconn){
    char *statement = sqlite3_mprintf(
            "INSERT INTO runners(uid, username, gender, height, weight) "
            "VALUES(%d, \"%s\", %d, %lf, %lf)",
            runner->uid,
            runner->username,
            runner->gender,
            runner->height,
            runner->weight);
    printf("Created sqlite statement\n");
    switch( sqlite3_exec(*sqconn, statement, 0, 0, 0) ){
        case SQLITE_OK:
            printf("Successfully added user to database.\n");
            return RSTAT_SUCCESS;
        default:
            fprintf(stderr, "Error: %s", sqlite3_errmsg(*sqconn));
            return RSTAT_FAIL;
    }
    return RSTAT_FAIL;
}

int rstat_init(const char *db_path, int flags, sqlite3 **sqconn) {

    if( (flags & RSTAT_CREATE) ) {
        printf("creating database...\n");
        create_database(db_path, sqconn);
    } else {
        int oerr = sqlite3_open_v2(db_path, sqconn, SQLITE_OPEN_READWRITE, NULL);
        switch(oerr) {
            case SQLITE_OK:
                printf("OK.\n");
                break;
            default:
                fprintf(stderr, "error: %s\n", sqlite3_errmsg(*sqconn));
                break;
        }
    }

    // If we got to here, we should be good to go.

    return RSTAT_SUCCESS;
}

int rstat_close(sqlite3 **sqconn) {
    if( sqconn != NULL ) {
        sqlite3_close(*sqconn);
        return RSTAT_SUCCESS;
    } else { // Something went wrong before hand.
        return RSTAT_FAIL;
    }
    return RSTAT_FAIL;
}
