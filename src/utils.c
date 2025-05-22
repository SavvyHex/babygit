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
