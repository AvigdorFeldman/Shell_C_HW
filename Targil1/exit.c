#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
	/*The main function. This program is called from the Standard shell when entered exit, 
	it deletes the Commands directory its files and sub-directories and then ends the program*/
	// Deletes the Commands directory, its files and sub-directories through rm -r
	char * argsr[4];
	int status;
	int pid = fork();
	if(pid==0){// The child process: Deletes the Commands directory, its files and sub-directories through rm -r
		argsr[0]="rm";
		argsr[1]="-r";
		argsr[2]="Commands";
		argsr[3]=NULL;
		execvp(argsr[0],argsr);
		exit(1);
	}
	else if (pid>0){ // The parent process: waits for the child to finish, prints GoodBye... and exits the program
		wait(&status);
		if(status!=0)
			exit(1);
		fprintf(stdout,"GoodBye...\n");
		exit(0);
		}
	else{
		// Fork failed
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
}
