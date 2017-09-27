//
// Created by liu on 17-9-26.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>

#include "utils.h"

void print_prompt() {
    printf("\033[1;34mve482sh $\033[0m ");
}

void print_incomplete() {
    printf("> ");
}

void print_pwd(ino_t ino) {
    static char temp[MAX_COMMAND_LENGTH + 1];
    getcwd(temp, MAX_COMMAND_LENGTH);
    printf("%s\n", temp);
}

void change_dir(const char *dirname) {
    const size_t len1 = strlen(dirname);
    if (strlen(dirname) == 0 || dirname[0] == '~') {
        struct passwd *pwd = getpwuid(getuid());
        const size_t len2 = strlen(pwd->pw_dir);
        char *temp = malloc(sizeof(char) * (len1 + len2));
        strcpy(temp, pwd->pw_dir);
        if (len1 > 1) {
            strcpy(temp + len2, dirname + 1);
            temp[len1 + len2 - 1] = '\0';
        } else {
            temp[len2] = '\0';
        }
        int fd = open(temp, O_RDONLY);
        if (fd < 0) printf("%s: No such file or directory\n", temp);
        else if (fchdir(fd) < 0) printf("%s: Not a directory\n", temp);
        free(temp);
    } else {
        int fd = open(dirname, O_RDONLY);
        if (fd < 0) printf("%s: No such file or directory\n", dirname);
        else if (fchdir(fd) < 0) printf("%s: Not a directory\n", dirname);
    }
}

void sigroutine(int dunno) {
    if (dunno == SIGINT) {
        printf("\n");
    }
}

void fork_and_exec(parsed_data_t *data, int current, int previous_fd[]) {
    if (current == data->num) return;
    int fd[2] = {};
    if (current < data->num - 1) {
        int error = pipe(fd);
        if (error == -1) {
            printf("pipe error\n");
            exit(-1);
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        command_t *command = &data->commands[current];
        // printf("command: %s\n", command->argv[0]);
        int fin = -1, fout = -1;

        if (current > 0) close(previous_fd[1]);
        if (command->input_state == IO_STD) {
            if (current > 0) fin = previous_fd[0];
        } else {
            // printf("input: %s\n", command->input);
            fin = open(command->input, O_RDONLY);
        }
        if (fin >= 0) {
            close(STDIN_FILENO);
            dup2(fin, STDIN_FILENO);
        }

        if (current < data->num - 1) close(fd[0]);
        if (command->output_state == IO_STD) {
            if (current < data->num - 1) fout = fd[1];

        } else {
            // printf("output: %s\n", command->output);
            if (command->output_state == IO_FILE)
                fout = open(command->output, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            else
                fout = open(command->output, O_CREAT | O_WRONLY | O_APPEND);
        }
        if (fout >= 0) {
            close(STDOUT_FILENO);
            dup2(fout, STDOUT_FILENO);
        }

        // printf("Child process: %s\n", command->argv[0]);
        if (strcmp(command->argv[0], "pwd") == 0) {
            print_pwd(0);
            exit(0);
        } else {
            int error = execvp(command->argv[0], command->argv);
            if (error == -1) {
                printf("%s: command not found\n", command->argv[0]);
            }
            exit(-1);
        }
        // if (fin >= 0) close(fin);
        // if (fout >= 0) close(fout);
    } else {
        if (current > 0) {
            close(previous_fd[0]);
            close(previous_fd[1]);
        }
        fork_and_exec(data, current + 1, fd);
        signal(SIGINT, sigroutine);
        waitpid(pid, NULL, 0);
    }
}

