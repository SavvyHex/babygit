#include <stdio.h>
#include <string.h>

#include "repository.h"
#include "commit.h"
#include "branch.h"
#include "staging.h"
#include "stash.h"
#include "merge.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }
    
    Repository* repo = load_repository();
    if (!repo && strcmp(argv[1], "init") != 0) {
        printf("Not a babygit repository. Run 'init' first.\n");
        return 1;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "init") == 0) {
        if (repo) {
            printf("Repository already initialized\n");
        } else {
            repo = init_repository();
        }
    }
    else if (strcmp(command, "add") == 0) {
        if (argc < 3) {
            printf("Usage: %s add <file>\n", argv[0]);
        } else {
            add_to_index(repo, argv[2]);
        }
    }
    else if (strcmp(command, "commit") == 0) {
        if (argc < 4) {
            printf("Usage: %s commit \"message\" \"author\"\n", argv[0]);
        } else {
            create_commit(repo, argv[2], argv[3]);
        }
    }
    else if (strcmp(command, "branch") == 0) {
        if (argc < 3) {
            printf("Usage: %s branch <name>\n", argv[0]);
        } else {
            create_branch(repo, argv[2]);
        }
    }
    else if (strcmp(command, "checkout") == 0) {
        if (argc < 3) {
            printf("Usage: %s checkout <branch>\n", argv[0]);
        } else {
            checkout_branch(repo, argv[2]);
        }
    }
    else if (strcmp(command, "status") == 0) {
        print_status(repo);
    }
    else if (strcmp(command, "merge") == 0) {
        if (argc < 3) {
            printf("Usage: %s merge <branch>\n", argv[0]);
        } else {
            merge_branch(repo, argv[2]);
        }
    }
    else if (strcmp(command, "stash") == 0) {
        if (argc < 3) {
            list_stashes(repo);
        } else if (strcmp(argv[2], "apply") == 0 && argc >= 4) {
            apply_stash(repo, atoi(argv[3]));
        } else {
            stash_changes(repo, argv[2]);
        }
    }
    else {
        printf("Unknown command: %s\n", command);
    }
    
    save_repository(repo);
    free_repository(repo);
    return 0;
}
