#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig) 
{
    const char msg[] = "Signal caught!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

int main() 
{
    struct sigaction act;
    
    // Set up the signal handler for SIGUSR1
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);
    // Send SIGUSR1 to itself
    raise(SIGUSR1);
    sleep(1); // Allow time for signal delivery
    printf("Changing SIGUSR1 disposition to SIG_IGN...\n");
    // Ignore SIGUSR1
    act.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &act, NULL);
    // Send SIGUSR1 again
    raise(SIGUSR1);
    sleep(1); // Allow time for signal delivery
    printf("If you see this message without 'Caught signal', SIGUSR1 was ignored.\n"); 
    return (0);
}