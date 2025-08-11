#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int writeWords(char* file1,char* file2, char* file3){
	/* The function gets 3 file names and from the first it puts its first word in second file and puts the second word in the third file
	and so on untill it reached to the end of the first file*/
	//variable declaration
	int word_count=0;
	int fd_file1, fd_file2, fd_merge;
	char buff;
	// Open the first file to read and the rest to write
	if((fd_merge = open(file1,O_RDONLY,0664))==-1){
		perror("Open failed");return -2;
	}
	if((fd_file1 = open(file2,O_WRONLY | O_CREAT| O_TRUNC,0664))==-1){
		perror("Open failed");close(fd_merge); return -2;
	}
	if((fd_file2 = open(file3,O_WRONLY | O_CREAT| O_TRUNC,0664))==-1){
		perror("Open failed");close(fd_merge);close(fd_file1); return -2;
	}	
	while(read(fd_merge,&buff,1)>0){ // The loop runs as long as the inputf file can be read
		if(write(word_count % 2 == 0 ? fd_file1 : fd_file2,&buff,1)==-1){ 
		// it will write a character to the file based on the modulo 2 of word_count, if its 0 its the first output file and if its 1 its the second output file
				perror("Write failed");close(fd_merge);close(fd_file1);close(fd_file2); return -3;
		}
		if(buff==' '){ // End of a word, increases word_count by 1
			word_count++;
		}		
	}
	close(fd_merge);close(fd_file1);close(fd_file2);
	return 0;
}
int main(int argc, char* argv[]){
	/*The main function*/
	if(argc!=4){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Variable decleration
	int fd_file1, fd_file2, fd_merge,offset=0;
	char buff;
	// Creates the path of the helper file
	char numOfWords[256]="Commands/Advanced/";
	strcat(numOfWords,argv[2]);
	strcat(numOfWords,"_");
	strcat(numOfWords,argv[3]);
	strcat(numOfWords,".txt");
	int fd_numOfWords=open(numOfWords,O_RDONLY,0664);
	if(fd_numOfWords==-1){ // Checks if the helper file doesn't exist (in other words we didn't merge the merged file using the other files)
	// So it calls writeWords() function to unmerge not according to the helper file
		if(writeWords(argv[1],argv[2],argv[3])!=0){
			perror("writeWords failed"); return -3;
		}
	}		
	else{
		// the amount of words of the input file and check if the amount of words is the same as the sum of amounts in the helper file
		int count1,count2,countmerge=0;
		char buffer[256]="";
		// Counts the amount of words in the input file
		if((fd_merge = open(argv[1],O_RDONLY,0664))==-1){
			perror("Open failed");return -2;
		}
		while(read(fd_merge, buffer, 1) > 0) {
	      		if(*buffer == ' '||*buffer=='\n')
				countmerge++;
	    	}
	    	close(fd_merge);
	    	// Gets the amount of words from the first file in the helper file
		while(read(fd_numOfWords, buffer, 1) > 0) {
	      		if(*buffer!=' '){
	      			break;
	      		}
	    	}
	    	count1 = atoi(buffer);  
	    	// Gets the amount of words from the second file in the helper file
		while(read(fd_numOfWords, buffer, 1) > 0) {
			if(*buffer!=' '){
	      			break;
	      		}
	    	}
	    	count2 = atoi(buffer);	
		close(fd_numOfWords);
		if(count1+count2!=countmerge){
		// If the amount of words isn't the same it will call writeWords() function to unmerge not according to the helper file
			if(writeWords(argv[1],argv[2],argv[3])!=0){
			perror("writeWords failed"); return -3;
			}
		}
		else{
		// If we reached here that means that the first file is a merged file and the helper file and merge file are compatibale,
		// according to the helper file it will unmerge the input file
			// Opens the merged file to read and the output files to write
			if((fd_merge = open(argv[1],O_RDONLY,0664))==-1){
				perror("Open failed");return -2;
			}
			if((fd_file1 = open(argv[2],O_WRONLY | O_CREAT,0664))==-1){
				perror("Open failed");close(fd_merge); return -2;
			}
			if((fd_file2 = open(argv[3],O_WRONLY | O_CREAT,0664))==-1){
				perror("Open failed");close(fd_file1);close(fd_merge); return -2;
			}	
			while(read(fd_merge,&buff,1)>0){
			// The loop runs as long as the merged file still has characters to read
				if(write(offset%2==0 ? fd_file1:fd_file2,&buff,1)==-1){
				// According to the value of offset, it will choose to which file to write (file1 or file2)
						perror("Write failed");close(fd_file1);close(fd_file2);close(fd_merge); return -3;
				}
				if(buff==' '){ // End of a word has been detected
					if(offset==0){
						// If the offset is 0 it points to the first output file, it checks that the amount of words in the second file isn't 0 and only then changes 
						// offset to 1 (points to the second file)
						count1--; // Decreases the amount of words left to write to file 1						
						if(count2>0){
							//count1--;
							offset=1;
						}
					}
					else{
						// Otherwies, the offset is 1 it points to the second output file, it checks that the amount of words in the first file isn't 0 and only then changes 
						// offset to 0 (points to the first file)
						count2--; // Decreases the amount of words left to write to file 2
						if(count1>0){
							//count2--;
							offset=0;
						}
					}
				}
			}
			close(fd_file1);close(fd_file2);close(fd_merge);
			// Deletes the helper file using program Delete
			int pid = fork();
			if(pid==0){
				if(execl("./Delete","Delete",numOfWords,NULL)==-1){
					exit(1);
				}
				exit(0);
			}
			if(pid>0){
				wait(NULL);
			}
			else{
				perror("Fork failed");
			}
		}
		
	}
	// Prints Mission Complete to the terminal and exits the program
	fprintf(stdout,"Mission Complete\n");
	exit(0);
}

