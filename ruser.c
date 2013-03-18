/*
 * Will Dignazio
 * Runner Stats (rstat) 2013
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>

#include "rstat.h"

int user_exists(sqlite3 **sqconn) {

    DEBUG(printf("Building search statement.\n"));
    char *search_statement = sqlite3_mprintf(
            "SELECT EXISTS(SELECT 1 FROM runners WHERE uid=\"%d\" LIMIT 1)",
            getuid()); // The current users UID

    sqlite3_stmt *res;
    // Produce safe bytecode for the search.
    sqlite3_prepare_v2(*sqconn, search_statement, 1000, &res, NULL);
    sqlite3_step(res); // We are gauranteed at least one row for this statement.
    DEBUG(printf("Stepped through return statement.\n"));
    // Free the allocated search statement
    sqlite3_free(search_statement);
    /* We must check whether the user exists within the
     * database, if so, they may not create another user
     * within it.
     */
    char exists = sqlite3_column_int(res, 0); // Will be either 0 or 1
    DEBUG(printf("User Exists: %s\n", exists ? "yes":"no"));
    if(exists) {
        fprintf(stderr, "Your user already exists in database.");
        sqlite3_finalize(res);
        return 1; // User exists, no go
    }
    sqlite3_finalize(res);
    return 0; // User does not exist
}

runner_t *get_runner_by_uid(uid_t uid, sqlite3 **sqconn) {
    char *search_statement = sqlite3_mprintf(
            "select runners.* from runners where runners.uid is %d",
            uid);
    sqlite3_stmt *res;
    sqlite3_prepare_v2(*sqconn, search_statement, 1000, &res, NULL);
    if( sqlite3_step(res) == SQLITE_ROW ) {
        // Found that user
        runner_t *runner = build_runner(
                sqlite3_column_int(res, 0),
                (char*)sqlite3_column_text(res, 1),
                sqlite3_column_int(res, 2),
                sqlite3_column_double(res, 3),
                sqlite3_column_double(res, 4));
        return runner;
    } else {
        DEBUG(printf("Could not find user by uid %d", uid));
        return NULL;
    }
    sqlite3_free(search_statement);
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
    DEBUG(printf("Created sqlite statement\n"));
    switch( sqlite3_exec(*sqconn, statement, 0, 0, 0) ){
        case SQLITE_OK:
            printf("Successfully added user to database.\n");
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

runner_t *build_runner(
        uid_t uid,
        char *username,
        char gender,
        double height,
        double weight) {
    runner_t *out_r = calloc(sizeof(runner_t), 1);
    out_r->uid = uid;
    out_r->username = username;
    out_r->height = height;
    out_r->weight = weight;
    out_r->next = NULL;
    return out_r;
}

int free_runner(runner_t *runner) {
    if(runner->username != NULL)
        free(runner->username);
    free(runner);
    return RSTAT_SUCCESS;
}

char *get_username() {
    struct passwd *pw;
    uid_t uid;

    /* Need to do this by euid so that we can't have
     * users spoofing other users. Like if I tried to
     * change my USER environment variable.
     */
    uid = geteuid();
    pw = getpwuid(uid);
    if( pw ) {
        char *out = calloc(sizeof(pw->pw_name), 1);
        strcpy(out, pw->pw_name);
        return out;
    } else {
        return NULL;
    }
}

runner_t *query_user_info() {

    runner_t *runner = calloc(sizeof(runner_t), 1);
    runner->username = get_username();
    runner->uid = getuid();
    runner->next = NULL;

gender:
    printf("Gender (M || F): ");
dirty:
    switch( getchar() ) {
        case 'M': case 'm':
            runner->gender = 'M';
            break;
        case 'F': case 'f':
            runner->gender = 'F';
            break;
        case '\n':
            goto dirty;
        default:
            goto gender;
    }

    do {
        printf("Enter Height (m): ");
        scanf("%*c"); // Destroy stdin
    } while( scanf("%lf", &runner->height) != 1);

    do {
        printf("Enter Weight (Kg): ");
        scanf("%*c"); // Destroy stdin
    } while( scanf("%lf", &runner->weight) != 1 );

    printf("Built Runner:\n");
    printf("\tUID: %d\n", runner->uid);
    printf("\tUsername: %s\n", runner->username);
    printf("\tGender: %c\n", runner->gender);
    printf("\tHeight(m): %lf\n", runner->height);
    printf("\tWeight(Kg): %lf\n", runner->weight);
    return runner;
}
