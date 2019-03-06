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

char** command = NULL;     // pointer to command locations
char*** argument = NULL;   // pointer to argument locations
int* argument_count = NULL;     // pointer to argument number
int command_count = 1;  // total command

// free the allocated memory
void freeMem() {
    // deallocation of allocated memory
    int i;

    // free argument
    if(argument != NULL) {
        for(i=0; i<command_count; ++i) {
            free(argument[i]);
        }
        free(argument);
        argument = NULL;
    }
    // free command
    if(command != NULL) {
        free(command);
        command = NULL;
    }
        
    
    // free argument_count
    if(argument_count != NULL) {
        free(argument_count);
        argument_count = NULL;
    }

}
// parse command and argument
int parser(char *cmdline) {
    
    freeMem();

    char* temp = NULL;  // temp usage
    // char* c = malloc(strlen((cmdline)+1) * sizeof(char));  // replace argument list in function
    // strcpy(c, cmdline);
    char *c = cmdline;
    int read_command = 0;   // indicate that command read or not
    int read_space = 0;     // indicate that space read or not
    int arg_count = 0;      // for counting argument in each command, will be set to 0 if new command reach
    int len = strlen(c);    // length of command line
    int i;  // count the total command
    int j;  // count command index
    for(i=0; i<len; ++i){
        if(c[i] == '|')
            ++command_count;
    }

    argument_count = calloc(command_count, sizeof(int));
    command = calloc(command_count, sizeof(char*));

    // count the arguments in each command
    read_space = 1; // assume read space
    read_command = 0; // assume not read command
    for(i=0, j=0; i<len; ++i){  // j is current command index
        // means next should be command
        if(c[i] == '|') {
            ++j;
            read_command = 0;
            read_space = 1;
            continue;
        }
        //  means starting of 
        else if(c[i] != ' ' && c[i] != '\t' && read_space == 1) {
            if(read_command == 0) {
                read_command = 1;
                read_space = 0;
                continue;
            }
            else {
                // special case
                if(c[i] == '"') {
                    // find the next "
                    temp = strchr(c+i+1, '"');
                    if(temp == NULL) {
                        printf("Wrong syntax\n");
                        freeMem();
                        return 0;
                    }
                    i = (temp-c)+1;
                    read_space = 0;
                    argument_count[j] += 1;
                    continue;
                }
                else {
                    read_space = 0;
                    argument_count[j] += 1;
                    continue;
                }
            }
        }
        else if(c[i] == ' ' || c[i] == '\t') {
            read_space = 1;
        }
    }
 
    // initialize the array of string pointers points to arg
    argument = calloc(command_count, sizeof(char**));
    for(i=0; i<command_count; ++i) {
        argument_count[i]+=2;
        argument[i] = calloc(argument_count[i], sizeof(char*));
    }


    read_command = 0;    // used in determind command got or not
    read_space = 1;   // used for check first char   
    for(i=0, j=0, arg_count=0; i<len ;++i) {
        if(c[i] == '|') {
            arg_count = 0; // set arg count to 0
            read_command = 0;   // set not read command
            read_space = 1;     // set read space as true
            c[i] = '\0';        // set null
            ++j;                // next command
            continue;
        }
        else if(c[i] != ' ' && c[i] != '\t' && read_space == 1){
            if(read_command == 0){
                command[j] = (char*)c+i;                 // set command
                argument[j][arg_count++] = (char*)c+i;   // set argument
                read_command = 1;
                read_space = 0;
                continue;
            }
            else{
                // special case
                if(c[i] == '"') {
                    // first saw "
                    c[i] = '\0';
                    argument[j][arg_count++] = (char*)c+i+1;
                    temp = strchr(c+i+1, '"');
                    if(temp == NULL){
                        printf("wrong syntax\n");
                        freeMem();
                        return 0;
                    }
                    *temp = '\0';
                    i = (temp-c)+1;
                    read_space = 0;
                    continue;
                }
                else{
                    argument[j][arg_count++] = c+i;
                    read_space = 0;
                    continue;
                }
            }
        }
        else if(c[i] == ' ' || c[i] == '\t'){
            read_space = 1;
            c[i] = '\0';
        }
    } 
    return 1;
}


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
    // for other command
    else {
        parser(cmdline);

        int count = 0;
        pid_t pid;

        int fds[2];
        pipe(fds);

        while(command_count != count) {
            pid = fork();
            if (pid == 0) {
                // child 

                // if first one and also more than 1 command, just write without read
                if(count == 0 && command_count != 1) {
                    close(fds[0]);      // close read
                    close(1);           // close stdout
                    dup2(fds[1], 1);    // dup to stdout
                    close(fds[1]);
                    // fprintf(stderr, "1executing %s\n", command[count]);
                    execvp(command[count], argument[count]);
                }
                // else it does read and write to stdout
                else if (count == (command_count - 1)) {
                    close(fds[1]);      // close write
                    close(0);           // close stdin
                    dup2(fds[0], 0);    // dup to stdin
                    close(fds[0]);
                    // fprintf(stderr, "2executing %s\n", command[count]);
                    execvp(command[count], argument[count]);
                }
                // else it does read and write to pipe
                else {
                    // read
                    close(0);           // close stdin
                    dup2(fds[0], 0);    // dup to stdin
                    close(fds[0]);
                    // write
                    close(1);           // close stdout
                    dup2(fds[1], 1);    // dup to stdout
                    close(fds[1]);
                    // fprintf(stderr, "3executing %s\n", command[count]);
                    execvp(command[count], argument[count]);
                }
            }
            else {
                // parent
                write(fds[0], "\0", 1);
                wait(0);
                // fprintf(stderr, "Parent%d\n", count);
                ++count;
            }
        }
        return;
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