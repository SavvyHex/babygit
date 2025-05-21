#include "branch.h"
#include "repository.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Branch* create_branch(Repository* repo, const char* branch_name) {
    if (!repo || !branch_name) return NULL;
    
    // Check if branch already exists
    if (find_branch(repo, branch_name)) {
        printf("Branch %s already exists\n", branch_name);
        return NULL;
    }

    Branch* branch = malloc(sizeof(Branch));
    strncpy(branch->name, branch_name, 255);
    branch->head = repo->current_branch ? repo->current_branch->head : NULL;
    branch->parent = repo->current_branch;
    branch->children = NULL;
    branch->next = NULL;

    // Add to repository's branch list
    if (!repo->branches) {
        repo->branches = branch;
    } else {
        Branch* last = repo->branches;
        while (last->next) last = last->next;
        last->next = branch;
    }

    // Create branch reference file
    char branch_path[256];
    snprintf(branch_path, sizeof(branch_path), ".babygit/refs/heads/%s", branch_name);
    FILE* branch_file = fopen(branch_path, "w");
    if (branch_file) {
        if (branch->head) {
            fprintf(branch_file, "%s", branch->head->hash);
        }
        fclose(branch_file);
    }

    printf("Created branch %s\n", branch_name);
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
    
    // Free all children recursively
    Branch* child = branch->children;
    while (child) {
        Branch* next_child = child->next;
        free_branch(child);
        child = next_child;
    }
    
    // Free the branch itself
    free(branch);
}

void checkout_branch(Repository* repo, const char* branch_name) {
    Branch* branch = find_branch(repo, branch_name);
    if (!branch) {
        printf("Branch %s not found\n", branch_name);
        return;
    }

    repo->current_branch = branch;
    
    // Update HEAD
    FILE* head = fopen(".babygit/HEAD", "w");
    if (head) {
        fprintf(head, "ref: refs/heads/%s\n", branch_name);
        fclose(head);
    }

    printf("Switched to branch %s\n", branch_name);
}
