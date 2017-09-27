#include "history.h"
#include <stdio.h>

int main() {
    char *str1 = "test1";
    char *str2 = "test2";
    char *str3 = "test3";
    char *str4 = "test4";
    add_history(str1);
    add_history(str2);
    add_history(str3);
    add_history(str4);

//    printf("%d %d %d\n", history_num, history_entry_pos, history_offset);
    printf("%s\n", get_history_previous());
    printf("%s\n", get_history_previous());
    printf("%s\n", get_history_next());
    printf("%s\n", get_history_previous());

    free_history();
    return 0;
}

