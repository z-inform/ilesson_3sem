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

int writefd(int dst, char* buf, int size);

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
    
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("file open error");
        return -1;
    }

    //-------------------------------

    struct sembuf wait[3] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    struct sembuf free[2] = {{0, -1, 0}, {1, 1, 0}};
    int size;

    printf("writer started; sem_id %d\n", file_key);
    errno = 0;
    while(1){
        semop(sem_id, wait, 3);
        size = *(int*) shmaddr;
        if (size == -1) break;
        if (writefd(fd, shmaddr + sizeof(int), size) == -1) {
            semop(sem_id, free, 2); 
            return -1;
        }
        printf("writer read %d from shmem\n", size);
        semop(sem_id, free, 2);       
        if (errno) {
            perror(NULL);
            return -1;
        }
    }

    semop(sem_id, free, 2);       

    printf("writer finished\n");

    //-------------------------------
    
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

int writefd(int dst, char* buf, int size){
    int write_bytes = 0;
    int offset = 0;
    char* write_err_msg = "Write error";

    while( offset < size ){
        write_bytes = write(dst, buf + offset, size);

        if( write_bytes == -1 ){
            perror(write_err_msg);
            return -1;
        }

        offset += write_bytes;
    }

    return 0;
}
