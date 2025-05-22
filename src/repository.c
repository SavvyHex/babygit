#include "repository.h"
#include "branch.h"
#include "commit.h"
#include "utils.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void ensure_main_branch(Repository *repo) {
  if (!find_branch(repo, "main")) {
    create_branch(repo, "main");
    checkout_branch(repo, "main");
  }
}

Repository *init_repository() {
  ensure_directory_exists(".babygit");
  ensure_directory_exists(".babygit/objects");
  ensure_directory_exists(".babygit/refs");
  ensure_directory_exists(".babygit/refs/heads");
  ensure_directory_exists(".babygit/refs/remotes");

  FILE *head = fopen(".babygit/HEAD", "w");
  if (!head) {
    perror("Failed to create HEAD file");
    return NULL;
  }
  fprintf(head, "ref: refs/heads/master\n");
  fclose(head);

  Repository *repo = malloc(sizeof(Repository));
  if (!repo)
    return NULL;

  repo->branches = NULL;
  repo->current_branch = NULL;
  repo->commits = NULL;
  repo->staged_files = NULL;
  repo->staged_count = 0;

  ensure_main_branch(repo);

  printf("Initialized empty babygit repository\n");
  return repo;
}

void save_repository(Repository *repo) {
  if (!repo)
    return;

  mkdir(".babygit", 0755);
  mkdir(".babygit/objects", 0755);
  mkdir(".babygit/refs", 0755);
  mkdir(".babygit/refs/heads", 0755);

  FILE *head_file = fopen(".babygit/HEAD", "w");
  if (head_file) {
    if (repo->current_branch) {
      fprintf(head_file, "ref: refs/heads/%s\n", repo->current_branch->name);
    } else {
      fprintf(head_file, "ref: refs/heads/main\n");
    }
    fclose(head_file);
  }

  Branch *branch = repo->branches;
  while (branch) {
    char branch_path[256];
    int written = snprintf(branch_path, sizeof(branch_path),
                           ".babygit/refs/heads/%s", branch->name);
    if (written < 0 || (size_t)written >= sizeof(branch_path)) {
      fprintf(stderr, "Branch path too long for branch: %s\n", branch->name);
      branch = branch->next;
      continue;
    }

    FILE *branch_file = fopen(branch_path, "w");
    if (branch_file) {
      if (branch->head) {
        fprintf(branch_file, "%s", branch->head->hash);
      }
      fclose(branch_file);
    }
    branch = branch->next;
  }

  FILE *index_file = fopen(".babygit/index", "w");
  if (index_file) {
    for (int i = 0; i < repo->staged_count; i++) {
      fprintf(index_file, "%s %s\n", repo->staged_files[i].hash,
              repo->staged_files[i].filename);
    }
    fclose(index_file);
  }
}

void free_repository(Repository *repo) {
  if (!repo)
    return;

  free_branch(repo->branches);

  Commit *current = repo->commits;
  while (current) {
    Commit *next = current->next;
    free(current);
    current = next;
  }

  if (repo->staged_files) {
    free(repo->staged_files);
  }

  free(repo);
}

Commit* parse_commit_content(const char* commit_hash, const char* content) {
    Commit *commit = malloc(sizeof(Commit));
    if (!commit) return NULL;

    memset(commit, 0, sizeof(Commit));
    strncpy(commit->hash, commit_hash, sizeof(commit->hash) - 1);

    const char *pos = content;
    char line[1024];
    while (*pos) {
        int i = 0;
        while (*pos && *pos != '\n' && i < (int)(sizeof(line) - 1)) {
            line[i++] = *pos++;
        }
        if (*pos == '\n') pos++;
        line[i] = '\0';

        if (strncmp(line, "parent ", 7) == 0) {
            strncpy(commit->parent_hash, line + 7, sizeof(commit->parent_hash) - 1);
        } else if (strncmp(line, "author ", 7) == 0) {
            strncpy(commit->author, line + 7, sizeof(commit->author) - 1);
        } else if (strncmp(line, "time ", 5) == 0) {
            commit->timestamp = (time_t)atol(line + 5);
        } else if (strncmp(line, "message ", 8) == 0) {
            strncpy(commit->message, line + 8, sizeof(commit->message) - 1);
        } 
    }

    commit->parent = NULL;
    commit->next = NULL;
    commit->second_parent[0] = '\0';

    return commit;
}

Commit* load_commit_by_hash(Repository *repo, const char *hash) {
    if (!repo || !hash) return NULL;

    // 1. Check if commit is already loaded in repo->commits linked list
    Commit *cur = repo->commits;
    while (cur) {
        if (strncmp(cur->hash, hash, 40) == 0) {
            return cur;
        }
        cur = cur->next;
    }

    // 2. If not found, load commit file from disk
    char commit_path[256];
    snprintf(commit_path, sizeof(commit_path), ".babygit/objects/%s", hash);

    FILE *f = fopen(commit_path, "r");
    if (!f) {
        printf("load_commit_by_hash: Commit file %s not found\n", commit_path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (filesize <= 0 || filesize >= 10000) {
        fclose(f);
        printf("load_commit_by_hash: Commit file size invalid or too large\n");
        return NULL;
    }

    char *content = malloc(filesize + 1);
    if (!content) {
        fclose(f);
        printf("load_commit_by_hash: malloc failed\n");
        return NULL;
    }

    fread(content, 1, filesize, f);
    content[filesize] = '\0';
    fclose(f);

    Commit *commit = parse_commit_content(hash, content);
    free(content);

    if (!commit) {
        printf("load_commit_by_hash: Failed to parse commit\n");
        return NULL;
    }

    commit->next = repo->commits;
    repo->commits = commit;

    return commit;
}

void load_branches(Repository *repo) {
  DIR *dir;
  struct dirent *entry;
  char path[256];

  snprintf(path, sizeof(path), ".babygit/refs/heads");
  dir = opendir(path);
  if (!dir)
    return;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      Branch *branch = create_branch(repo, entry->d_name);
      if (!branch) continue;

      char ref_path[512];
      snprintf(ref_path, sizeof(ref_path), ".babygit/refs/heads/%s", entry->d_name);

      FILE *f = fopen(ref_path, "r");
      if (!f) continue;

      char commit_hash[41];
      if (fgets(commit_hash, sizeof(commit_hash), f)) {
        commit_hash[strcspn(commit_hash, "\r\n")] = 0;

        Commit *commit = load_commit_by_hash(repo, commit_hash);
        if (commit) {
          branch->head = commit;
        }
      }
      fclose(f);
    }
  }
  closedir(dir);
}

Repository *load_repository() {
  if (access(".babygit", F_OK) != 0)
    return NULL;

  Repository *repo = malloc(sizeof(Repository));
  repo->branches = NULL;
  repo->current_branch = NULL;
  repo->commits = NULL;
  repo->staged_files = NULL;
  repo->staged_count = 0;

  DIR *dir = opendir(".babygit/refs/heads");
  if (dir) {
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
      create_branch_silent(repo, entry->d_name);
    }
    closedir(dir);
  }

  FILE *head_file = fopen(".babygit/HEAD", "r");
  if (head_file) {
    char branch_name[256];
    if (fscanf(head_file, "ref: refs/heads/%255s", branch_name) == 1) {
      checkout_branch(repo, branch_name);
    }
    fclose(head_file);
  }

  Branch *branch = repo->current_branch;
  if (!branch) {
    fprintf(stderr, "Current branch not set. HEAD might be corrupt.\n");
    return NULL;
  }

  char branch_path[256];
  int written = snprintf(branch_path, sizeof(branch_path),
                         ".babygit/refs/heads/%s", branch->name);
  if (written < 0 || (size_t)written >= sizeof(branch_path)) {
    fprintf(stderr, "Branch path too long for branch: %s\n", branch->name);
    return NULL;
  }

  FILE *branch_file = fopen(branch_path, "r");
  if (branch_file) {
    char commit_hash[65];
    if (fscanf(branch_file, "%64s", commit_hash) == 1) {
      Commit *head_commit = load_commit(commit_hash);
      if (head_commit) {
        repo->current_branch->head = head_commit;
      }
    }
    fclose(branch_file);
  }

  return repo;
}
