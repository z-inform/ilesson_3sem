#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define NUTS 1
#define BOLTS 2
#define DONE 3

int bolt(int q_id);
int nut(int q_id);
int init_queue(int q_id);

struct dummy_msg_struct {
    long mtype;
    char mtext[1];
};

int main(int argc, char* argv[]){
    if (argc != 2) {
        printf("incorrect args\n");
        return 0;
    }

    int work_time = atoi(argv[1]);

    int q_id = msgget(IPC_PRIVATE, IPC_CREAT | 0660); //bolts, nuts, done
    if (q_id == -1){
        perror("queue creation error");
        return -1;
    }
    init_queue(q_id);

    int pid = fork();
    if (!pid) {
        nut(q_id);
        return 0;
    }
    pid = fork();
    if (!pid) {
        nut(q_id);
        return 0;
    }
    pid = fork();
    if (!pid) {
        bolt(q_id);
        return 0;
    }

    usleep(work_time);

    printf("work done\n");
    msgctl(q_id, IPC_RMID, NULL);
    for(int i = 0; i < 3; i++){
        wait(NULL);
    }

    return 0;
}

int bolt(int q_id){
    struct dummy_msg_struct msg;
    struct dummy_msg_struct nuts_done = {NUTS, {0}};
    int has_bolt = 0;
    printf("[%d] puts bolts\n", getpid());
    while (1) {
        if (!has_bolt) {
            printf("[%d] got a bolt\n", getpid());
            has_bolt++;
        }
        msgrcv(q_id, &msg, 1, BOLTS, 0);
        if (*msg.mtext) {
            printf("[%d] has added a bolt\n", getpid());
            has_bolt--;
            msgrcv(q_id, &msg, 1, DONE, 0);
            if (*msg.mtext) {
                printf("[%d] swapped the part\n", getpid());
                msgsnd(q_id, &nuts_done, 1, 0);
                msgsnd(q_id, &nuts_done, 1, 0);
                init_queue(q_id);
            }
        }
        if (errno != 0) 
            return -1;
    }
}

int nut(int q_id){
    struct dummy_msg_struct msg;
    struct dummy_msg_struct bolts_done = {BOLTS, {0}};
    struct dummy_msg_struct nuts_done = {NUTS, {0}};
    int has_nut = 0;
    printf("[%d] puts nuts\n", getpid());
    while (1) {
        if (!has_nut) {
            printf("[%d] got a nut\n", getpid());
            has_nut++;
        }
        msgrcv(q_id, &msg, 1, NUTS, 0);
        if (*msg.mtext) {
            printf("[%d] has added a nut\n", getpid());
            has_nut--;
            msgrcv(q_id, &msg, 1, DONE, 0);
            if (*msg.mtext) {
                printf("[%d] swapped the part\n", getpid());
                msgsnd(q_id, &bolts_done, 1, 0);
                msgsnd(q_id, &nuts_done, 1, 0);
                init_queue(q_id);
            }
        }
        if (errno != 0) 
            return -1;
    }
}

int init_queue(int q_id){
    errno = 0;
    struct dummy_msg_struct nut_msg = {NUTS, {1}};
    struct dummy_msg_struct bolt_msg = {BOLTS, {1}};
    struct dummy_msg_struct not_done_msg = {DONE, {0}};
    struct dummy_msg_struct done_msg = {DONE, {1}};
    msgsnd(q_id, &nut_msg, 1, 0);
    msgsnd(q_id, &nut_msg, 1, 0);
    msgsnd(q_id, &bolt_msg, 1, 0);
    msgsnd(q_id, &not_done_msg, 1, 0);
    msgsnd(q_id, &not_done_msg, 1, 0);
    //msgsnd(q_id, &not_done_msg, 1, 0);
    msgsnd(q_id, &done_msg, 1, 0);
    if (errno != 0) {
        perror("queue init error");
        return -1;
    }
    return 0;
}

