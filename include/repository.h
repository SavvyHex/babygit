#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "branch.h"
#include "object_types.h"

// Repository functions
Repository* init_repository();
Repository* load_repository();
void save_repository(Repository* repo);
void free_repository(Repository* repo);

#endif
