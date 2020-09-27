#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main()
{
	pid_t pid;
	int status;
	int ret;

	printf("Parent pid=%d: Hello world from the parent\n",getpid());

	pid = fork();

	if (pid < 0) {
		perror("Parent: fork failed:");
		exit(1);
	}

	if (pid == 0) { // Child executes this block
		printf(" Child pid=%d: Hello World... I am the child process\n",getpid());
		printf(" Child pid=%d: That's all from me... good bye!\n",getpid());
		exit(99);
	}

	if (pid > 0) { //Parent executes this block
		printf("Parent pid=%d: Spawned a child with pid %d\n", getpid(), pid);
      printf("Parent pid=%d: Waiting for the child to exit\n",getpid());
		ret = waitpid(pid, &status, 0);
		if (ret < 0) {
			perror("Parent: waitpid failed:");
			exit(2);
		}


		printf("Parent pid=%d: Child pid=%d exited with status %d\n", getpid(), pid, WEXITSTATUS(status));
		printf("Parent pid=%d: ... and I'm done here too.\n",getpid());
	}

	return 0;
}
