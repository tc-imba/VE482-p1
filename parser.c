//
// Created by liu on 17-9-26.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "parser.h"

void input_trim(char *buffer, char **start) {
    *start = buffer;
    while (**start == ' ') (*start)++;
    char *end = buffer + strlen(buffer) - 2;
    while (*end == ' ')end--;
    *(end + 1) = '\0';
}

void add_argv(command_t *command, const char *str, const size_t length) {
    if (command->argc == 0) {
        command->argv = malloc(sizeof(char *) * 2);
    } else {
        command->argv = realloc(command->argv, sizeof(char *) * (command->argc + 2));
    }
    command->argv[command->argc] = malloc(sizeof(char) * (length + 1));
    strncpy(command->argv[command->argc], str, length);
    command->argv[command->argc][length] = '\0';
    command->argc++;
    command->argv[command->argc] = NULL;
}

void command_init(command_t *command) {
    command->argc = 0;
    command->argv = NULL;
    command->input = NULL;
    command->output = NULL;
    command->input_state = IO_STD;
    command->output_state = IO_STD;
}

void command_clear(command_t *command) {
    if (command->argv) {
        for (int i = 0; i <= command->argc; i++) {
            if (command->argv[i]) free(command->argv[i]);
        }
        free(command->argv);
    }
    if (command->input) free(command->input);
    if (command->output) free(command->output);
    command->input_state = IO_STD;
    command->output_state = IO_STD;
}

parsed_data_t *input_parse_init() {
    static parsed_data_t data = {0, NULL};
    if (data.commands != NULL) {
        for (int i = 0; i < data.num; i++) {
            command_clear(&data.commands[i]);
        }
        free(data.commands);
        data.commands = NULL;
    }
    data.num = 0;
    return &data;
}

char temp_buffer[MAX_COMMAND_LENGTH + 1] = {};

parsed_data_t *input_parse(char *buffer) {
    parse_state state = PARSE_COMMAND;

    parsed_data_t *data = input_parse_init();
    data->num = 0;

    command_t *new_command;
    char *start = buffer;
    while (*start != '\0') {
        bool skip = false;

        while (*start == ' ') start++;
        if (*start == '\0') break;

        string_state s_state = STRING_NORMAL;
        while (true) {
            if (s_state==' ')
        }

        char *pos = start;
        size_t length = 0;


        while (*start != ' ' && *start != '\0') {
            if (*start == '\"' || *start == '\'') {
                char *pos = strchr(start + 1, *pos);
                if (pos == NULL) {
                    // Not completed
                    state = PARSE_QUOTE;
                    break;
                }
                strncpy(temp_buffer + length, start + 1, pos - start - 1);
                length += pos - start - 1;
                start = pos + 1;
            } else {
                char *pos = strchr(start, ' ');
                if (pos == NULL) {
                    pos = start + strlen(start);

                    length += strlen(start);
                } else {

                }
                length = pos - start;
            }

        }

/*        if (*start == '\"') {
            pos = strchr(start + 1, '\"');
            if (pos == NULL) {
                // Not completed
                state = PARSE_QUOTE_DOUBLE;
                break;
            }
            start++;
            pos--;
        } else if (*start == '\'') {
            pos = strchr(start + 1, '\'');
            if (pos == NULL) {
                // Not completed
                state = PARSE_QUOTE_SINGLE;
                break;
            }

            start++;
            pos++;
        } else {
            pos = strchr(start, ' ');
            if (pos == NULL) pos = start + strlen(start);
            length = pos - start;
        }*/


        if (length == 1) {
            if (*start == '>') {
                state = PARSE_OUTPUT;
                new_command->output_state = IO_FILE;
                skip = true;

            } else if (*start == '<') {
                state = PARSE_INPUT;
                new_command->input_state = IO_FILE;
                skip = true;
            } else if (*start == '|') {
                state = PARSE_COMMAND;
                skip = true;
            }
        } else if (length == 2 && *start == '>' && *(start + 1) == '>') {
            state = PARSE_OUTPUT;
            new_command->output_state = IO_FILE_APPEND;
            skip = true;
        }

        if (!skip) {
            switch (state) {
            case PARSE_COMMAND:
                if (data->num == 0) {
                    data->commands = malloc(sizeof(command_t));
                } else {
                    data->commands = realloc(data->commands, sizeof(command_t) * (data->num + 1));
                }
                new_command = &data->commands[data->num++];
                command_init(new_command);
                add_argv(new_command, start, length);
                state = PARSE_OPTION;
//            printf("Find command %s\n", new_command->argv[0]);
                break;
            case PARSE_OPTION:
                add_argv(new_command, start, length);
                break;
//            printf("Find option %s\n", new_command->argv[new_command->argc - 1]);
            case PARSE_INPUT:
                if (new_command->input) free(new_command->input);
                new_command->input = malloc(sizeof(char) * (length + 1));
                strncpy(new_command->input, start, length);
                new_command->input[length] = '\0';
                state = PARSE_OPTION;
                break;
            case PARSE_OUTPUT:
                if (new_command->output) free(new_command->output);
                new_command->output = malloc(sizeof(char) * (length + 1));
                strncpy(new_command->output, start, length);
                new_command->output[length] = '\0';
                state = PARSE_OPTION;
                break;
            }
        }

        start = pos;
        while (*start == ' ')start++;
    }

    if (state != PARSE_OPTION) {
        // Not completed

    }


    return data;
}
