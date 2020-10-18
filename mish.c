#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROMPT "MISH>"
#define WHITESPACE " \t\v\f\n\r"
#define MAXSUBS 5

/*-----------------------------------------------------------------------------
Global Variables */
FILE *logfile=NULL;


/*-----------------------------------------------------------------------------
Internal Function Declarations */
int getCmdLine(char ** cmdBuf,size_t *cmdBufSize,int logging);
int tokenizeCmdLine(char *** tokensPtr,int *numTokens,char *cmdBuf);
void tildeSubstitution(char **tokens,int * freeToks);
int runCommand(char **tokens,int *logging,char **prevPWDptr) ;
void freeCommand(char **tokens,int *freeToks);

int main(char **argv) {

	/* Local Variables
	------------------------------------------------------------------------------------------------------------*/
	char *cmdBuf=NULL; // Space to hold command
	size_t cmdBufSize=0; // Size of that space

	char ** tokens=NULL; // An array of pointers to tokens in cmdBuf (usually)
	int numTokens=32; // Current maximum size of the tokens array

	int logging=0; // Are we logging commands (1) or not (0)
	char * prevPWD; // Pointer to the previous working directory
	int freeToks[MAXSUBS]; // Tokens which have been malloced instead of pointing to cmdBuf

	/* Initializations
	-------------------------------------------------------------------------------------------------------------*/
	setbuf(stdout,0);
	setbuf(stderr,0);

	tokens=malloc(sizeof(char *) * numTokens);
	prevPWD=get_current_dir_name();
	cmdBufSize=256; // probably big enough, but getline will expand if needed
	cmdBuf=(char *)malloc(cmdBufSize);
	for(int i=0;i<MAXSUBS;i++) {freeToks[i]=-1; }

	/* Main command loop
	--------------------------------------------------------------------------------------------------------------*/
	printf("Starting mish shell\n");
	while(1) {
		printf("%s",PROMPT);
		int rc=getCmdLine(&cmdBuf,&cmdBufSize,logging);
		if (rc==1) continue;
		if (rc!=0) break;

		rc=tokenizeCmdLine(&tokens,&numTokens,cmdBuf);
		if (rc==1) continue;
		if (rc!=0) break;

		tildeSubstitution(tokens,freeToks);

		if (runCommand(tokens,&logging,&prevPWD)) break;
		freeCommand(tokens,freeToks);
	}

	/* Clean up and exit
	-----------------------------------------------------------------------------------------------------*/
	if (logging) fclose(logfile);
	free(prevPWD);
	free(cmdBuf);
	cmdBuf=NULL;
	free(tokens);
	printf("\nEnding mish shell... good bye.\n");
	return 0;
}

int getCmdLine(char ** cmdBuf,size_t *cmdBufSize,int logging) {
	/* Read next newline delimited command from standard input
	    update cmdBuf and cmdBufSize (if reallocate needed)
	    write to command to logfile if we are logging
	    return 0 to process, 1 to continue, -1 to end
	----------------------------------------------------------------------------------------------*/
	if (getline(cmdBuf,cmdBufSize,stdin)<0) {
		if (feof(stdin)) return -1; // Break out of loop with no message
		perror("getCmdLine invocation of getline: ");
		return -1;
	}
	char * command=(*cmdBuf);
	if (command[0]=='\0') return 1; // Ignore empty lines (but not all whitespace lines, leave that to tokenizer)
	if (logging) fprintf(logfile,"%s",command);
	// Extra credit... write command line to stdout if stdin is redirected
	if (!isatty(0)) printf("%s",command);
	return 0;
}

int tokenizeCmdLine(char *** tokensPtr,int *numTokens,char *cmdBuf) {
	/* Break up cmd based on white space
	   Updated *tokens and numToks if realloc required
	   Writes to tokens array with address of each whitespace delimited token
	   "End" of tokens array delimited by a NULL address
	---------------------------------------------------------------------------------------------------------------*/
	int n=0;
	char **tokens=(*tokensPtr); // Undo redirection for clarity
	char *this_token;
	while( (this_token= strsep(&cmdBuf, WHITESPACE)) !=NULL) {
		if (*this_token=='\0') continue;
		tokens[n]=this_token;
		n++;
		if (n>=(*numTokens)) { // increase the size of the tokens
			(*numTokens) *=2; // Double the size
			assert( (tokens = realloc(tokens,sizeof(char *) * (*numTokens))) != NULL);
			(*tokensPtr)=tokens; // Update caller
		}
	}
	tokens[n]=NULL;
	if (n==0) return 1; // Command line was whitespace only
	return 0;
}

void tildeSubstitution(char **tokens,int * freeToks) {
	/* Extra credit... replace leading tilde (~) in any argument with $HOME
	   Needs to malloc space for expanded token, and keep track of that so it
	   can be freed when this command is finished
	   Uses freeToks array to keep track of tokens which need to be freed
	   Returns 0 for success, non-zero for failure
	------------------------------------------------------------------------------------------------------------------*/
	char * home=getenv("HOME");
	for(int i=0;tokens[i]!=NULL;i++) {
		char *this_token=tokens[i];
		if (this_token[0]=='~') {
			char * newTok=(char *)malloc(strlen(this_token)+strlen(home)+1);
			strcpy(newTok,home);
			strcat(newTok,this_token+1);
			tokens[i]=newTok;
			int j;
			for(j=0;j<MAXSUBS;j++) if (freeToks[j]==-1) break;
			assert(j<MAXSUBS);
			freeToks[j]=i;
		}
	}
}

int runCommand(char **tokens,int *logging,char **prevPWDptr) {

	// Handle exit criteria
	if (0==strcmp(tokens[0],"exit") && tokens[1]==NULL) return -1;

	// Handle cd command
	if (0==strcmp(tokens[0],"cd")) {
		char * prevPWD=(*prevPWDptr); //Undo redirection for clarity
		if (tokens[1]==NULL) { // No arguments
			free(prevPWD);
			prevPWD=get_current_dir_name();
			char *home=getenv("HOME");
			if (chdir(home)) perror("cd error:");
			(*prevPWDptr)=prevPWD;
			return 0;
		}
		if (tokens[2]!=NULL) {
			fprintf(stderr,"cd command ignoring all but the first argument: ");
			for(int j=2;tokens[j]!=NULL;j++) fprintf(stderr,"%s ",tokens[j]);
			fprintf(stderr,"\n");
		}
		if (0==strcmp(tokens[1],"-")) {
			char * pwd=get_current_dir_name();
			if (chdir(prevPWD)) perror("cd error:");
			free(prevPWD);
			prevPWD=pwd;
			(*prevPWDptr)=prevPWD;
			return 0;
		}
		free(prevPWD);
		prevPWD=get_current_dir_name();
		if (chdir(tokens[1])) perror("cd error:");
		(*prevPWDptr)=prevPWD;
		return 0;
	}

	// Handle pwd command
	if (0==strcmp(tokens[0],"pwd")) {
		if (tokens[1]!=NULL) {
			fprintf(stderr,"pwd ignores other command line arguments\n");
		}
		char *pwd=get_current_dir_name();
		printf("%s\n",pwd);
		free(pwd);
		return 0;
	}

	// Handle log command
	if (0==strcmp(tokens[0],"log")) {
		if (tokens[1]==NULL) { // No arguments... close log file
			if ((*logging)) { fclose(logfile); }
			(*logging)=0;
			return 0;
		}
		if (tokens[2]!=NULL) {
			fprintf(stderr,"log command ingoring extra arguments\n");
		}
		if ((*logging)) { fclose(logfile); }
		(*logging)=0;
		logfile=fopen(tokens[1],"w");
		if (logfile==NULL) perror("Opening logfile:");
		else (*logging)=1;
		return 0;
	}

	// It's not a special command... run it
	int pid=fork(); // Start a new process
	if (pid<0) {
		perror("Forking a child process: ");
		return 0;
	}
	if (pid==0) { // This is the child process
		if (-1==execvp(tokens[0],tokens)) {
				perror("Child execvp invocation: ");
				fprintf(stderr,"Executing command: ");
				for(int j=0;tokens[j]!=NULL;j++) fprintf(stderr,"%s ",tokens[j]);
				fprintf(stderr,"\n");
				return 0;
		}
		// If there was not an error, execvp will never return
	}
	// This is the parent process
	int status;
	int ret = waitpid(pid, &status, 0);
	if (ret < 0) {
		perror("waitpid failed:");
		fprintf(stderr,"Executing command: ");
		for(int j=0;tokens[j]!=NULL;j++) fprintf(stderr,"%s ",tokens[j]);
		fprintf(stderr,"\n");
		return errno;
	}
	if ((*logging) && status!= 0) fprintf(logfile,"Command %s exited with status %d\n", tokens[0],WEXITSTATUS(status));
	return 0;
}

void freeCommand(char **tokens,int *freeToks) {
	for(int i=0;i<MAXSUBS && freeToks[i]!=-1;i++) {
		free(tokens[freeToks[i]]);
		freeToks[i]=-1;
	}
}
