#ifndef COMMIT_H
#define COMMIT_H

#include <time.h>
#include "object_types.h"

Commit* create_commit(Repository* repo, const char* message, const char* author);
Commit* find_commit(Repository* repo, const char* hash);
void free_commit(Commit* commit);

#endif
