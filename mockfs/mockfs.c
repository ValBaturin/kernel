#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <argp.h>
#include <editline/readline.h>

// 1 MB disk
#define BLOCK_SIZE 512
#define BLOCK_NUM 2048

const char *argp_program_version = "0.1";
const char *argp_program_bug_address =
    "<val.baturin@serokell.io> or <val@baturin.me>";
static char doc[] = "mockfs";
static char args_doc[] = "";
static struct argp_option options[] = {
    {"persist", 'e', 0, 0, "persistant storage, so reliable (NOT IMPLEMENTED)"},
    {0}};

struct arguments {
  enum { PRST, NPRST } mode;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 'p':
    arguments->mode = PRST;
    break;
  case ARGP_KEY_ARG:
    return 0;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

bool is_absolute(char **token) {
  *token = strtok(NULL, "/");
  if ((*token != NULL) && ((*token)[-1] == '/')) {
    return true;
  } else {
    return false;
  }
}

void next_obj(char **token) { *token = strtok(NULL, "/"); }

enum objTYPE { FSFILE, FSDIR };

struct disk {
  void *storage;
};

void init_disk(struct disk *d, int bsize, int bnum) {
  d->storage = calloc(BLOCK_NUM, BLOCK_SIZE);
}

// TODO check that fsobj struct size is actually blocksize
struct fsobj {
  bool available;
  enum objTYPE type;
  char name[20]; // LIMIT ON NAME IS 20 CHARS
  int size;
  void *data[BLOCK_SIZE - (1 + 1 + 20 + 4)];
};

struct filesystem {
  int current_ptr; // which fs object is currently working dir;
};

void init_fs(struct filesystem *fs, int size) {
  fs->capacity = size;
  fs->fsobjs = calloc(fs->capacity, sizeof(struct fsobj *));
  fs->current_ptr = 0;

  fs->fsobjs[fs->current_ptr] = malloc(sizeof(struct fsobj *));
  struct fsobj *curr = fs->fsobjs[fs->current_ptr];
  curr->size = 0;
  curr->type = FSDIR;
  curr->name = "/";
  curr->data = malloc(10); // init some memory for later use of realloc
}

int main(int argc, char **argv) {

  struct arguments arguments;

  arguments.mode = NPRST;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  struct filesystem fs;
  init_fs(&fs, 1024);
  struct fsobj *curr = fs.fsobjs[fs.current_ptr];

  char *current_dir = "/";
  char prompt[256];
  snprintf(prompt, sizeof(prompt), "%s\n> ", current_dir);

  while (1) {
    char *input = readline(prompt);
    add_history(input);

    char *token = strtok(input, " ");

    if (strcmp(token, "cd") == 0) {
      printf("cd\n");
    } else if (strcmp(token, "mkdir") == 0) {
      is_absolute(&token);
      curr->size++;
      curr->data = realloc(curr->data, (curr->size) * sizeof(struct fsobj *));
      curr->data[curr->size - 1] = malloc(sizeof(struct fsobj));

      struct fsobj *new_dir = curr->data[curr->size - 1];
      new_dir->size = 0;
      new_dir->type = FSDIR;

      new_dir->name = malloc(80 * sizeof(char *));

      snprintf(new_dir->name, sizeof(new_dir->name), "%s", token);
      new_dir->data = malloc(1); // init some memory for later use of realloc

    } else if (strcmp(token, "touch") == 0) {
      printf("touch\n");
    } else if (strcmp(token, "ls") == 0) {
      struct fsobj **content = curr->data;
      for (int i = 0; i < curr->size; i++) {
        printf("%s\n", content[i]->name);
      }

    } else if (strcmp(token, "rm") == 0) {
      printf("rm\n");
    } else if (strcmp(token, "rmdir") == 0) {
      printf("rmdir\n");
    } else if (strcmp(token, "cat") == 0) {
      printf("cat\n");
    }

    free(input);
  }
  return 0;
}
