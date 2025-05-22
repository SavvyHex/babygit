#include "merge.h"
#include "repository.h"
#include "commit.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_FILES 1024

typedef struct {
    char filename[256];
    char hash[41];
} FileRecord;

typedef struct {
    char filename[256];
    char base_hash[41];
    char our_hash[41];
    char their_hash[41];
} FileEntry;

static char* find_hash_for_file(const char* filename, FileRecord* files, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(files[i].filename, filename) == 0)
            return files[i].hash;
    }
    return NULL;
}

static int filename_exists(char filenames[][256], int count, const char* filename) {
    for (int i = 0; i < count; i++) {
        if (strcmp(filenames[i], filename) == 0)
            return 1;
    }
    return 0;
}


static void collect_unique_filenames(FileRecord* base_files, int base_count,
                                     FileRecord* our_files, int our_count,
                                     FileRecord* their_files, int their_count,
                                     char filenames[][256], int* filename_count) {
    *filename_count = 0;
    for (int i = 0; i < base_count; i++) {
        if (!filename_exists(filenames, *filename_count, base_files[i].filename))
            strcpy(filenames[(*filename_count)++], base_files[i].filename);
    }
    for (int i = 0; i < our_count; i++) {
        if (!filename_exists(filenames, *filename_count, our_files[i].filename))
            strcpy(filenames[(*filename_count)++], our_files[i].filename);
    }
    for (int i = 0; i < their_count; i++) {
        if (!filename_exists(filenames, *filename_count, their_files[i].filename))
            strcpy(filenames[(*filename_count)++], their_files[i].filename);
    }
}

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

// Fill in the list of files and blob hashes from the commit’s tree
int get_commit_files(Commit* commit, FileRecord* files, int* count) {
    char* commit_content = read_object_file(commit->hash);
    if (!commit_content) {
        fprintf(stderr, "Failed to read commit object: %s\n", commit->hash);
        return 0;
    }

    char tree_hash[41];
    if (!extract_tree_hash(commit_content, tree_hash)) {
        fprintf(stderr, "Tree hash not found in commit.\n");
        free(commit_content);
        return 0;
    }
    free(commit_content);

    char* tree_content = read_object_file(tree_hash);
    if (!tree_content) {
        fprintf(stderr, "Failed to read tree object: %s\n", tree_hash);
        return 0;
    }

    *count = 0;
    char* line = strtok(tree_content, "\n");
    while (line != NULL && *count < MAX_FILES) {
        char hash[41], filename[256];
        if (sscanf(line, "%40s %255[^\n]", hash, filename) == 2) {
            strncpy(files[*count].hash, hash, 41);
            strncpy(files[*count].filename, filename, 256);
            (*count)++;
        }
        line = strtok(NULL, "\n");
    }

    free(tree_content);
    return 1;
}

void perform_three_way_merge(Commit* base, Commit* ours, Commit* theirs) {
    FileRecord base_files[MAX_FILES], our_files[MAX_FILES], their_files[MAX_FILES];
    int base_count = 0, our_count = 0, their_count = 0;

    get_commit_files(base, base_files, &base_count);
    get_commit_files(ours, our_files, &our_count);
    get_commit_files(theirs, their_files, &their_count);

    char filenames[MAX_FILES][256];
    int filename_count = 0;

    collect_unique_filenames(base_files, base_count, our_files, our_count, their_files, their_count, filenames, &filename_count);

    printf("Starting three-way merge...\n");

    for (int i = 0; i < filename_count; i++) {
        const char* fname = filenames[i];
        char* base_hash = find_hash_for_file(fname, base_files, base_count);
        char* our_hash = find_hash_for_file(fname, our_files, our_count);
        char* their_hash = find_hash_for_file(fname, their_files, their_count);

        char* base_content = base_hash ? read_object_file(base_hash) : NULL;
        char* our_content = our_hash ? read_object_file(our_hash) : NULL;
        char* their_content = their_hash ? read_object_file(their_hash) : NULL;

        char* merged_content = NULL;
        int conflict = compare_file_versions(fname,
                                             base_content ? base_content : "",
                                             our_content ? our_content : "",
                                             their_content ? their_content : "",
                                             &merged_content);

        FILE* f = fopen(fname, "w");
        if (!f) {
            perror("fopen");
            continue;
        }

        fprintf(f, "%s", merged_content);
        fclose(f);

        if (conflict) {
            printf("Conflict in %s. Marked with conflict markers.\n", fname);
        } else {
            printf("Merged %s successfully.\n", fname);
        }

        free(merged_content);
        if (base_content) free(base_content);
        if (our_content) free(our_content);
        if (their_content) free(their_content);
    }

    printf("Merge completed.\n");
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
    perform_three_way_merge(ancestor, ours, theirs);

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
