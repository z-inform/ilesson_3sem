#include "stdio.h"
#include "unistd.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 4096

int fdcopy(int src, int dest);
int readfd(int src, char* buf, int size);
int writefd(int dst, char* buf, int size); 

int main(int argc, char* argv[]){
    int src = 0;
    int close_err;
    char* file_err_msg = "File error";
    char* close_err_msg = "File close error";

    if( argc == 1 ){
        fdcopy(0,0);
    }
    else{
        for(int i = 1; i < argc; i++){
            src = open(argv[i], O_RDONLY);
            if( src == -1 ){
                perror(file_err_msg);
                return -1;
            }
            fdcopy(src, 0);
            close_err = close(src);
            if( close_err == -1 ){
                perror(close_err_msg);
                return -1;
            }
        }
    }


    return 0;
}

int readfd(int src, char* buf, int size){
    int read_bytes;
    char* read_err_msg = "Read error";

    read_bytes = read(src, buf, size);

    switch( read_bytes ){
    case 0: 
        return 0;
    case -1:
        perror(read_err_msg);
        return -1;
    }

    return read_bytes;
}

int writefd(int dst, char* buf, int size){
    int write_bytes = 0;
    int offset = 0;
    char* write_err_msg = "Write error";

    while( offset < size ){
        write_bytes = write(dst, buf + offset, size);

        if( write_bytes == -1 ){
            perror(write_err_msg);
            return -1;
        }

        offset += write_bytes;
    }

    return 0;
}

int fdcopy(int src, int dst){
    int read_err = 0;
    int write_err = 0;

    char buf[BUFSIZE];

    read_err = readfd(src, buf, BUFSIZE);

    while( read_err != 0 ){
        if( read_err == -1 ) 
            return -1;
        write_err = writefd(dst, buf, read_err);
        if( write_err == -1 ) 
            return -1;
        read_err = readfd(src, buf, BUFSIZE);
    }

    return 0;
}



