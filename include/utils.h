#ifndef UTILS_H
#define UTILS_H

#include <openssl/sha.h>

void calculate_hash(const char* content, size_t len, char* output);
int file_exists(const char* path);
void ensure_directory_exists(const char* path);
char* read_object_file(const char* hash);

#endif
