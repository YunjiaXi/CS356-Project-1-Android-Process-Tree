#include <unistd.h>  //for execl(),fork(),getpid()
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) 
{
	pid_t child_pid = fork();	
	
	if (child_pid < 0)
		printf("Error!");
	else if (child_pid == 0)
  {
		printf("517030910102 Child, %d\n", getpid());	
		execl("/data/misc/ptreeARM", "ptreeARM", NULL);	// execute ptree in child process	
	}
	else
		printf("517030910102 Parent, %d\n", getpid());	

	return 0;
}

