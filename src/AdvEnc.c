#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
int callExecvp(char* command, char* arg1, char* arg2){
	// The function gets strings of the unix command and its arguments and uses fork to create a child process that will call execvp on that command
	// Arguments for execvp declaration and intializeation
	char * args[4];
	int pid,status;
	args[0] = command;	
	args[1] = arg1; 
	args[2] = arg2;
	args[3] = NULL;
	pid = fork();
	if(pid == 0){ // Child process: calls execvp on command
		execvp(command,args);
		exit(1);
	}
	else if(pid>0){ // Parent process: waits for command to finish
		wait(&status);
		if(status!=0)
			exit(1);
	}
	else{ // Fork failure
		perror("Fork failed"); return -3;
	}
}
int main(int argc , char * argv[]){
	/*The main function. The program is called from the Security shell, it gets a name of a file and a character to encrypt the recieved file using XOR,
	then it moves the file to the Encryption_File/Adv_Enc directory and changes the file's premission to not readable and not writable*/
	// Checks that the amount of arguments is 3 otherwise will print "Missing parameters!!!\n" and exit the program with -1
	if(argc!=3){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Checks if the path of the file is in the current directory, if its not it will print "Encryption option not Supported\n" and exit the program with -2
	char * path=argv[1];
	int fd_file;
    	if(fd_file=open(path,O_RDONLY)==-1){
    		fprintf(stdout, "Encryption option not Supported\n"); return -2;
    	}
    	close(fd_file);
    	// This fork is to call program Enc so it would perform XOR on the file with the recieved character and in the end will move it to the Encryption_File directory
	char * args[4];
	for(int i=0;i<4;i++)
		args[i]=NULL;
	args[0] = "Enc";
	args[1] = argv[1];
	args[2] = argv[2];
	args[3] = NULL;
	int pid = fork();
	int status;
	if(pid == 0){ // Child process: calls Enc with the recieved file name and character
		execve("./Enc",args,NULL);
		exit(1);
	}
	else if(pid>0){ // Parent process: Waits untill Enc will finish
		wait(&status);
		if(status!=0){
			perror("Enc ERROR");
			exit(1);}
	}
	else{ // Fork failure
		perror("Fork failed"); return -3;
	}
	// After Enc completed it moved the file to Encryption_File but it needs to be moved to Encryption_File/Adv_Enc directory
	// So we call callExecvp() to do the command mv
	char pathmv[256] = "Encryption_File/";
	strcat(pathmv,argv[1]);
	if(callExecvp("mv",pathmv,"Encryption_File/Adv_Enc/")!=0){
		perror("mv");exit(1);}
	// After the mv command is completed all thats left is to remove permissions of read and write
	// It will call callExecvp() to do the command chmod -rw on the encrypted file
	char pathch[256]="Encryption_File/Adv_Enc/";
	strcat(pathch,argv[1]);
	if(callExecvp("chmod","u-rw,g-rw,o-rw",pathch)!=0){
		perror("chmod");exit(1);}
	exit(0);
}
