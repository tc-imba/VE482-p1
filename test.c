
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


/*自定义的信号捕捉函数*//*

void sig_int(int signo)
{
    printf("catch signal SIGINT\n");//单次打印  
    sleep(1);
    printf("----slept 10 s\n");
}

int main(void)
{
    struct sigaction act;

    act.sa_handler = sig_int;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);      //不屏蔽任何信号  
//    sigaddset(&act.sa_mask, SIGQUIT);

    sigaction(SIGQUIT, &act, NULL);

    printf("------------main slept 10\n");
    sleep(1);

    while(1);//该循环只是为了保证有足够的时间来测试函数特性  

    return 0;
}  */

#include <stdio.h>
#include <termios.h>

int main()
{
    int fd = STDIN_FILENO;
    struct termios raw;
    int rs = tcgetattr(fd, &raw);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    rs = tcsetattr(fd, TCSAFLUSH, &raw);


    while (1)
    {
        char c;
        int nread = read(STDIN_FILENO, &c, 1);
        if (nread <= 0)
        {
            break;
        }
        printf("%c", c);
        fflush(stdout);
    }
}

