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


    char cmd[20] = "cd  ";

    char *temp_path = cmd+2;

    // get the first non space and non \t
    while(*temp_path != '\0' && !(*temp_path != ' ' && *temp_path != '\t')) {
        ++temp_path;
    }

    if(strlen(temp_path) == 0) {
        cmd[0] = '~';
        cmd[1] = '\0';
        temp_path = cmd;
    }

    const char*p = getenv("HOME");

    printf("temp_path:\"%s\"\n", p);

    char *directory;
    directory = (char *)(get_current_dir_name());
    printf("dir:%s\n", directory);
    
    
    char *base_ = strdup(directory);

    char *base;
    base = basename(base_);
    printf("base:%s\n", base);

    free(directory);
    chdir(temp_path);
    directory = (char *)(get_current_dir_name());
    printf("dir:%s\n", directory);


    free(base_);
    free(directory);
    return 0;
}