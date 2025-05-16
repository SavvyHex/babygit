#ifndef MYGIT_TYPES_H
#define MYGIT_TYPES_H

#include <time.h>

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

typedef struct Branch {
    char name[256];
    Commit* head;
    struct Branch* parent;
    struct Branch* children;
    struct Branch* next;
} Branch;

typedef struct FileStatus {
    char filename[256];
    char hash[41];
    int status;
} FileStatus;

typedef struct Stash {
    char message[256];
    Commit* commit;
    struct Stash* next;
} Stash;

typedef struct Repository {
    Branch* branches;
    Branch* current_branch;
    Commit* commits;
    FileStatus* staged_files;
    int staged_count;
    Stash* stashes;
} Repository;

#endif
