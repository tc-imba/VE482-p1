#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>

#include "editor.h"

#define MAX_COMMAND_LENGTH 1024

typedef enum
{
    IO_STD,
    IO_FILE,
    IO_FILE_APPEND
} io_state;

typedef struct command_t
{
    int argc;
    char **argv;
    char *input, *output;
    io_state input_state, output_state;
} command_t;

typedef struct parsed_data_t
{
    int num;
    command_t *commands;
} parsed_data_t;

typedef enum
{
    PARSE_COMMAND,
    PARSE_OPTION,
    PARSE_INPUT,
    PARSE_OUTPUT
} parse_state;

void prompt()
{
    printf("\033[1;34mve482sh $\033[0m ");
}

void input_trim(char *buffer, char **start)
{
    *start = buffer;
    while (**start == ' ') (*start)++;
    char *end = buffer + strlen(buffer) - 2;
    while (*end == ' ')end--;
    *(end + 1) = '\0';
}

void add_argv(command_t *command, const char *str, const size_t length)
{
    if (command->argc == 0)
    {
        command->argv = malloc(sizeof(char *) * 2);
    } else
    {
        command->argv = realloc(command->argv, sizeof(char *) * (command->argc + 2));
    }
    command->argv[command->argc] = malloc(sizeof(char) * (length + 1));
    strncpy(command->argv[command->argc], str, length);
    command->argv[command->argc][length] = '\0';
    command->argc++;
    command->argv[command->argc] = NULL;
}

void command_init(command_t *command)
{
    command->argc = 0;
    command->argv = NULL;
    command->input = NULL;
    command->output = NULL;
    command->input_state = IO_STD;
    command->output_state = IO_STD;
}

void command_clear(command_t *command)
{
    if (command->argv)
    {
        for (int i = 0; i <= command->argc; i++)
        {
            if (command->argv[i]) free(command->argv[i]);
        }
        free(command->argv);
    }
    if (command->input) free(command->input);
    if (command->output) free(command->output);
    command->input_state = IO_STD;
    command->output_state = IO_STD;
}

parsed_data_t *input_parse_init()
{
    static parsed_data_t data = {0, NULL};
    if (data.commands != NULL)
    {
        for (int i = 0; i < data.num; i++)
        {
            command_clear(&data.commands[i]);
        }
        free(data.commands);
        data.commands = NULL;
    }
    data.num = 0;
    return &data;
}

parsed_data_t *input_parse(char *buffer)
{
    parse_state state = PARSE_COMMAND;

    parsed_data_t *data = input_parse_init();
    data->num = 0;

    command_t *new_command;
    char *start = buffer;
    while (*start != '\0')
    {
        bool skip = false;
        char *pos = strchr(start, ' ');
        if (pos == NULL) pos = start + strlen(start);
        size_t length = pos - start;

        if (length == 1)
        {
            if (*start == '>')
            {
                state = PARSE_OUTPUT;
                new_command->output_state = IO_FILE;
                skip = true;

            } else if (*start == '<')
            {
                state = PARSE_INPUT;
                new_command->input_state = IO_FILE;
                skip = true;
            } else if (*start == '|')
            {
                state = PARSE_COMMAND;
                skip = true;
            }
        } else if (length == 2 && *start == '>' && *(start + 1) == '>')
        {
            state = PARSE_OUTPUT;
            new_command->output_state = IO_FILE_APPEND;
            skip = true;
        }

        if (!skip)
        {
            switch (state)
            {
            case PARSE_COMMAND:
                if (data->num == 0)
                {
                    data->commands = malloc(sizeof(command_t));
                } else
                {
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
    return data;
}

void print_pwd(ino_t ino)
{
    struct stat st;
    if (stat(".", &st) == -1 || ino == st.st_ino) return;
    DIR *dir = opendir(".");
    struct dirent *dt;
    char *str = NULL;
    if (ino > 0)
    {
        while ((dt = readdir(dir)) != NULL)
        {
            if (ino == dt->d_ino)
            {
                str = malloc(sizeof(char) * (dt->d_reclen + 1));
                strcpy(str, dt->d_name);
                break;
            }
        }
    }
    closedir(dir);
    chdir("..");
    print_pwd(st.st_ino);
    chdir(str);
    if (str)
    {
        printf("/%s", str);
        free(str);
    }
    if (ino == 0) printf("\n");
}

void fork_and_exec(parsed_data_t *data, int current, int previous_fd[])
{
    if (current == data->num) return;
    int fd[2] = {};
    if (current < data->num - 1)
    {
        int error = pipe(fd);
        if (error == -1)
        {
            printf("pipe error");
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        command_t *command = &data->commands[current];
//        printf("command: %s\n", command->argv[0]);
        int fin = -1, fout = -1;

        if (current > 0) close(previous_fd[1]);
        if (command->input_state == IO_STD)
        {
            if (current > 0) fin = previous_fd[0];
        } else
        {
//            printf("input: %s\n", command->input);
            fin = open(command->input, O_RDONLY);
        }
        if (fin >= 0)
        {
            close(STDIN_FILENO);
            dup2(fin, STDIN_FILENO);
        }

        if (current < data->num - 1) close(fd[0]);
        if (command->output_state == IO_STD)
        {
            if (current < data->num - 1) fout = fd[1];

        } else
        {
//            printf("output: %s\n", command->output);
            if (command->output_state == IO_FILE)
                fout = open(command->output, O_CREAT | O_TRUNC | O_WRONLY, 0644);
            else
                fout = open(command->output, O_CREAT | O_WRONLY | O_APPEND);
        }
        if (fout >= 0)
        {
            close(STDOUT_FILENO);
            dup2(fout, STDOUT_FILENO);
        }

//        printf("Child process: %s\n", command->argv[0]);
        if (strcmp(command->argv[0], "pwd") == 0)
        {
            print_pwd(0);
            exit(0);
        } else
        {
            int error = execvp(command->argv[0], command->argv);
            if (error == -1)
            {
                printf("%s: command not found\n", command->argv[0]);
            }
        }
        if (fin >= 0) close(fin);
        if (fout >= 0) close(fout);

    } else
    {
        if (current > 0)
        {
            close(previous_fd[0]);
            close(previous_fd[1]);
        }
        fork_and_exec(data, current + 1, fd);
        waitpid(pid, NULL, 0);
    }
}

int main(int argc, char *argv[])
{
    char buffer[MAX_COMMAND_LENGTH + 2] = {};
    char *buffer_start;
    while (true)
    {
        prompt();

        if (isatty(STDIN_FILENO))
        {
//            printf("Terminal mode!\n");
            enable_editor_mode();
            int rs = editor_mode_read(buffer);
            disable_editor_mode();
            if (rs == -1) continue;
            else if (rs == -2) break;
        } else
        {
            printf("Text mode!\n");
        }

//        fgets(buffer, sizeof(buffer), stdin);
        input_trim(buffer, &buffer_start);
//        printf("%s\n", buffer_start);

        if (strcmp(buffer_start, "exit") == 0)
        {
            break;
        }

        parsed_data_t *data = input_parse(buffer_start);
//        printf("%d\n", data->num);
        if (data->num > 0 && strcmp(data->commands[0].argv[0], "cd") == 0)
        {
            int fd = open(data->commands[0].argv[1], O_RDONLY);
            if (fd < 0) printf("%s: No such file or directory\n", data->commands[0].argv[1]);
            else if (fchdir(fd) < 0) printf("%s: Not a directory\n", data->commands[0].argv[1]);
        } else
        {
            fork_and_exec(data, 0, NULL);
        }

    }

    input_parse_init();

//    std::cout << "Hello, World!" << std::endl;
    return 0;
}