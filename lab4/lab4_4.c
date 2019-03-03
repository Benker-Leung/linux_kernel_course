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

        char ***args;
        args = malloc(sizeof(char**) * 2);
        for (int i=0; i<2; i++){
            args[i] = malloc(sizeof(char*) * 10);
            for (int j=0; j<10; j++){
                args[i][j] = NULL;
            }
        }
        
        int second;
        int count = 0;
        int cmd2 = 0;
        int count_ = 0;




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
                    // printf("command addr:%d\n", i);

                    arg[count++] = NULL;// terminate the first one
                    second = count;
                    arg[count++] = c+i;   // set command
                    arg[count++] = c+i;   // set command
                    ++cmd2;
                    count_ = 0;
                    // arr[count++] = i;
                    command = 1;
                    sawspace = 0;
                    continue;
                }
                else{

                    //steps:
                    // 1: find the '\0' or space, if '\0' found, then stop
                    // next
                    // 2: (space found) next find '-', '"', '|', 'char', '\0', if '\0', stop
                    // next
                    // 3.1: (-| found), should create a new arg, and space before it should be '\0'
                    // 3.2: (" found), should get next ", set i be position at "
                    // 3.3: (c found), should get until next space


                    // if it is -, take special care
                    if(c[i] == '-') {

                        arg[count++] = c+i;
                        args[cmd2][count_++] = c+i;
                        printf("Heihei:%s\n", arg[count-1]);
                        // arr[count++] = i;
                        // printf("arg addr:%d\n", i);
                        keep = i;
                        // find '\0' or ' '
                        while(c[keep] != '\0' && c[keep] != ' ' && c[keep] != '|'){
                            ++keep;
                        }
                        // if null, then can end
                        if(c[keep] == '\0' || c[keep] == '|'){
                            i = keep - 1;
                            continue;
                        }
                        // if it is space
                        else {
                            // check next non-space
                            keep += 1;
                            while(c[keep] != '\0' && c[keep] != ' ' && c[keep] != '|' && c[keep] != '-' && c[keep] != '"'){
                                keep += 1;
                            }

                            // stop checking
                            if(c[keep] == '\0') {
                                break;
                            }
                            // start a new arg
                            else if(c[keep] == '-' || c[keep] == '|') {
                                if(c[keep-1] == ' ')
                                    c[keep-1] = '\0';
                                i = keep - 1;
                                continue;
                            }
                            // get until next "
                            else if(c[keep] == '"') {
                                temp = strchr(&c[keep+1], '"');
                                // wrong syntax
                                if(temp == NULL){
                                    printf("Stupid syntax\n");
                                    return;
                                }
                                i = temp-c;
                                continue;
                            }
                            // if simple char, get next ' '
                            else{
                                keep += 1;
                                while(c[keep] != ' ' && c[keep] != '\0' && c[keep] != '|'){
                                    ++keep;
                                }
                                if(c[keep] == '\0' || c[keep] == '|'){
                                    i = keep - 1;
                                    continue;
                                }
                                else{
                                    // this should set to be '\0'
                                    i = keep - 1;
                                    continue;
                                }
                            }

                        }

                    }
                    else{
                        arg[count++] = c+i;
                        args[cmd2][count_++] = c+i;
                        // arr[count++] = i;
                        // printf("argument addr:%d\n", i);
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

        for(int i=0, j=0; i<count; i++){
            if(arg[i] == NULL){
                printf("New command:\n");
                j = 0;
            }
            // command
            else if(j==0){
                printf("cmd:%s\n", arg[i]);
                j = 1;
            }
            else{
                printf("arg:%s\n", arg[i]);
            }
        }
        arg[count++] = NULL;    // attach NULL to the last one
        // return;
        int pfds[2];
        pipe(pfds);

        int stdin = dup(0);
        int stdout = dup(1);

        dup2(pfds[0], 0);

        pid_t pid= fork();
        
        if(pid == 0) {   // child
            close(1);
            dup2(pfds[1], 1);
            execvp(arg[1], args[0]);
        }
        else {  //parent
            close(0);   // close stdin
            dup2(pfds[0], 0);
            close(pfds[1]);
            wait(0);

            pid_t temp = fork();

            if(temp==0) { // child
                execvp(arg[second], args[1]);
            }
            else {  // parent
                wait(0);
                dup2(stdin, 0);
                dup2(stdout, 1);
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