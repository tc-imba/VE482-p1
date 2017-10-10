//
// Created by liu on 17-9-20.
//

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "history.h"
#include "utils.h"

enum KEY_ACTION {
    KEY_NULL = 0,        /* NULL */
    CTRL_A = 1,         /* Ctrl+a */
    CTRL_B = 2,         /* Ctrl-b */
    CTRL_C = 3,         /* Ctrl-c */
    CTRL_D = 4,         /* Ctrl-d */
    CTRL_E = 5,         /* Ctrl-e */
    CTRL_F = 6,         /* Ctrl-f */
    CTRL_H = 8,         /* Ctrl-h */
    TAB = 9,            /* Tab */
    CTRL_K = 11,        /* Ctrl+k */
    CTRL_L = 12,        /* Ctrl+l */
    ENTER = 13,         /* Enter */
    CTRL_N = 14,        /* Ctrl-n */
    CTRL_P = 16,        /* Ctrl-p */
    CTRL_T = 20,        /* Ctrl-t */
    CTRL_U = 21,        /* Ctrl+u */
    CTRL_W = 23,        /* Ctrl+w */
    ESC = 27,           /* Escape */
    BACKSPACE = 127    /* Backspace */
};

enum KEY_SPECIAL {
    SPECIAL_BEGIN = 91,
    ARROW_UP = 65,
    ARROW_DOWN = 66,
    ARROW_RIGHT = 67,
    ARROW_LEFT = 68
};

static struct termios origin_termios;
static int editor_mode = 0;
static int editor_line_length[MAX_COMMAND_LINES] = {0};
static int editor_line_now = 0;
static int editor_line_max = 0;
static int editor_offset_now = 0;
#define editor_offset_max editor_line_length[editor_line_now]
static int editor_total_now = 0;
static int editor_total_max = 0;
static int editor_special_mode = 0;

int enable_editor_mode() {
    if (editor_mode) return 0;
    int fd = STDIN_FILENO;
    int rs = tcgetattr(fd, &origin_termios);
    if (rs == -1) return -1;
    fflush(stdout);
    struct termios editor_termios = origin_termios;
    editor_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    editor_termios.c_oflag &= ~(OPOST);
    editor_termios.c_cflag |= (CS8);
    editor_termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    editor_termios.c_cc[VMIN] = 1;
    editor_termios.c_cc[VTIME] = 0;
    rs = tcsetattr(fd, TCSAFLUSH, &editor_termios);
    if (rs == -1) return -1;
    editor_mode = 1;
    return 0;
}

int disable_editor_mode() {
    if (!editor_mode) return 0;
    int fd = STDIN_FILENO;
    int rs = tcsetattr(fd, TCSAFLUSH, &origin_termios);
    if (rs == -1) return -1;
    editor_mode = 0;
    return 0;
}

void read_from_history(char *buffer, int previous, int *now, int *length, int *line) {
    set_history(buffer);
    const char *temp;
    if (previous) temp = get_history_previous();
    else temp = get_history_next();
    if (temp) {
        printf("\r\33[K");
        strcpy(buffer, temp);
        print_prompt();
        *now = *length = strlen(buffer);
        while (*temp) {
            if (*temp == '\n') {
                printf("\n\r");
                print_incomplete();
                (*line)++;
            } else printf("%c", *temp);
            temp++;
        }
        fflush(stdout);
    }
}

void editor_mode_init() {
    for (int i = 0; i <= editor_line_max; i++) {
        editor_line_length[i] = 0;
    }
    editor_line_max = 0;
    editor_line_now = 0;
    editor_offset_now = 0;
    editor_total_now = 0;
    editor_total_max = 0;
    editor_special_mode = 0;
}

// Deal with special characters
// @TODO add support for history
int editor_mode_special(char *buffer, char c) {
    switch (c) {
    case SPECIAL_BEGIN:
        if (editor_special_mode == 1) {
            editor_special_mode = 2;
            return 1;
        }
        editor_special_mode = 0;
        break;
    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_RIGHT:
    case ARROW_LEFT:
        if (editor_special_mode == 2) {
            if (c == ARROW_LEFT && editor_total_now > 0) {
                if (editor_offset_now == 0) {
                    // line begin
                    printf("not implemented\n");
                } else {
                    // normal
                    printf("\b");
                    editor_offset_now--;
                }
                editor_total_now--;
            } else if (c == ARROW_RIGHT && editor_total_now < editor_total_max) {
                if (editor_line_now < editor_line_max && editor_offset_now == editor_line_length[editor_line_now]) {
                    // line end
                    editor_total_now++;
                } else {
                    // normal
                    printf("%c", buffer[editor_total_now++]);
                    editor_offset_now++;
                }
            } else if (c == ARROW_UP) {
                //read_from_history(buffer, 1, &now, &length, &line);
            } else {
                //read_from_history(buffer, 0, &now, &length, &line);
            }
            editor_special_mode = 0;
            return 1;
        }
        editor_special_mode = 0;
        break;
    default:
        editor_special_mode = 0;
        break;
    }
    return 0;
}

editor_state editor_mode_normal(char *buffer, char c) {
    switch (c) {
    case ESC:
        editor_special_mode = 1;
        break;
    case ENTER:
        buffer[editor_total_max++] = '\n';
        buffer[editor_total_max] = '\0';
        return EDITOR_ENDL;
    case BACKSPACE:
    case CTRL_H:
        if (editor_total_now > 0) {
            memmove(buffer + editor_total_now - 1, buffer + editor_total_now, editor_total_max - editor_total_now + 1);
            if (editor_offset_now == 0) {
                // Delete a line buffer[editor_total_now - 1] == '\n'
                printf("not implemented\n");
                //printf("\33[A");

            } else {
                // Delete a char
                editor_total_now--; // total pos
                editor_total_max--; // total length
                editor_offset_now--; // current pos on this line
                editor_offset_max--; // max chars on this line

                printf("\b\33[K"); // clear the line
                char *temp = buffer + editor_total_now;
                int length = editor_offset_max - editor_offset_now + 1;
                for (int i = 0; i < length; i++) {
                    printf("%c", *temp++);
                }
                for (int i = 0; i < length - 1; i++) {
                    printf("\b");
                }
            }
            //now--;
            //length--;
        }
        break;
    case CTRL_C:
        return EDITOR_INTERRUPT;
    case CTRL_D:
        if (editor_total_max == 0) return EDITOR_EXIT;
        else {
            //buffer[now] = '\0';
            //for (int i = 0; i < length - now; i++) printf(" ");
            //for (int i = 0; i < length - now; i++) printf("\b");
            //length = now;
        }
        break;
    case TAB:

        break;
    default:
        if (editor_total_now <= editor_total_max) {
            memmove(buffer + editor_total_now + 1, buffer + editor_total_now,
                    editor_total_max - editor_total_now + 1); // \0 is always moved
            buffer[editor_total_now] = c;
            printf("\33[K"); // clear the line

            char *temp = buffer + editor_total_now;
            int length = editor_offset_max - editor_offset_now + 1;
            for (int i = 0; i < length; i++) {
                printf("%c", *temp++);
            }
            for (int i = 0; i < length - 1; i++) {
                printf("\b");
            }

            editor_total_now++; // total pos
            editor_total_max++; // total length
            editor_offset_now++; // current pos on this line
            editor_offset_max++; // max chars on this line

        } else printf("error\n");
        break;
    }
    return EDITOR_READING;
}

editor_state editor_mode_read(char *buffer) {
    int fd = STDIN_FILENO;
    char c;

    editor_mode_init();

    buffer[0] = '\0'; // Initial \0
    reset_history();
    while (1) {
        long nread = read(fd, &c, 1); // Read a char from stdin
        if (nread < 0) {
            return EDITOR_ERROR;
        }
        if (editor_special_mode) {
            int result = editor_mode_special(buffer, c);
            if (result) {
                fflush(stdout);
                continue;
            }
        }
        editor_state result = editor_mode_normal(buffer, c);
        fflush(stdout);
        if (result != EDITOR_READING)
            return result;
    }
}

