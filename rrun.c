/*
 * Will Dignazio
 * Running Statistics (rstat) 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>
#include <sqlite3.h>

#include "rstat.h"

run_t *get_runs(runner_t *runner, sqlite3 **sqconn) {
    char *sqlite3_query =
        "SELECT runs.* FROM runners "
        "INNER JOIN runs ON runners.uid = runs.uid "
        "GROUP BY runners.uid";

    run_t *run = NULL;
    sqlite3_stmt *res;
    const char *tail;
    int perr = sqlite3_prepare_v2(*sqconn, sqlite3_query, 1000, &res, &tail);
    if(perr != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare run join statement.");
        return NULL;
    }

    /* Go through all the runs, building a linked
     * list of them for display. */
    run_t *run_head = run;
    while( sqlite3_step(res) == SQLITE_ROW ) {
        printf("Found run at %s: ", sqlite3_column_text(res, QUERY_ROW_LOCATION));
        run_head->next = build_run(sqlite3_column_int(res, QUERY_ROW_UID),
                                (char*)sqlite3_column_text(res, QUERY_ROW_LOCATION),
                                sqlite3_column_double(res, QUERY_ROW_TIME),
                                sqlite3_column_double(res, QUERY_ROW_DISTANCE),
                                sqlite3_column_double(res, QUERY_ROW_TEMPERATURE));
        if(run_head->next != NULL)
            run_head = run_head->next;
    }
    sqlite3_finalize(res);
    return run;
}

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
            sqlite3_free(statement);
            return RSTAT_SUCCESS;
        default:
            fprintf(stderr, "Error: %s", sqlite3_errmsg(*sqconn));
            sqlite3_free(statement);
            return RSTAT_FAIL;
    }
    sqlite3_free(statement);
    return RSTAT_FAIL;
}

int free_run(run_t* run) {
    if(run->location != NULL)
        free(run->location);
    free(run);
    return RSTAT_SUCCESS;
}

run_t *query_run_info() {
    run_t *run = calloc(sizeof(run_t), 1);
    // Get the essential user details
    run->uid = getuid();

    // We're going to try and to this as safely as possible...
    char loc_buffer[100] = {0};
    printf("Enter Run Location (100 Characters Max):\n");
    printf("%zd\n", strlen(loc_buffer));
    while(strlen(loc_buffer) <= 0) {
        printf(">");
        memset(loc_buffer, 0, sizeof(loc_buffer));
        fgets(loc_buffer, sizeof(loc_buffer), stdin);
        if(loc_buffer[strlen(loc_buffer)-1] == '\n') // Remove trailing newline
            loc_buffer[strlen(loc_buffer)-1] = '\0';
    }

    /* We've got the string, now copy it to dynamic memory and set it. */
    char *str = malloc(strlen(loc_buffer)); // Allocate for run
    strcpy(str, loc_buffer); // Copy in
    run->location = str; // Set

    printf("Run time (Minutes):\n");
    do {
        printf("> ");
        scanf("%*c"); // Flush stdin
    } while( scanf("%lf", &run->time) != 1);

    printf("Run Distance (Meters):\n");
    do {
        printf("> ");
        scanf("%*c");
    } while( scanf("%lf", &run->distance) != 1);

    printf("Run Temperature (Celcius):\n");
    do {
        printf("> ");
        scanf("%*c");
    } while( scanf("%lf", &run->temperature) != 1);

    return run;
}
