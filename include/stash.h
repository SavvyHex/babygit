#ifndef STASH_H
#define STASH_H

#include "commit.h"
#include "repository.h"
#include "object_types.h"

void stash_changes(Repository* repo, const char* message);
void apply_stash(Repository* repo, int stash_index);
void list_stashes(Repository* repo);

#endif
