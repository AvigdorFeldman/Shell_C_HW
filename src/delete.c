#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){
	/*The main function. This program is called from the Advanced shell it gets a name of a file and if it exits, deletes it using rm command in unix*/
	// Checks that the amount of arguments is 2 otherwise will print "Missing parameters!!!\n" and exit the program with -1
	if(argc!=2){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Variable declaration
	char * argsr[3];
	int status;
	int pid = fork();
	if(pid==0){ // The child process: calls execvp with the rm command on the recieved argument(name of the file)	
		argsr[0]="rm";
		argsr[1]=argv[1];
		argsr[2]=NULL;
		execvp(argsr[0],argsr);
		exit(1); // if execvp fails
	}
	else if (pid>0){ // The Parent process: waits untill the child process finishes and exits the program
		wait(&status);
		if(status!=0)
			exit(1);
		exit(0);
		}
	else{
		// Fork failed
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
}
