#ifndef BRANCH_H
#define BRANCH_H

#include "commit.h"
#include "repository.h"

typedef struct Branch {
    char name[256];
    Commit* head;
    struct Branch* parent;
    struct Branch* children;
    struct Branch* next;
} Branch;

Branch* create_branch(Repository* repo, const char* branch_name);
Branch* find_branch(Repository* repo, const char* branch_name);
void checkout_branch(Repository* repo, const char* branch_name);
void free_branch(Branch* branch);

#endif
