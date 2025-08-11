#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){
	/*The main function. This program is called from the Advanced shell when entered size "NameOfFile", 
	it prints the amount of bytes(characters) in the file to the terminal*/
	// Checks that the amount of arguments is 2 otherwise will print "Missing parameters!!!\n" and exit the program with -1
	if(argc!=2){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Variable declaration
	int fd_file, amountOfBytes=0;
	char buff;
	// Opens the file recieved as an argument
	if((fd_file = open(argv[1],O_RDONLY,0664))==-1){
		fprintf(stdout,"File not exist!\n");return -2;
	}
	// Reads 1 character in each iteration the number of iterations will be the size of the file
	while(read(fd_file,&buff,1)>0){
		amountOfBytes++;
	}
	// Prints the size of the file and exits with 0
	fprintf(stdout,"%d\n",amountOfBytes);
	exit(0);
}
