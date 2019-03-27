#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 256

int append_file(char* infile, char* outfile) {

    char buf[BUFSIZE];
    int writeChar = 0;
    int readTemp = 0;
    int fin = open(infile, O_RDONLY);
    int fout = open(outfile, O_APPEND | O_WRONLY);
    int status = 0;
    if(fout <= 0 || fin <= 0) {
        printf("fin:[%d], fout:[%d]\n", fin, fout);
        close(fin);
        close(fout);
        return -1;
    }
    memset(buf, 0, BUFSIZE);
    while((readTemp = read(fin, buf, BUFSIZE)) > 0) {
        status = write(fout, buf, readTemp);
        if(status < 0) {
            printf("error during write\n");
            return -1;
        }
        writeChar += status;
        memset(buf, 0, BUFSIZE);
    }

    close(fin);
    close(fout);
    return writeChar;
}

int read_file(char* infile) {

    char buf[BUFSIZE];
    int readChar = 0;
    int readTemp = 0;
    int fin = open(infile, O_RDONLY);

    // check if fail to open or not
    if(fin < 0) {
        printf("open infile fail [%d]\n", fin);
        return -1;
    }

    memset(buf, 0, BUFSIZE);
    printf("========================================\n:");
    while((readTemp = read(fin, buf, BUFSIZE)) > 0) {
        printf("%s", buf);
        readChar += readTemp;
        memset(buf, 0, BUFSIZE);
    } 
    printf(":\n========================================\n");
    close(fin);
    return readChar;
}

int main() {

    // int readByte = read_file("t");
    // printf("Read byte: [%d]\n", readByte);

    int writeByte = append_file("t", "tt");
    printf("Appended byte: [%d]\n", writeByte);


    return 0;
}


