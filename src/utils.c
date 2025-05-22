#include "utils.h"

#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void calculate_hash(const char *content, size_t len, char *output) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1((unsigned char *)content, len, hash);

  for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
    sprintf(output + (i * 2), "%02x", hash[i]);
  }
  output[40] = '\0';
}

int file_exists(const char *path) {
  struct stat buffer;
  return stat(path, &buffer) == 0;
}

void ensure_directory_exists(const char *path) {
  struct stat st = {0};
  if (stat(path, &st) == -1) {
    mkdir(path, 0755);
  }
}

char* read_object_file(const char* hash) {
    char path[256];
    snprintf(path, sizeof(path), ".babygit/objects/%s", hash);
    
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);
    
    return content;
}

static int extract_tree_hash(const char* commit_content, char* tree_hash_out) {
    // Assume the commit content starts with something like: "tree <hash>\n"
    const char* tree_line_prefix = "tree ";
    const size_t hash_length = 40;

    const char* tree_line = strstr(commit_content, tree_line_prefix);
    if (!tree_line) {
        fprintf(stderr, "Error: tree line not found in commit content.\n");
        return 0;
    }

    tree_line += strlen(tree_line_prefix); // Move past "tree "
    strncpy(tree_hash_out, tree_line, hash_length);
    tree_hash_out[hash_length] = '\0'; // Null-terminate

    return 1;
}
