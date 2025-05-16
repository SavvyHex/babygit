#include <stdio.h>
#include <string.h>

#include "merge.h"
#include "branch.h"
#include "commit.h"

void merge_branch(Repository *repo, const char *branch_name) {
  if (!repo || !branch_name)
    return;

  Branch *to_merge = find_branch(repo, branch_name);
  if (!to_merge) {
    printf("Branch %s not found\n", branch_name);
    return;
  }

  if (!repo->current_branch || !repo->current_branch->head || !to_merge->head) {
    printf("Cannot merge: invalid branch state\n");
    return;
  }

  printf("Merging branch %s into %s\n", branch_name,
         repo->current_branch->name);
  // TODO: Implement actual merge logic
}

// void resolve_merge_conflict(Repository *repo, const char *filepath) {
//   printf("Resolving conflict in %s\n", filepath);
//   // TODO: Implement conflict resolution
// }
