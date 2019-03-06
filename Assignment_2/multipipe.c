#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

int main() {

        char *command[2] = {"ls", "sort"};
        char *argument1[2] = {"ls", NULL};
        char *argument2[3] = {"sort", NULL, NULL};


        int command_count = 2;
        int count = 0;
        pid_t pid;

        int fds[2];
        pipe(fds);

        while(command_count != count) {
            pid = fork();
            if (pid == 0) {
                // child 

                // if first one and also more than 1 command, just write without read
                if(count == 0 && command_count != 1) {
                    close(fds[0]);      // close read
                    close(1);           // close stdout
                    dup2(fds[1], 1);    // dup to stdout
                    fprintf(stderr, "1 executing %s\n", command[count]);
                    int k = 0;
                    while(argument1[k] != NULL) {
                        fprintf(stderr, "arg:%s\n", argument1[k]);
                        ++k;
                    }
                    execvp(command[count], argument1);
                }
                // else it does read and write to stdout
                else if (count == (command_count - 1)) {
                    close(fds[1]);      // close write
                    close(0);           // close stdin
                    dup2(fds[0], 0);    // dup to stdin
                    fprintf(stderr, "2 executing %s\n", command[count]);
                    int k = 0;
                    while(argument2[k] != NULL) {
                        fprintf(stderr, "arg:%s\n", argument2[k]);
                        ++k;
                    }
                    execvp(command[count], argument2);
                }
                // else it does read and write to pipe
                else {
                    // read
                    close(0);           // close stdin
                    dup2(fds[0], 0);    // dup to stdin
                    
                    // write
                    close(1);           // close stdout
                    dup2(fds[1], 1);    // dup to stdout
                    fprintf(stderr, "3 executing %s\n", command[count]);
                    execvp(command[count], &argument1[count]);
                }
            }
            else {
                // parent
                wait(0);
                fprintf(stderr, "Parent%d\n", count);
                ++count;
            }
        }

    return 0;
}