#include <dirent.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// Basic object types
typedef enum {
  OBJ_BLOB,
  OBJ_TREE,
  OBJ_COMMIT
} ObjectType;

// Commit represented as a node of a Linked List
typedef struct Commit{
  char hash[41];
  char parent_hash[41];
  char second_parent[41];
  char author[256];
  char message[256];
  time_t timestamp;
  struct Commit* parent;
  struct Commit* next;
} Commit;

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

// Repository state
typedef struct {
    Branch* branches;
    Branch* current_branch;
    Commit* commits;
    Stash* stashes;
    FileStatus* staged_files;
    int staged_count;
} Repository;

// Simplified Hashing
void calculate_hash(const char* content, size_t len, char* output){
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1((unsigned char*)content, len, hash);

  for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
    sprintf(output + (i * 2), "%02", hash[i]);
  }
}
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

// Function to create a Repository
Repository* init_repository(){
  mkdir(".babygit", 0755);
  mkdir(".babygit/objects", 0755);
  mkdir(".babygit/refs", 0755);
  mkdir(".babygit/refs/heads", 0755);
  mkdir(".babygit/refs/remotes", 0755);

  FILE* head = fopen(".babygit/HEAD", "w");
  fprintf(head, "ref: refs/heads/master\n");
  fclose("head");

  Repository* repo = malloc(sizeof(Repository));
  repo->branches = NULL;
  repo->commits = NULL;
  repo->stashes = NULL;
  repo->staged_files = NULL;
  repo->staged_count = 0;

  Branch* master = create_branch(repo, "master");
  repo->current_branch = master;

  printf("Initialized babygit repository\n");
  return repo;
}
