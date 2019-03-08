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
void freeDir();     // free memory allocated to store directory
int parser();       // allocate memory to point to cmd and arg, retunr 1 on success
void freeCmd();     // free memory on point to cmd and arg
extern char* get_current_dir_name(void);
extern char *dirname(char *path);
extern char *basename(char *path);
extern char *strdup(const char *s);
extern pid_t wait(int *stat_loc);

char *directory = NULL;     // used to store current directory
char *base = NULL;          // used to store the last part
char *dirc = NULL;          // used to store the part except last
char *base_ = NULL;         // points to base, do not free this
char *dirc_ = NULL;         // points to dirc, do not free this

char** command = NULL;     // pointer to command locations
char*** argument = NULL;   // pointer to argument locations
int* argument_count = NULL;     // pointer to argument number
int command_count = 1;  // total command


/* The main function implementation */
int main()
{
	char cmdline[MAX_CMDLINE_LEN];
	
	while (1) 
	{
        // free memory if allocated
        freeDir();
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
    char *Cd = "cd";

    char cmd[10];
    int time;
    int status;
    sscanf(cmdline, "%s", cmd);
    
    // for exit command
    if(strcmp(Exit, cmd) == 0) {
        printf("myshell is terminated with pid %d\n", getpid());
        freeDir();
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
            freeDir();
            exit(0);
        }
        else {
            pid_t child_pid = wait(&status);
            printf("child pid %d is terminated with status %d\n", child_pid, status);
            return;
        }
    }
    // for cd command
    else if(strcmp(Cd, cmd) == 0) {

        // searching start after cd
        char *temp_path = cmdline+2;
        // get the first non space and non \t
        while(*temp_path != '\0' && !(*temp_path != ' ' && *temp_path != '\t')) {
            ++temp_path;
        }
        // if null, should go to HOME directory
        if(*temp_path == '\0') {
            chdir(getenv("HOME"));
        }
        // else go to user define directory
        else {
            chdir(temp_path);
        }
        return;
    }
    // for other command
    else {


        parser(cmdline);

        int i;
        // int j;
        // printf("%d\n", command_count);
        // printf("%s\n", cmdline);
        // for(i=0; i<command_count; ++i) {
        //     for(j=0; j<argument_count[i]; ++j) {
        //         printf("Arg[%d][%d]:%s\n", i, j, argument[i][j]);
        //     }
        // }
        // return;
        int count = 0;
        pid_t pid;

        // create enough pipe
        int **fds;
        fds = calloc(command_count, sizeof(int*));
    
        for(i=0; i<command_count; ++i) {
            fds[i] = calloc(2, sizeof(int));
            pipe(fds[i]);
        }

        while(command_count != count) {
            pid = fork();
            if (pid == 0) {
                // child 

                // in case of first and more than 1 command, just write without read
                if(count == 0 && command_count != 1) {
                    close(fds[count][0]);      // close curr read
                    close(1);                  // close stdout
                    dup2(fds[count][1], 1);    // dup to stdout

                    // for running curr dir file
                    char *temp_cmd = calloc(strlen(command[count])+strlen(directory)+2, sizeof(char));
                    strcpy(temp_cmd, directory);
                    if(strlen(directory) != 1)
                        *(temp_cmd+strlen(directory)) = '/';
                    strcpy(temp_cmd+strlen(directory)+1, command[count]);
                    execv(temp_cmd, argument[count]);
                    
                    // if curr dir has not such file, try run it from $PATH, and free temp
                    free(temp_cmd);

                    // fprintf(stderr, "1 executing %s\n", command[count]);
                    execvp(command[count], argument[count]);
                }
                // in case of last or first with only one command,
                // it does read from prev pipe and write to current pipe, stdout
                else if (count == (command_count - 1)) {
                    close(fds[count][1]);      // close curr write
                    // if more than one command, need to dup prev pipe to stdin
                    if(count != 0) {
                        close(0);                    // close stdin
                        dup2(fds[count-1][0], 0);    // dup prev pipe to stdin
                    }


                    // for running curr dir file
                    char *temp_cmd = calloc(strlen(command[count])+strlen(directory)+2, sizeof(char));
                    strcpy(temp_cmd, directory);
                    if(strlen(directory) != 1)
                        *(temp_cmd+strlen(directory)) = '/';
                    strcpy(temp_cmd+strlen(directory)+1, command[count]);
                    execv(temp_cmd, argument[count]);
                    
                    // if curr dir has not such file, try run it from $PATH, and free temp
                    free(temp_cmd);

                    // fprintf(stderr, "2 executing %d, %s\n", count, command[count]);
                    execvp(command[count], argument[count]);
                }
                // in case of middle, it does read from prev and write to curr pipe
                else {
                    // read
                    close(fds[count-1][1]);     // close curr write
                    close(0);                   // close stdin
                    dup2(fds[count-1][0], 0);   // dup prev pipe to stdin
                    // write
                    close(fds[count][0]);       // close curr read
                    close(1);                   // close stdout
                    dup2(fds[count][1], 1);     // dup curr pipe to stdout


                    // for running curr dir file
                    char *temp_cmd = calloc(strlen(command[count])+strlen(directory)+2, sizeof(char));
                    strcpy(temp_cmd, directory);
                    if(strlen(directory) != 1)
                        *(temp_cmd+strlen(directory)) = '/';
                    strcpy(temp_cmd+strlen(directory)+1, command[count]);
                    execv(temp_cmd, argument[count]);
                    
                    // if curr dir has not such file, try run it from $PATH, and free temp
                    free(temp_cmd);

                    // fprintf(stderr, "3 executing %s\n", command[count]);
                    execvp(command[count], argument[count]);
                }
                fprintf(stderr, "myshell: command not found: %s\n", command[count]);
                exit(0);
            }
            else {
                // parent
                
                // in case of more than one command, close the prev write pipe
                if(count != 0) {
                    close(fds[count-1][1]);
                }
                close(fds[count][1]);   // close prev write
                wait(0);
                // fprintf(stderr, "Parent%d\n", count);
                ++count;
            }
        }

        // free fds pointer
        for(i=0; i<command_count; ++i) {
            free(fds[i]);
        }
        free(fds);

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

// free memory allocated to store directory
void freeDir(){

    if(directory != NULL) {
        free(directory);
        directory = NULL;
    }
    if(base != NULL) {
        free(base);
        base = NULL;
    }
    
    if(dirc != NULL) {
        free(dirc);
        dirc = NULL;
    }    

}

// free the allocated memory
void freeCmd() {
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
// parse command and argument, return 1 on success
int parser(char *cmdline) {
    
    freeCmd();

    command_count = 1;  // total command

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

    /* count the number of command */
    for(i=0; i<len; ++i){
        if(c[i] == '\'')
            c[i] = '"';
        if(c[i] == '|')
            ++command_count;
    }

    // allocate int to count argument size, and mem to cmd size
    argument_count = calloc(command_count, sizeof(int));
    command = calloc(command_count, sizeof(char*));

    /* count the arguments in each command */
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
                        // printf("Wrong syntax\n");
                        freeCmd();
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
 
    /* initialize the array of string pointers points to arg */
    argument = calloc(command_count, sizeof(char**));
    for(i=0; i<command_count; ++i) {
        argument_count[i]+=2;
        argument[i] = calloc(argument_count[i], sizeof(char*));
    }


    read_command = 0;    // used in determind command got or not
    read_space = 1;   // used for check first char   
    /* assign pointers value point to cmd and arg */
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
                        // printf("wrong syntax\n");
                        freeCmd();
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