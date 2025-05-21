#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merge.h"
#include "branch.h"
#include "commit.h"
#include "utils.h"  // for calculate_hash()

void merge_branch(Repository *repo, const char *branch_name) {
    if (!repo || !branch_name) return;

    Branch *target = find_branch(repo, branch_name);
    Branch *current = repo->current_branch;

    printf("DEBUG: current branch = %s\n", current ? current->name : "NULL");
    printf("DEBUG: current head = %s\n", current && current->head ? current->head->hash : "NULL");

    if (!target) {
        printf("Branch %s not found\n", branch_name);
        return;
    }

    printf("DEBUG: merging branch = %s\n", target->name);
    printf("DEBUG: target head = %s\n", target->head ? target->head->hash : "NULL");

    if (!target->head || !current || !current->head) {
        printf("Cannot merge: invalid branch state\n");
        return;
    }

    if (target == current) {
        printf("Cannot merge branch into itself.\n");
        return;
    }

    // Check for fast-forward merge
    if (strcmp(current->head->hash, target->head->parent_hash) == 0) {
        printf("Fast-forward merge\n");
        current->head = target->head;
        return;
    }

    printf("Merging branch %s into %s\n", target->name, current->name);

    // Prepare merged content buffer
    char merged_content[2048] = "";

    // Build path to target commit file
    char filepath[256];
    snprintf(filepath, sizeof(filepath), ".babygit/objects/%s", target->head->hash);

    FILE *target_file = fopen(filepath, "r");
    if (!target_file) {
        printf("Failed to read target commit.\n");
        return;
    }

    // Skip lines until "files" section found
    char line[1024];
    while (fgets(line, sizeof(line), target_file)) {
        if (strncmp(line, "files", 5) == 0)
            break;
    }

    // Read file entries from target commit and append to merged_content
    while (fgets(line, sizeof(line), target_file)) {
        char hash[41], filename[256];
        if (sscanf(line, "%40s %255s", hash, filename) == 2) {
            strcat(merged_content, hash);
            strcat(merged_content, " ");
            strcat(merged_content, filename);
            strcat(merged_content, "\n");
        }
    }
    fclose(target_file);

    // Create new merge commit
    Commit *merge_commit = malloc(sizeof(Commit));
    if (!merge_commit) {
        printf("Out of memory.\n");
        return;
    }

    snprintf(merge_commit->author, sizeof(merge_commit->author), "merge-tool");
    snprintf(merge_commit->message, sizeof(merge_commit->message), "Merged branch %s", target->name);
    merge_commit->timestamp = time(NULL);

    strncpy(merge_commit->parent_hash, current->head->hash, sizeof(merge_commit->parent_hash));
    strncpy(merge_commit->second_parent, target->head->hash, sizeof(merge_commit->second_parent));
    merge_commit->parent = current->head;
    merge_commit->next = repo->commits;

    // Compose full commit content string
    char full[4096];
    snprintf(full, sizeof(full),
             "parent %s\nparent2 %s\nauthor %s\ntime %ld\nmessage %s\nfiles\n%s",
             merge_commit->parent_hash,
             merge_commit->second_parent,
             merge_commit->author,
             (long)merge_commit->timestamp,
             merge_commit->message,
             merged_content);

    // Calculate hash for the new commit
    calculate_hash(full, strlen(full), merge_commit->hash);

    // Save the new commit object to disk
    snprintf(filepath, sizeof(filepath), ".babygit/objects/%s", merge_commit->hash);
    FILE *file = fopen(filepath, "w");
    if (!file) {
        free(merge_commit);
        printf("Could not save merge commit.\n");
        return;
    }

    fprintf(file, "%s", full);
    fclose(file);

    // Update repo state
    repo->commits = merge_commit;
    current->head = merge_commit;

    printf("Merge successful: %s\n", merge_commit->hash);
}
