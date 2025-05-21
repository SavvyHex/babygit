#include "repository.h"
#include "utils.h"
#include "commit.h"
#include "branch.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

Repository *init_repository() {
  // Create directory structure
  ensure_directory_exists(".babygit");
  ensure_directory_exists(".babygit/objects");
  ensure_directory_exists(".babygit/refs");
  ensure_directory_exists(".babygit/refs/heads");
  ensure_directory_exists(".babygit/refs/remotes");

  // Create HEAD file
  FILE *head = fopen(".babygit/HEAD", "w");
  if (!head) {
    perror("Failed to create HEAD file");
    return NULL;
  }
  fprintf(head, "ref: refs/heads/master\n");
  fclose(head);

  // Initialize repository structure
  Repository *repo = malloc(sizeof(Repository));
  if (!repo)
    return NULL;

  repo->branches = NULL;
  repo->current_branch = NULL;
  repo->commits = NULL;
  repo->staged_files = NULL;
  repo->staged_count = 0;

  printf("Initialized empty babygit repository\n");
  return repo;
}

void save_repository(Repository *repo) {
  if (!repo)
    return;
  // TODO: Implement actual saving to disk
}

void free_repository(Repository *repo) {
  if (!repo)
    return;

  // Free branches (recursive)
  free_branch(repo->branches);

  // Free commits
  Commit *current = repo->commits;
  while (current) {
    Commit *next = current->next;
    free(current);
    current = next;
  }

  // Free staged files
  if (repo->staged_files) {
    free(repo->staged_files);
  }

  free(repo);
}

void load_branches(Repository* repo) {
    DIR* dir;
    struct dirent* entry;
    char path[256];
    
    snprintf(path, sizeof(path), ".babygit/refs/heads");
    dir = opendir(path);
    if (!dir) return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            // Create branch entry for each file in refs/heads
            create_branch(repo, entry->d_name);
        }
    }
    closedir(dir);
}

Repository* load_repository() {
    if (!file_exists(".babygit/HEAD")) return NULL;

    Repository* repo = malloc(sizeof(Repository));
    repo->branches = NULL;
    repo->current_branch = NULL;
    repo->commits = NULL;
    repo->staged_files = NULL;
    repo->staged_count = 0;

    load_branches(repo);
    return repo;
}
