#ifndef UTILS_H
#define UTILS_H

#include <openssl/sha.h>

void calculate_hash(const char* content, size_t len, char* output);
int file_exists(const char* path);
void ensure_directory_exists(const char* path);
char* read_object_file(const char* hash);
int extract_tree_hash(const char* commit_content, char* tree_hash_out);

#endif
