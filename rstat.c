/*
 * Will Dignazio
 * Running Stats (rstat) 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <argp.h>

const char *rstat_version = "rstat v0.1";
const char *rstat_bug_address = "<slackwill@csh.rit.edu>";

/* rstat documentation */
static char doc[] =
"rstat is a running statistics and record tool. It can \
be used in a large group setting to compare times and \
track improvements over time.";

static char args_doc[] = "DB_PATH";

static struct argp_option options[] = {
    {"database path", 'p', 0,  0, "Set rstat database path"},
    { 0 }
};

struct arguments
{
    char *args[1]; // One for each argument
    char *db_path;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state) {
    /* Get input argument from argp_parse, and store in
     * a arguments structure. */
    struct arguments *arguments = state->input;

    switch( key ) {
        case 'p':
            arguments->db_path = arg;
            break;
        /* Handle default options for end of argument parsing */
        case ARGP_KEY_ARG: // Too many arguments
            if(state->arg_num > 1)
                argp_usage(state);
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END: // To few arguments
            if(state->arg_num < 1)
                argp_usage(state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char *argv[]) {
    struct arguments arguments;

    /* Set default values */
    arguments.db_path = "/var/lib/rstat/rstat.db";

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    return 0;
}

