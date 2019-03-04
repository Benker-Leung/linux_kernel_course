#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>

extern char* get_current_dir_name(void);
extern char *dirname(char *path);
extern char *basename(char *path);
extern char *strdup(const char *s);

int main() {

    char *directory;
    directory = (char *)(get_current_dir_name());
    printf("dir:%s\n", directory);
    
    
    char *base_ = strdup(directory);
    char *dirc_ = strdup(directory);

    char *base;
    base = basename(base_);
    printf("base:%s\n", base);

    char *dirc;
    dirc = dirname(dirc_);
    printf("dirc:%s\n", dirc);

    printf("dir:%s\n", directory);

    free(base_);
    free(dirc_);
    free(directory);
    return 0;
}