#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>



void command_loop()
{	
	int exit_counter = 1;
	
	do{
	printf("MISH >\n");
	
	}while(exit_counter = 1);
}


int  cd(){
	return 0;
}

int pwd(){
	char pwd[1024];
        getcwd(pwd, sizeof(pwd));
        printf("current director:\n%s\n",pwd);
	return 0;
}

int  exit_cmd(){
	return 0;
}

int  log_file(){
	return 0;
}

int  fork_exec(){
	return 0;
}

int  main(){char pwd[1024];

        printf("Starting Up !\n");
        command_loop();

        return 0;
}
