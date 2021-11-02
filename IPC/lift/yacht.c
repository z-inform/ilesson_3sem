#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char* argv[]){
        
    int sem_id = semget(IPC_PRIVATE, 4, 0660 | IPC_CREAT | IPC_EXCL); //yacht; ramp; single ride ended; all rides ended

    struct sembuf get_on[3] = {{1, -1, 0}, {0, 1, 0}, {1, 1, 0}}; //take ramp; add to passengers; free ramp
    struct sembuf get_off[3] = {{1, -1, 0}, {0, -1, 0}, {1, 1, 0}};
    
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    int pass_num = atoi(argv[1]);
    int ride_num = atoi(argv[2]);

    for(int i = 0; i < pass_num; i++) {

        int id = fork();

        if (id == -1) {
            perror("fork error");
            return -1;
        }

        if (id)
            while(1){
                struct sembuf check_end = {3, -1, IPC_NOWAIT | SEM_UNDO};
                if (semop(sem_id, &check_end, 1) == -1) { //check if rides have ended
                    if (errno != EAGAIN){
                        perror("semaphore error");
                        return -1;
                    }
                } else 
                    return 0;
            }
    }

    for (int i = 0; i < ride_num; i++) {


    }

    printf("Closing the rides\n");
    struct sembuf end_rides = {3, 1, 0};
    if (semop(sem_id, &end_rides, 1) == -1) {
        perror("closing semaphor error");
        return -1;
    }

    printf("All closed\n");

    if (semctl(sem_id, IPC_RMID, 0) == -1) {
        perror("semaphor removal error");
        return -1;
    }
    



    return 0;
}

