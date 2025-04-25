
/* Write a program that uses two pipes to enable (!) bidirectional communication
between a parent and child process. The parent process should loop reading a
block of text from standard input and use one of the pipes to send the text to the
child, which converts it to uppercase and sends it back to the parent via the other
pipe. The parent reads the data coming back from the child and echoes it on
standard output before continuing around the loop once more. */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = toupper((unsigned char)str[i]);
}

int main(void) {
    int fd1[2]; // Parent -> Child
    int fd2[2]; // Child -> Parent
    pid_t cpid;
    char buf_input[MAX_INPUT_SIZE];
    ssize_t n;

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {
        // --- Child process ---
        close(fd1[1]); // close write end of fd1
        close(fd2[0]); // close read end of fd2

        while ((n = read(fd1[0], buf_input, sizeof(buf_input) - 1)) > 0) {
            buf_input[n] = '\0';
            to_uppercase(buf_input);
            write(fd2[1], buf_input, strlen(buf_input));
        }

        close(fd1[0]);
        close(fd2[1]);
        exit(EXIT_SUCCESS);

    } else {
        // --- Parent process ---
        close(fd1[0]); // close read end of fd1
        close(fd2[1]); // close write end of fd2

        while (1) {
            printf("Enter text: ");
            fflush(stdout);  // ensure prompt appears
            if (fgets(buf_input, sizeof(buf_input), stdin) == NULL)
                break;

            // Send to child
            write(fd1[1], buf_input, strlen(buf_input));

            // Read response from child
            n = read(fd2[0], buf_input, sizeof(buf_input) - 1);
            if (n > 0) {
                buf_input[n] = '\0';
                printf("Uppercase: %s", buf_input);
            }
        }

        close(fd1[1]);
        close(fd2[0]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}
