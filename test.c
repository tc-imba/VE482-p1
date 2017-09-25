
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
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

