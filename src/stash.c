#include "../include/stash.h"
#include "../include/commit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stash_changes(Repository* repo, const char* message) {
    if (!repo || !message) return;
    
    // Create a stash commit
    Commit* stash_commit = create_commit(repo, message, "stash");
    if (!stash_commit) return;
    
    // Create stash item
    Stash* stash = malloc(sizeof(Stash));
    if (!stash) return;
    
    strncpy(stash->message, message, 255);
    stash->commit = stash_commit;
    stash->next = NULL;
    
    // Add to stash list
    if (!repo->stashes) {
        repo->stashes = stash;
    } else {
        Stash* last = repo->stashes;
        while (last->next) last = last->next;
        last->next = stash;
    }
    
    printf("Saved working directory to stash: %s\n", message);
}

void apply_stash(Repository* repo, int stash_index) {
    if (!repo) return;
    
    Stash* stash = repo->stashes;
    for (int i = 0; i < stash_index && stash; i++) {
        stash = stash->next;
    }
    
    if (!stash) {
        printf("Stash not found\n");
        return;
    }
    
    printf("Applying stash: %s\n", stash->message);
    // TODO: Implement actual stash application
}

void list_stashes(Repository* repo) {
    if (!repo) return;
    
    Stash* stash = repo->stashes;
    int index = 0;
    
    printf("Stashes:\n");
    while (stash) {
        printf("%d: %s (%s)\n", index++, stash->message, stash->commit->hash);
        stash = stash->next;
    }
}
