#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int counterOfCommands(){
	// The function returns the number of the next command that is going to be in the advanced commands log
	// Open the commands log
	int fd_commands = open("Commands/Advanced/Advanced_Commands.txt", O_RDONLY);
    	if (fd_commands == -1) {
        	perror("open Advanced_Commands.txt failed");
        	return 1;
    	}
    	int counter = 1;
    	char buf;
    	while(read(fd_commands,&buf,1)){
    		// Gets the number of the next succesfull program in the log by counting the amount of '\n' and adds 1 
    		if(buf=='\n')
    			counter++;
    	}
    	close(fd_commands);
    	return counter;
}
int main() {
	/*The main function. It implements a shell which can call 6 programs, any other command will print to the screen Not Supported.
	The shell runs untill "Esc" is entered to exit the shell. It also keeps a log of successfull programs
	in the directory it created.*/
	// Variable declaration
	char buff[256];
	char *args[5]; // Array to store arguments for execve
	int status;
	char counterString[256]="";
	// Creates the Advanced directory in Commands using mkdir -p
	char * argsc[4];
	int pid = fork();
	if(pid==0){	
		argsc[0]="mkdir";
		argsc[1]="-p";
		argsc[2]="Commands/Advanced";
		argsc[3]=NULL;
		execvp(argsc[0],argsc);
		exit(1);
	}
	else if (pid>0){
		wait(&status);
		if(status!=0)
			exit(1);
		}
	else{
		// Fork failed
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
    	// Open (or create) the file "Advanced_commands.txt" inside the "Commands/Advanced" directory
    	int fd_commands = open("Commands/Advanced/Advanced_Commands.txt", O_WRONLY | O_APPEND | O_CREAT , 0664);
    	if (fd_commands == -1) {
        	perror("open failed");
        	return 1;
    	}
    	// Gets the number of the next succesfull program using counterOfCommands()
    	int counter = counterOfCommands();
    	// The loop of the shell, will continue as long as Esc wasn't entered or some kind of OS calling failed
    	while (1) {
        	// Prompt current shell
        	fprintf(stdout, "AdvShell> ");
        
        	// Read a full line of input (with spaces) to string buff
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

		// Check for "Esc" to exit the loop
		if (strcmp(buff, "Esc") == 0) {
			for(int i=0;i<5;i++)
				if(args[i]!=NULL)
					free(args[i]);
			break;
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
			// End of a word(argument)
			if(buff[i]==' '||buff[i]=='\0'){
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
		int pid = fork();
		if (pid == 0) { // The child process: it will do one of the 6 programs below, any other program(except call onto Esc), will print Not Supported
		    if(args[0]!=NULL){
			    if(strcmp(args[0],"Merge")==0||strcmp(args[0],"UnMerge")==0||strcmp(args[0],"FindMax")==0||strcmp(args[0],"Size")==0||strcmp(args[0],"Delete")==0){
			    	execve(args[0],args,NULL);
			    }
			    else if(strcmp(args[0],"History")==0){
			    // Calling History program is a bit different, since history prints the Advanced_commands it needs to be in the log before the print,
			    // so we entered History in the log before calling it.
			    	sprintf(counterString,"%d",counter);
		    		strcat(counterString,". ");	  		
		    		if(write(fd_commands,counterString,strlen(counterString))==-1)
			    		perror("Writing failed");
		    		if(write(fd_commands,buff,strlen(buff))==-1)
			    		perror("Writing failed");
		    		if(write(fd_commands,"\n",strlen("\n"))==-1)
		    			perror("Writing failed");
		    		// Also, since we have only 1 History program, we must specify which log to print, in this case since we are in the Advanced shell it will print
		    		// the log of the Advanced shell
			    	args[arg_count++]= "Commands/Advanced/Advanced_Commands.txt";
			    	args[arg_count] = NULL;
			    	execve(args[0],args,NULL);
			    }
			    else{
			    	fprintf(stdout, "Not Supported\n");
			    }
			} 
			exit(1);
		  
		} else if (pid > 0) {
		    // Parent process: wait for child to finish, adds the successfull programs (except History) to the log	    
		    wait(&status);
		    if(status==0 && strcmp(args[0],"History") == 0){
		    	// we may have increased counter in the child process but variables values aren't shared between parent process to child process, so we increase it here
		    	// on a note this is only possible for a child that called history to get here
		    	counter++;
		    }
		    // Adds the rest of the successfull programs
		    else if(status==0 &&strcmp(args[0],"History") != 0){
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
	// Closes the Advanced commands log and exits the shell
	close(fd_commands);
	return 0;
}
