#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

int main(){
    char **cmd;
    char ***args;
    char cmdline[] = " ls -la | grep l";
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
    for(int i=0, temp=0, cc=0; ; i++){
        // add argument to next
        if (cmdline[i] == '\0' || cmdline[i] == '|') {
            args[cc] = malloc(sizeof(char*) * temp);
            args[cc][temp-1] = NULL;
            ++cc;
            temp = 0;
            
            if (cmdline[i] == '\0') {
                break;
            }
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
        }
        // if prev is space and n, then 
        else if (prev == ' ' && cmdline[i] != ' ') {
            // add the pointer of this command
            if(flag == 'n'){
                cmd[j] = (char*)(&cmdline[i]);
                flag = 'y';
                prev = cmdline[i];
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


    for(int i=0; i<cmdcount; ++i){
        printf("String %d:%s\n", i, cmd[i]);
        for(int j=0; args[i][j] != NULL; j++){
            printf("arg: %s\n", args[i][j]);
        }
    }


    pid_t pid = fork();

    if(pid == 0){
        printf("cmd:%s\nargs:%s\n", cmd[0], args[0][0]);
        execvp(cmd[0], args[0]);
    }
    else {
        wait(0);
    }


    for(int i=0; i<cmdcount; i++){
        free(args[i]);
    }
    free(args);
    free(cmd);
    
}