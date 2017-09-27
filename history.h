//
// Created by 38569 on 2017/9/27.
//

#ifndef P1_HISTORY_H
#define P1_HISTORY_H

void reset_history();

void add_history(const char *buffer);

void set_history(const char *buffer);

const char *get_history_previous();

const char *get_history_next();

void free_history();


#endif //P1_HISTORY_H
