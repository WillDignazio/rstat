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

runner_t *build_runner(
        uid_t uid,
        char *username,
        char gender,
        double height,
        double weight) {
    runner_t *out_r = malloc(sizeof(runner_t));
    out_r->uid = uid;
    out_r->username = username;
    out_r->height = height;
    out_r->weight = weight;
    return out_r;
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

runner_t *query_user_info(sqlite3 *sqconn) {

    runner_t *runner = calloc(sizeof(runner_t), 1);
    runner->username = get_username();

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
    printf("\tUsername: %s\n", runner->username);
    printf("\tGender: %c\n", runner->gender);
    printf("\tHeight(m): %lf\n", runner->height);
    printf("\tWeight(Kg): %lf\n", runner->weight);
    return runner;
}
