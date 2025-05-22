#include "stash.h"
#include "commit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stash_changes(Repository *repo, const char *message) {
  if (!repo || !message)
    return;

  Commit *stash_commit = create_commit(repo, message, "stash");
  if (!stash_commit)
    return;

  Stash *stash = malloc(sizeof(Stash));
  if (!stash)
    return;

  strncpy(stash->message, message, 255);
  stash->commit = stash_commit;
  stash->next = NULL;

  if (!repo->stashes) {
    repo->stashes = stash;
  } else {
    Stash *last = repo->stashes;
    while (last->next)
      last = last->next;
    last->next = stash;
  }

  printf("Saved working directory to stash: %s\n", message);
}

static void restore_file_from_commit(const char *filename, const char *hash) {
  char object_path[256];
  snprintf(object_path, sizeof(object_path), ".babygit/objects/%s", hash);

  FILE *src = fopen(object_path, "r");
  if (!src) {
    printf("Failed to open object file for %s\n", filename);
    return;
  }

  FILE *dst = fopen(filename, "w");
  if (!dst) {
    printf("Failed to write file %s\n", filename);
    fclose(src);
    return;
  }

  char buf[1024];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
    fwrite(buf, 1, n, dst);
  }

  fclose(src);
  fclose(dst);
}

void apply_stash(Repository *repo, int stash_index) {
  if (!repo)
    return;

  Stash *stash = repo->stashes;
  for (int i = 0; i < stash_index && stash; i++) {
    stash = stash->next;
  }

  if (!stash) {
    printf("Stash not found\n");
    return;
  }

  printf("Applying stash: %s\n", stash->message);

  char commit_path[256];
  snprintf(commit_path, sizeof(commit_path), ".babygit/objects/%s",
           stash->commit->hash);
  FILE *f = fopen(commit_path, "r");
  if (!f) {
    printf("Failed to open stash commit file\n");
    return;
  }

  char line[512];
  while (fgets(line, sizeof(line), f)) {
    if (strncmp(line, "file ", 5) == 0) {
      char filename[256], file_hash[65];
      if (sscanf(line, "file %255s %64s", filename, file_hash) == 2) {
        restore_file_from_commit(filename, file_hash);
        printf("Restored %s\n", filename);
      }
    }
  }

  fclose(f);
}

void list_stashes(Repository *repo) {
  if (!repo)
    return;

  Stash *stash = repo->stashes;
  int index = 0;

  printf("Stashes:\n");
  while (stash) {
    printf("%d: %s (%s)\n", index++, stash->message, stash->commit->hash);
    stash = stash->next;
  }
}
