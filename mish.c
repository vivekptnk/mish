
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define DELIM "\t\v\f\n\r"

char newString[1024][1024];

int exit_flag = 1;

int readline();


int fork_exec();
int exit_cmd();

void command_loop();

int readline(){
	char *line;
	size_t bufsize = 1024;
	size_t commands;


	line  = (char *)malloc(bufsize * sizeof(char));
	if (line == NULL){
		perror("Unable to allocate buffer");
		exit(1);
	}

	commands = getline(&line, &bufsize, stdin);


	int j=0, ctr=0;

	for(int i=0; i<=(strlen(line));i++)
	{
		if(line[i]==' '|| line[i]=='\0' || line[i]=='\t' || line[i]=='\v' || line[i]=='\f' || line[i]=='\n' || line[i]=='\r'){
			newString[ctr][i] = '\0';
			ctr++;
			j=0;
		} else {
			newString[ctr][j] = line[i];
		}
	}
	return 0;
}


int exit_cmd(){
	exit_flag = 0;
}

int fork_exec(){
        pid_t pid;
        int status;

        pid = fork();

        if (pid  < 0 ) {
                perror("FORK ERROR");
                exit(1);
        } else if (pid == 0){
                if (execvp(newString[0], char *const __argv[]) <0 )  printf("COMMAND NOT FOUND : %s\n", newString[0]), exit(2);
        } else {
                do{
                        waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        return 1;
}

int parse_array(){
	int i=0;
	int j=0;
	int length1 = strlen(char *newString);

	for(i ; i<length1; i++){
		if (newString[i] == "cd"){
			if (chdir(newString[i+1])!=0){
		                perror("cd failed");
       			 } else {
               			 chdir(newString[i+1]);
       			 }
			return exit_cmd();

		} else if (newString[i] == "pwd") {
			char pwd[1024];
			getcwd(pwd, sizeof(pwd));
			printf("%s\n",pwd);
			return exit_cmd();

		} else if (newString[i] == "log") {
			FILE *fp;
			fp = fopen(newString[i+1],"w+");
			do{
				fputs(newString[i],fp);
				i++;
			}while(newString[i+1]!="log");
			fclose(fp);
			return exit_cmd();
		} else if (newString[i] == "exit"){
			return exit_cmd();
		} else {
			fork_exec(newString);
		}
	}
}


void command_loop()
{

        do{
        printf("MISH>");
        readline();
	parse_array();
        }while(exit_flag = 1);
}

int main(){
	command_loop();
	return 0;
}
