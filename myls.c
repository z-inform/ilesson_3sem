#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int optind;

int print_dir(char* name, char* opts);
int print_file(char* name, char* opts);

int main(int argc, char* argv[]){
    char options[5] = {0}; //alinR
    struct option opts[6] = {{ .name = "all", .has_arg = 0, .flag = NULL, .val = 'a'},
                             { .name = "long", .has_arg = 0, .flag = NULL, .val = 'l'},
                             { .name = "inode", .has_arg = 0, .flag = NULL, .val = 'i'},
                             { .name = "numeric-uid-gid", .has_arg = 0, .flag = NULL, .val = 'n'},
                             { .name = "recursive", .has_arg = 0, .flag = NULL, .val = 'R'},
                             {0}};

    int status;
    do{
        status = getopt_long(argc, argv, "alinR", opts, NULL);
        switch (status) {
            case 'a': options[0] = 1;
                      break;
            case 'l': options[1] = 1;
                      break;
            case 'i': options[2] = 1;
                      break;
            case 'n': options[3] = 1;
                      break;
            case 'R': options[4] = 1;
                      break;
        }
    }while(status != -1);

    if (optind == argc) {
        print_dir("./", options);
    } else {
        print_dir(argv[optind], options);
    }
    
    return 0;
}

int print_file(char* name, char* opts){
    struct stat statbuf;
    lstat(name, &statbuf);
    printf("%s ", name);
    return 0;
}

int print_dir(char* name, char* opts){
    DIR* cur_dir = opendir(name);
    if (cur_dir == NULL) {
        perror("uls (dir error)");
        return -1;
    }
    while(1){
        errno = 0;
        struct dirent* file = readdir(cur_dir);
        if (file == NULL) {
            if (errno != 0) {
                perror("uls (file error)");
                return -1;
            } else {
                break;
            }
        } 
        if (opts[4] && (file -> d_type == DT_DIR)) {
            if (!(!strcmp(file -> d_name, ".") || !strcmp(file -> d_name, "..")))
                print_dir(file -> d_name, opts);
        }
        print_file(file -> d_name, opts);
        printf("\n");
    }
    return 0;
}

