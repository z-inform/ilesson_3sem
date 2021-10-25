#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int judge(int run_count, int q_id);
int runner(int runner_id, int q_id);

int main(int argc, char* argv[]){

    if (argc != 2) {
        printf("Incorrect args\n");
        return -1;
    }

    int run_count = atoi(argv[1]);
    
    //set up queue
    int q_id = msgget(IPC_PRIVATE, IPC_CREAT | 0660); //rw-rw----
    
    for (int i = 0; i < run_count; i++){
        int pid = fork();
        if (!pid) {
            //runner();
            return 0;
        }
    }

    //judge();
    //close queue
    
    return 0;
}

int judge(int run_count, int q_id){
    printf("Judge on the field\n");
    for (long i = 1; i <= run_count; i++) 
        if (msgrcv(q_id, NULL, 0, i, MSG_NOERROR) == -1){ //wait for all runners
            perror("judge rcv error");
            return -1;
        }
    printf("All runners present\nStarting the run");

    long mtype = 1;
    if (msgsnd(q_id, &mtype, 0, 0) == -1){
        perror("judge snd error");
        return -1;
    }
    printf("First runner to start\n");

    if (msgrcv(q_id, NULL, 0, run_count, MSG_NOERROR) == -1){
        perror("judge rcv error");
        return -1;
    }

    printf("Last runner finished\n");
    for (long i = 1; i <= run_count; i++)
        if (msgsnd(q_id, &i, 0, 0) == -1){
            perror("judge snd error");
            return -1;
        }
    printf("Run ended");
    return 0;
}

int runner(int runner_id, int q_id){




    return 0;
}

