#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    char* err_msg = "fork error";
    unsigned int counter = 0;
    unsigned int num;
    int pid;

    if( argc != 2 ){
        printf("Incorrect number of arguments\n");
        return -1;
    }
    else{

        num = *argv[1] - '0';
        printf("Allparent ppid [%d]\n", getpppid());
        while( counter < num ){

            pid = fork();
            if( pid == -1 ){
                perror(err_msg);
                return -1;
            }
            else if( pid == 0 ){
                printf("ppid: [%d] pid: [%d]\n", getppid(), getpid());
                counter++;
            }
            else if( pid != 0 ){
                wait(NULL);
                return 0;
            }

        }

    }


    return 0;
}
