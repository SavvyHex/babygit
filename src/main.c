#include <stdio.h>
#include <string.h>

#include "repository.h"
#include "commit.h"
#include "branch.h"
#include "staging.h"

// Basic object types
typedef enum {
  OBJ_BLOB,
  OBJ_TREE,
  OBJ_COMMIT
} ObjectType;

// Stash item
typedef struct Stash {
  char message[256];
  Commit* commit;
  struct Stash* next;
} Stash;
