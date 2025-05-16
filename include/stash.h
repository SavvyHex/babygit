#ifndef STASH_H
#define STASH_H

#include "commit.h"

typedef struct Stash {
    char message[256];
    Commit* commit;
    struct Stash* next;
} Stash;

void stash_changes(Repository* repo, const char* message);
void apply_stash(Repository* repo, int stash_index);
void list_stashes(Repository* repo);

#endif
