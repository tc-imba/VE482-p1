#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "editor.h"
#include "parser.h"
#include "utils.h"
#include "history.h"

// The input buffer
char buffer[MAX_COMMAND_LENGTH + 2] = {};

// The parsed buffer
char parse_buffer[MAX_COMMAND_LENGTH + 2] = {};

// Current parse state (PARSE_OPTION means complete)
parse_state state = PARSE_OPTION;


void init() {
    parse_buffer[0] = '\0';
}

void save_history() {
    add_history(parse_buffer);
};

void terminate() {
    input_parse_init();
    free_history();
    exit(0);
}

parsed_data_t *parse_from_tty() {
    init();
    parsed_data_t *data = NULL;
    bool flag = true;
    bool complete = true;
    while (flag) {
        if (complete) print_prompt();
        else print_incomplete();
        enable_editor_mode();
        editor_state _editor_state = editor_mode_read(buffer);
        disable_editor_mode();
        switch (_editor_state) {
        case EDITOR_INTERRUPT:
            printf("^C\n");
            flag = false;
            data = NULL;
            break;
        case EDITOR_EXIT:
            if (complete) {
                printf("\n");
                terminate();
            }
            else printf("syntax error: unexpected end of file\n");
            flag = false;
            data = NULL;
            break;
        case EDITOR_ENDL:
            printf("\n");
            input_preprocess(buffer, parse_buffer, state);
            data = input_parse(parse_buffer);
            state = data->state;
            if (data->num < 0) {
                complete = false;
            }
            else {
                flag = false;
            }
            break;
        case EDITOR_ERROR:
            printf("Unknown error!\n");
            flag = false;
            data = NULL;
            break;
        }
    }
    return data;
}

parsed_data_t *parse_from_file() {
    init();
    parsed_data_t *data = NULL;
    return data;
}

int main(int argc, char *argv[]) {
    parsed_data_t *data = NULL;
    while (true) {
        if (isatty(STDIN_FILENO)) {
            // TTY mode
            data = parse_from_tty();
        }
        else {
            // File mode
            printf("File mode!\n");
            // @TODO This part will be complete in the future
            // It is not a requirement of the project
            data = parse_from_file();
            exit(0);
        }

        // Terminate at exit
        if (strcmp(parse_buffer, "exit") == 0) {
            terminate();
        }

        // Process incomplete command
        if (!data || data->num < 0) {
            continue;
        }

        // Process cd
        if (data->num > 0 && strcmp(data->commands[0].argv[0], "cd") == 0) {
            if (data->commands[0].argc > 1) change_dir(data->commands[0].argv[1]);
            else change_dir("~");
            save_history();
            continue;
        }

        // Process normal command
        fork_and_exec(data, 0, NULL);
        if (data->num > 0) {
            save_history();
        }

    }
}