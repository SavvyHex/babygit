#include <stdio.h>
#include <string.h>

#include "merge.h"
#include "branch.h"
#include "commit.h"

void merge_branch(Repository *repo, const char *branch_name) {
    if (!repo || !branch_name) return;

    Branch *target = find_branch(repo, branch_name);
    Branch *current = repo->current_branch;

    printf("DEBUG: current branch = %s\n", repo->current_branch ? repo->current_branch->name : "NULL");
    printf("DEBUG: current head = %s\n", repo->current_branch && repo->current_branch->head ? repo->current_branch->head->hash : "NULL");
    printf("DEBUG: merging branch = %s\n", to_merge->name);
    printf("DEBUG: target head = %s\n", to_merge->head ? to_merge->head->hash : "NULL");

    if (!target || !target->head || !current || !current->head) {
        printf("Cannot merge: invalid branch state\n");
        return;
    }

    if (target == current) {
        printf("Cannot merge branch into itself.\n");
        return;
    }

    // Check if already up to date
    if (strcmp(current->head->hash, target->head->parent_hash) == 0) {
        printf("Fast-forward merge\n");
        current->head = target->head;
        return;
    }

    printf("Merging branch %s into %s\n", target->name, current->name);

    // Start building merged file state
    char merged_content[2048] = "";
    int conflicts = 0;

    // Loop through target commit's file list
    FILE *target_file = fopen((".babygit/objects/" + (string)target->head->hash).c_str(), "r");
    if (!target_file) {
        printf("Failed to read target commit.\n");
        return;
    }

    // Skip to "files" section
    char line[1024];
    while (fgets(line, sizeof(line), target_file)) {
        if (strncmp(line, "files", 5) == 0)
            break;
    }

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

    // Create merge commit manually
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

    // Build final commit content
    char full[4096];
    snprintf(full, sizeof(full),
             "parent %s\nparent2 %s\nauthor %s\ntime %ld\nmessage %s\nfiles\n%s",
             merge_commit->parent_hash,
             merge_commit->second_parent,
             merge_commit->author,
             merge_commit->timestamp,
             merge_commit->message,
             merged_content);

    calculate_hash(full, strlen(full), merge_commit->hash);

    // Save commit to disk
    char path[256];
    snprintf(path, sizeof(path), ".babygit/objects/%s", merge_commit->hash);
    FILE *file = fopen(path, "w");
    if (!file) {
        free(merge_commit);
        printf("Could not save merge commit.\n");
        return;
    }

    fprintf(file, "%s", full);
    fclose(file);

    repo->commits = merge_commit;
    current->head = merge_commit;

    printf("Merge successful: %s\n", merge_commit->hash);
}
