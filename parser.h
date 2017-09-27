//
// Created by liu on 17-9-26.
//

#ifndef P1_PARSER_H
#define P1_PARSER_H

#define MAX_COMMAND_LENGTH 1024

typedef enum {
    IO_STD,
    IO_FILE,
    IO_FILE_APPEND
} io_state;

typedef struct command_t {
    int argc;
    char **argv;
    char *input, *output;
    io_state input_state, output_state;
} command_t;

typedef struct parsed_data_t {
    int num;
    command_t *commands;
} parsed_data_t;

typedef enum {
    PARSE_COMMAND,
    PARSE_OPTION,
    PARSE_INPUT,
    PARSE_OUTPUT,
    PARSE_QUOTE
} parse_state;

typedef enum {
    STRING_NORMAL,
    STRING_QUOTE_SINGLE,
    STRING_QUOTE_DOUBLE
} string_state;

void input_preprocess(char *buffer, char *parse_buffer);

parsed_data_t *input_parse_init();

parsed_data_t *input_parse(char *buffer);


#endif //P1_PARSER_H
