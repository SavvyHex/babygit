#include "branch.h"
#include "repository.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_branch_head(Branch* branch) {
    char path[512];
    snprintf(path, sizeof(path), ".babygit/refs/heads/%s", branch->name);
    FILE* f = fopen(path, "r");
    if (!f) {
        branch->head = NULL;
        return;
    }
    char hash[41];
    if (fgets(hash, sizeof(hash), f)) {
        // remove trailing newline
        hash[strcspn(hash, "\n")] = 0;
        // find commit object by hash
        branch->head = find_commit_by_hash(hash);
    } else {
        branch->head = NULL;
    }
    fclose(f);
}

Branch* create_branch(Repository* repo, const char* name) {
    if (!repo || !name) return NULL;

    if (find_branch(repo, name)) {
        printf("Branch %s already exists\n", name);
        return NULL;
    }

    Branch* branch = malloc(sizeof(Branch));
    if (!branch) return NULL;

    memset(branch, 0, sizeof(Branch));
    strncpy(branch->name, name, sizeof(branch->name) - 1);
    branch->name[sizeof(branch->name) - 1] = '\0';

    branch->head = repo->current_branch ? repo->current_branch->head : NULL;
    branch->parent = repo->current_branch;
    branch->children = NULL;
    branch->next = NULL;

    // Add as child to parent branch if applicable
    if (branch->parent) {
        branch->next = branch->parent->children;
        branch->parent->children = branch;
    }

    // Add to repository branch list at end
    if (!repo->branches) {
        repo->branches = branch;
    } else {
        Branch* cur = repo->branches;
        while (cur->next) cur = cur->next;
        cur->next = branch;
    }

    // Create/update branch ref file
    char path[256];
    snprintf(path, sizeof(path), ".babygit/refs/heads/%s", name);
    FILE* f = fopen(path, "w");
    if (f) {
        if (branch->head) {
            fprintf(f, "%s\n", branch->head->hash);
        }
        fclose(f);
    } else {
        printf("Warning: Could not create branch reference file for %s\n", name);
    }

    printf("Created branch %s\n", name);
    load_branch_head(branch);
    return branch;
}

Branch* find_branch(Repository* repo, const char* branch_name) {
    if (!repo || !branch_name) return NULL;

    Branch* current = repo->branches;
    while (current) {
        if (strcmp(current->name, branch_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void free_branch(Branch* branch) {
    if (!branch) return;

    Branch* child = branch->children;
    while (child) {
        Branch* next_child = child->next;
        free_branch(child);
        child = next_child;
    }

    free(branch);
}

void checkout_branch(Repository* repo, const char* branch_name) {
    Branch* branch = find_branch(repo, branch_name);
    if (!branch) {
        printf("Branch %s not found\n", branch_name);
        return;
    }

    load_branch_head(branch);

    repo->current_branch = branch;

    FILE* head = fopen(".babygit/HEAD", "w");
    if (head) {
        fprintf(head, "ref: refs/heads/%s\n", branch_name);
        fclose(head);
    } else {
        printf("Warning: Could not update HEAD file\n");
    }

    printf("Switched to branch %s\n", branch_name);
}

Branch* create_branch_silent(Repository* repo, const char* name) {
    if (!repo || !name) return NULL;

    if (find_branch(repo, name)) {
        return NULL;
    }

    Branch* branch = malloc(sizeof(Branch));
    if (!branch) return NULL;

    memset(branch, 0, sizeof(Branch));
    strncpy(branch->name, name, sizeof(branch->name) - 1);
    branch->name[sizeof(branch->name) - 1] = '\0';

    branch->head = NULL;
    branch->parent = NULL;
    branch->children = NULL;

    // Insert at head of branch list
    branch->next = repo->branches;
    repo->branches = branch;

    // No ref file creation for silent branch

    return branch;
}

// Additional utility: update branch ref file when head changes
void update_branch_ref(Branch* branch) {
    if (!branch) return;
    char path[256];
    snprintf(path, sizeof(path), ".babygit/refs/heads/%s", branch->name);
    FILE* f = fopen(path, "w");
    if (f) {
        if (branch->head) {
            fprintf(f, "%s\n", branch->head->hash);
        } else {
            fprintf(f, "\n"); // Clear ref if no head
        }
        fclose(f);
    }
}

// Update the head pointer of a branch and save ref
void set_branch_head(Branch* branch, Commit* commit) {
    if (!branch) return;
    branch->head = commit;
    update_branch_ref(branch);
}

void load_all_branch_heads(Repository* repo) {
    Branch* cur = repo->branches;
    while (cur) {
        load_branch_head(cur);
        cur = cur->next;
    }
}
