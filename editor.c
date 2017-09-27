//
// Created by liu on 17-9-20.
//

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include "editor.h"

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

struct termios origin_termios;
int editor_mode = 0;

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

int editor_mode_read(char *buffer) {
    int fd = STDIN_FILENO;
    int end_read = 0;
    int special_mode = 0;
    char c;
    int length = 0, now = 0;
    buffer[0] = '\0'; // Initial \0
    while (!end_read) {
        long nread = read(fd, &c, 1);
        if (special_mode) {
            switch (c) {
            case SPECIAL_BEGIN:
                if (special_mode == 1) {
                    special_mode = 2;
                    continue;
                }
                special_mode = 0;
                break;
            case ARROW_UP:
            case ARROW_DOWN:
            case ARROW_RIGHT:
            case ARROW_LEFT:
                if (special_mode == 2) {
                    if (c == ARROW_LEFT && now > 0) {
                        now--;
                        printf("\b");
                        fflush(stdout);
                    } else if (c == ARROW_RIGHT && now < length) {
                        printf("%c", buffer[now++]);
                        fflush(stdout);
                    }
                    special_mode = 0;
                    continue;
                }
                special_mode = 0;
                break;
            default:
                special_mode = 0;
                break;
            }
        }
        switch (c) {
        case ESC:
            special_mode = 1;
            break;
        case ENTER:
            buffer[length++] = '\n';
            buffer[length] = '\0';
            end_read = 1;
            break;
        case BACKSPACE:
        case CTRL_H:
            if (now > 0) {
                memmove(buffer + now - 1, buffer + now, length - now + 1);
                printf("\b%s ", buffer + now - 1);
                for (int i = 0; i < length - now + 1; i++) printf("\b");
                now--;
                length--;
            }
            break;
        case CTRL_C:
            printf("^C");
            return -1;
        case CTRL_D:
            if (length == 0) return -2;
            else {
                buffer[now] = '\0';
                for (int i = 0; i < length - now; i++) printf(" ");
                for (int i = 0; i < length - now; i++) printf("\b");
                length = now;
            }
        case TAB:

            break;
        default:
            if (now <= length) {
                memmove(buffer + now + 1, buffer + now, length - now + 1); // \0 is always moved
                buffer[now] = c;
                printf("%s", buffer + now);
                now++;
                length++;
                for (int i = 0; i < length - now; i++) printf("\b");
            } else printf("error\n");
            break;
        }
        fflush(stdout);
    }
    return 0;
}

