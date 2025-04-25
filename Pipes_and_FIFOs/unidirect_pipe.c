#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
 #include <sys/wait.h>

/* Simple program creates interprocess communication between child and parent*/

#define BUF_SIZE 10

int main(int argc, char **argv)
{
    char buf[BUF_SIZE];
    int  fds[2];
    ssize_t num_read;

    if (argc < 2)
        return 0;
    if (pipe(fds) == -1) /* Create the pipe */
        exit(EXIT_FAILURE);
    switch (fork()) /* Create a child process */
    {
        case -1:
            exit(EXIT_FAILURE);
        case 0: /* Child */
            if (close(fds[1]) == -1) /* Close unused write end */
                exit(EXIT_FAILURE);
                 /* Child now reads from pipe and write to stdout*/
            for(;;)
            {
                num_read = read(fds[0], buf, BUF_SIZE);
                if (num_read == -1)
                    exit(EXIT_FAILURE);
                if (num_read == 0)
                    break;
                if (write(STDOUT_FILENO, buf, num_read) != num_read)
                {
                    printf("\nchild - partial/failed write\n");
                    exit(EXIT_FAILURE);
                }     
            }
            write(STDOUT_FILENO, "\n", 1);
            if (close(fds[0]) == -1)
                exit(EXIT_FAILURE);
            break;
        default: /* Parent */
            if (close(fds[0]) == -1) /* Close unused read end */
                exit(EXIT_FAILURE);   
            if (write(fds[1], argv[1], strlen(argv[1])) != strlen(argv[1])) /* Parent now writes to pipe */
            {
                printf("\nparent - partial/failed write");
                exit(EXIT_FAILURE);
            }
            close (fds[1]); /* Child will see EOF */
            wait(NULL);
            exit(EXIT_SUCCESS);
        break;
    }
}
