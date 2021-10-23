#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;

extern int optind;

struct keys{
    uchar op_all;
    uchar op_long;
    uchar op_inode;
    uchar op_numeric;
    uchar op_rec;
};

int print_dir(char* name, struct keys opts);
int print_file(char* name, struct keys opts);
int read_dir(DIR* dir, struct dirent** file);


int main(int argc, char* argv[]){
    struct option opts[6] = {{ .name = "all", .has_arg = 0, .flag = NULL, .val = 'a'},
                             { .name = "long", .has_arg = 0, .flag = NULL, .val = 'l'},
                             { .name = "inode", .has_arg = 0, .flag = NULL, .val = 'i'},
                             { .name = "numeric-uid-gid", .has_arg = 0, .flag = NULL, .val = 'n'},
                             { .name = "recursive", .has_arg = 0, .flag = NULL, .val = 'R'},
                             {0}};

    struct keys options = {0};
        

    int status;
    do{
        status = getopt_long(argc, argv, "alinR", opts, NULL);
        switch (status) {
            case 'a': options.op_all = 1;
                      break;
            case 'l': options.op_long = 1;
                      break;
            case 'i': options.op_inode = 1;
                      break;
            case 'n': options.op_numeric = 1;
                      break;
            case 'R': options.op_rec = 1;
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

int print_file(char* name, struct keys opts){
    struct stat statbuf;
    lstat(name, &statbuf);
    printf("%s ", name);
    return 0;
}

int print_dir(char* name, struct keys opts){
    int status = 0;
    DIR* cur_dir = opendir(name);
    struct dirent* file;
    if (cur_dir == NULL) {
        perror("uls (dir error)");
        return -1;
    }
    if (opts.op_rec) 
        printf("%s:\n", name);
    while(1){
        status = read_dir(cur_dir, &file);
        if (status != 0) break;
        if (file -> d_name[0] != '.' || opts.op_all) {
            print_file(file -> d_name, opts);
            printf("\n");
        }
    }
    if (opts.op_rec) {
        printf("\n");
        rewinddir(cur_dir);
        while(1){
            status = read_dir(cur_dir, &file);
            if (status != 0) break;
            if (file -> d_type == DT_DIR) {
                if (!(!strcmp(file -> d_name, ".") || !strcmp(file -> d_name, ".."))){
                    if (!(file -> d_name[0] == '.' && opts.op_all)){
                        unsigned int name_len = strlen(name) + strlen(file -> d_name) + 2;
                        char full_dir[name_len];
                        full_dir[0] = 0;
                        strcat(full_dir, name);
                        strcat(full_dir, file -> d_name);
                        full_dir[name_len - 2] = '/';
                        full_dir[name_len - 1] = 0;
                        print_dir(full_dir, opts);
                    }
                }
            }
        }
    }
    return status;
}

int read_dir(DIR* dir, struct dirent** file){
    errno = 0;
    *file = readdir(dir);
    if (*file == NULL) {
        if (errno != 0) {
            perror("file error");
            return -1;
        } else {
            return 1;
        }
    }
    return 0;
}

