#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define ftoken "key.token"

int readfd(int src, char* buf, int size);

int main(int argc, char* argv[]){
    char close_shm = 1;
    char close_sem = 1;
    int file_key = ftok(ftoken, 1);
    if (errno) {
        perror("ftok error");
        return -1;
    }
    
    int sem_id = semget(file_key, 2, 0660 | IPC_CREAT | IPC_EXCL);
    if ((sem_id == -1) && (errno != EEXIST)) {
        perror("semget error");
        return -1;
    }
    if (errno == EEXIST){
        if ((sem_id = semget(file_key, 2, 0)) == -1) {
            perror("existing semget error");
            return -1;
        }
        close_sem = 0;
    } else {
        unsigned short init[2] = {0, 1};
        semctl(sem_id, 0, SETALL, init);
    }

    int shm_id = shmget(file_key, 4096, 0660 | IPC_CREAT | IPC_CREAT);
    if ((shm_id == -1) && (errno != EEXIST)) {
        perror("shmget error");
        return -1;
    }
    if (errno == EEXIST){
        if ((shm_id = shmget(file_key, 4096, 0)) == -1) {
            perror("existing shmget error");
            return -1;
        }
        close_shm = 0;
    }

    if (argc != 2) {
        printf("Incorrect args\n");
        return -1;
    }

    void* shmaddr = shmat(shm_id, NULL, 0);
    if (shmaddr == (void*) -1) {
        perror("shmat error");
        return -1;
    }
    
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("file open error");
        return -1;
    }

    //-----------------------------------------

    struct sembuf wait[3] = {{0, 0, 0}, {0, 1, 0}, {1, -1, 0}};
    struct sembuf free = {0, -1, 0};
    int size;

    printf("reader started; sem_id %d\n", file_key);
    errno = 0;
    while(1){
        semop(sem_id, wait, 3);
        size = readfd(fd, shmaddr + sizeof(int), 4096 - sizeof(int));
        if (size == -1)
            return -1;
        *((int*) shmaddr) = size;
        printf("reader put %d to shmem\n", size);
        if (size == 0) {
            *((int*) shmaddr) = -1;
            semop(sem_id, &free, 1);
            break;
        }
        semop(sem_id, &free, 1);
        if (errno) {
            perror(NULL);
            return -1;
        }
    }

    printf("reader finished and waiting\n");
    semop(sem_id, wait, 3);

    //-----------------------------------------

    
    if (shmdt(shmaddr) == -1) {
        perror("shmdt error");
        return -1;
    }
    
    if (close_sem)
        if (semctl(sem_id, 0, IPC_RMID)) {
            perror("sem close error");
            return -1;
        }
    
    if (close_shm)
        if (shmctl(shm_id, IPC_RMID, NULL)) {
            perror("shmem close error");
            return -1;
        }

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





