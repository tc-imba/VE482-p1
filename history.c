//
// Created by 38569 on 2017/9/27.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "history.h"

#define MAX_HISTORY 3

char *history[MAX_HISTORY];
char history_now[1026] = {};
int history_num = 0;
int history_entry_pos = 0;
int history_offset = 0;

void reset_history() {
    history_offset = 0;
}

void add_history(const char *buffer) {
    reset_history();
    if (history_num > 0) {
        int previous = (history_entry_pos - 1 + MAX_HISTORY) % MAX_HISTORY;
        if (strcmp(history[previous], buffer) == 0) return;
    }
    if (history_num < MAX_HISTORY) {
        history[history_entry_pos] = malloc(sizeof(char) * (strlen(buffer) + 1));
        history_num++;
    } else {
        history[history_entry_pos] = realloc(history[history_entry_pos],
                                             sizeof(char) * (strlen(buffer) + 1));
    }
    strcpy(history[history_entry_pos], buffer);
    history_entry_pos = (history_entry_pos + 1) % MAX_HISTORY;
}

void set_history(const char *buffer) {
    if (history_offset > 0) {
        int index = (history_entry_pos - history_offset + MAX_HISTORY) % (MAX_HISTORY);
        history[index] = realloc(history[index], sizeof(char) * (strlen(buffer) + 1));
        strcpy(history[index], buffer);
    } else {
        strcpy(history_now, buffer);
    }
}

const char *get_history_previous() {
    if (history_offset >= history_num) return NULL;
    history_offset++;
    int index = (history_entry_pos - history_offset + MAX_HISTORY) % (MAX_HISTORY);
    return history[index];
}

const char *get_history_next() {
    if (history_offset <= 0) return NULL;
    history_offset--;
    if (history_offset == 0) return history_now;
    int index = (history_entry_pos - history_offset + MAX_HISTORY) % (MAX_HISTORY);
    return history[index];
}

void free_history() {
    for (int i = 0; i < history_num; i++) {
        free(history[i]);
    }
    history_num = 0;
    history_entry_pos = 0;
    history_offset = 0;
}
