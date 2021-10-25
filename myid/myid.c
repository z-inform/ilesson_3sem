#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){

    struct passwd* user = NULL;
    int num_groups = 0;
    struct group* prim_group = NULL;
    gid_t prim_gid = -1;
    gid_t* group_list = NULL;

    switch(argc){
        case 1:
            user = getpwuid(getuid());
            break;
        case 2:
            user = getpwnam(argv[1]);
            break;
        default:
            printf("myid: extra operand '%s'\n", argv[2]);
            return 1;
        break;
    }

    if( user == NULL ){
      printf("myid: '%s': no such user\n", argv[1]);
      return 1;
    }


    switch(argc){
        case 1:
            prim_group = getgrgid(getgid());
            //prim_gid = prim_group->gr_gid;
            num_groups = getgroups(0, NULL);
            group_list = (gid_t*) calloc(num_groups, sizeof(int));
            getgroups(num_groups, group_list);
            break;
        case 2:
            prim_group = getgrgid(user->pw_gid);
            //prim_gid = prim_group->gr_gid;
            getgrouplist(user->pw_name, prim_group->gr_gid, NULL, &num_groups);
            group_list = (gid_t*) calloc(num_groups, sizeof(int));
            getgrouplist(user->pw_name, prim_group->gr_gid, group_list, &num_groups);
            break;
    }

    printf("uid=%d(%s) gid=%d(%s) groups=%d(%s)", user->pw_uid, user->pw_name, 
            prim_group->gr_gid, prim_group->gr_name, prim_group->gr_gid, prim_group->gr_name);
    for(int i = 0; i < num_groups; i++){
        struct group* group = getgrgid(group_list[i]);
        if( group->gr_gid == prim_group->gr_gid ) 
            i++;
        else{
            printf(",%d(%s)", group->gr_gid, group->gr_name);
        }
    }
    printf("\n");
    return 0;
}




