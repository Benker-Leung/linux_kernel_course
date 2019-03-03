#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

int main(){

    char cc[] = "a -b |grep -shit \"so great\"";

    char *c = (char*)malloc(strlen(cc) * sizeof(char) +1);
    strcpy(c, cc);

    char *temp;
    temp = c;

    int arr[10];
    int count = 0;

    int command = 0;
    int keep = 0;
    int sawspace = 1;
    int len = strlen(c);
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
                arr[count++] = i;
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
                    arr[count++] = i;
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
                    arr[count++] = i;
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

    for(int i=0; i<4; i++){
        printf("%s\n", &c[arr[i]]);
    }


    return 0;
}