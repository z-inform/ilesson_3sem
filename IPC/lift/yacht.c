#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]){
        
    int sem_id = semget(IPC_PRIVATE, 4, 0660); //yacht; ramp; ride ended; allow enter
    unsigned short init_array[4] = {0, 1, 1, 1};
    semctl(sem_id, 0, SETALL, init_array);

    
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    int pass_num = atoi(argv[1]);
    int ride_num = atoi(argv[2]);
    int seats_num = atoi(argv[3]);

    if ((pass_num <= 0) || (ride_num <= 0) || (seats_num <= 0)) {
        printf("Incorrect args\n");
        return -1;
    }

    if (pass_num < seats_num) 
        seats_num = pass_num;

    struct sembuf get_on[3] = {{1, -1, 0}, {0, -1, 0}, {3, 0, 0}};
    struct sembuf get_off[3] = {{1, -1, 0}, {0, 1, 0}, {2, 0, 0}};
    struct sembuf free_ramp[1] = {{1, 1, 0}};

    for(int i = 0; i < pass_num; i++) {

        int id = fork();

        if (id == -1) {
            perror("fork error");
            return -1;
        }

        if (id){
            printf("[%d] Passenger registered\n", getpid());
            while(1){
                if ((semop(sem_id, get_on, 3) == -1) && ((errno == EIDRM) || (errno == EINVAL))) {//wait to get on the yacht             
                    printf("[%d] died\n", getpid());
                    return 0;
                }

                semop(sem_id, free_ramp, 1);
                printf("[%d] got on the yacht\n", getpid());
                semop(sem_id, get_off, 3); //exit the yacht
                semop(sem_id, free_ramp, 1);
                printf("[%d] exited the yacht\n", getpid());
            }
        }
    }


    struct sembuf pass_on[2] = {{0, seats_num, 0}, {3, -1, -1}};
    struct sembuf wait_pass_on[1] = {{0, 0, 0}};
    struct sembuf ride[2] = {{2, -1, 0}, {3, 1, 0}};
    struct sembuf wait_off[2] = {{0, -seats_num, 0}, {2, 1, 0}};
    
    printf("Captain init the sem\n");

    for (int i = 0; i < ride_num; i++) {
        printf("Captain: Accepting passengers\n");
        semop(sem_id, pass_on, 2);
        semop(sem_id, wait_pass_on, 1);
        //usleep(5);
        printf("Captain: All passengers seated\n");
        semop(sem_id, ride, 2);
        //semop(sem_id, free_ramp, 1);
        printf("Captain: ride ended\n");
        semop(sem_id, wait_off, 2);
        //usleep(5);
        printf("Captain: all off\n");
    }

    printf("Rides closed\n");
    if (semctl(sem_id, IPC_RMID, 0) == -1) {
        perror("semaphor removal error");
        return -1;
    }
    printf("Captain left\n");

    return 0;
}

