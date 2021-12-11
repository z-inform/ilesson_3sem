#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/msg.h>

#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
    if (argc != 3) {
        printf("incorrect number of args\n");
        return -1;
    }

    key_t token = ftok(argv[1], 0);
    printf("token %d\n", token);
    int shmid = shmget(token, strlen(argv[2]), IPC_CREAT | IPC_EXCL | 0660);
    if (shmid == -1){
        perror("Shared memory error");
        return -1;
    }

    int semid = semget(token, 2, IPC_CREAT | IPC_EXCL | 0660);
    if (semid == -1) {
        perror("Semaphore error");
        return -1;
    }

    /*
    int msgid = msgget(token, IPC_CREAT | IPC_EXCL | 0660);
    if (msgid == -1) {
        perror("Message queue error");
        return -1;
    }
    */
    
    unsigned short sem_init[2] = {0, 1};
    semctl(semid, 0, SETALL, sem_init);

    for(int i = 0; i < 33; i++){
        int pid = fork();
        if (!pid) {
            execv(argv[1], argv + 1);    
        }
    }

    for(int i = 0; i < 33; i++){
        wait(NULL);
    }

    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    //msgctl(msgid, IPC_RMID, NULL);


    return 0;
}
