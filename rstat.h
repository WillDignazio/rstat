#ifndef RSTAT_GAURD_H
#define RSTAT_GAURD_H

#include <sqlite3.h>

enum {
    RSTAT_SUCCESS,
    RSTAT_FAIL,
    RSTAT_CREATE,
    RSTAT_NOCREATE
};

/*
 * Struct that represents a user within the sqlite3
 * database. Each field respresents a table value for
 * each particular user.
 */
typedef struct {
    uid_t uid;                  // Username may change, but UID won't
    char *username;             // WARNING May change
    char gender;                //'m' or 'f'
    double height;               // Meters
    double weight;               // Kilograms
} runner_t;

/*
 * If the user wants to create a new database properly,
 * they can use --create and give a path to create the database at.
 * This function will be called and the a database at the specified
 * location.
 */
int create_database(const char *, sqlite3 **);

/*
 * Builds a runner type struct, returns the address of
 * the allocated memory for the struct.
 *
 * Its arguments are the users UID, username, gender, height, weight,
 * and the runners total kilometers.
 */
runner_t *build_runner(uid_t, char*, char, double, double);

/*
 * Puts a runner in the specified database connection.
 * Creates an SQL statement and uses exec to execute the
 * sqlite3 statement.
 */
int put_runner(runner_t *, sqlite3 **);

/*
 * Initialize the rstat client, forms an active
 * connection to the sqlite database.
 *
 * The parameters are the path to the database.
 */
int rstat_init(const char *, int flags, sqlite3 **);

/*
 * Close up the connection to the sqlite3 database,
 * removes any other connections that will be made, and
 * frees up any allocated memory.
 */
int rstat_close(sqlite3 **);

/*
 * Gets the username of the current client, this is
 * based on the euid of the client. This evades some
 * simple tricks that a user could do to spoof the
 * program into logging in as somebody else.
 *
 * Must be freed when the call is done.
 */
char *get_username();

/*
 * Queries for relevant user information, and
 * fills in the blanks for some other parts. Username
 * and other account details are gotten from
 * querying the operating system.
 */
runner_t *query_user_info();

/*
 * Frees a runner and its constituent pointers, wraps
 * around free to make it easier.
 */
int free_runner(runner_t *);

#endif
