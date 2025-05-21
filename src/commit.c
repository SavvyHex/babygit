#include "commit.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

Commit *create_commit(Repository *repo, const char *message, const char *author) {
    if (!repo || !message || !author) {
        printf("create_commit: Invalid parameters\n");
        return NULL;
    }

    if (repo->staged_count == 0) {
        printf("create_commit: No staged files to commit\n");
        return NULL;
    }

    Commit *commit = malloc(sizeof(Commit));
    if (!commit) {
        printf("create_commit: malloc failed\n");
        return NULL;
    }

    // Metadata
    strncpy(commit->author, author, sizeof(commit->author) - 1);
    commit->author[sizeof(commit->author) - 1] = '\0';

    strncpy(commit->message, message, sizeof(commit->message) - 1);
    commit->message[sizeof(commit->message) - 1] = '\0';

    commit->timestamp = time(NULL);
    commit->parent = NULL;
    commit->next = NULL;
    commit->parent_hash[0] = '\0';
    commit->second_parent[0] = '\0';

    if (repo->current_branch && repo->current_branch->head) {
        commit->parent = repo->current_branch->head;
        strncpy(commit->parent_hash, commit->parent->hash, sizeof(commit->parent_hash) - 1);
        commit->parent_hash[sizeof(commit->parent_hash) - 1] = '\0';
    }

    printf("DEBUG: Creating commit on branch '%s'\n", repo->current_branch ? repo->current_branch->name : "NULL");
    printf("DEBUG: Parent commit hash: %s\n", commit->parent_hash[0] ? commit->parent_hash : "None");

    char files_buf[2048] = "";
    for (int i = 0; i < repo->staged_count; i++) {
        strcat(files_buf, "file ");
        strcat(files_buf, repo->staged_files[i].filename);
        strcat(files_buf, " ");
        strcat(files_buf, repo->staged_files[i].hash);
        strcat(files_buf, "\n");
    }

    char commit_content[4096];
    snprintf(commit_content, sizeof(commit_content),
         "parent %s\nauthor %s\ntime %ld\nmessage %s\nfiles\n%s",
         commit->parent_hash, commit->author, commit->timestamp,
         commit->message, files_buf);

    calculate_hash(commit_content, strlen(commit_content), commit->hash);

    char commit_path[256];
    snprintf(commit_path, sizeof(commit_path), ".babygit/objects/%s", commit->hash);
    FILE *commit_file = fopen(commit_path, "w");
    if (!commit_file) {
        printf("create_commit: Failed to open commit file %s for writing\n", commit_path);
        free(commit);
        return NULL;
    }

    int written = fprintf(commit_file, "%s", commit_content);
    fclose(commit_file);

    if (written < 0) {
        printf("create_commit: Failed to write commit content\n");
        free(commit);
        return NULL;
    }

    // Update current branch HEAD
    if (repo->current_branch) {
        printf("DEBUG: Updating HEAD of branch '%s' to new commit %s\n", repo->current_branch->name, commit->hash);
        repo->current_branch->head = commit;
    }

    // Update branch list HEAD as well
    Branch *cur_branch = repo->branches;
    while (cur_branch) {
        if (strcmp(cur_branch->name, repo->current_branch->name) == 0) {
            cur_branch->head = commit;
            break;
        }
        cur_branch = cur_branch->next;
    }

    // Add to repo commit list
    commit->next = repo->commits;
    repo->commits = commit;

    // Clear staging
    repo->staged_count = 0;
    if (repo->staged_files) {
        free(repo->staged_files);
        repo->staged_files = NULL;
    }
    remove(".babygit/index");

    return commit;
}

Commit *find_commit(Repository *repo, const char *hash) {
  if (!repo || !hash)
    return NULL;

  Commit *current = repo->commits;
  while (current) {
    if (strcmp(current->hash, hash) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

Commit* load_commit(const char* hash) {
    if (!hash) return NULL;

    char path[256];
    snprintf(path, sizeof(path), ".babygit/objects/%s", hash);
    FILE* file = fopen(path, "r");
    if (!file) return NULL;

    Commit* commit = malloc(sizeof(Commit));
    if (!commit) {
        fclose(file);
        return NULL;
    }

    char line[1024];
    commit->parent_hash[0] = '\0';
    commit->author[0] = '\0';
    commit->message[0] = '\0';
    commit->timestamp = 0;
    commit->parent = NULL;
    commit->next = NULL;

    strncpy(commit->hash, hash, sizeof(commit->hash));
    commit->hash[sizeof(commit->hash) - 1] = '\0';

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "parent ", 7) == 0) {
            sscanf(line + 7, "%40s", commit->parent_hash);
        } else if (strncmp(line, "author ", 7) == 0) {
            sscanf(line + 7, "%255[^\n]", commit->author);
        } else if (strncmp(line, "time ", 5) == 0) {
            sscanf(line + 5, "%ld", &commit->timestamp);
        } else if (strncmp(line, "message ", 8) == 0) {
            sscanf(line + 8, "%1023[^\n]", commit->message);
        }
        // ignoring files for now
    }

    fclose(file);
    return commit;
}

void free_commit(Commit *commit) {
    free(commit);
}

Commit* find_commit_by_hash(Repository* repo, const char* hash){
    if (!hash) return NULL;
    Commit* current = repo->commits; // Adjust this to your actual commit list
    while (current) {
        if (strcmp(current->hash, hash) == 0) {
            return current;
        }
        current = current->next;  // or however commits are linked/stored
    }
    return NULL;
}
