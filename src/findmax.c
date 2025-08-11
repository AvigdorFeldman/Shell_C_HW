#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){
	/*The main function. This program is called from the Advanced shell it gets two names of files as arguments and prints the name of the file with the biggest size out of the two*/
	// Checks that the amount of arguments is 3 otherwise will print "Missing parameters!!!\n" and exit the program with -1
	if(argc!=3){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Variable declaration
	int fd_file1, fd_file2, amountOfBytes1=0, amountOfBytes2=0;
	char buff;
	// Opens each of the files
	if((fd_file1 = open(argv[1],O_RDONLY,0664))==-1){
		perror("Open failed"); return -2;
	}
	if((fd_file2 = open(argv[2],O_RDONLY,0664))==-1){
		perror("Open failed");close(fd_file1); return -2;
	}
	// Get the size of each file
	while(read(fd_file1,&buff,1)>0){
		amountOfBytes1++;
	}
	while(read(fd_file2,&buff,1)>0){
		amountOfBytes2++;
	}
	// Compares the sizes and decides on the biggest, then it prints it to the terminal and exits
	if(amountOfBytes1>amountOfBytes2)
		fprintf(stdout,"%s\n",argv[1]);
	else
		fprintf(stdout,"%s\n",argv[2]);
	exit(0);
}
