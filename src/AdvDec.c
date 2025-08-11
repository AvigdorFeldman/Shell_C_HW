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
	args[1]=arg1; 
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
	/*The main function. The program is called from the Security shell, it gets a name of a file and a character to add permission to read and write the  encrypted file,
	then it moves the file to the Encryption_File directory then decrypts the recieved file using Dec,
	then it moves the file to the current directory */
	// Checks that the amount of arguments is 3 otherwise will print "Missing parameters!!!\n" and exit the program with -1
	if(argc!=3){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Checks if the path of the file is in the Encryption_File/Adv_Enc/ directory, if its not it will print "Adv Decryption option not Supported\n" and exit the program with -2
	char path[256] = "Encryption_File/Adv_Enc/";
    	strcat(path, argv[1]);
    	if(access(path, F_OK) == -1){
    		fprintf(stdout, "Adv Decryption option not Supported\n"); return -2;
    	}
    	// Firstly we will add read and write permissions
    	// It will call callExecvp() to do the command chmod +rw on the encrypted file
	char pathch[256]="Encryption_File/Adv_Enc/";
	strcat(pathch,argv[1]);
	if(callExecvp("chmod","+rw",pathch)!=0){
		perror("chmod");exit(1);}
	// After it added read and write permissions we will move the file to Encryption_File directory
	// So we call callExecvp() to do the command mv
	char pathmv[256] = "Encryption_File/Adv_Enc/";
	strcat(pathmv,argv[1]);
	if(callExecvp("mv",pathmv,"Encryption_File/")!=0){
		perror("mv");exit(1);}
	// After the file has been moved to Encryption_File directory it will call Dec 
	// This fork is to call program Dec so it would perform XOR on the file with the recieved character and in the end will move it to the current directory
	// Initialize variables
	char * args[4];
	for(int i=0;i<4;i++)
		args[i]=NULL;
	args[0] = "Dec";
	args[1] = argv[1];
	args[2] = argv[2];
	args[3] = NULL;
	int status;
	int pid = fork();
	if(pid == 0){ // Child process: calls Dec with the recieved file name and character
		execve("./Dec",args,NULL);
		exit(1);
	}
	else if(pid>0){ // Parent process: Waits untill Dec will finish
		wait(&status);
		if(status!=0)
			exit(1);
		exit(0);
	}
	else{
		perror("Fork failed"); return -3;
	}
	
	
}
