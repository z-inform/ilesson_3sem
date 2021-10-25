#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    char* PS1;
    if (argc == 2) {
        PS1 = argv[1];
    } else {
        PS1 = "enter command";
    }
                     
    while(!feof(stdin)){
        printf("%s: ", PS1);

        char* cmd_string = malloc(800);
        fgets(cmd_string, 800, stdin);
        /*
        do{
            ch_read = fgets(stdin);
            cmd_string[len] = ch_read;
            len++;
        }while((ch_read != '\n') && (ch_read != EOF) && (len < 800));

        if( len == 800 ){
            printf("Cmd string should be no more than 800 symbols\nIgnoring cmds\n");
            continue;
        }
        */

        cmd_string[strlen(cmd_string) - 1] = 0;

        if ((cmd_string == NULL) || !strcmp(cmd_string, "exit")) {
            break;
        }

        //printf("input cmd_string: [%s]\n", cmd_string);

        char* cmds[800];
        int cmd_count;
        cmds[0] = strtok(cmd_string, "|");
        if( cmds[0] == NULL )
            continue;

        for(cmd_count = 1; cmds[cmd_count - 1] != NULL; cmd_count++){
            cmds[cmd_count] = strtok(NULL, "|");
        }
        cmd_count--;
        /*
        printf("Commands: ");
        for(int i = 0; i < cmd_count; i++){
            printf("[%s] ", cmds[i]);
        }
        printf("\n");
        */

        int pass_pipe = -1;
        int pid;
        for(int i = 0; i < cmd_count; i++){
            int pipefd[2];
            if( pipe(pipefd) == -1 ){
                printf("Pipe error\n");
                break;
            }

            pid = fork();
            if( pid == -1 ){
                printf("Fork error\n");
                break;
            }

            if( pid != 0 ){//parent
                close(pipefd[1]);
                close(pass_pipe);
                pass_pipe = pipefd[0];
            }
            else{//child
                if( pass_pipe != -1 )
                    if( dup2(pass_pipe, 0) == -1 ){
                        printf("Fd duplication error\n");
                        return -1;
                    }

                if( i < (cmd_count - 1) )//if not last command
                    if( dup2(pipefd[1], 1) == -1 ){
                        printf("Fd duplication error\n");
                        return -1;
                    }

                close(pipefd[0]);
                close(pipefd[1]);
                close(pass_pipe);

                char** exec_args = cmds + i;
                int argc_num;
                exec_args[0] = strtok(cmds[i], " ");
                for(argc_num = 1; exec_args[argc_num - 1] != NULL; argc_num++){
                    exec_args[argc_num] = strtok(NULL, " ");
                }

                execvp(exec_args[0], exec_args);
                perror(exec_args[0]);
                return -1;
            }


        }
        waitpid(pid, NULL, WUNTRACED);
        close(pass_pipe);



        

    }
    return 0;
}

