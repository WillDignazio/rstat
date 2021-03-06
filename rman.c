/*
 * Copyright (c) 2013, Will Dignazio
 * See the file LICENSE file for copying permission.
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
    DEBUG(printf("Creating %s\n", db_path));

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

    DEBUG(printf("Creating handler.\n"));
    /* Open connection with intention to create */
    int v2err = sqlite3_open_v2(db_path, sqconn,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    switch(v2err) {
        case SQLITE_OK:
            DEBUG(printf("Create OK.\n"));
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
                    DEBUG(printf("Successfully created runner table.\n"));
                    break;
                default:
                    fprintf(stderr, "%s\n", sqlite3_errmsg(*sqconn));
                    return RSTAT_FAIL;
            }

            char *run_table_sql =
                "CREATE TABLE IF NOT EXISTS runs("
                        "uid INTEGER NOT NULL,"
                        "location TEXT NOT NULL,"
                        "time REAL NOT NULL,"
                        "distance REAL NOT NULL,"
                        "temperature REAL NOT NULL,"
                        "FOREIGN KEY (uid) REFERENCES runners(uid))";

            switch( sqlite3_exec(*sqconn, run_table_sql, 0, 0, 0) )  {
                case SQLITE_OK:
                    DEBUG(printf("Successfully create runs table.\n"));
                    break;
                default:
                    fprintf(stderr, "%s\n", sqlite3_errmsg(*sqconn));
                    sqlite3_close(*sqconn);
                    return RSTAT_FAIL;
            }
            break;
        default:
            fprintf(stderr, "error: %s\n", sqlite3_errmsg(*sqconn));
            sqlite3_close(*sqconn);
            return RSTAT_FAIL;
    }
    sqlite3_close(*sqconn);
    // Build initial user
    return RSTAT_SUCCESS;
}

int rstat_init(const char *db_path, int flags, sqlite3 **sqconn) {

    if( (flags & RSTAT_DATABASE_CREATE) ) {
        DEBUG(printf("creating database...\n"));
        int cerr = create_database(db_path, sqconn);
        switch(cerr) {
            case RSTAT_SUCCESS:
                DEBUG(printf("Successfully created and opened database.\n"));
                break;
            case RSTAT_FAIL:
                fprintf(stderr, "Failed to create and open database.\n");
                sqlite3_close(*sqconn);
                return RSTAT_FAIL; // Just stop here before anything else happens
        }
    }

    DEBUG(printf("Opening path %s\n", db_path));
    int oerr = sqlite3_open_v2(db_path, sqconn, SQLITE_OPEN_READWRITE, NULL);
    switch(oerr) {
        case SQLITE_OK:
            DEBUG(printf("OK.\n"));
            break;
        default:
            fprintf(stderr, "error: %s\n", sqlite3_errmsg(*sqconn));
            return RSTAT_FAIL;
    }

    if( (flags & RSTAT_USER_CREATE) ){
        DEBUG(printf("creating user...\n"));
        /*
         * Before we go any further, we should check to
         * see if this users UID exists within the database. In
         * which case this user would not be allowed to create an
         * identical user in the database.
         */
        if(!user_exists(sqconn)) {
            runner_t *nrunner = query_user_info();
            int perr = put_runner(nrunner, sqconn);
            switch(perr) {
                case RSTAT_SUCCESS:
                    DEBUG(printf("Successfully created and put user.\n"));
                    break;
                case RSTAT_FAIL:
                    fprintf(stderr, "Failed to create and put user.\n");
                    free_runner(nrunner);
                    return RSTAT_FAIL;
            }
        }
    }

    if( (flags & RSTAT_RUN_ADD) ) {
        DEBUG(printf("creating run...\n"));
        run_t *run = query_run_info();
        int rerr = put_run(run, sqconn);
        switch(rerr) {
            case RSTAT_SUCCESS:
                DEBUG(printf("Successfully created and put run.\n"));
                break;
            case RSTAT_FAIL:
                fprintf(stderr, "Failed to create and put run.\n");
                free_run(run);
                return RSTAT_FAIL;
        }
        free_run(run);
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
