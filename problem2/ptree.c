
#include <sys/syscall.h>
#include <unistd.h>	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __NR_pstreecall 356

struct prinfo {
	pid_t parent_pid;		        /* process id of parent */
	pid_t pid;			            /* process id */
	pid_t first_child_pid;		  /* pid of youngest child */
	pid_t next_sibling_pid;		  /* pid of older sibling */

	long state;			            /* current state of process */
	long uid;			              /* user id of process owner */
	char comm[64];			        /* name of program executed */
};

void printTree(struct prinfo *buf, int *nr)
{
    int pid_pos[1000] = {0};  //the position of pid
    int tab_num = 0;    //the number of tab

    
    printf("%s,%d,%ld,%d,%d,%d,%ld\n", buf[0].comm, buf[0].pid, buf[0].state,
    buf[0].parent_pid, buf[0].first_child_pid, buf[0].next_sibling_pid, buf[0].uid);

    int i=1;
    while(i < *nr)
    {
        //calculate number of tab
        if(buf[i].parent_pid == buf[i-1].pid)  //buf[i-1] is parent of buf[i], forward
            tab_num++;
        else if (buf[i].parent_pid != buf[i-1].parent_pid) //buf[i] is a sibling of buf[i-1]'s ancestor, backward
        {
            int tmp = buf[i-1].parent_pid;
            tab_num--;
            while(buf[i].parent_pid!=buf[pid_pos[tmp]].parent_pid) 
            {
                tmp = buf[pid_pos[tmp]].parent_pid; //find parent
                tab_num--;
            }
        }

        //record the position of pid
        pid_pos[buf[i].pid] = i;

        //print tab
        int j = 0;
        while(j < tab_num)
        {
            printf("\t");
            j++;
        }

        //print process information
        printf("%s,%d,%ld,%d,%d,%d,%ld\n", buf[i].comm, buf[i].pid, buf[i].state,
        buf[i].parent_pid, buf[i].first_child_pid, buf[i].next_sibling_pid, buf[i].uid);

        i++;
    }
}

int main(int argc, char **argv)
{
    if (argc != 1)
    {
        printf("Input Error!");
        exit(EXIT_FAILURE);
    }

    // allocate dynamic space
	struct prinfo *buf = malloc(1000 * sizeof(struct prinfo));
	int *nr = malloc(sizeof(int));
	if (buf == NULL || nr == NULL) {
		printf("Allocation error!\n");
		exit(EXIT_FAILURE);
	}

    syscall(__NR_pstreecall, buf, nr);
	printf("There are %d processes!\n", *nr);
    printTree(buf,nr);

    //free the dynamic space
    free(buf);
    free(nr);

    return 0;
}

