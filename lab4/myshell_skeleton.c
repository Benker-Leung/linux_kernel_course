#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

#define MAX_CMDLINE_LEN 256
#define MAX_CMD_COUNT 2

/* function prototypes go here... */

void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);
void lab3_child_cmd();
int wantToQuit(char* cmd);
void terminate_process();


/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];
	
	while (1) 
	{
		show_prompt();
		if ( get_cmd_line(cmdline) == -1 )
			continue; /* empty line handling */
		pid_t pid = fork();
        if(pid == 0){ // child, allow exec return, but not quit myshell
            process_cmd(cmdline);
            exit(0);
        }
        else{ // parent, wait until child process is done, then continue the shell
            int child_status;
            wait(&child_status);
            // printf("child_status: %d\n", child_status); // DEBUG
            if(child_status == 256){
                printf("error when executing command\n");
                continue;
            }
            else if(child_status != 0){

                break;
            }
        }
	}
	return 0;
}


void process_cmd(char *cmdline)
{
    /*
        currently support 1 pipe (2 commands)
        for ease of implementation, 2 commands have MAX_CMDLINE_LEN
        space consuming, can be optimized using dynamic allocation
    */
    char* commands[MAX_CMD_COUNT][MAX_CMDLINE_LEN];

    char line[MAX_CMDLINE_LEN]; // copy cmdline for manipulation
    strcpy(line, cmdline);

    char delim[] = " ";
    int word_count = 0, command_count = 0;
    char* ptr = strtok(line, delim);
    while(ptr != NULL || command_count+1 > MAX_CMD_COUNT){ // nothing more to read or when pipe limit is reached, then stop
        if(!strcmp(ptr, "|")){
            // printf("PIPE DETECTED::::::ptr: %d, value: %s\n", (int)ptr, ptr); // DEBUG
            commands[command_count][word_count] = (char*)NULL;
            command_count++;
            word_count = 0;
            ptr = strtok(NULL, delim);
            continue;
        }
        // printf("ptr: %d, value: %s\n", (int)ptr, ptr); // DEBUG
        commands[command_count][word_count++] = ptr;
        ptr = strtok(NULL, delim);
    }
    commands[command_count][word_count] = (char *)NULL;
    // printf("read complete\nfirst command arguments:\n"); // DEBUG

    if(command_count){ // pipe is present
        int pfds[2];
        pipe(pfds); // create new pipe

        pid_t pid = fork();
        if(pid == 0){ // child process
            close(STDOUT_FILENO);
            dup2(pfds[1], STDOUT_FILENO);
            close(pfds[0]); // doesn't need read
            if(wantToQuit(commands[0][0])){
                terminate_process();
            }
            if(!strcmp(commands[0][0], "child")){
                lab3_child_cmd(cmdline);
            }
            else{
                execvp(commands[0][0], commands[0]);
            }
            exit(0);
        }else{ // parent process
            close(STDIN_FILENO);
            dup2(pfds[0], STDIN_FILENO);
            close(pfds[1]); // doesn't need write
            int child_status;
            wait(&child_status);
            if(child_status != 0){
                exit(-2);
            }
            if(!strcmp(commands[1][0], "child")){
                lab3_child_cmd(cmdline);
            }
            else{
                execvp(commands[1][0], commands[1]);
            }
            exit(0);
        }
    }else{
        if(wantToQuit(commands[0][0])){
                terminate_process();
        }
        if(!strcmp(commands[0][0], "child")){
            lab3_child_cmd(cmdline);
        }
        else{
            execvp(commands[0][0], commands[0]);
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

void lab3_child_cmd(char *cmdline)
{
    char *childCmd = strstr(cmdline, "child"); // get substring of child from cmdline
    if (childCmd)
    { // command is child
        char *space = strstr(childCmd, " ");
        if (space)
        {
            ++space;
            // printf("%s\n", space);
            pid_t pid = fork();
            if (pid == 0)
            { // child
                printf("child pid %d is started\n", getpid());
                int i = atoi(space);
                sleep(i);
                exit(0);
            }
            else
            {
                pid_t child_id;
                int child_status;
                child_id = wait(&child_status);
                printf("child pid %d is terminated with status %d\n", child_id, child_status);
                exit(0);
            }
        }
    }
}

// support multiple language exit, including fuckyou
int wantToQuit(char* cmd){
    if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit") || !strcmp(cmd, "bye") || !strcmp(cmd, "aufwiedersehen") || !strcmp(cmd, "zaijian") || !strcmp(cmd, "sayonara") || !strcmp(cmd, "fuckyou")){
        return 1;
    }
    return 0;
}

void terminate_process(){
    printf("myshell is terminated with pid %d\n", getpid());
    exit(-1);
}