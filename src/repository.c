#include "repository.h"
#include "utils.h"
#include "commit.h"
#include "branch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

void ensure_main_branch(Repository* repo) {
    if (!find_branch(repo, "main")) {
        create_branch(repo, "main");
        checkout_branch(repo, "main");
    }
}

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

  // Create initial 'main' branch
  create_branch(repo, "main");
  checkout_branch(repo, "main");

  printf("Initialized empty babygit repository\n");
  return repo;
}

void save_repository(Repository* repo) {
    if (!repo) return;

    // Ensure .babygit directory structure exists
    mkdir(".babygit", 0755);
    mkdir(".babygit/objects", 0755);
    mkdir(".babygit/refs", 0755);
    mkdir(".babygit/refs/heads", 0755);

    // Save HEAD
    FILE* head_file = fopen(".babygit/HEAD", "w");
    if (head_file) {
        if (repo->current_branch) {
            fprintf(head_file, "ref: refs/heads/%s\n", repo->current_branch->name);
        } else {
            fprintf(head_file, "ref: refs/heads/main\n");
        }
        fclose(head_file);
    }

    // Save branches
    Branch* branch = repo->branches;
    while (branch) {
        char branch_path[256];
        int written = snprintf(branch_path, sizeof(branch_path),
                           ".babygit/refs/heads/%s", branch->name);
        if (written < 0 || (size_t)written >= sizeof(branch_path)){
            fprintf(stderr, "Branch path too long for branch: %s\n", branch->name);
            continue; // or return NULL if you're in load_repository
        }
        
        FILE* branch_file = fopen(branch_path, "w");
        if (branch_file) {
            if (branch->head) {
                fprintf(branch_file, "%s", branch->head->hash);
            }
            fclose(branch_file);
        }
        branch = branch->next;
    }

    // Save staged files (simplified example)
    FILE* index_file = fopen(".babygit/index", "w");
    if (index_file) {
        for (int i = 0; i < repo->staged_count; i++) {
            fprintf(index_file, "%s %s\n", 
                   repo->staged_files[i].hash, 
                   repo->staged_files[i].filename);
        }
        fclose(index_file);
    }
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
    // Check .babygit exists first
    if (access(".babygit", F_OK) != 0) return NULL;

    Repository* repo = malloc(sizeof(Repository));
    repo->branches = NULL;
    repo->current_branch = NULL;
    repo->commits = NULL;
    repo->staged_files = NULL;
    repo->staged_count = 0;
    
    // Load existing branches
    DIR* dir = opendir(".babygit/refs/heads");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || 
                strcmp(entry->d_name, "..") == 0) continue;
                
            create_branch(repo, entry->d_name);
        }
        closedir(dir);
    }

    // Load HEAD
    FILE* head_file = fopen(".babygit/HEAD", "r");
    if (head_file) {
        char branch_name[256];
        if (fscanf(head_file, "ref: refs/heads/%255s", branch_name) == 1) {
            checkout_branch(repo, branch_name);
        }
        fclose(head_file);
    }

    char branch_path[256];
    int written = snprintf(branch_path, sizeof(branch_path),
                           ".babygit/refs/heads/%s", branch->name);
    if (written < 0 || written >= sizeof(branch_path)) {
        fprintf(stderr, "Branch path too long for branch: %s\n", branch->name);
        return NULL;
    }

    FILE* branch_file = fopen(branch_path, "r");
    if (branch_file) {
        char commit_hash[65];
        if (fscanf(branch_file, "%64s", commit_hash) == 1) {
            Commit* head_commit = load_commit(commit_hash);
            if (head_commit) {
                repo->current_branch->head = head_commit;
            }
        }
        fclose(branch_file);
    }
    
    return repo;
}
