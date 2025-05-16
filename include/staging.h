#ifndef STAGING_H
#define STAGING_H

typedef struct {
    char filename[256];
    char hash[41];
    int status;
} FileStatus;

void add_to_index(Repository* repo, const char* filepath);
void clear_staging_area(Repository* repo);
void print_status(Repository* repo);
void update_file_status(Repository* repo);

#endif
