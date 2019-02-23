#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() 
{
	pid_t pid;
	int i;

	/* Create a new process by duplicating 
           the calling process */
	pid = fork();  

	if ( pid > 0 ) { /* parent process */
		for (i=0; i<10; i++)
			printf("Parent %d\n",i);
		wait(0);
	}
	else { /* child process */
		for (i=0; i<10; i++)
			printf("Child %d\n",i);
		
	}
	return 0;
}
