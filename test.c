//
// Created by liu on 17-9-18.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


void fork_and_exec(char *data[3][3], int num, int current, int previous_fd[2])
{
    if (current >= num) return;
    int fd[2] = {};
    if (current < num - 1)
    {
        pipe(fd);
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        if (current < num - 1)
        {
            close(fd[0]);
            close(STDOUT_FILENO);
            dup2(fd[1], STDOUT_FILENO);
        }
        if (current > 0)
        {
            close(previous_fd[1]);
            close(STDIN_FILENO);
            dup2(previous_fd[0], STDIN_FILENO);
        }
        execvp(data[current][0], data[current]);
    } else
    {
        if (current > 0)
        {
            close(previous_fd[0]);
            close(previous_fd[1]);
        }
        fork_and_exec(data, num, current + 1, fd);
        waitpid(pid, NULL, 0);
    }
}


int main()
{
    char *data[4][3] = {
            {"ls",   "-al", 0},
            {"grep", "a",   0},
            {"grep", "m",   0},
            {"grep", "c",   0}
    };
//    printf("%s\n", data[0][0]);
    fork_and_exec(data, 4, 0, NULL);
//    for (int i = 0; i < 4; i++)
//    {
//        wait(NULL);
//    }
    return 0;
}


//int main()
//{
//
//    pid_t pid;
//    int fd1[2], fd2[2], i;
//
//    printf("%d\n", pipe(fd1));
//    printf("%d\n", pipe(fd2));
//
//    for (i = 0; i < 3; i++)
//    {
//        if ((pid = fork()) == 0)
//        {
//            break;
//        }
//    }
//
//    if (i == 0)
//    {           //兄
//        close(fd1[0]);               //写,关闭读端
//        dup2(fd1[1], STDOUT_FILENO);
//        close(fd2[0]);
//        close(fd2[1]);
//        execlp("ls", "ls", "-al", NULL);
//    } else if (i == 1)
//    {    //弟
//        close(fd1[1]);               //读，关闭写端
//        dup2(fd1[0], STDIN_FILENO);
//        close(fd2[0]);               //写,关闭读端
//        dup2(fd2[1], STDOUT_FILENO);
//        execlp("grep", "grep", "a", NULL);
//    } else if (i == 2)
//    {    //弟
//        close(fd2[1]);               //读，关闭写端
//        dup2(fd2[0], STDIN_FILENO);
//        close(fd1[0]);
//        close(fd1[1]);
//        execlp("grep", "grep", "a", NULL);
//    } else
//    {
//        close(fd1[0]);
//        close(fd1[1]);
//        close(fd2[0]);
//        close(fd2[1]);
//        for (i = 0; i < 3; i++)       //两个儿子wait两次
//            wait(NULL);
//    }
//
//    return 0;
//}
