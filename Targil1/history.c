#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){
	/*The main function. This program can be called either from the Advanced shell or Security shell,
	 it recieves as an argument a name of a commands log and prints it to the terminal*/
	// Checks that the amount of arguments is 2 otherwise will print "Missing parameters!!!\n" and exit the program with -1, 
	//but since we usually call to it from one of the Advanced or Security shells the recieved arguments have the name (also relative path)
	// of the log so it shouldn't print "Missing parameters!!!\n" unless we call it from a different shell
	if(argc!=2){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Variable declaration
	char buff[256]="";
	int fd_shellHistory;
	// Opens the file(log) recieved as an argument
	if((fd_shellHistory = open(argv[1], O_RDONLY, 0644)) == -1){
		perror("Open ShellHistory: ");
	}
	// Prints the command log to the terminal and exits the program
	while(read(fd_shellHistory,&buff,256) > 0){
		fprintf(stdout,"%s",buff);
	}
	return 0;
}
