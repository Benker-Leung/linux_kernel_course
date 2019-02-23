#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
	int child_status;
	pid_t child_pid ;
	pid_t pid = fork();
	if ( pid == 0 ) { /* child */
		return 1;
	} else { /* parent */
		printf("Parent PID %d\n", getpid());
		child_pid = wait(&child_status);
		printf("Child PID %d with status %d\n", 
			child_pid, child_status);
	}
	return 0;
}
