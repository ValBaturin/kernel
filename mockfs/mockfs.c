#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <argp.h>
#include <editline/readline.h>

// 1 MB disk
#define BLOCK_SIZE 512
#define BLOCK_NUM 2048
#define NAME_LIMIT 20

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

// struct disk {
//  void *storage;
//};
//
// void init_disk(struct disk *d) { d->storage = calloc(BLOCK_NUM, BLOCK_SIZE);
// }

// TODO check that fsobj struct size is actually blocksize
struct fsobj {
  bool busy; // defaults to zero aka false due to calloc
  enum objTYPE type;
  char name[NAME_LIMIT];
  int size; // must be less than data size for now

  // for FSDIR data is a list of contect inside the dir
  void *data[BLOCK_SIZE - (1 + 1 + NAME_LIMIT + 4)];
};

struct filesystem {
  void *disk;
  int current_ptr; // which fs object is currently a working dir;
};

struct fsobj *get_fso(struct filesystem *fs, int i) {
  return (struct fsobj *)&(fs->disk[i * BLOCK_SIZE]);
}

int get_available_fsobj(struct filesystem *fs) {
  for (int i = 0; i < BLOCK_NUM; ++i) {
    if (!get_fso(fs, i)->busy)
      return i;
  }
  return -1;
}

void new_fsobj_dir(struct fsobj *available_fsobj, char *name) {
  available_fsobj->busy = true;
  available_fsobj->type = FSDIR;
  snprintf(available_fsobj->name, sizeof(available_fsobj->name), "%s", name);
}

void new_fsobj_file(struct fsobj *available_fsobj, char *name) {
  available_fsobj->busy = true;
  available_fsobj->type = FSFILE;
  snprintf(available_fsobj->name, sizeof(available_fsobj->name), "%s", name);
}

void init_fs(struct filesystem *fs) {
  fs->disk = calloc(BLOCK_NUM, BLOCK_SIZE);
  fs->current_ptr = 0;

  struct fsobj *curr = get_fso(fs, fs->current_ptr);
  curr->busy = true;
  curr->type = FSDIR;
  snprintf(curr->name, sizeof(curr->name), "/");
  curr->size = 0;
}

int main(int argc, char **argv) {

  struct arguments arguments;

  arguments.mode = NPRST;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  struct filesystem *fs;
  fs = malloc(sizeof(struct filesystem));
  init_fs(fs);
  struct fsobj *curr = get_fso(fs, fs->current_ptr);

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
      // todo remove is_absolute from here
      // is_absolute(&token);
      char *token = strtok(NULL, " ");

      int i = get_available_fsobj(fs);

      curr->size++;
      curr->data[(curr->size - 1) * sizeof(int)] = i;

      new_fsobj_dir(get_fso(fs, i), token);

    } else if (strcmp(token, "touch") == 0) {

      // todo remove is_absolute from here
      is_absolute(&token);

      int i = get_available_fsobj(fs);

      curr->size++;
      curr->data[(curr->size - 1) * sizeof(int)] = i;

      new_fsobj_file(get_fso(fs, i), token);

    } else if (strcmp(token, "ls") == 0) {
      for (int i = 0; i < curr->size; i++) {
        struct fsobj *elem = get_fso(fs, (int)curr->data[i * sizeof(int)]);
        if (elem->type == FSDIR) {
          printf("Type: dir\tName: %s\tSize: %d\n", elem->name, elem->size);
        } else if (elem->type == FSFILE) {
          printf("Type: file\tName: %s\tSize: %d\n", elem->name, elem->size);
        }
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
