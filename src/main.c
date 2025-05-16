#include <stdio.h>
#include <string.h>

#include "repository.h"
#include "commit.h"
#include "branch.h"
#include "staging.h"
#include "stash.h"
#include "merge.h"

// Basic object types
typedef enum {
  OBJ_BLOB,
  OBJ_TREE,
  OBJ_COMMIT
} ObjectType;
