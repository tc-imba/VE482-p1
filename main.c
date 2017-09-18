#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        printf("Child");
    } else
    {
        printf("Parent");
        getchar();
    }

//    std::cout << "Hello, World!" << std::endl;
    return 0;
}