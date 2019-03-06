#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>


char** command = NULL;     // pointer to command locations
char*** argument = NULL;   // pointer to argument locations
int* argument_count = NULL;     // pointer to argument number
int command_count = 1;  // total command


void freeMem() {
    // deallocation of allocated memory
    int i;

    // free argument
    if(argument != NULL) {
        for(i=0; i<command_count; ++i) {
            free(argument[i]);
        }
        free(argument);
    }
    // free command
    if(command != NULL)
        free(command);
    
    // free argument_count
    if(argument_count != NULL)
        free(argument_count);

}

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


int main(int argc, char* argv[]) {



    char cmdline[100] = "cat myshell.c haha";
    parser(cmdline);

    int i;
    int j;

    printf("Total command: %d\n", command_count);

    for(i=0; i<command_count; ++i) {
        printf("Command%d:%s\n", i, command[i]);
    }
    printf("\n");
    for(i=0; i<command_count; ++i) {
        for(j=0; j<argument_count[i]; ++j) {
            printf("Arg[%d][%d]: %s\n", i, j, argument[i][j]);
        }
        printf("\n");
    }    

    freeMem();

    return 0;
}