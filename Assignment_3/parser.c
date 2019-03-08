#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>


char** command = NULL;     // pointer to command locations
char*** argument = NULL;   // pointer to argument locations
int* argument_count = NULL;     // pointer to argument number
int command_count = 1;  // total command

int parser(char *cmdline);
void freeCmd();


int DEFAULT_ARUGMENT_ELE = 10;    // default argument element size

// default have 10 argument, including cmd itself
typedef struct Command_ {

    char* command;              // pointer to command
    
    int used;                   // space used
    int left;                   // left pointers in argument_list, default 10
    char **argument_list;       // array of pointer to arguments

    char* output_file;          // output file if any
    char* input_file;           // input file if any

    struct Command_* next;      // pointing to next command

} CMD;
// init 
//(CMD){.command=NULL, .used=0, .left=DEFAULT_ARUGMENT_ELE, .argument_list=malloc(sizeof(char*)*DEFAULT_ARUGMENT_ELE), .output_file=NULL, .input_file=NULL, .next=NULL};


CMD* newparser(char* cmdline) {

    char* c = cmdline;

    CMD *cmd = (CMD*)malloc(sizeof(CMD));
    *cmd = (CMD){.command=NULL, .used=0, .left=DEFAULT_ARUGMENT_ELE, .argument_list=malloc(sizeof(char*)*DEFAULT_ARUGMENT_ELE), .output_file=NULL, .input_file=NULL, .next=NULL};

    CMD *curr = cmd;

    int len = strlen(cmdline);
    int i;
    int prev_space = 1;
    char tempChar;
    char* tempResult;
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
                curr->argument_list[(curr->used)++] = c+i;
                --(curr->left);
                i = tempResult-c+1;
                prev_space = 0;
                break;

            // next cmd
            case '|':
                c[i] = '\0';
                curr->next = malloc(sizeof(CMD));
                *(curr->next) = (CMD){.command=NULL, .used=0, .left=DEFAULT_ARUGMENT_ELE, .argument_list=malloc(sizeof(char*)*DEFAULT_ARUGMENT_ELE), .output_file=NULL, .input_file=NULL, .next=NULL};
                curr = curr->next;
                prev_space = 1;
                break;

            // TODO
            case '>':
            case '<':

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
                    curr->argument_list[(curr->used)++] = c+i;
                    --(curr->left);
                }
                // argument
                else {
                    curr->argument_list[curr->used++] = c+i;
                    --(curr->left);
                }
                break;

        }

    }
    return cmd;
}


int main() {

    char cmdline[100] = "cat input.txt|grep '\"haha hehe\"   '";


    
    CMD*cmd = newparser(cmdline);

    while(cmd != NULL) {

        printf("Cmd:%s\n", cmd->command);
        int i;
        for(i=0; i<cmd->used; ++i) {
            printf("\tArg:%s\n", cmd->argument_list[i]);
        }
        printf("\n");
        cmd = cmd->next;
    }

    // parser(cmdline);
    
    // int i;
    // int j;
    // printf("%d\n", command_count);
    // printf("%s\n", cmdline);
    // for(i=0; i<command_count; ++i) {
    //     printf("Cmd[%d]:%s\n", i, command[i]);
    //     for(j=0; j<argument_count[i]; ++j) {
    //         printf("Arg[%d][%d]:%s\n", i, j, argument[i][j]);
    //     }
    //     printf("\n");
    // }

    return 0;
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