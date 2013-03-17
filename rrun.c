/*
 * Will Dignazio
 * Running Statistics (rstat) 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "rstat.h"

run_t *build_run(uid_t uid,
                 char *location,
                 double time,
                 double distance,
                 double temperature) {
    run_t *run = calloc(sizeof(run_t), 1);
    run->uid = uid;
    run->time = time;
    run->distance = distance;
    run->temperature = temperature;
    run->next = NULL;
    return run;
}

int put_run(run_t *run, sqlite3 **sqconn) {
    char *statement = sqlite3_mprintf(
            "INSERT INTO run(uid, location, time, distance, temperature) "
            "VALUES(%d, \"%s\", %lf, %lf, %lf)",
            run->uid,
            run->location,
            run->time,
            run->distance,
            run->temperature);
    printf("Created sql run statement.\n");
    switch( sqlite3_exec(*sqconn, statement, 0, 0, 0) ) {
        case SQLITE_OK:
            printf("Successfully put run into table.");
            return RSTAT_SUCCESS;
        default:
            fprintf(stderr, "Error: %s", sqlite3_errmsg(*sqconn));
            return RSTAT_FAIL;
    }
    return RSTAT_FAIL;
}
