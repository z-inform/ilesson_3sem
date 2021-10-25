#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char* argv[]){
    int pid;
    int arr[argc - 1];

    for(int i = 0; i < (argc - 1); i++){
        arr[i] = atoi(argv[i + 1]);
    }

    for(int i = 0; i < (argc - 1); i++){
        pid = fork();
        if( !pid ){
            usleep(1000 * arr[i]);
            printf("%d ", arr[i]);
            return 0;
        }
    }

    return 0;
}
