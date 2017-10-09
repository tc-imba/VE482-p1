//
// Created by liu on 17-9-26.
//

#ifndef P1_UTILS_H
#define P1_UTILS_H

#include <dirent.h>
#include "parser.h"

void terminate();

void print_prompt();

void print_incomplete();

void print_pwd();

void change_dir(const char *dirname);

void fork_and_exec(parsed_data_t *data, int current, int previous_fd[]);

#endif //P1_UTILS_H
