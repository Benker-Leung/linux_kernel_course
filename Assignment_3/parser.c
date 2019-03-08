#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>


int DEFAULT_ARUGMENT_ELE = 5;    // default argument element size, at least one

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

int commandCount(CMD *p) {
    int i = 0;
    while(p != NULL) {
        ++i;
        p = p->next;
    }
    return i;
}

void initCMD(CMD *p) {
    *p = (CMD){.command=NULL, .used=0, .left=DEFAULT_ARUGMENT_ELE-1, .argument_list=calloc(DEFAULT_ARUGMENT_ELE, sizeof(char*)), .output_file=NULL, .input_file=NULL, .next=NULL};
}

void autoFillArg(CMD *p) {
    if(p->left == 0) {
        p->left = DEFAULT_ARUGMENT_ELE;
        p->argument_list = realloc(p->argument_list, p->used+1+DEFAULT_ARUGMENT_ELE);
        return;
    }
}

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


CMD* newparser(char* cmdline) {

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


int main() {

    char cmdline[100] = "./test < input.txt | head -n 3 | wc -l > output.txt";


    
    CMD*cmd = newparser(cmdline);

    while(cmd != NULL) {

        printf("Cmd: [%s]\n", cmd->command);
        if(cmd->input_file != NULL) {
            printf("\tIn: [%s]\n", cmd->input_file);
        }
        if(cmd->output_file != NULL) {
            printf("\tOut: [%s]\n", cmd->output_file);
        }
        int i;
        for(i=0; i<cmd->used; ++i) {
            printf("\tArg: [%s]\n", cmd->argument_list[i]);
        }

        printf("\t\tused arg: [%d], left arg: [%d]\n", cmd->used, cmd->left);
        printf("\n");
        cmd = cmd->next;
    }

    freeCMD(cmd);

    return 0;
}
