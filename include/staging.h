#ifndef STAGING_H
#define STAGING_H

#include "object_types.h"

void add_to_index(Repository* repo, const char* filepath);
void clear_staging_area(Repository* repo);
void print_status(Repository* repo);
void update_file_status(Repository* repo);

#endif
