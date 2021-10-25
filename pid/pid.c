#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(){
    printf("My pid: %d\nParent pid: %d\n", getpid(), getppid());
    return 0;
}
