#include "utils.h"

#include <openssl/sha.h>
#include <stdio.h>
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
