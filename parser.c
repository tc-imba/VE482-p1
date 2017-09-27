//
// Created by liu on 17-9-26.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "parser.h"

void input_preprocess(char *buffer, char *parse_buffer) {
    size_t len1 = strlen(parse_buffer);
    while (buffer[0] == ' ') buffer++;
    size_t len2 = strlen(buffer);
    while (len2 > 0 && (buffer[len2 - 1] == ' ' || buffer[len2 - 1] == '\n')) len2--;
    strncpy(parse_buffer + len1, buffer, len2);
    parse_buffer[len1 + len2] = '\n';
    parse_buffer[len1 + len2 + 1] = '\0';
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

        string_state s_state = STRING_NORMAL;
        size_t length = 0;

        while (*start) {
            if (s_state == STRING_NORMAL) {
                if (*start == ' ' || *start == '\n') {
                    start++;
                    break;
                } else if (*start == '\"') {
                    s_state = STRING_QUOTE_DOUBLE;
                    start++;
                } else if (*start == '\'') {
                    s_state = STRING_QUOTE_SINGLE;
                    start++;
                } else {
                    temp_buffer[length++] = *(start++);
                }
            } else {
                if (*start == '\"' && s_state == STRING_QUOTE_DOUBLE) {
                    s_state = STRING_NORMAL;
                    start++;
                } else if (*start == '\'' && s_state == STRING_QUOTE_SINGLE) {
                    s_state = STRING_NORMAL;
                    start++;
                } else {
                    temp_buffer[length++] = *(start++);
                }
            }
        }
        if (length == 0) continue;
        if (s_state != STRING_NORMAL) {
            state = PARSE_QUOTE;
            break;
        }

        temp_buffer[length] = '\0';
//        printf("%s\n", temp_buffer);

        if (length == 1) {
            if (temp_buffer[0] == '>') {
                state = PARSE_OUTPUT;
                new_command->output_state = IO_FILE;
                skip = true;

            } else if (temp_buffer[0] == '<') {
                state = PARSE_INPUT;
                new_command->input_state = IO_FILE;
                skip = true;
            } else if (temp_buffer[0] == '|') {
                state = PARSE_COMMAND;
                skip = true;
            }
        } else if (length == 2 && temp_buffer[0] == '>' && temp_buffer[1] == '>') {
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
                add_argv(new_command, temp_buffer, length);
                state = PARSE_OPTION;
//            printf("Find command %s\n", new_command->argv[0]);
                break;
            case PARSE_OPTION:
                add_argv(new_command, temp_buffer, length);
                break;
//            printf("Find option %s\n", new_command->argv[new_command->argc - 1]);
            case PARSE_INPUT:
                if (new_command->input) free(new_command->input);
                new_command->input = malloc(sizeof(char) * (length + 1));
                strcpy(new_command->input, temp_buffer);
//                new_command->input[length] = '\0';
                state = PARSE_OPTION;
                break;
            case PARSE_OUTPUT:
                if (new_command->output) free(new_command->output);
                new_command->output = malloc(sizeof(char) * (length + 1));
                strcpy(new_command->output, temp_buffer);
//                new_command->output[length] = '\0';
                state = PARSE_OPTION;
                break;
            }
        }
        if (!*start) break;
        while (*start == ' ' || *start == '\n') start++;
    }

    if (state != PARSE_OPTION) {
        // Not completed
        data->num = -1;
    }


    return data;
}
