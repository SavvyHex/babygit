#include "branch.h"
#include "repository.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Branch* create_branch(Repository* repo, const char* name) {
    // Validate inputs
    if (!repo || !name) return NULL;
    
    // Check if branch already exists
    if (find_branch(repo, name)) {
        return NULL;
    }

    // Allocate and initialize new branch
    Branch* branch = malloc(sizeof(Branch));
    if (!branch) return NULL;
    
    strncpy(branch->name, name, sizeof(branch->name) - 1);
    branch->name[sizeof(branch->name) - 1] = '\0'; // Ensure null-termination
    
    // Set head commit to current branch's head if exists
    branch->head = repo->current_branch ? repo->current_branch->head : NULL;
    
    branch->parent = repo->current_branch;
    branch->children = NULL;
    branch->next = NULL;

    // Add to repository's branch list
    if (!repo->branches) {
        repo->branches = branch;
    } else {
        Branch* current = repo->branches;
        while (current->next) current = current->next;
        current->next = branch;
    }
    
    // Create reference file
    char path[256];
    snprintf(path, sizeof(path), ".babygit/refs/heads/%s", name);
    
    FILE* f = fopen(path, "w");
    if (f) {
        if (branch->head) {
            fprintf(f, "%s", branch->head->hash); // Write commit hash if exists
        }
        fclose(f);
    } else {
        printf("Warning: Could not create branch reference file\n");
        // Don't fail - we still have in-memory representation
    }
    
    printf("Created branch %s\n", name);
    return branch;
}

Branch* find_branch(Repository* repo, const char* branch_name) {
    if (!repo || !branch_name) return NULL;

    for (int i = 0; i < repo->branch_count; i++) {
        if (strcmp(repo->branches[i].name, branch_name) == 0) {
            return &repo->branches[i];  // Return a pointer, not a copy
        }
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

Branch* create_branch_silent(Repository* repo, const char* name) {
    Branch* branch = malloc(sizeof(Branch));
    strncpy(branch->name, name, sizeof(branch->name));
    branch->head = NULL;
    branch->next = repo->branches;
    repo->branches = branch;
    return branch;
}
