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
        

        char *c = cmdline;

        char *temp = c; // used in checking - 

        char *arg[10];

        int count = 0;
        int command = 0;    // used in determind command got or not
        int keep = 0;       // used for temp storage
        int sawspace = 1;   // used for check first char
        int len = strlen(c);// used to stop loop
        for(int i=0; i<len ;++i) {
            if(c[i] == '|') {
                command = 0;
                sawspace = 1;
                c[i] = '\0';
                continue;
            }
            else if(c[i] != ' ' && sawspace == 1){
                if(command == 0){
                    arg[count++] = NULL;// terminate the first one
                    arg[count++] = c+i;   // set command
                    arg[count++] = c+i;   // set command
                    command = 1;
                    sawspace = 0;
                    continue;
                }
                else{
                    // special case
                    if(c[i] == '"') {
                        // first saw "
                        c[i] = '\0';
                        arg[count++] = c+i+1;
                        temp = strchr(c+i+1, '"');
                        if(temp == NULL){
                            printf("wrong syntax\n");
                            return;
                        }
                        *temp = '\0';
                        i = (temp-c)+1;
                        sawspace = 0;
                        continue;
                    }
                    else{
                        arg[count++] = c+i;
                        sawspace = 0;
                        continue;
                    }
                }
            }
            else if(c[i] == ' '){
                sawspace = 1;
                c[i] = '\0';
            }
        }

        // for(int i=0; i<len; i++){
        //     printf("%d:%c.\n", i, c[i]);
        // }

        arg[count++] = NULL;    // attach NULL to the last one

        // initialize pointers
        char ***p;
        int size = 4;
        p = malloc(sizeof(char**) *2);
        for(int i=0; i<2; i++){
            p[i] = malloc(sizeof(char*) * size);
            for(int j=0; j<size; j++){
                p[i][j] = NULL;
            }
        }

        int cmd = -1;   //  check the command
        int argv = 0;   //  check the argv, iterative
        

        for(int i=0, j=0; i<count; i++){
            if(arg[i] == NULL){
                // printf("\n");
                j = 0;
                if(argv != 0){
                    p[cmd][argv] = NULL;
                }
                argv = 0;
                ++cmd;

            }
            // command
            else if(j==0){
                // printf("cmd:%s\n", arg[i]);
                j = 1;
            }
            else{
                // printf("arg:%s\n", arg[i]);
                p[cmd][argv++] = arg[i];
            }
        }

        
        // for(int i=0; i<2; i++){
        //     for(int j=0; j<size; j++){
        //         if(p[i][j] != NULL){
        //             printf("%d:arg: %s\n", j, p[i][j]);
        //         }else{
        //             printf("%d:NULL\n", j);
        //         }
        //     }
        // }
        
        

        // return;
        // ========================= Debug =======================

        int pfds[2];
        pipe(pfds);

        int stdin = dup(0);
        int stdout = dup(1);

        dup2(pfds[0], 0);

        pid_t pid= fork();
        
        if(pid == 0) {   // child
            close(1);   // close stdout
            dup2(pfds[1], 1);
            // printf("Calling:%s, with:%s, %s\n", p[0][0], p[0][0], p[0][1]);
            execvp(p[0][0], p[0]);
        }
        else {  //parent
            close(0);   // close stdin
            dup2(pfds[0], 0);
            close(pfds[1]);
            wait(0);

            pid_t temp = fork();

            if(temp==0) { // child
                // p[1][0] = "grep";
                // p[1][1] = "go here";
                // p[1][2] = "go here";
                // printf("Calling:%s, with:%s, %s\n", p[1][0], p[1][0], p[1][1]);
                execvp(p[1][0], p[1]);
            }
            else {  // parent
                wait(0);
                dup2(stdin, 0);
                dup2(stdout, 1);
            }        
        }

        // free
        // printf("free...\n");
        for(int i=0; i<2; i++){
            free(p[i]);
        }
        free(p);

    }
    // ==============================================================================
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