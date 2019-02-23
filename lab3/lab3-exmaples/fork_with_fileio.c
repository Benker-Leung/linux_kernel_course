#include <stdio.h>
#include <unistd.h> /* POSIX header */
#include <fcntl.h> /* open for POSIX */
#include <sys/types.h>

/* foobar.txt:
A text file contains 6 characters
Content: foobar  */

int main() 
{
	char c;
	int fd = open("foobar.txt", O_RDONLY,0);
	pid_t pid = fork();
	if ( pid == 0 ) { /* child process */
		read(fd, &c, 1); /* read a char */
		printf("c by child = %c\n", c);
		return 0; /* terminate */		
	}
	wait(0); /* wait for the child process */
	read(fd, &c, 1);
	printf("c by parent = %c\n", c);
	close(fd);
	return 0;
}

