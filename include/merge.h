#ifndef MERGE_H
#define MERGE_H

#include "object_types.h"

void merge_branch(Repository* repo, const char* branch_name);
void resolve_merge_conflict(Repository* repo, const char* filepath);

#endif
