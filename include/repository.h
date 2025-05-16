#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "commit.h"
#include "branch.h"

// Repository state
typedef struct {
    Branch* branches;
    Branch* current_branch;
    Commit* commits;
    Stash* stashes;
    FileStatus* staged_files;
    int staged_count;
} Repository;

// Repository functions
Repository* init_repository();
Repository* load_repository();
void save_repository(Repository* repo);
void free_repository(Repository* repo);

#endif
