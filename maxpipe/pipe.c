#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <errno.h>
#include <stdio.h>

int watchdog(int semid, int pipes[2]);

int main(){



    int semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0660);
    int init[2] = {0, 0};
    semctl(semid, 0, SETALL, init);
    if (semid == -1) {
        perror("sem creation error");
        return -1;
    }

    int pipes[2];
    if (pipe(pipes) == -1) {
        perror("pipe creation error");
        return -1;
    }

    //fcntl(pipes[0], F_SETPIPE_SZ, 32768);
    printf("System pipe size %d\n", fcntl(pipes[0], F_GETPIPE_SZ));

    int pid = fork();

    if (!pid)
        return watchdog(semid, pipes);

    struct sembuf byte_rcv[1] = {{0, -1, 0}};
    struct sembuf check_blocked[1] = {{0, -1, IPC_NOWAIT}};
    struct sembuf cycle_done[1] = {{1, 1, 0}};

    unsigned char byte;
    int size = 0;

    while(1){
        semop(semid, byte_rcv, 1);
        size++;
        usleep(500);
        errno = 0;
        if ((semop(semid, check_blocked, 1) == -1) && (errno != EAGAIN)) {
            perror("wah problems");
            return -1;
        }
        if (errno == EAGAIN) {
            printf("possible max reached\n");
            usleep(1000);
            errno = 0;
            semop(semid, check_blocked, 1);
            if (errno == EAGAIN) {
                read(pipes[0], &byte, 1);
                sleep(1);
                errno = 0;
                semop(semid, check_blocked, 1);
                if (errno == EINVAL) {
                    printf("Не ну тут уже планировщик вообще меня обижает, я так работать не буду\n");
                    break;
                }
                printf("pipe size %d bytes\n", size - 1);
                break;
            }
            printf("continue\n");
        }
        read(pipes[0], &byte, 1);
        semop(semid, cycle_done, 1);
        printf("%d\n", size);
    }

    kill(pid, SIGTERM);
    wait(NULL);
    return 0;
}

int watchdog(int semid, int pipes[2]){
    unsigned char byte = 0xEE;
    struct sembuf byte_sent[1] = {{0, 1, 0}};
    struct sembuf cycle_wait[1] = {{1, -1, 0}};
    
    while(1){
        write(pipes[1], &byte, 1);
        semop(semid, byte_sent, 1);
        write(pipes[1], &byte, 1);
        semop(semid, byte_sent, 1);
        semop(semid, cycle_wait, 1);
    }
}

