#ifndef COMMIT_H
#define COMMIT_H

#include <time.h>
#include "repository.h"

typedef struct Commit {
    char hash[41];
    char parent_hash[41];
    char second_parent[41];
    char author[256];
    char message[1024];
    time_t timestamp;
    struct Commit* parent;
    struct Commit* next;
} Commit;

Commit* create_commit(Repository* repo, const char* message, const char* author);
Commit* find_commit(Repository* repo, const char* hash);
void free_commit(Commit* commit);

#endif
