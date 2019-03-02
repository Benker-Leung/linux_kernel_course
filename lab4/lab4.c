#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#define MAX_CMDLINE_LEN 256

/* function prototypes go here... */

void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);


/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];
	
	while (1) 
	{
		show_prompt();
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */
		
		process_cmd(cmdline);
	}
	return 0;
}


void process_cmd(char *cmdline)
{
    char cmd1[20];
    char cmd2[20];
    char argv[10];
    int pfds[2];
    pipe(pfds);

    int stdin = dup(0);
    int stdout = dup(1);

    dup2(pfds[0], 0);

    sscanf(cmdline, "%s | %s %s", cmd1, cmd2, argv);

    pid_t pid= fork();
    
    if(pid == 0) {   // child
    
        close(1);   // close stdout
        dup2(pfds[1], 1);
        execlp(cmd1, cmd1, NULL);
        
    }
    else {  //parent

        close(0);   // close stdin
        dup2(pfds[0], 0);
        close(pfds[1]);
        wait(0);


        pid_t temp = fork();

        if(temp==0) { // child
            if(argv[0])
                execlp(cmd2, cmd2, argv, NULL);
            else
                execlp(cmd2, cmd2, NULL);
        }
        else {  // parent
            wait(0);
            dup2(stdin, 0);
            dup2(stdout, 1);
        }        
    }

}


void show_prompt() 
{
	printf("myshell> ");
}

int get_cmd_line(char *cmdline) 
{
    int i;
    int n;
    if (!fgets(cmdline, MAX_CMDLINE_LEN, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(cmdline);
    cmdline[--n] = '\0';
    i = 0;
    while (i < n && cmdline[i] == ' ') {
        ++i;
    }
    if (i == n) {
        // Empty command
        return -1;
    }
    return 0;
}