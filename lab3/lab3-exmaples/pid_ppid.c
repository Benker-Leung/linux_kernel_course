#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) 
{
    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("Current program PID: %d\n", pid);
    printf("Parent program PID: %d\n", ppid);

    return 0;
}