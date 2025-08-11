#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int mkDir(char* dirName){
	// The function gets a string and create a directory based on that string(path and name)
	// Variable declaration
	int pid;
	int status;
	char * argsc[4]; // Array to store arguments for execvp
	pid = fork();
	if(pid==0){ // Child process: creates the directory using mkdir -p command	
		argsc[0]="mkdir";
		argsc[1]="-p";
		argsc[2]=dirName;
		argsc[3]=NULL;
		execvp(argsc[0],argsc);
		exit(1);
	}
	else if (pid>0){ // Parent Process: Waits until the child process finishes and exits the function
		wait(&status);
		if(status!=0)
			exit(1);
	}
	else{// Fork failed
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
}

int main() {
	/*The main function. It implements a shell which can call unix commands up to 3 parameters and calls to two
	other shells: Advanced(Adv) and Security, any other command will print to the screen Not Supported.
	The shell runs untill "exit" is entered to end the program. It also keeps a log of successfull unix commands
	in the directory it created, also creates directories for the Security shell functions*/
	// Variable declaration
	char buff[256];
	char *args[5]; // Array to store arguments for execve/execvp
	int pid;
	int status,counter=1; // counter and counterString will be used to indicate the number of the command in the log
	char counterString[256]="";
	// Creates directories of the log
	mkDir("Commands");
	// Creates directories for the Security shell functions
	mkDir("Encryption_File");
	mkDir("Encryption_File/Adv_Enc");
    	// Open (or creates) the file "commands.txt" inside the "Commands" directory, and checks if open was successfull
    	int fd_commands = open("Commands/Standard_Commands.txt", O_WRONLY | O_CREAT, 0664);
    	if (fd_commands == -1) {
        	perror("open failed");
        	return 1;
    	}
    	// The loop of the shell, will continue as long as exit wasn't entered or some kind of OS calling failed
    	while (1) {
        	// Prompt current shell
        	fprintf(stdout, "StandShell> ");   
        	// Reads a full line of input (with spaces) to string buff
        	if (fgets(buff,256,stdin) == NULL) {
            		perror("Error reading input");
            		continue;
        	}
        	// Continues the loop if "\n"(enter) was entered
		if(strcmp(buff,"\n")==0)
			continue;
		// Remove the '\n' character at the end if it exists
		if(strlen(buff)>0 && buff[strlen(buff)-1]=='\n')
			buff[strlen(buff)-1]='\0';

		// Checks for "exit" to exit the shell, and will call exit program to exit to delete Commands and it's sub-directories and exit the program with "GoodBye"
		if (strcmp(buff, "exit") == 0) {
			close(fd_commands);
			execl("./exit","exit",NULL);
		}
		// Initializes args
		for (int i = 0; i < 5; i++) {
            		args[i] = NULL;
        	}
		// Tokenize the input to parse the command and its arguments
		int arg_count = 0;
		char temp[256];
		int j=0;
		for(char i=0;i<(strlen(buff)+1)&&arg_count<5;i++){
			if(buff[i]==' '||buff[i]=='\0'){
			// End of a word(argument)
				temp[j] = '\0';
				args[arg_count] = malloc(strlen(temp) + 1); // Allocate memory for the argument
                    		strcpy(args[arg_count], temp); // Puts the argument in args
                    		strcpy(temp,"");
                    		arg_count++;
				j=0;
			}
			else{
				temp[j++] = buff[i];
			}
		}
		// Puts NULL so when the program will be called with execv to indicate the end of args
		args[arg_count]=NULL;
		// Does fork on the process to execute the command/program
		pid = fork();
		if (pid == 0) {// The child process: calls to the command/program using execvp/e
		    if(args[0]!=NULL){
			    if(strcmp(args[0],"Adv")==0||strcmp(args[0],"Security")==0){
			    	execve(args[0],args,NULL);
			    	exit(1);
			    }
			    else{
			    	if(execvp(args[0],args) == -1){ 
					fprintf(stdout,"Not Supported\n");
				}
					
			  } 
		  }
		  exit(1);
		} else if (pid > 0) {
		    // Parent process: waits for child to finish and adds a successfull command to the commands log of the shell
		    wait(&status);
		    if(status==0){
		    	sprintf(counterString,"%d",counter);
		    	strcat(counterString,". ");
		    	counter++;
		    	if(write(fd_commands,counterString,strlen(counterString))==-1)
			    		perror("Writing failed");
		    	if(write(fd_commands,buff,strlen(buff))==-1)
			    		perror("Writing failed");
		    	if(write(fd_commands,"\n",strlen("\n"))==-1)
		    			perror("Writing failed");
		    }
		} else {
		    // Fork failed
		    perror("Fork failed");
		    exit(EXIT_FAILURE);
		}
	}
	return 0;
}
