#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

int main() {

    int i = 2;
    int count = 0;
    pid_t pid;

    char buf[3];
    strcpy(buf, "hi");

    int fds[2];
    pipe(fds);

    while(1) {
        pid = fork();
        --i;
        if(pid == 0) {
            // first time fork
            if(count == 0) {
                // child write to pipe
                printf("%d:Child writing buf:%s\n", count, buf);
                close(fds[0]);
                write(fds[1], buf, 2);
                exit(0);
            }
            else {
                // child read from pipe
                read(fds[0], buf, 3);
                printf("%d:Child reading buf:%s\n", count, buf);
                // child write to pipe
                printf("%d:Child writing buf:%s\n", count, buf);
                write(fds[1], buf, 3);
                exit(0);
            }
        }
        else {
            if(i==0) {
                // parent wait for child
                wait(0);
                // parent read from pipe
                printf("%d:Parent reading buf:%s\n", count, buf);
                read(fds[0], buf, 3);
                // parent write to stdout                
                // break;
                printf("%d:Parent final read:%s\n", count, buf);
                break;
            }
            else {
                // parent wait for child
                wait(0);
                // parent read from pipe
                printf("%d:Parent reading buf:%s\n", count, buf);
                read(fds[0], buf, 3);
                // parent write to pipe
                printf("%d:Parent writing buf:%s\n", count, buf);
                write(fds[1], buf, 3);
            }
        }
        ++count;
    }


    return 0;
}