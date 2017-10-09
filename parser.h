//
// Created by liu on 17-9-26.
//

#ifndef P1_PARSER_H
#define P1_PARSER_H

#include "defines.h"

typedef enum {
    IO_STD,
    IO_FILE,
    IO_FILE_APPEND
} io_state;

typedef enum {
    PARSE_COMMAND,
    PARSE_OPTION,
    PARSE_INPUT,
    PARSE_OUTPUT,
    PARSE_QUOTE,
    PARSE_ERROR
} parse_state;

typedef enum {
    STRING_NORMAL,
    STRING_QUOTE_SINGLE,
    STRING_QUOTE_DOUBLE,
    STRING_REDIRECTION
} string_state;

typedef enum {
    ERROR_UNKNOWN,
    ERROR_DUPLICATED_INPUT,
    ERROR_DUPLICATED_OUTPUT,
    ERROR_SYNTAX,
    ERROR_SYNTAX_INPUT,
    ERROR_SYNTAX_OUTPUT,
    ERROR_SYNTAX_PIPE,
    ERROR_MISSING_PROGRAM
} error_code;

typedef struct command_t {
    int argc;
    char **argv;
    char *input, *output;
    io_state input_state, output_state;
} command_t;

typedef struct parsed_data_t {
    int num;
    command_t *commands;
    parse_state state;
    error_code error;
} parsed_data_t;

void input_preprocess(char *buffer, char *parse_buffer, parse_state state);

parsed_data_t *input_parse_init();

parsed_data_t *input_parse(char *buffer);


#endif //P1_PARSER_H
