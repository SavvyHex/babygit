#include "commit.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

Commit *create_commit(Repository *repo, const char *message,
                      const char *author) {
  if (!repo || !message || !author)
    return NULL;

  Commit *commit = malloc(sizeof(Commit));
  if (!commit)
    return NULL;

  // Set commit metadata
  strncpy(commit->author, author, 255);
  strncpy(commit->message, message, 1023);
  commit->timestamp = time(NULL);
  commit->next = NULL;
  commit->parent = NULL;
  commit->parent_hash[0] = '\0';
  commit->second_parent[0] = '\0';

  // Set parent if exists
  if (repo->current_branch && repo->current_branch->head) {
    commit->parent = repo->current_branch->head;
    strncpy(commit->parent_hash, repo->current_branch->head->hash, 40);
  }

  // Generate commit content and hash
  char commit_content[2048];
  sprintf(commit_content, "parent %s\nauthor %s\ntime %ld\nmessage %s\n",
          commit->parent_hash, author, commit->timestamp, message);

  calculate_hash(commit_content, strlen(commit_content), commit->hash);

  // Store commit object
  char commit_path[256];
  sprintf(commit_path, ".babygit/objects/%s", commit->hash);
  FILE *commit_file = fopen(commit_path, "w");
  if (!commit_file) {
    free(commit);
    return NULL;
  }
  fprintf(commit_file, "%s", commit_content);
  fclose(commit_file);

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
    char path[256];
    snprintf(path, sizeof(path), ".babygit/objects/%s", hash);

    FILE* file = fopen(path, "r");
    if (!file) {
        perror("Failed to open commit file");
        return NULL;
    }

    Commit* commit = malloc(sizeof(Commit));
    if (!commit) {
        fclose(file);
        return NULL;
    }

    strncpy(commit->hash, hash, sizeof(commit->hash));
    commit->hash[sizeof(commit->hash) - 1] = '\0';

    // Read message and author (assuming each is one line)
    if (!fgets(commit->message, sizeof(commit->message), file) ||
        !fgets(commit->author, sizeof(commit->author), file)) {
        free(commit);
        fclose(file);
        return NULL;
    }

    // Remove newline characters
    commit->message[strcspn(commit->message, "\n")] = 0;
    commit->author[strcspn(commit->author, "\n")] = 0;

    // For simplicity, we’re ignoring parent/next for now
    commit->parent = NULL;
    commit->next = NULL;

    fclose(file);
    return commit;
}

void free_commit(Commit *commit) { free(commit); }
