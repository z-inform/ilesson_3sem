#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SONG argv[1]

int main(int argc, char* argv[]){
    if (argc != 2) {
        printf("incorrect number of args");
        return -1;
    }
    
    int song_len = strlen(SONG);
    char my_letter = 0;

    key_t token = ftok(argv[0], 0);
    int shmid = shmget(token, song_len, 0660);
    if (shmid == -1){
        perror("Shared memory get error");
        return -1;
    }

    int semid = semget(token, 2, 0660);
    if (semid == -1){
        perror("Semaphore get error");
        return -1;
    }
    
    /*
    int msgid = msgget(token, 0660);
    if (msgid == -1){
        perror("Message queue get error");
        return -1;
    }
    */

    char* shared_song = shmat(shmid, NULL, 0);
    if (shared_song == (void*)-1) {
        perror("Memory attach error");
        return -1;
    }
    struct sembuf free_str[1] = {{0, -1, 0}};
    struct sembuf wait_str[2] = {{0, 0, 0}, {0, 1, 0}};

    for(int i = 0; i < song_len; i++){
        semop(semid, wait_str, 2);
        if (shared_song[i] == 0) {
            my_letter = SONG[i];
            for(int k = 0; k < song_len; k++){
                if (SONG[k] == my_letter) shared_song[k] = my_letter;
            }
            semop(semid, free_str, 1);
            break;
        }
        semop(semid, free_str, 1);
    }
    if (my_letter == 0) {
        printf("[%d] мне не досталось буквы, пойду прыгну в колодец\n", getpid());
        return 0;
    }
    printf("[%d] letter %c\n", getpid(), my_letter);

    for(int i = 1; i <= song_len; i++){
        if (SONG[i - 1] == my_letter) {
            struct sembuf wait_turn[1] = {{1, -i, 0}};
            struct sembuf next_turn[1] = {{1, i + 1, 0}};
            semop(semid, wait_turn, 1);
            printf("[%d] %c\n", getpid(), my_letter);
            semop(semid, next_turn, 1);
        }
    }

    return 0;
}

