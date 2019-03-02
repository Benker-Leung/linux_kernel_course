#include <stdio.h>
#define MAXSTRS 5
int main() 
{		
	int i;
	FILE *pipe_fp;
	char *strings[MAXSTRS] = {"Ann","Dog","Bob","Egg","Cat"};
	
	pipe_fp = popen("sort", "w"); // create a "sort" pipe
	if (!pipe_fp) {
		perror("popen"); // error handling
		exit(1);
	} else {
		for (i=0; i<MAXSTRS;i++) {
			fputs(strings[i], pipe_fp); // send a string to the pipe
			fputc('\n', pipe_fp); // send the endline character
		}
	}
	pclose(pipe_fp); // close the pipe
	return 0;	
}
