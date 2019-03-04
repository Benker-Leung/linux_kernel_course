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


// benker added
void free_all();
extern char* get_current_dir_name(void);
extern char *dirname(char *path);
extern char *basename(char *path);
extern char *strdup(const char *s);

char *directory;
char *base;
char *dirc;
char *base_;
char *dirc_;


/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];
	
    
    
	while (1) 
	{
        // should check the pwd each loop
        directory = (char *)(get_current_dir_name());
        // get base
        base = strdup(directory);
        // get cd .. part
        dirc = strdup(directory);

        
        base_ = basename(base);
        dirc_ = dirname(dirc);

		show_prompt(base_);
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */
		
		process_cmd(cmdline);
	}
	return 0;
}


void process_cmd(char *cmdline)
{










    // ============================= Below are code in Lab3 supporting child 2 =======================

    char *Exit = "exit";
    char *Child = "child";

    char cmd[10];
    int time;
    int status;
    sscanf(cmdline, "%s", cmd);
    
    // for exit command
    if(strcmp(Exit, cmd) == 0) {
        printf("myshell is terminated with pid %d\n", getpid());
        free_all();
        exit(0);
    }
    // for child command 
    else if(strcmp(Child, cmd) == 0) {
        // get the sleep time in seconds
        sscanf(cmdline, "child %d", &time);
        if(time <= 0)
            return;

        pid_t pid = fork();
        // for child pid
        if(pid == 0) {
            printf("child pid %d is started and sleep for %d seconds\n", getpid(), time);
            sleep(time);
            free_all();
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
	printf("[%s] myshell> ", base_);
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

void free_all(){

    printf("Freeing space...\n");
    free(directory);
    free(base);
    free(dirc);

}