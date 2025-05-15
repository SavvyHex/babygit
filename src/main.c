#include <dirent.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// Commit represented as a node of a Linked List
typedef struct Commit{
  char hash[41];
  char parent_hash[41];
  char second_parent[41];
  char author[256];
  char message[256];
  time_t timestamp;
  struct Commit* parent;
  struct Commit* next;
} Commit;

// Branch represented as a tree node
typedef struct Branch{
  char name[256];
  Commit* head;
  struct Brach* parent;
  struct Brach* children;
  struct Brach* next;
} Branch;
