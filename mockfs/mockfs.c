#include <stdlib.h>

#include <editline/readline.h>
#include <argp.h>

const char *argp_program_version = "0.1";
const char *argp_program_bug_address = "<val.baturin@serokell.io> or <val@baturin.me>";
static char doc[] = "mockfs";
static char args_doc[] = "";
static struct argp_option options[] = { 
    { "persist", 'e', 0, 0, "persistant storage, so reliable (NOT IMPLEMENTED THO)"},
    { 0 } 
};

struct arguments {
    enum { PRST, NPRST } mode;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
    case 'p': arguments->mode = PRST; break;
    case ARGP_KEY_ARG: return 0;
    default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}
 
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char** argv) {

    struct arguments arguments;

    arguments.mode = NPRST;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    while(1) {
        char* input = readline("> ");


        add_history(input);
        free(input);
    }
    return 0;
}
