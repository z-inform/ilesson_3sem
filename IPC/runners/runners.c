#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int judge(int run_count, int q_id);
int runner(int runner_id, int q_id, long judge_id);

struct dummy_msg_struct {
    long mtype;
    char mtext[0];
};

int main(int argc, char* argv[]){

    if (argc != 2) {
        printf("Incorrect args\n");
        return -1;
    }

    int run_count = atoi(argv[1]);
    
    int q_id = msgget(IPC_PRIVATE, IPC_CREAT | 0660); /*rw-rw----*/
    
    for (int i = 1; i <= run_count; i++){
        int pid = fork();
        if (!pid) {
            runner(i, q_id, (long) run_count + 1);
            return 0;
        }
    }

    judge(run_count, q_id);
    msgctl(q_id, IPC_RMID, NULL);
    
    return 0;
}

int judge(int run_count, int q_id){
    struct dummy_msg_struct dummy_msg;
    printf("Judge on the field\n");
    for (long i = 1; i <= run_count; i++) 
        if (msgrcv(q_id, &dummy_msg, 0, run_count + 1, MSG_NOERROR) == -1){ /*wait for all runners*/
            perror("judge rcv error");
            return -1;
        }
    printf("All runners present\nStarting the run\n");

    long mtype = 1;
    if (msgsnd(q_id, &mtype, 0, 0) == -1){
        perror("judge snd error");
        return -1;
    }
    printf("First runner to start\n");

    if (msgrcv(q_id, &dummy_msg, 0, run_count + 1, MSG_NOERROR) == -1){
        perror("judge rcv error");
        return -1;
    }

    printf("Last runner finished\n");
    printf("Run ended\n");
    for (long i = 1; i <= run_count; i++)
        if (msgsnd(q_id, &i, 0, 0) == -1){
            perror("judge snd error");
            return -1;
        }
    return 0;
}

int runner(int runner_id, int q_id, long judge_id){
    struct dummy_msg_struct dummy_msg;
    printf("Runner %d arrived\n", runner_id);
    long mtype = judge_id;
    if (msgsnd(q_id, &mtype, 0, 0) == -1) {
        perror("runner send error");
        return -1;
    }

    mtype = (long) runner_id;
    if (msgrcv(q_id, &dummy_msg, 0, mtype, MSG_NOERROR) == -1) {
        perror("runner rcv error");
        return -1;
    }

    mtype++;
    if (msgsnd(q_id, &mtype, 0, 0) == -1) {
        perror("runner snd error");
        return -1;
    }

    printf("Runner %d passed\n", runner_id);

    if (msgrcv(q_id, &dummy_msg, 0, (long) runner_id, MSG_NOERROR) == -1) {
        perror("runner rcv error");
        return -1;
    }

    printf("Runner %d left\n", runner_id);

    return 0;
}

