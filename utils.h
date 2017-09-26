//
// Created by liu on 17-9-26.
//

#ifndef P1_UTILS_H
#define P1_UTILS_H

#include <dirent.h>
#include "parser.h"

void print_prompt();

void print_pwd(ino_t ino);

void change_dir(const char *dirname);

void fork_and_exec(parsed_data_t *data, int current, int previous_fd[]);

#endif //P1_UTILS_H
