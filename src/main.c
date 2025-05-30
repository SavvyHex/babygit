#include "branch.h"
#include "commit.h"
#include "merge.h"
#include "repository.h"
#include "staging.h"
#include "stash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <command> [args...]\n", argv[0]);
    return 1;
  }

  Repository *repo = load_repository();
  load_index(repo);

  if (!repo && strcmp(argv[1], "init") != 0) {
    printf("Not a babygit repository. Run 'init' first.\n");
    return 1;
  }

  const char *command = argv[1];

  if (strcmp(command, "init") == 0) {
    if (repo) {
      printf("Repository already initialized\n");
    } else {
      repo = init_repository();
    }
  } else if (strcmp(command, "add") == 0) {
    if (argc < 3) {
      printf("Usage: %s add <file>\n", argv[0]);
    } else {
        if (strcmp(argv[2], ".") == 0) {
            update_file_status(repo);  // Stage all files in current directory
        } else {
            add_to_index(repo, argv[2]);
        }
    }
  } else if (strcmp(command, "commit") == 0) {
    if (argc < 4) {
      printf("Usage: %s commit \"message\" \"author\"\n", argv[0]);
    } else {
      Commit *commit = create_commit(repo, argv[2], argv[3]);
      if (commit) {
        printf("Committed: %s\n", commit->hash);
        clear_staging_area(repo);
        repo->current_branch->head = commit;
      } else {
        printf("Commit failed. Nothing to commit or an error occurred.\n");
      }
    }
  } else if (strcmp(command, "branch") == 0) {
    if (argc < 3) {
      printf("Usage: %s branch <name>\n", argv[0]);
    } else {
      create_branch(repo, argv[2]);
      load_all_branch_heads(repo);
    }
  } else if (strcmp(command, "checkout") == 0) {
    if (argc < 3) {
      printf("Usage: %s checkout <branch>\n", argv[0]);
    } else {
      load_all_branch_heads(repo);
      checkout_branch(repo, argv[2]);
    }
  } else if (strcmp(command, "status") == 0) {
    print_status(repo);
  } else if (strcmp(command, "merge") == 0) {
    if (argc < 3) {
      printf("Usage: %s merge <branch>\n", argv[0]);
    } else {
      merge_branch(repo, argv[2]);
    }
  } else if (strcmp(command, "stash") == 0) {
    if (argc < 3) {
      list_stashes(repo);
    } else if (strcmp(argv[2], "apply") == 0 && argc >= 4) {
      apply_stash(repo, atoi(argv[3]));
    } else {
      stash_changes(repo, argv[2]);
    }
  } else {
    printf("Unknown command: %s\n", command);
  }

  save_repository(repo);
  free_repository(repo);
  save_index(repo);
  return 0;
}
