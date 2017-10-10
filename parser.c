//
// Created by liu on 17-9-26.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "parser.h"

static char temp_buffer[MAX_COMMAND_LENGTH + 2] = {0};

// Init a command
void command_init(command_t *command) {
    command->argc = 0;
    command->argv = NULL;
    command->input = NULL;
    command->output = NULL;
    command->input_state = IO_STD;
    command->output_state = IO_STD;
}

// Clear a command
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

// Adds argv to a command
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

// Init the parse, must be called before exit to avoid memory leak
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
    data.error = ERROR_UNKNOWN;
    return &data;
}


// merge the new line into previous buffer
// @TODO Check the length
void input_preprocess(char *buffer, char *parse_buffer, parse_state state) {
    size_t len1 = strlen(parse_buffer);
    if (len1 > 0) {
        if (state == PARSE_QUOTE) parse_buffer[len1] = '\n';
        else parse_buffer[len1] = ' ';
        len1++;
    }
    while (buffer[0] == ' ') buffer++;
    size_t len2 = strlen(buffer);
    while (len2 > 0 && (buffer[len2 - 1] == ' ' || buffer[len2 - 1] == '\n')) len2--;
    strncpy(parse_buffer + len1, buffer, len2);
    parse_buffer[len1 + len2] = '\0';
}


parsed_data_t *input_parse(char *buffer) {
    parsed_data_t *data = input_parse_init();
    data->num = 0;
    data->commands = malloc(sizeof(command_t));

    // Init the first command
    command_t *new_command = &data->commands[data->num++];
    command_init(new_command);
    data->state = PARSE_COMMAND;

    char *now = buffer;
    while (*now != '\0') {
        bool skip = false;

        string_state s_state = STRING_NORMAL;
        size_t length = 0;

        while (*now) {
            if (s_state == STRING_REDIRECTION) {
                if (*now == '>' && temp_buffer[0] == '>') {
                    temp_buffer[length++] = *(now++);
                }
                s_state = STRING_NORMAL;

                /*if (*now == '<' || *now == '>' || *now == '|') {

                } else {
                    s_state = STRING_NORMAL;
                }*/
                break;
            } else if (s_state == STRING_NORMAL) {
                if (*now == '<' || *now == '>' || *now == '|') {
                    if (length == 0) {
                        temp_buffer[length++] = *(now++);
                        if (*now) s_state = STRING_REDIRECTION;
                    } else break;
                } else if (*now == ' ' || *now == '\n') {
                    now++;
                    break;
                } else if (*now == '\"') {
                    s_state = STRING_QUOTE_DOUBLE;
                    now++;
                } else if (*now == '\'') {
                    s_state = STRING_QUOTE_SINGLE;
                    now++;
                } else {
                    temp_buffer[length++] = *(now++);
                }
            } else {
                if (*now == '\"' && s_state == STRING_QUOTE_DOUBLE) {
                    s_state = STRING_NORMAL;
                    now++;
                } else if (*now == '\'' && s_state == STRING_QUOTE_SINGLE) {
                    s_state = STRING_NORMAL;
                    now++;
                } else {
                    temp_buffer[length++] = *(now++);
                }
            }
        }
        temp_buffer[length] = '\0';
        //printf("%s\n", temp_buffer);

        if (s_state == STRING_REDIRECTION) {
            // Syntax Error
            data->state = PARSE_ERROR;
            data->error = ERROR_SYNTAX;
            break;
        }
        if (s_state != STRING_NORMAL) {
            data->state = PARSE_QUOTE;
            break;
        }
        if (length == 0) continue;

//        printf("%s\n", temp_buffer);

        if (length == 1) {
            if (temp_buffer[0] == '>') {
                if (data->state == PARSE_INPUT || data->state == PARSE_OUTPUT) {
                    // Syntax Error
                    data->state = PARSE_ERROR;
                    data->error = ERROR_SYNTAX_OUTPUT;
                    break;
                }
                data->state = PARSE_OUTPUT;
                new_command->output_state = IO_FILE;
                skip = true;
            } else if (temp_buffer[0] == '<') {
                if (data->state == PARSE_INPUT || data->state == PARSE_OUTPUT) {
                    // Syntax Error
                    data->state = PARSE_ERROR;
                    data->error = ERROR_SYNTAX_INPUT;
                    break;
                }
                data->state = PARSE_INPUT;
                new_command->input_state = IO_FILE;
                skip = true;
            } else if (temp_buffer[0] == '|') {
                if (data->state == PARSE_COMMAND) {
                    // Missing program
                    data->state = PARSE_ERROR;
                    data->error = ERROR_MISSING_PROGRAM;
                    break;
                } else if (data->state == PARSE_INPUT || data->state == PARSE_OUTPUT) {
                    // Syntax Error
                    data->state = PARSE_ERROR;
                    data->error = ERROR_SYNTAX_PIPE;
                    break;
                } else if (new_command->output) {
                    // Duplicated output redirection
                    data->state = PARSE_ERROR;
                    data->error = ERROR_DUPLICATED_OUTPUT;
                    break;
                }
                // Init a new command after |
                data->commands = realloc(data->commands, sizeof(command_t) * (data->num + 1));
                new_command = &data->commands[data->num++];
                command_init(new_command);
                data->state = PARSE_COMMAND;
                skip = true;
            }
        } else if (length == 2 && temp_buffer[0] == '>' && temp_buffer[1] == '>') {
            if (data->state == PARSE_INPUT || data->state == PARSE_OUTPUT) {
                // Syntax Error
                data->state = PARSE_ERROR;
                data->error = ERROR_SYNTAX_OUTPUT;
                break;
            }
            data->state = PARSE_OUTPUT;
            new_command->output_state = IO_FILE_APPEND;
            skip = true;
        }

        if (!skip) {
            switch (data->state) {
            case PARSE_COMMAND:
                add_argv(new_command, temp_buffer, length);
                data->state = PARSE_OPTION;
//            printf("Find command %s\n", new_command->argv[0]);
                break;
            case PARSE_OPTION:
                add_argv(new_command, temp_buffer, length);
                break;
//            printf("Find option %s\n", new_command->argv[new_command->argc - 1]);
            case PARSE_INPUT:
                if (new_command->input) {
                    // Duplicated input redirection
                    data->state = PARSE_ERROR;
                    data->error = ERROR_DUPLICATED_INPUT;
                    break;
                }
                new_command->input = malloc(sizeof(char) * (length + 1));
                strcpy(new_command->input, temp_buffer);
//                new_command->input[length] = '\0';
                if (new_command->argc == 0) {
                    data->state = PARSE_COMMAND;
                } else {
                    data->state = PARSE_OPTION;
                }
                break;
            case PARSE_OUTPUT:
                if (new_command->output) {
                    // Duplicated output redirection
                    data->state = PARSE_ERROR;
                    data->error = ERROR_DUPLICATED_OUTPUT;
                    break;
                }
                new_command->output = malloc(sizeof(char) * (length + 1));
                strcpy(new_command->output, temp_buffer);
                if (new_command->argc == 0) {
                    data->state = PARSE_COMMAND;
                } else {
                    data->state = PARSE_OPTION;
                }
                break;
            default:
                break;
            }
        }
        if (data->state == PARSE_ERROR) break;
        if (!*now) break;
        while (*now == ' ' || *now == '\n') now++;
    }

    //if (data->num == 0) return data;

    /*if (state != PARSE_OPTION) {
        // Not completed
        data->num = -1;
    }*/
    return data;
}
