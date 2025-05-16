#include <dirent.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "repository.h"

// Basic object types
typedef enum {
  OBJ_BLOB,
  OBJ_TREE,
  OBJ_COMMIT
} ObjectType;

// Branch represented as a tree node
typedef struct Branch{
  char name[256];
  Commit* head;
  struct Brach* parent;
  struct Brach* children;
  struct Brach* next;
} Branch;

// File status
// 0 = unmodified
// 1 = modified
// 2 = added
// 3 = deleted
typedef struct {
  char filename[256];
  char has[41];
  int status;
};

// Stash item
typedef struct Stash {
  char message[256];
  Commit* commit;
  struct Stash* next;
} Stash;

// Function to create a branch
Branch* create_branch(Repository* repo, const char* branch_name){
  Branch* branch = malloc(sizeof(Branch));
  strncpy(branch->name, branch_name, 255);
  branch->head = repo->current_branch? repo->current_branch->head : NULL;
  branch->parent = repo->current_branch;
  branch->children = NULL;
  branch->next = NULL;

  if(!repo->branches){
    repo->branches = branch;
  } else {
    if (repo->current_branch) {
      if (!repo->current_branch->children) {
        repo->current_branch->children = branch;
      } else {
        Branch* last = repo->current_branch->children;
        while (last->next) {
          last = last->next;
        }
        last->next = branch;
      }
    } else {
      Branch* last = repo->branches;
      while (last->next) {
        last = last->next;
      }
      last->next = branch;
    }
  }

  char branch_path[256];
  sprintf(branch_path, ".babygit/refs/head/%s\n", branch_name);
  FILE* branch_file = fopen(branch_path, "w");
  if (branch->head) {
    fprintf(branch_file, "%s\n", branch->head->hash);
  }
  fclose(branch_file);
  
  printf("Created branch : %s\n", branch_name);
  return branch;
}

