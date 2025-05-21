#ifndef BRANCH_H
#define BRANCH_H

#include "object_types.h"

Branch* create_branch(Repository* repo, const char* branch_name);
Branch* find_branch(Repository* repo, const char* branch_name);
void checkout_branch(Repository* repo, const char* branch_name);
void free_branch(Branch* branch);
Branch* create_branch_silent(Repository* repo, const char* name);

#endif
