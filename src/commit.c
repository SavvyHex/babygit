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
    strncpy(commit->message, message, sizeof(commit->message) - 1);
    commit->timestamp = time(NULL);
    commit->parent = NULL;
    commit->next = NULL;
    commit->parent_hash[0] = '\0';
    commit->second_parent[0] = '\0';

    if (repo->current_branch && repo->current_branch->head) {
        commit->parent = repo->current_branch->head;
        strncpy(commit->parent_hash, commit->parent->hash, sizeof(commit->parent_hash) - 1);
    }

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
         "parent %s\nauthor %s\ntime %ld\nmessage %s\n%s",
         commit->parent_hash, commit->author, commit->timestamp,
         commit->message, files_buf);

    // Hash the commit content
    calculate_hash(commit_content, strlen(commit_content), commit->hash);

    // Save to disk
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

    // Update current branch
    if (repo->current_branch) {
        repo->current_branch->head = commit;
    }

    // Add to repo->commits list
    commit->next = repo->commits;
    repo->commits = commit;

    // Clear staging area
    repo->staged_count = 0;
    free(repo->staged_files);
    repo->staged_files = NULL;
    remove(".babygit/index");  // Clear index file

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
        // Ignore files section for now
    }

    fclose(file);
    return commit;
}

void free_commit(Commit *commit) { free(commit); }
