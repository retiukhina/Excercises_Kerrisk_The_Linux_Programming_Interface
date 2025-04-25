
/*  Demonstrate the use of popen() and pclose(). 
    This program reads filename wildcard patterns from standard input and
    passes each pattern to a popen() call that returns the output from ls(1)
    for the wildcard pattern. The program displays the returned output. */

#include <ctype.h>
#include "include/print_wait_status.h"
#include "include/tlpi_hdr.h"
#include <linux/limits.h>

#define POPEN_FMT "/bin/ls -d %s 2> /dev/null"
#define PAT_SIZE 50
#define PCMD_BUF_SIZE (sizeof(POPEN_FMT) + PAT_SIZE)

int main(int argc, char *argv[])
{
    char pat[PAT_SIZE];  // Array to store user-inputted pattern (e.g., file name pattern)
    char popenCmd[PCMD_BUF_SIZE];  // Array to store the final command string for 'popen'
    FILE *fp;  // File pointer for reading the output of the command executed by 'popen'
    Boolean badPattern;  // Boolean flag to check for invalid characters in the input pattern
    int len, status, fileCnt, j;  // Integer variables for length of the pattern, status code, file count, and iterator
    char pathname[PATH_MAX];  // Array to store each pathname read from the command output

    for (;;) {  // Infinite loop to continuously prompt the user for input
        printf("pattern: ");  // Prompt the user to input a pattern
        fflush(stdout);  // Flush the output buffer to ensure the prompt is printed immediately
        if (fgets(pat, PAT_SIZE, stdin) == NULL)  // Read a line from standard input into 'pat'
            break;  // Exit the loop if the input is NULL (e.g., EOF or error)
        len = strlen(pat);  // Get the length of the input pattern string
        if (len <= 1)  // If the pattern is just an empty string or a newline
            continue;  // Skip the current iteration and ask for another pattern
        if (pat[len - 1] == '\n')  // If the last character of the input is a newline
            pat[len - 1] = '\0';  // Remove the newline character by replacing it with a null terminator
        
            /* Ensure that the pattern contains only valid characters,
        i.e., letters, digits, underscore, dot, and the shell
        globbing characters. (Our definition of valid is more
        restrictive than the shell, which permits other characters
        to be included in a filename if they are quoted.) */
        
        for (j = 0, badPattern = FALSE; j < len && !badPattern; j++)
            if (!isalnum((unsigned char) pat[j]) &&  // If the character is not alphanumeric
                strchr("_*?[^-].", pat[j]) == NULL)  // Or not a valid wildcard character
                    badPattern = TRUE;  // Set the flag to indicate an invalid pattern
        if (badPattern) {  // If an invalid pattern is detected
            printf("Bad pattern character: %c\n", pat[j - 1]);  // Print an error message
            continue;  // Skip the current iteration and ask for another pattern
        }
        /* Build and execute command to glob 'pat' */
        snprintf(popenCmd, PCMD_BUF_SIZE, POPEN_FMT, pat);  // Construct the shell command to match files using the pattern
        popenCmd[PCMD_BUF_SIZE - 1] = '\0';  // Ensure the command is null-terminated (to avoid buffer overflow)
        fp = popen(popenCmd, "r");  // Open a pipe to the command, with "r" meaning read-only mode
        if (fp == NULL) {  // If the 'popen' call fails
            printf("popen() failed\n");  // Print an error message
            continue;  // Skip the current iteration and ask for another pattern
        }
        /* Read resulting list of pathnames until EOF */
        fileCnt = 0;  // Initialize the file count
        while (fgets(pathname, PATH_MAX, fp) != NULL) {  // Read each pathname from the command output
            printf("%s", pathname);  // Print the pathname to standard output
            fileCnt++;  // Increment the file count for each matching file
        }
        /* Close pipe, fetch and display termination status */
        status = pclose(fp);  // Close the pipe and retrieve the exit status of the command
        printf("%d matching file%s\n", fileCnt, (fileCnt != 1) ? "s" : "");  // Print the number of matching files
        printf("pclose() status == %#x\n", (unsigned int) status);  // Print the exit status code in hexadecimal
        
        if (status != -1)  // If the exit status is valid (not -1)
            printWaitStatus("\t", status);  // Call printWaitStatus to display the status in a human-readable format
    }
    exit(EXIT_SUCCESS);  // Exit the program with a success status code
}