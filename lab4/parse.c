#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

int main(){

    char cc[] = "ls -la | grep -e \"-lab\"";    // pretend input


    char *c = cc;

    char *temp = c; // used in checking - 


    char *arg[10];

    
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
                                return 0;
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
    arg[count++] = NULL;    // attach NULL to the last one

    for(int i=0, j=0; i<count; i++){
        if(arg[i] == NULL){
            printf("\n");
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
    
    // Add your testing code below


    return 0;
}