#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;

extern int optind;

struct arg_struct{
    uchar op_all;
    uchar op_long;
    uchar op_inode;
    uchar op_numeric;
    uchar op_rec;
};

int print_dir(char* name, struct arg_struct opts);
int print_file(char* name, struct arg_struct opts);
int read_dir(DIR* dir, struct dirent** file);

int main(int argc, char* argv[]){
    struct option opts[6] = {{ .name = "all", .has_arg = 0, .flag = NULL, .val = 'a'},
                             { .name = "long", .has_arg = 0, .flag = NULL, .val = 'l'},
                             { .name = "inode", .has_arg = 0, .flag = NULL, .val = 'i'},
                             { .name = "numeric-uid-gid", .has_arg = 0, .flag = NULL, .val = 'n'},
                             { .name = "recursive", .has_arg = 0, .flag = NULL, .val = 'R'},
                             {0}};

    struct arg_struct options = {0};
        

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
                      options.op_long = 1;
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

int print_file(char* name, struct arg_struct opts){
    struct stat statbuf;
    if (opts.op_inode) {
        lstat(name, &statbuf);
        if (errno) {
            perror("lstat error");
            return -1;
        }
        printf("%lu ", statbuf.st_ino);
    }
    if (opts.op_long) {
        lstat(name, &statbuf);
        if (errno) {
            perror("lstat error");
            return -1;
        }
        switch (S_IFMT & statbuf.st_mode) {
            case S_IFIFO:
                printf("p");
                break;
            case S_IFCHR:
                printf("c");
                break;
            case S_IFDIR:
                printf("d");
                break;
            case S_IFBLK:
                printf("b");
                break;
            case S_IFREG:
                printf("-");
                break;
            case S_IFLNK:
                printf("l");
                break;
            case S_IFSOCK:
                printf("s");
                break;
            default:
                printf("?");
        }

    for(int i = 8; i >= 0; i--){
        if (statbuf.st_mode & (1 << i)) {
            switch ((i + 1) % 3) {
                case 0: 
                    printf("r");
                    break;
                case 1: 
                    printf("x");
                    break;
                case 2:
                    printf("w");
                    break;
            }
        } else 
            printf("-");
    }

    printf(" %lu ", statbuf.st_nlink);
    
    if (!opts.op_numeric) {
        errno = 0;
        struct passwd* pass = getpwuid(statbuf.st_uid);
        if (!errno)
            printf("%s ", pass->pw_name);
        else printf("? ");
        errno = 0;
        struct group* gr = getgrgid(statbuf.st_gid);
        if (!errno)
            printf("%s ", gr->gr_name);
        else printf("? ");
    } else 
        printf("%d %d ", statbuf.st_uid, statbuf.st_gid);

    printf("%6lu ", statbuf.st_size);

    char time_str[15] = {0};
    if (strftime(time_str, 15, "%b %d %H:%M", gmtime(&statbuf.st_mtime)) == 0) //timestamps work somewhat strange
        printf("?");
    else
        printf("%s ", time_str);

    printf("%s ", name);

    } else {
        printf("%s ", name);
    }
    return 0;
}

int print_dir(char* name, struct arg_struct opts){
    int status = 0;
    DIR* cur_dir = opendir(name);
    struct dirent* file;
    if (cur_dir == NULL) {
        perror("uls (dir error)");
        printf("name: %s\n", name);
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
                        size_t len = strlen(name);
                        unsigned int name_len = len + strlen(file -> d_name) + 2;
                        char endchar = name[len - 1];
                        if (endchar != '/') { 
                            name_len++;
                        }
                        char full_dir[name_len];
                        full_dir[0] = 0;
                        strcat(full_dir, name);
                        if (endchar != '/') {
                            full_dir[len] = '/'; 
                            full_dir[len + 1] = 0; 
                        }
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

