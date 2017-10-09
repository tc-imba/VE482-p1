//
// Created by liu on 17-9-20.
//

#ifndef P1_EDITOR_H
#define P1_EDITOR_H

typedef enum {
    EDITOR_READING,
    EDITOR_ENDL,
    EDITOR_INTERRUPT,
    EDITOR_EXIT,
    EDITOR_ERROR
} editor_state;

int enable_editor_mode();

int disable_editor_mode();

editor_state editor_mode_read(char *buffer);

#endif //P1_EDITOR_H
