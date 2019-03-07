#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

int open(const char *pathname, int flags);


int main() {

    int pds[2];
    pipe(pds);

    FILE* fp = fopen("msg.txt", "r");
    int f = fileno(fp);

    pid_t pid = fork();

    if(pid == 0) {
        close(0);
        dup2(f, 0);
        execlp("cat", "cat", NULL);
    }
    else {
        wait(0);
        printf("Done\n");

    }
    fclose(fp);

    return 0;
}