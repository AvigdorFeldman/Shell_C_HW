#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc , char * argv[]){
	/*The main function. The program is called from the Security shell, it gets a name of a file and a character to encrypt the recieved file using XOR,
	then it moves the file to the Encryption_File directory*/
	// Checks that the amount of arguments is 3 otherwise will print "Missing parameters!!!\n" and exit the program with -1
	if(argc!=3){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Checks if the path of the file is in the current directory, if its not it will print "Encryption option not Supported\n" and exit the program with -2
	char path[256] = "";
	int fd_file1;
    	strcat(path, argv[1]);
    	if(fd_file1=open(path,O_RDONLY)==-1){
    		fprintf(stdout, "Encryption option not Supported\n"); return -2;
    	}
    	close(fd_file1);
    	// Variable declaration
    	strcpy(path,"");
	char * args[4];
	unsigned char tav = argv[2][0]; // The recieved character
	int fd_file;
	unsigned char ch;	
	// Opens the file to encryption
	if((fd_file = open(argv[1],O_RDWR,0664))==-1){
		perror("Open failed");close(fd_file); return -3;
	}
	// Gets the size of the file using lseek
	int size = lseek(fd_file, 0, SEEK_END);
	if(size==-1){ // Checks that lseek was successfull
		perror("Failed lseek 1");close(fd_file); return -4;
	}
	if(lseek(fd_file, 0, SEEK_SET)==-1){ // Returns the pointer to the start of the file
		perror("Failed lseek 2");close(fd_file); return -4;
	}
	// The encryption of the file using the recieved character and XOR
	for(int i=0;i<size;i++){ 
		if(read(fd_file,&ch,1)==-1){ // Reads one character
			perror("Failed Read");close(fd_file); return -4;		
		}
		if(lseek(fd_file, -1, SEEK_CUR)==-1){ //Returns the pointer to before the character was read
				perror("Failed lseek 3");close(fd_file); return -4;
		}
		// XOR 
		ch^=tav;
		if((write(fd_file,&ch,1))==-1){ // Write the result after the XOR to the file
			perror("Failed Write");close(fd_file); return -5;
		}
	}
	close(fd_file);
	// Moves the encrypted file to the Encrypt_File directory
	int pid,status;
	for(int i=0;i<4;i++)
		args[i]=NULL;
	args[0] = "mv";
	args[1] = argv[1];
	args[2] = "Encryption_File";
	args[3]=NULL;
	pid = fork();
	if(pid == 0){ // The child process: moves the file to Encrypt_File using execvp on command mv
		execvp("mv",args);
		exit(1);
	}
	else if(pid>0){ // The parent process: waits for the child process to finish and exits the program
		wait(&status);
		if(status!=0)
			exit(1);
		exit(0);
	}
	else{
		perror("Fork failed"); return -3;
	}	
}
