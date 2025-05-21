#include "staging.h"
#include "utils.h"
#include "branch.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void add_to_index(Repository *repo, const char *filepath) {
  if (!repo || !filepath)
    return;

  FILE *file = fopen(filepath, "rb");
  if (!file) {
    perror("Failed to open file");
    return;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *content = malloc(size);
  if (!content) {
    fclose(file);
    return;
  }

  fread(content, 1, size, file);
  fclose(file);

  char hash[41];
  calculate_hash(content, size, hash);
  free(content);

  // Check if file already staged
  for (int i = 0; i < repo->staged_count; i++) {
    if (strcmp(repo->staged_files[i].filename, filepath) == 0) {
      strcpy(repo->staged_files[i].hash, hash);
      repo->staged_files[i].status = 1; // Modified
      return;
    }
  }

  // Add new file to staging
  FileStatus *new_files = realloc(repo->staged_files, (repo->staged_count + 1) *
                                                          sizeof(FileStatus));
  if (!new_files)
    return;

  repo->staged_files = new_files;
  strncpy(repo->staged_files[repo->staged_count].filename, filepath, 255);
  strncpy(repo->staged_files[repo->staged_count].hash, hash, 40);
  repo->staged_files[repo->staged_count].status = 2; // Added
  repo->staged_count++;

  printf("Added %s to staging area\n", filepath);
}

void clear_staging_area(Repository *repo) {
  if (!repo)
    return;

  if (repo->staged_files) {
    free(repo->staged_files);
    repo->staged_files = NULL;
  }
  repo->staged_count = 0;

  FILE *index = fopen(".babygit/index", "w");
  if (index)
    fclose(index);
}

void update_file_status(Repository *repo) {
  if (!repo)
    return;

  clear_staging_area(repo);

  DIR *dir = opendir(".");
  if (!dir)
    return;

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type != DT_REG)
      continue;

    add_to_index(repo, entry->d_name);
  }
  closedir(dir);
}

void print_status(Repository *repo) {
  if (!repo)
    return;

  update_file_status(repo);

  printf("On branch %s\n",
         repo->current_branch ? repo->current_branch->name : "none");
  printf("\nStaged changes:\n");

  for (int i = 0; i < repo->staged_count; i++) {
    const char *status;
    switch (repo->staged_files[i].status) {
    case 0:
      status = "unmodified";
      break;
    case 1:
      status = "modified";
      break;
    case 2:
      status = "added";
      break;
    case 3:
      status = "deleted";
      break;
    default:
      status = "unknown";
    }
    printf("  %s: %s\n", status, repo->staged_files[i].filename);
  }
}
