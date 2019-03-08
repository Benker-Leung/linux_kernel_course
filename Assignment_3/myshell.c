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

// default have 10 argument, including cmd itself
typedef struct Command_ {

    char* command;              // pointer to command
    
    int used;                   // index to the top arugment
    int left;                   // left pointers in argument_list, default 10, NULL occupy 1
    char **argument_list;       // array of pointer to arguments

    char* output_file;          // output file if any
    char* input_file;           // input file if any

    struct Command_* next;      // pointing to next command

} CMD;


void freeDir();     // free memory allocated to store directory
void freeCMD(CMD *p);     // free memory on linked list of cmd
void autoFillArg(CMD *p);   // auto add memory when required for arg list
void initCMD(CMD *p);       // init cmd object
int commandCount(CMD *p);   // count # of commands
CMD* parser(char* cmdline); // return link list
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


int DEFAULT_ARUGMENT_ELE = 5;    // default argument element size, at least one



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


        CMD *cmd = parser(cmdline);

        int i;

        int count = 0;
        pid_t pid;

        // create enough pipe
        int **fds;
        int command_count = commandCount(cmd);
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
                    char *temp_cmd = calloc(strlen(cmd->command)+strlen(directory)+2, sizeof(char));
                    strcpy(temp_cmd, directory);
                    if(strlen(directory) != 1)
                        *(temp_cmd+strlen(directory)) = '/';
                    strcpy(temp_cmd+strlen(directory)+1, cmd->command);
                    execv(temp_cmd, cmd->argument_list);
                    
                    // if curr dir has not such file, try run it from $PATH, and free temp
                    free(temp_cmd);

                    // fprintf(stderr, "1 executing %s\n", command[count]);
                    execvp(cmd->command, cmd->argument_list);
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
                    char *temp_cmd = calloc(strlen(cmd->command)+strlen(directory)+2, sizeof(char));
                    strcpy(temp_cmd, directory);
                    if(strlen(directory) != 1)
                        *(temp_cmd+strlen(directory)) = '/';
                    strcpy(temp_cmd+strlen(directory)+1, cmd->command);
                    execv(temp_cmd, cmd->argument_list);
                    
                    // if curr dir has not such file, try run it from $PATH, and free temp
                    free(temp_cmd);

                    // fprintf(stderr, "2 executing %d, %s\n", count, command[count]);
                    execvp(cmd->command, cmd->argument_list);
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
                    char *temp_cmd = calloc(strlen(cmd->command)+strlen(directory)+2, sizeof(char));
                    strcpy(temp_cmd, directory);
                    if(strlen(directory) != 1)
                        *(temp_cmd+strlen(directory)) = '/';
                    strcpy(temp_cmd+strlen(directory)+1, cmd->command);
                    execv(temp_cmd, cmd->argument_list);
                    
                    // if curr dir has not such file, try run it from $PATH, and free temp
                    free(temp_cmd);

                    // fprintf(stderr, "3 executing %s\n", command[count]);
                    execvp(cmd->command, cmd->argument_list);
                }
                fprintf(stderr, "myshell: command not found: %s\n", cmd->command);
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
                cmd = cmd->next;
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

// free cmd linked list
void freeCMD(CMD *p) {
    CMD *curr = p;
    CMD *temp = p;

    while(curr != NULL) {
        temp = curr;
        curr = curr->next;
        free(temp->argument_list);
        free(temp);
    }
}

// auto fill the memory when required
void autoFillArg(CMD *p) {
    if(p->left == 0) {
        p->left = DEFAULT_ARUGMENT_ELE;
        p->argument_list = realloc(p->argument_list, p->used+1+DEFAULT_ARUGMENT_ELE);
        return;
    }
}

// init CMD object
void initCMD(CMD *p) {
    *p = (CMD){.command=NULL, .used=0, .left=DEFAULT_ARUGMENT_ELE-1, .argument_list=calloc(DEFAULT_ARUGMENT_ELE, sizeof(char*)), .output_file=NULL, .input_file=NULL, .next=NULL};
}

// count the number of command
int commandCount(CMD *p) {
    int i = 0;
    while(p != NULL) {
        ++i;
        p = p->next;
    }
    return i;
}

// parse command and return linked list
CMD* parser(char* cmdline) {

    char* c = cmdline;

    CMD *cmd = (CMD*)malloc(sizeof(CMD));
    initCMD(cmd);

    CMD *curr = cmd;

    int len = strlen(cmdline);
    int i;
    int prev_space = 1;
    char tempChar;
    char* tempResult;
    int output = 0;
    int input = 0;
    for(i=0; i<len; ++i) {

        switch(c[i]) {

            // end of string case
            case '\0':
                return cmd;

            // continue string
            case '\'':
            case '"':
                tempChar = c[i];
                ++i;
                tempResult = strchr(c+i, tempChar);
                // not found, quit searching
                if(tempResult == NULL) {
                    return cmd;
                }
                *tempResult = '\0';
                autoFillArg(curr);
                curr->argument_list[(curr->used)++] = c+i;
                --(curr->left);
                i = tempResult-c+1;
                prev_space = 0;
                break;

            // next cmd
            case '|':
                c[i] = '\0';
                curr->next = malloc(sizeof(CMD));
                initCMD(curr->next);
                curr = curr->next;
                prev_space = 1;
                break;

            // TODO
            case '>':
                c[i] = '\0';
                output = 1;
                break;
            case '<':
                c[i] = '\0';
                input = 1;
                break;

            case ' ':
                c[i] = '\0';
                prev_space = 1;
                break;
            
            // char
            default:
                if(prev_space == 0)
                    break;
                prev_space = 0;

                // command
                if(curr->command == NULL) {
                    curr->command = c+i;
                    autoFillArg(curr);
                    curr->argument_list[(curr->used)++] = c+i;
                    --(curr->left);
                }
                // argument
                else {
                    if(input == 1) {
                        input = 0;
                        curr->input_file = c+i;
                        break;
                    }
                    else if(output == 1) {
                        output = 0;
                        curr->output_file = c+i;
                        break;
                    }
                    else {
                        autoFillArg(curr);
                        curr->argument_list[curr->used++] = c+i;
                        --(curr->left);
                    }
                }
                break;

        }

    }
    return cmd;
}