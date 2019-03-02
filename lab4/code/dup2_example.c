#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
	int pfds[2];
	pipe(pfds);
	pid_t pid = fork(); /* 0 (child), non-zero (parent) */
	if ( pid == 0) { /* child */
		close(1);      /* close stdout */
		dup2(pfds[1], 1);   /* make stdout as pipe input*/
		close(pfds[0]); /* don't need this */
		execlp("ls", "ls", NULL);
	} else { /* The parent process */ 
		close(0);        /* close stdin */
		dup2(pfds[0], 0);  /* make stdin as pipe output*/
		close(pfds[1]); /* don't need this */
		wait(0); /* wait for the child process */
		execlp("wc", "wc", "-l", NULL);
	}
	return 0;
}
