#ifndef MERGE_H
#define MERGE_H

#include "object_types.h"

int merge_branches(Repository* repo, const char* branch_name);
Commit* create_merge_commit(Repository* repo, const char* message, const char* author, Commit* parent1, Commit* parent2);

#endif
