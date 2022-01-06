#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

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

    fcntl(pipes[0], F_SETPIPE_SZ, 32768);
    printf("System pipe size %d\n", fcntl(pipes[0], F_GETPIPE_SZ));

    int pid = fork();

    if (!pid)
        return watchdog(semid, pipes);

    struct sembuf byte_rcv[1] = {{0, -1, 0}};
    struct sembuf check_blocked[1] = {{0, -1, IPC_NOWAIT}};
    struct sembuf cycle_done[1] = {{1, 1, 0}};

    unsigned char byte;
    int size = 0;

    struct timespec timeout = {10, 0};
    int fd = open("pipedump", O_WRONLY);
    if (fd == -1) {
        perror("file open error");
        return -1;
    }

    while(1){
        if ((semtimedop(semid, byte_rcv, 1, &timeout) == -1) && (errno == EAGAIN)) {
            printf("%d\n", size);
            printf("Semaphore operations broke\n");
            if (splice(pipes[0], NULL, fd, NULL, 32768, SPLICE_F_NONBLOCK) == -1) {
                perror("splice error");
                return -1;
            }
            printf("Pipe dumped\n");
            printf("attempting to revive by reading and rewriting a byte\n");
            read(pipes[0], &byte, 1);
            write(pipes[1], &byte, 1);
        }
        size++;
        usleep(500);
        errno = 0;
        if ((semop(semid, check_blocked, 1) == -1) && (errno != EAGAIN)) {
            printf("%d\n", size);
            perror("wah problems");
            return -1;
        }
        if (errno == EAGAIN) {
            printf("%d\n", size);
            printf("possible max reached\n");
            usleep(1000000);
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
                if (splice(pipes[0], NULL, fd, NULL, 32768, SPLICE_F_NONBLOCK) == -1) {
                    perror("splice error");
                    return -1;
                }
                printf("Pipe dumped\n");
                printf("pipe size %d bytes\n", size - 1);
                break;
            }
            printf("continue\n");
        }
        read(pipes[0], &byte, 1);
        semop(semid, cycle_done, 1);
    }

    kill(pid, SIGTERM);
    wait(NULL);
    return 0;
}

int watchdog(int semid, int pipes[2]){
    unsigned char byte = 0;
    struct sembuf byte_sent[1] = {{0, 1, 0}};
    struct sembuf cycle_wait[1] = {{1, -1, 0}};
    while(1){
        write(pipes[1], &byte, 1);
        semop(semid, byte_sent, 1);
        write(pipes[1], &byte, 1);
        semop(semid, byte_sent, 1);
        semop(semid, cycle_wait, 1);
        byte++;
    }
}

