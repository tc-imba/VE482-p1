#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include "editor.h"
#include "parser.h"
#include "utils.h"


int main(int argc, char *argv[]) {
    char buffer[MAX_COMMAND_LENGTH + 2] = {};
    char *buffer_start;
    while (true) {
        print_prompt();

        if (isatty(STDIN_FILENO)) {
            enable_editor_mode();
            int rs = editor_mode_read(buffer);
            disable_editor_mode();
            if (rs == -1) continue;
            else if (rs == -2) break;
        } else {
            printf("Text mode!\n");
        }

        input_trim(buffer, &buffer_start);
        if (strcmp(buffer_start, "exit") == 0) {
            break;
        }

        parsed_data_t *data = input_parse(buffer_start);
        if (data->num > 0 && strcmp(data->commands[0].argv[0], "cd") == 0) {
            if (data->commands[0].argc > 1) {
                change_dir(data->commands[0].argv[1]);
            } else {
                change_dir("~");
            }
        } else {
            fork_and_exec(data, 0, NULL);
        }
    }

    input_parse_init();
    return 0;
}