#ifndef RSTAT_GAURD_H
#define RSTAT_GAURD_H

#include <sqlite3.h>

#define RSTAT_USER_CREATE (unsigned char) 0x1
#define RSTAT_DATABASE_CREATE (unsigned char) 0x2

enum {
    RSTAT_SUCCESS,
    RSTAT_FAIL,
} RSTAT_STATUS;

enum {
    QUERY_ROW_UID,
    QUERY_ROW_LOCATION,
    QUERY_ROW_TIME,
    QUERY_ROW_DISTNCE,
    QUERY_ROW_TEMPERATURE
} QUERY_ROWS;

/*
 * Struct that represents a user within the sqlite3
 * database. Each field respresents a table value for
 * each particular user.
 */
typedef struct runner {
    uid_t uid;                  // Username may change, but UID won't
    char *username;             // WARNING May change
    char gender;                //'M' or 'F'
    double height;              // Meters
    double weight;              // Kilograms
    struct runner* next;        // For lists of users
} runner_t;

/*
 * Struct that represents a run that the runner has gone on.
 * Should contain basic information about how long and what
 * amount of time was spent running.
 */
typedef struct run {
    uid_t uid;          // Associative key
    char *location;     // String
    double time;        // Minutes
    double distance;    // Meters
    double temperature; // Deg Celcius
    struct run* next;   // For lists of runs
} run_t;

/*
 * If the user wants to create a new database properly,
 * they can use --create and give a path to create the database at.
 * This function will be called and the a database at the specified
 * location.
 */
int create_database(const char *, sqlite3 **);

/*
 * Checks whether the current user exists within the
 * running database. This is determined by grabbing the
 * users UID and looking for its existance in the database.
 */
int user_exists(sqlite3**);

/*
 * Builds a runner type struct, returns the address of
 * the allocated memory for the struct.
 *
 * Its arguments are the users UID, username, gender, height, weight,
 * and the runners total kilometers.
 */
runner_t *build_runner(uid_t, char*, char, double, double);

/*
 * Build run, builds a run object that represents a
 * run a runner has gone on.
 */
run_t *build_run(uid_t, char*, double, double, double);

/*
 * Puts a runner in the specified database connection.
 * Creates an SQL statement and uses exec to execute the
 * sqlite3 statement.
 */
int put_runner(runner_t *, sqlite3 **);

/*
 * Puts a run in the specified database connection
 */
int put_run(run_t *, sqlite3 **);

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
