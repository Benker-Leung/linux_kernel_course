#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SYSCALL_NUM_XMERGE 356
extern int optind;

struct xmerge_param {

    char *outfile;
    char **infiles;
    unsigned int num_files;
    int oflags;
    mode_t mode;
    int *ofile_count;

};

int main(int argc, char* argv[]) {

    if(argc < 3) {
        fprintf(stderr, "Usage: %s [-a] [-c] [-m bits] outfile infile1 [infile2...]\n", argv[0]);
        exit(-1);
    }

    int opt = 0;
    int startOpt = 0;           // all files starting from here
    int i = 0;
    int temp = 0;
    struct xmerge_param xp;
    xp.oflags = 0;
    xp.mode = 0;
    xp.num_files = 0;

    // parse the command
    while((opt=getopt(argc, argv, "acm")) != -1) {
        switch(opt){
            case 'a':
                // printf("got a\n");
                xp.oflags |= O_APPEND;
                break;
            case 'c':
                // printf("got c\n");
                xp.oflags |= O_CREAT;
                break;
            case 'm':
                // printf("got m\n");
                temp = atoi(argv[optind]);
                printf("temp:%d\n", temp);
                i = temp/100; temp -= i*100;
                xp.mode |= i;
                xp.mode <<= 3;
                i = temp/10; temp -= i*10;
                xp.mode |= i; 
                i = (temp%10);
                xp.mode <<= 3;
                xp.mode |= i;
                startOpt = optind + 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-a] [-c] [-m bits] outfile infile1 [infile2...]\n", argv[0]);
                exit(-1);
        }
    }

    if(xp.oflags == 0) {
        xp.oflags = O_CREAT | O_WRONLY | O_APPEND;
    }
    if(xp.mode == 0) {
        xp.mode = S_IRUSR | S_IWUSR;
    }

    startOpt = startOpt==0?optind:startOpt;
    printf("startOpt:[%d]\n", startOpt);

    if(startOpt+1 >= argc) {
        printf("Wrong length of command\n");
        fprintf(stderr, "Usage: %s [-a] [-c] [-m bits] outfile infile1 [infile2...]\n", argv[0]);
        exit(-1);
    }

    xp.outfile = argv[startOpt++];
    xp.infiles = calloc(argc-startOpt, sizeof(char*));
    i=0;
    while(startOpt<argc) {
        xp.infiles[i++] = argv[startOpt++];
        xp.num_files++;
    }

    i = syscall(SYSCALL_NUM_XMERGE, &xp, sizeof(struct xmerge_param));

    printf("return:[%d]\n", i);


    return 0;
}