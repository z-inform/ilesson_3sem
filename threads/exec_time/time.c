#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>

//-q, --quiet - no stdout of child process
//output to stderr (child output to stdout)

extern int optind;

#define BUFSIZE 4096

int readfd(int src, char* buf, int size);
int writefd(int dst, char* buf, int size); 

int main(int argc, char* argv[]){
    struct timespec cur_time;
    struct timespec exec_time;
    struct option opts[2] = {{.name = "quiet", .has_arg = 0, .flag = NULL, .val = 'q'},
                             {0}};
    int quiet = 0;
    int pipefd[2];

    long long char_cnt = 0;
    long int word_cnt = 0;
    int line_cnt = 0;

    //pipe segment
    if( pipe(pipefd) == -1 ){
        perror("Pipe creation error");
        return -1;
    }

    //option processing
    int opt_status = getopt_long(argc, argv, "+q", opts, NULL);
    if( opt_status == 'q' ){
        quiet = 1;
    }

    if( (argc - optind) == 0 ){
        printf("No exetutable specified\n");
        return -1;
    }

    //fork
    int pid = fork();
    clock_gettime(CLOCK_MONOTONIC, &exec_time);
    
    if( pid == -1 ){
        perror("Fork error\n");
        return -1;
    } 

    else if( pid != 0 ){//parent
        close(pipefd[1]);
        
        char buffer[BUFSIZE];
        int read_status;
        int write_status;

        char in_word = 0;

        do{
            read_status = readfd(pipefd[0], buffer, BUFSIZE);
            if( read_status == -1 ){//read from pipe
                close(pipefd[0]);
                wait(NULL);
                return -1;
            }
            //count some things
            char_cnt += read_status;
            for(int i = 0; buffer[i] != 0; i++)
                if( buffer[i] == '\n' ) line_cnt++;
            
            for(int i = 0; buffer[i] != 0; i++){
                if( in_word && isspace(buffer[i]) ){
                    in_word = 0;
                    word_cnt++;
                }
                if( !in_word && !isspace(buffer[i]) ){
                    in_word = 1;
                }
            }




            if( !quiet ){//write child output to stdout
                write_status = writefd(1, buffer, read_status);
                if( write_status == -1 ){
                    close(pipefd[0]);
                    wait(NULL);
                    return -1;
                }
            }
        }
        while( read_status != 0 );

        wait(NULL);
    }

    else{//child
        if( dup2(pipefd[1], 1) == -1 ){
            perror("Fd duplication error");
            return -1;
        }
        close(pipefd[1]);//close write pipefd
        close(pipefd[0]);//close read pipefd

        char* exec_args[argc - (optind - 1)];//(optind - 1) = num of options for base 
        for(int i = optind; i < argc; i++){
            exec_args[i - optind] = argv[i];
        }
        exec_args[argc - optind] = NULL;
        execvp(argv[optind], exec_args);
        perror("Exec error\n");
        return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &cur_time);
    if( cur_time.tv_nsec < exec_time.tv_nsec ){
        cur_time.tv_sec--;
        cur_time.tv_nsec = cur_time.tv_nsec + 1e9 - exec_time.tv_nsec;
        exec_time.tv_nsec = 0;
    }
    
    dprintf(2, "Char count: %lld\n", char_cnt);
    dprintf(2, "Word count: %ld\n", word_cnt);
    dprintf(2, "Line count: %d\n", line_cnt);
    dprintf(2, "Exec time: %ld.%09lds\n", cur_time.tv_sec - exec_time.tv_sec, cur_time.tv_nsec - exec_time.tv_nsec);


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


