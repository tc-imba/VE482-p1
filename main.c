#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 1024

typedef struct parsed_data_t
{
    char* command;
    char** agrv;

} parsed_data;

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

char **input_parse(char *buffer)
{
//    char *pos = strchr()
}

int main(int argc, char *argv[])
{
    char buffer[MAX_COMMAND_LENGTH + 2] = {};
    char *buffer_start;
    while (true)
    {
        prompt();
        fgets(buffer, sizeof(buffer), stdin);
        input_trim(buffer, &buffer_start);
        if (strcmp(buffer, "exit\n") == 0)
        {
            break;
        }
        pid_t pid = fork();
        if (pid == 0)
        {
            int error = execvp("ls", argv);
            if (error == -1)
            {
                printf("error\n");
            }
            break;
        } else
        {
//            printf("%ld,%s\n", strlen(buffer_start), buffer_start);
            waitpid(pid, NULL, 0);
        }

    }




//    std::cout << "Hello, World!" << std::endl;
    return 0;
}