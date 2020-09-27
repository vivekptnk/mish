#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define WHITESPACES "\t\v\f\n\r"


char *read_line();
char **tokenize();
int cd();
int pwd();
int exit_cmd(char **);
int log_file();
int fork_exec(char **);
void command_loop();
int exception_count();
int exception_exec(char **);


char *exception_strings[] = {"exit", "cd", "pwd", "log" };

int (*exception_functions[])(char **) = { &cd, &pwd, &exit_cmd, &log_file };

int exception_count(){
	return sizeof(exception_strings)  / sizeof(char *);
}


char *  read_line(){
	char * line = NULL;
	ssize_t buffer = 0;
	
	if (getline(&line, &buffer, stdin) == -1){
		if (feof(stdin)){
			exit(EXIT_SUCCESS);
		} else {
			perror("readline");
			exit(EXIT_FAILURE);
		}
	}
	
	return line;
}

#define BUFFERSIZE 64

char ** tokenize(char * line){
	int buffer = BUFFERSIZE, pos = 0;
	char ** tokens = malloc(buffer * sizeof(char*));
	char * token, **backup_token;

	if (!token){
		fprintf(stderr, "ERROR\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, WHITESPACES);
	while (token != NULL){
		tokens[pos] = token;
		pos++;
		if (pos >= buffer){
			buffer += BUFFERSIZE;
			tokens = realloc(tokens, buffer * sizeof(char*));
			if(!tokens){
				free(backup_token);
				fprintf(stderr, "ERRROR\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, WHITESPACES);
	}
	tokens[pos] = NULL;
	return tokens;
}

int  cd(char **args){
	if (args[1] == NULL) {
		fprintf(stderr, "ERROR, no filename found");
	} else {
		if (chdir(args[1]) != 0){
		perror("Directory not found");
		}
	}
	return 1;
}



int pwd(){
	char pwd[1024];
        getcwd(pwd, sizeof(pwd));
        printf("%s\n",pwd);
	return 0;
}
 
int  exit_cmd(char **args){
	if (args[0] == "exit"){
		return 0;
	} else {
	return 1;
	}
}

int  log_file(){
	return 0;
}

int  fork_exec(char **args){
	pid_t pid;
	int status;
//	int ret;

	if (strcmp(args[0], "exit") == 0){
		return exit_cmd(args);
	}

	pid = fork();

	if (pid  < 0 ) {
		perror("FORK ERROR");
		exit(1);
	} else if (pid == 0){
		if (execvp(args[0], args) <0 )  printf("COMMAND NOT FOUND : %s\n", args[0]), exit(2);
	} else {
		do{
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int exception_exec(char **args){
	
	if (args[0] == NULL){
		return 1;
	}

	for (int i=0 ; i<exception_count(); i++){
		if(strcmp(args[0], exception_strings[i])==0){
			return (*exception_functions[i])(args);
		}
	}

	return fork_exec(args);
}



void command_loop()
{       
        char * line;
        char ** args;
        int exit_counter = 1;

        do{
        printf("MISH>");
        line = read_line();
	printf("%s", line);
        args = tokenize(line);
	printf("%s\n",args[0]);
	printf("%s\n", args[1]);
        exit_counter = exit_cmd(args);
        free(line);
        free(args);
        }while(exit_counter = 1);
}


int  main(){

        printf("Starting Up !\n");
        command_loop();
	printf("Goodbye");
        return 0;
}
