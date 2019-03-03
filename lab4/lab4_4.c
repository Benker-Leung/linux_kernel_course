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

    int found = 0;
    while(cmdline[found] != '\0'){
        if (cmdline[found] == '|') {
            found = -1;
            break;
        }
        ++found;
    }

    // if pipe needed
    if(found == -1) {     
        
        char **cmd;
        char ***args;
        int cmdcount = 1;


        // count the commands
        for(int i=0; cmdline[i] != '\0'; i++){
            if (cmdline[i] == '|')
                ++cmdcount;
        }

        
        cmd = malloc(sizeof(char*) * cmdcount);
        args = malloc(sizeof(char**) * cmdcount);

        // count and add the arguments
        char prev = ' ';
        int keep = 0;
        for(int i=0, temp=1, cc=0; ; i++){
            // add argument to next
            if (cmdline[i] == '\0' || cmdline[i] == '|') {
                args[cc] = malloc(sizeof(char*) * temp);
                args[cc][temp-1] = NULL;
                ++cc;
                temp = 1;
                prev = ' ';
                if (cmdline[i] == '\0') {
                    break;
                }
                continue;
            }
            else if(prev == ' ' && cmdline[i] != ' ') {
                ++temp;
            }
            prev = cmdline[i];
        }
        
        prev = ' ';
        char flag = 'n';    // n stands for not yet capture the command
        // j is index of command, k is index of arg of that command
        for(int i=0, j=0, k=0; ; ++i){
            // ready to append NULL to the args to the command
            if(cmdline[i] == '\0' || cmdline[i] == '|') {
                flag = 'n';
                args[j][k] = (char*)NULL;
                k = 0;
                ++j;
                if(cmdline[i] == '\0') {
                    break;
                }
                cmdline[i] = '\0';
                prev = ' ';
                continue;
            }
            // if prev is space and n, then 
            else if (prev == ' ' && cmdline[i] != ' ') {
                // add the pointer of this command
                if(flag == 'n'){
                    cmd[j] = (char*)(&cmdline[i]);
                    flag = 'y';
                    prev = cmdline[i];

                    args[j][k] = (char*)(&cmdline[i]);
                    ++k;
                }
                // add the argument of the command
                else {
                    args[j][k] = (char*)&(cmdline[i]);
                    ++k;
                }
            }
            else if (cmdline[i] == ' ') {
                prev = ' ';
                cmdline[i] = '\0';
                continue;
            }
            prev = cmdline[i];
        }

        // printf("Command 0 : %s, args: %s\n", cmd[0], args[0][0]);
        // printf("Command 1 : %s, args: %s\n", cmd[1], args[1][0]);

        int pfds[2];
        pipe(pfds);

        int stdin = dup(0);
        int stdout = dup(1);

        dup2(pfds[0], 0);

        pid_t pid= fork();
        
        if(pid == 0) {   // child
        
            close(1);   // close stdout
            dup2(pfds[1], 1);
            execvp(cmd[0], args[0]);
            
        }
        else {  //parent

            close(0);   // close stdin
            dup2(pfds[0], 0);
            close(pfds[1]);
            wait(0);

            pid_t temp = fork();

            if(temp==0) { // child
                execvp(cmd[1], args[1]);
            }
            else {  // parent
                wait(0);
                dup2(stdin, 0);
                dup2(stdout, 1);

                // free memory
                for(int i=0; i<cmdcount; i++){
                    free(args[i]);
                }
                free(args);
                free(cmd);
            }        
        }
    }

    else {
        // pipe not required
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