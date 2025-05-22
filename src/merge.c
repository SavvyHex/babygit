#include "merge.h"
#include "repository.h"
#include "commit.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// File comparison structure
typedef struct {
    char filename[256];
    char base_hash[41];
    char our_hash[41];
    char their_hash[41];
    int conflict; // 0 = no conflict, 1 = conflict
} FileMergeStatus;

// Find the common ancestor of two commits
static Commit* find_common_ancestor(Commit* a, Commit* b) {
    // Simple implementation - for real Git you'd need a more sophisticated algorithm
    while (a) {
        Commit* temp = b;
        while (temp) {
            if (strcmp(a->hash, temp->hash) == 0) {
                return a;
            }
            temp = temp->parent;
        }
        a = a->parent;
    }
    return NULL;
}

static int compare_file_versions(const char* filename, 
                               const char* base_content,
                               const char* our_content, 
                               const char* their_content,
                               char** merged_content) {
    // Case 1: All versions identical
    if (our_content && their_content && base_content &&
        strcmp(our_content, their_content) == 0 &&
        strcmp(our_content, base_content) == 0) {
        *merged_content = strdup(our_content);
        return 0;
    }

    // Case 2: Theirs changed, ours didn't (take theirs)
    if (base_content && our_content && their_content &&
        strcmp(base_content, our_content) == 0 &&
        strcmp(base_content, their_content) != 0) {
        *merged_content = strdup(their_content);
        return 0;
    }

    // Case 3: Ours changed, theirs didn't (take ours)
    if (base_content && our_content && their_content &&
        strcmp(base_content, their_content) == 0 &&
        strcmp(base_content, our_content) != 0) {
        *merged_content = strdup(our_content);
        return 0;
    }

    // Case 4: Both changed differently - conflict
    if (our_content && their_content &&
        strcmp(our_content, their_content) != 0) {
        // Generate conflict marker
        size_t len = strlen(our_content) + strlen(their_content) + 100;
        *merged_content = malloc(len);
        snprintf(*merged_content, len,
                "<<<<<<< ours\n%s=======\n%s>>>>>>> theirs\n",
                our_content, their_content);
        return 1; // Conflict
    }

    // Case 5: File created in one branch
    if (!base_content) {
        if (our_content && !their_content) {
            *merged_content = strdup(our_content);
            return 0;
        }
        if (!our_content && their_content) {
            *merged_content = strdup(their_content);
            return 0;
        }
        if (our_content && their_content) {
            // Both created differently - conflict
            size_t len = strlen(our_content) + strlen(their_content) + 100;
            *merged_content = malloc(len);
            snprintf(*merged_content, len,
                    "<<<<<<< ours\n%s=======\n%s>>>>>>> theirs\n",
                    our_content, their_content);
            return 1;
        }
    }

    // Case 6: File deleted in one branch
    if (base_content) {
        if (!our_content && their_content) {
            // We deleted, they modified - conflict
            size_t len = strlen(their_content) + 100;
            *merged_content = malloc(len);
            snprintf(*merged_content, len,
                    "<<<<<<< deleted\n=======\n%s>>>>>>> theirs\n",
                    their_content);
            return 1;
        }
        if (our_content && !their_content) {
            // They deleted, we modified - conflict
            size_t len = strlen(our_content) + 100;
            *merged_content = malloc(len);
            snprintf(*merged_content, len,
                    "<<<<<<< ours\n%s=======\n>>>>>>> deleted\n",
                    our_content);
            return 1;
        }
    }

    *merged_content = NULL;
    return 0;
}

static int perform_three_way_merge(Repository* repo, 
                                 Commit* base_commit,
                                 Commit* our_commit, 
                                 Commit* their_commit) {
    FileMergeStatus files[100]; // Array to track files
    int file_count = 0;
    int has_conflicts = 0;

    // 1. Collect all unique files from all three commits
    // (Implementation would need to walk commit trees here)
    
    // 2. For each file, compare all three versions
    for (int i = 0; i < file_count; i++) {
        char* base_content = NULL; // Would load from base commit
        char* our_content = NULL;  // Would load from our commit
        char* their_content = NULL; // Would load from their commit
        char* merged_content = NULL;

        // Get file contents from each version
        // (Implementation would need to load file contents from objects)

        int conflict = compare_file_versions(files[i].filename,
                                           base_content,
                                           our_content,
                                           their_content,
                                           &merged_content);

        if (conflict) {
            printf("Conflict in %s\n", files[i].filename);
            files[i].conflict = 1;
            has_conflicts = 1;
            
            // Write conflict markers to working directory
            FILE* f = fopen(files[i].filename, "w");
            if (f) {
                fwrite(merged_content, 1, strlen(merged_content), f);
                fclose(f);
            }
        } else if (merged_content) {
            // Write merged version to working directory
            FILE* f = fopen(files[i].filename, "w");
            if (f) {
                fwrite(merged_content, 1, strlen(merged_content), f);
                fclose(f);
            }
        }

        free(base_content);
        free(our_content);
        free(their_content);
        free(merged_content);
    }

    return has_conflicts;
}

Commit* create_merge_commit(Repository* repo, const char* message, const char* author, 
                          Commit* parent1, Commit* parent2) {
    Commit* merge_commit = malloc(sizeof(Commit));
    strncpy(merge_commit->message, message, sizeof(merge_commit->message)-1);
    strncpy(merge_commit->author, author, sizeof(merge_commit->author)-1);
    merge_commit->timestamp = time(NULL);
    merge_commit->parent = parent1;
    
    // Store second parent hash
    if (parent2) {
        strncpy(merge_commit->parent_hash, parent1->hash, sizeof(merge_commit->parent_hash)-1);
        strncpy(merge_commit->second_parent, parent2->hash, sizeof(merge_commit->second_parent)-1);
    }
    
    // Generate commit hash
    char commit_content[2048];
    snprintf(commit_content, sizeof(commit_content), 
             "parent %s\nparent %s\nauthor %s\ntime %ld\nmessage %s\n", 
             parent1->hash, parent2 ? parent2->hash : "", 
             author, merge_commit->timestamp, message);
    calculate_hash(commit_content, strlen(commit_content), merge_commit->hash);
    
    // Store commit object
    char path[256];
    snprintf(path, sizeof(path), ".babygit/objects/%s", merge_commit->hash);
    FILE* f = fopen(path, "w");
    if (f) {
        fprintf(f, "%s", commit_content);
        fclose(f);
    }
    
    return merge_commit;
}

int merge_branches(Repository* repo, const char* branch_name) {
    if (!repo || !branch_name) return -1;
    
    // Get current branch and target branch
    Branch* current_branch = repo->current_branch;
    Branch* target_branch = find_branch(repo, branch_name);
    
    if (!current_branch || !target_branch) {
        printf("Error: Branches not found\n");
        return -1;
    }
    
    // Get the commits
    Commit* ours = current_branch->head;
    Commit* theirs = target_branch->head;
    
    if (!ours || !theirs) {
        printf("Error: Branches have no commits\n");
        return -1;
    }
    
    // Find common ancestor
    Commit* ancestor = find_common_ancestor(ours, theirs);
    if (!ancestor) {
        printf("Error: No common ancestor found\n");
        return -1;
    }
    
    // Perform the merge
    if (perform_three_way_merge(repo, ancestor, ours, theirs) != 0) {
        printf("Merge conflicts detected!\n");
        return 1; // Return conflict status
    }
    
    // Create merge commit
    char message[256];
    snprintf(message, sizeof(message), "Merge branch '%s'", branch_name);
    Commit* merge_commit = create_merge_commit(repo, message, "merger", ours, theirs);
    
    // Update current branch
    current_branch->head = merge_commit;
    
    // Update repository state
    char branch_path[256];
    snprintf(branch_path, sizeof(branch_path), ".babygit/refs/heads/%s", current_branch->name);
    FILE* f = fopen(branch_path, "w");
    if (f) {
        fprintf(f, "%s", merge_commit->hash);
        fclose(f);
    }
    
    printf("Successfully merged branch '%s'\n", branch_name);
    return 0;
}
