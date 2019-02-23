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
		for (i=0; i<100; i++)
			printf("Parent %d\n",i);
	}
	else { /* child process */
		for (i=0; i<100; i++)
			printf("Child %d\n",i);
		
	}
	return 0;
}
