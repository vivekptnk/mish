#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define WHITESPACES "\t\v\f\n\r"

char *  read_line(){
	char * line = NULL;
	size_t buffer = 0;
	
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

char ** tokenize(char * line){
	int buffer = 64, pos = 0;
	char ** tokens = malloc(buffer * sizeof(char*));
	char *token;

	token = strtok(line, WHITESPACES);
	while (token != NULL){
		tokens[pos] = token;
		pos++;
		if (pos >= buffer){
			buffer += 64;
			tokens = realloc(tokens, buffer * sizeof(char*));
		}

		token = strtok(NULL, WHITESPACES);
	}
	tokens[pos] = NULL;
	return tokens;
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

void command_loop()
{       
        char * line;
        char ** args;
        int exit_counter = 1;

        do{
        printf("MISH>");
        line = read_line();
        args = tokenize(line);
        exit_counter = exit_cmd(args);
        free(line);
        free(args);
        }while(exit_counter = 1);
}


int  main(){

        printf("Starting Up !\n");
        command_loop();

        return 0;
}
