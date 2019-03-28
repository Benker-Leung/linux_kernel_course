#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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

void printUsage(char* cmd) {
    fprintf(stderr, "Usage: %s\t[flags] outfile infile infile2 ...\n", cmd);
    fprintf(stderr, "\t[-a] O_APPEND flag attach\n");
    fprintf(stderr, "\t[-c] O_CREAT flag attach\n");
    fprintf(stderr, "\t[-m ???] change file mode bit\n");
    return;
}

int main(int argc, char* argv[]) {

    if(argc < 3) {
        printUsage(argv[0]);
        exit(-1);
    }

    long res = -1;              // used to store the return value
    int opt = 0;                // used for getopt to temp store the option char
    int startOpt = 0;           // all files starting from here (index of argv)
    int i = 0;                  // for counting
    int temp = 0;               // for temp usage
    int defaultOflags = 1;      // assume use default oflags, if not set it to 0
    int defaultMode = 1;        // assume use default mode, if not set it to 0
    struct xmerge_param xp;     // use to store the param that used by kernel space

    // set the xmerge_param default values
    xp.oflags = 0;
    xp.mode = 0;
    xp.num_files = 0;
    xp.ofile_count = calloc(1, sizeof(int));
    *xp.ofile_count = 0;

    // parse the command
    while((opt=getopt(argc, argv, "acm")) != -1) {
        switch(opt){
            case 'a':       // append case
                // printf("append mode accepted\n");
                defaultOflags = 0;
                xp.oflags |= O_APPEND;
                break;
            case 'c':       // create case
                // printf("creat mode accepted\n");
                defaultOflags = 0;
                xp.oflags |= O_CREAT;
                break;
            case 'm':       // change mode case
                temp = atoi(argv[optind]);  // get the mode in base 10

                for(i=0; i<3; ++i) {
                    defaultMode = temp%10;
                    if(defaultMode > 7) {
                        fprintf(stderr, "Error mode bit detected!\n");
                        exit(-1);
                    }
                    defaultMode <<= (i*3);
                    xp.mode |= defaultMode;
                    temp /= 10;
                }
                defaultMode = 0;        // set to 0 indicate not to use default mode
                startOpt = optind + 1;
                // printf("change mode of file to [%d](in integer base 10)\n", xp.mode);
                break;
            default:
                printUsage(argv[0]);
                exit(-1);
        }
    }

    // set default oflags
    if(defaultOflags) {
        // printf("use default oflags\n");
        xp.oflags = O_CREAT | O_WRONLY | O_APPEND;
    }
    // set default mode
    if(defaultMode) {
        // printf("user default mode\n");
        xp.mode = S_IRUSR | S_IWUSR;
    }

    // set the correct startOpt
    startOpt = startOpt==0?optind:startOpt;
    // if beginning at startOpt(included), less than two argv, then quit
    if(startOpt+1 >= argc) {
        printf("Wrong length of command\n");
        printUsage(argv[0]);
        exit(-1);
    }

    // set the output file pointer
    xp.outfile = argv[startOpt++];
    // allocate memory to char* array
    xp.infiles = calloc(argc-startOpt, sizeof(char*));
    i=0;
    while(startOpt<argc) {
        xp.infiles[i++] = argv[startOpt++];
        xp.num_files++;
    }

    res = syscall(SYSCALL_NUM_XMERGE, &xp, sizeof(struct xmerge_param));
    

    if(res < 0) {
        if(errno == EFAULT)
            printf("[xmerge error]: Error when copying memory between user-space and kernel space\n");
        else
            printf("[xmerge error]: File operation problems\n");
        exit(-1);
    }
    
    // print syscall result
    printf("In total, %d files have been successfully merged!\n", *xp.ofile_count);
    printf("The total read size: %d bytes.\n", res);


    free(xp.infiles);
    free(xp.ofile_count);
    return 0;
}
