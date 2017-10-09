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
static char buffer[MAX_COMMAND_LENGTH + 2] = {};

// The parsed buffer
static char parse_buffer[MAX_COMMAND_LENGTH + 2] = {};

// Current parse state (PARSE_OPTION means complete)
static parse_state state = PARSE_OPTION;


void init() {
    parse_buffer[0] = '\0';
}

void save_history() {
    add_history(parse_buffer);
};

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
            fprintf(stderr, "^C\n");
            flag = false;
            data = NULL;
            break;
        case EDITOR_EXIT:
            if (complete) {
                fprintf(stderr, "exit\n");
                terminate();
            } else fprintf(stderr, "syntax error: unexpected end of file\n");
            flag = false;
            data = NULL;
            break;
        case EDITOR_ENDL:
            fprintf(stderr, "\n");
            input_preprocess(buffer, parse_buffer, state);
            data = input_parse(parse_buffer);
            state = data->state;
            if (data->state == PARSE_ERROR) {
                flag = false;
            } else if (data->state != PARSE_OPTION) {
                complete = false;
            } else {
                flag = false;
            }
            break;
        case EDITOR_ERROR:
            fprintf(stderr, "Unknown error!\n");
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
        } else {
            // File mode
            fprintf(stderr, "File mode!\n");
            // @TODO This part will be complete in the future
            // It is not a requirement of the project
            data = parse_from_file();
            terminate();
            exit(0);
        }

        // Terminate at exit
        if (strcmp(parse_buffer, "exit") == 0) {
            terminate();
        }

        if (!data) {
            //printf("error: Syntax Error\n");
            continue;
        }

        if (data->state == PARSE_ERROR)
        {
            switch (data->error)
            {
            case ERROR_DUPLICATED_INPUT:
                fprintf(stderr, "error: duplicated input redirection\n");
                break;
            case ERROR_DUPLICATED_OUTPUT:
                fprintf(stderr, "error: duplicated output redirection\n");
                break;
            case ERROR_SYNTAX:
                fprintf(stderr, "syntax error\n");
                break;
            case ERROR_SYNTAX_INPUT:
                fprintf(stderr, "syntax error near unexpected token `<'\n");
                break;
            case ERROR_SYNTAX_OUTPUT:
                fprintf(stderr, "syntax error near unexpected token `>'\n");
                break;
            case ERROR_SYNTAX_PIPE:
                fprintf(stderr, "syntax error near unexpected token `|'\n");
                break;
            case ERROR_MISSING_PROGRAM:
                fprintf(stderr, "error: missing program\n");
                break;
            default:
                fprintf(stderr, "error: unknown error\n");
                break;
            }
            continue;
        }

        // Process incomplete command
        if (data->state != PARSE_OPTION) {
            continue;
        }

        // Process cd
        if (data->num > 0 && strcmp(data->commands[0].argv[0], "cd") == 0) {
            switch (data->commands[0].argc) {
            case 1:
                change_dir("~");
                break;
            case 2:
                change_dir(data->commands[0].argv[1]);
                break;
            default:
                fprintf(stderr, "cd: too many arguments\n");
                break;
            }
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