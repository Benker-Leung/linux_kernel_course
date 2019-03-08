#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

int open(const char *pathname, int flags);

typedef struct Command_ {
    int argument_size;
    char* argument_list;
    char* input_file;
    char* output_file;
} Command;


int main() {


    Command c = (Command){.argument_size=10, .argument_list=NULL, .input_file=NULL, .output_file=NULL};
    printf("Command size:%d", c.argument_size);
    // char command[1][4] = {"cat"};

    // char *a1 = "cat";
    // char *a2 = "input.txt";

    // char ***argument = calloc(1, sizeof(char**));
    // argument[0] = calloc(3, sizeof(char*));
    // argument[0][0] = a1;
    // argument[0][1] = a2;
    // argument[0][2] = NULL;

    // execvp(command[0], argument[0]);



    /*============================= command < file =============================*/
    
    // FILE* fp = fopen("output.txt", "r");
    // int f = fileno(fp);


    // if(fp == NULL) {
    //     printf("No such file\n");
    //     exit(-1);
    // }

    // pid_t pid = fork();

    // if(pid == 0) {
    //     // child
    //     close(0);
    //     dup2(f, 0);
    //     execlp("cat", "cat", NULL);
    // }
    // else {
    //     // parent
    //     wait(0);
    // }
    




    /*============================= command > file =============================*/
    // char file[100] = "/home/test/Documents/COMP4511/Assignment_3/input.txt";
    // printf("Remove result:%d\n", remove(file));


    // FILE* fp = fopen(file, "wr");
    // int f = fileno(fp);

    // pid_t pid = fork();

    // if(pid == 0) {
    //     close(1);
    //     dup2(f, 1);
    //     execlp("ls", "ls", NULL);
    // }
    // else {
    //     wait(0);
    //     printf("Done\n");

    // }
    // fclose(fp);

    return 0;
}