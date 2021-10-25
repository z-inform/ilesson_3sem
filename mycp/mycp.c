#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#define BUFSIZE 4096

int fdcopy(int src, int dest);
int readfd(int src, char* buf, int size);
int writefd(int dst, char* buf, int size); 
int f2fcp(char* src, char* dst, int verbose, int force, int interactive);

extern int optind;


int main(int argc, char* argv[]){
    int verbose = 0;
    int force = 0;
    int interactive = 0;
    struct option opts[4] = {{ .name = "verbose", .has_arg = 0, .flag = NULL, .val = 'v' },
                             { .name = "force", .has_arg = 0, .flag = NULL, .val = 'f' },
                             { .name = "interactive", .has_arg = 0, .flag = NULL, .val = 'i' },
                             {0}};
    struct stat filestat;

    int status = getopt_long(argc, argv, "vfi", opts, NULL); 
    while( status != -1 ){
        switch (status){
            case 'v': verbose = 1;
                      break;;
            case 'f': force = 1;
                      break;;
            case 'i': interactive = 1;
                      break;;
        }
        status = getopt_long(argc, argv, "vfi", opts, NULL); 
    }


    if( (argc - optind) > 2 ){
        stat(argv[argc - 1], &filestat);
        if( S_ISDIR(filestat.st_mode) != 0 ){
            printf("%s is not a directory\n", argv[argc - 1]);
            return -1;
        }
        for(int i = optind; i < argc - 2; i++){
            //copy to dir function
        }
    }
    else if( (argc - optind) == 2 ){
        return f2fcp(argv[optind], argv[optind + 1], verbose, force, interactive);
    }
    else{
        printf("mycp: missing file operand\n");
        return -1;
    }

    return 0;
}

int f2fcp(char* src, char* dst, int verbose, int force, int interactive){
    struct stat filestat;
    char* fulldst = NULL;


    if( (stat(dst, &filestat) != -1) && (S_ISDIR(filestat.st_mode) == 1) ){
        fulldst = (char*) malloc(strlen(dst) + strlen(src) + 2);
        if( fulldst == NULL ){
            perror("Malloc error");
            return -1;
        }
        strcat(fulldst, dst);
        strcat(fulldst, "/");
        strcat(fulldst, src);
    }
    else{
        fulldst = (char*) malloc(strlen(dst));
        if( fulldst == NULL ){
            perror("Malloc error");
            return -1;
        }
        strcat(fulldst, dst);
    }

    if( stat(fulldst, &filestat) == 0 ){
        if( interactive ){ //prompt before overwrite
            char conf;
            printf("mycp: overwrite '%s'? ", fulldst);
            scanf("%c", &conf);
            if( (conf == 'n') || (conf == 'N') )
                return 1;
        }
        if( force ){ //check if file can be opened and delete if, if can not
            int fd = open(fulldst, O_RDWR | O_APPEND);
            if( fd == -1 ){
                if( unlink(fulldst) == -1 ){
                    perror("mycp: unlink error");
                    return -1;
                }
            }
            close(fd);
        }
    }


    if( verbose )
        printf("%s -> %s\n", src, fulldst);

    int dstfd = open(fulldst, O_WRONLY | O_CREAT, 0664);
    if( dstfd == -1 ){
        perror("mycp: destination file open error");
        return -1;
    }

    int srcfd = open(src, O_RDONLY);
    if( srcfd == -1 ){
        perror("mycp: source file open error");
        return -1;
    }

    int status = fdcopy(srcfd, dstfd);
    close(srcfd);
    close(dstfd);

    return status;
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



