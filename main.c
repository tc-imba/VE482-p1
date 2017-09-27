#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include "editor.h"
#include "parser.h"
#include "utils.h"
#include "history.h"

int main(int argc, char *argv[]) {
    char buffer[MAX_COMMAND_LENGTH + 2] = {};
    char parse_buffer[MAX_COMMAND_LENGTH + 2] = {};
    parse_state state = PARSE_OPTION;
//    char *buffer_start;
    bool complete = true;
    while (true) {
        if (complete) {
            print_prompt();
        } else {
            print_incomplete();
        }

        if (isatty(STDIN_FILENO)) {
            enable_editor_mode();
            int rs = editor_mode_read(buffer);
            disable_editor_mode();
            if (rs == -1) {
                // Interrupt
                printf("\n");
                complete = true;
                parse_buffer[0] = '\0';
                continue;
            } else if (rs == -2) {
                // EOF
                if (complete) {
                    printf("\n");
                    break;
                } else {
                    printf("syntax error: unexpected end of file\n");
                    complete = true;
                    parse_buffer[0] = '\0';
                    continue;
                }
            } else {
                printf("\n");
            }
        } else {
            printf("Text mode!\n");
        }

        input_preprocess(buffer, parse_buffer, state);

        if (strcmp(parse_buffer, "exit") == 0) {
            break;
        }

        parsed_data_t *data = input_parse(parse_buffer);
        state = data->state;

        if (data->num < 0) {
            complete = false;
        } else if (data->num > 0 && strcmp(data->commands[0].argv[0], "cd") == 0) {
            if (data->commands[0].argc > 1) {
                change_dir(data->commands[0].argv[1]);
            } else {
                change_dir("~");
            }
            complete = true;
            add_history(parse_buffer);
            parse_buffer[0] = '\0';
        } else {
            fork_and_exec(data, 0, NULL);
            complete = true;
            if (data->num > 0) add_history(parse_buffer);
            parse_buffer[0] = '\0';
        }
    }

    input_parse_init();
    free_history();
    return 0;
}