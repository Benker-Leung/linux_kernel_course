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
    char *Exit = "exit";
    char *Child = "child";

    char cmd[10];
    int time;
    int status;
    sscanf(cmdline, "%s", cmd);
    
    // for exit command
    if(strcmp(Exit, cmd) == 0) {
        printf("myshell is terminated with pid %d\n", getpid());
        exit(0);
    }
    // for child command 
    else if(strcmp(Child, cmd) == 0) {
        sscanf(cmdline, "child %d", &time);
        if(time <= 0)
            return;

        pid_t pid = fork();
        // for child pid
        if(pid == 0) {
            printf("child pid %d is started and sleep for %d seconds\n", getpid(), time);
            sleep(time);
            exit(0);
        }
        else {
            pid_t child_pid = wait(&status);
            printf("child pid %d is terminated with status %d\n", child_pid, status);
            return;
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