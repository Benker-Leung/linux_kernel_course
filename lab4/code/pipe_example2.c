#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h> // pipe function

int main()
{
	int pfds[2];
	char buf[30];
	pipe(pfds); 
	pid_t pid = fork(); // create a child process

	if ( pid != 0 ) {
		printf("PARENT: writing to pipe\n");
		close(pfds[0]);
		write(pfds[1], "test", 5);
		wait(0); // wait until child returns		
		printf("PARENT: exiting\n");
	}
	else {
		printf("CHILD: reading from pipe\n");
		close(pfds[1]);
		read(pfds[0], buf, 5);
		printf("CHILD: read %s \n", buf);
	}

	return 0;
}
