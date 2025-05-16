#include "../include/branch.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Branch* create_branch(Repository* repo, const char* branch_name) {
    if (!repo || !branch_name) return NULL;
    
    Branch* branch = malloc(sizeof(Branch));
    if (!branch) return NULL;
    
    strncpy(branch->name, branch_name, 255);
    branch->head = repo->current_branch ? repo->current_branch->head : NULL;
    branch->parent = repo->current_branch;
    branch->children = NULL;
    branch->next = NULL;
    
    // Add to branch tree
    if (!repo->branches) {
        repo->branches = branch;
    } else {
        if (repo->current_branch) {
            if (!repo->current_branch->children) {
                repo->current_branch->children = branch;
            } else {
                Branch* last = repo->current_branch->children;
                while (last->next) last = last->next;
                last->next = branch;
            }
        } else {
            Branch* last = repo->branches;
            while (last->next) last = last->next;
            last->next = branch;
        }
    }
    
    // Create branch reference file
    char branch_path[256];
    sprintf(branch_path, ".babygit/refs/heads/%s", branch_name);
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
    
    // Search through all branches (DFS)
    Branch* stack[100];
    int top = -1;
    
    if (repo->branches) {
        stack[++top] = repo->branches;
    }
    
    while (top >= 0) {
        Branch* current = stack[top--];
        
        if (strcmp(current->name, branch_name) == 0) {
            return current;
        }
        
        // Push children to stack
        Branch* child = current->children;
        while (child) {
            stack[++top] = child;
            child = child->next;
        }
    }
    
    return NULL;
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

void free_branch(Branch* branch) {
    if (!branch) return;
    
    // Free children recursively
    free_branch(branch->children);
    
    // Free siblings
    free_branch(branch->next);
    
    free(branch);
}
