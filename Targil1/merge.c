#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){
	/*The main function. This program is called from the Advanced shell, it gets three file names: the last file will be a merged file from both of the other files
	in the next order: first word from the first file, second word from the second file and it repeats that until one of the files is has been read completly, then it
	will add the rest of the words of the other file, also it counts how many words there are in each file (first and second files) and puts it in a temorary file
	in Commands/Advanced for later use (UnMerge)*/
	// Checks that the amount of arguments is 4 otherwise will print "Missing parameters!!!\n" and exit the program with -1
	if(argc!=4){fprintf(stdout,"Missing parameters!!!\n"); return -1;}
	// Variable declaration
	int fd_file1, fd_file2, fd_merge,rebytes1,rebytes2,fd_numOfWords,count1=0,count2=0;
	char buff;
	// Opens each of the files, the input files as read only and the output as write only
	if((fd_file1 = open(argv[1],O_RDONLY,0664))==-1){
		perror("Open failed"); return -2;
	}
	if((fd_file2 = open(argv[2],O_RDONLY,0664))==-1){
		perror("Open failed");close(fd_file1); return -2;
	}
	if((fd_merge = open(argv[3],O_WRONLY | O_CREAT,0664))==-1){
		perror("Open failed");close(fd_file1);close(fd_file2); return -2;
	}
	// The loop will merge the to files in the order explained before and will count the words of each of the input files
	while(1){
		while((rebytes1=read(fd_file1,&buff,1))>0){ 
		// Reads a word from the first file (character by chracter) and writes it to the merged file, the loop ends when a word has been written to the merged file
			if(buff!='\n'&&buff!=' '){ // Not the end of a word
				if(write(fd_merge,&buff,rebytes1)==-1){
						perror("Writing error");close(fd_file1);close(fd_file2);close(fd_merge);return -3;}	
			}
			else{ // End of a word
				if(buff == ' '||buff=='\n')
					count1++;
				buff=' ';
				if(write(fd_merge,&buff,rebytes1)==-1){
						perror("Writing error");close(fd_file1);close(fd_file2);close(fd_merge);return -3;}
				break;
			}

		}
		while((rebytes2=read(fd_file2,&buff,1))>0){
		// Reads a word from the second file (character by chracter) and writes it to the merged file, the loop ends when a word has been written to the merged file
			if(buff!='\n'&&buff!=' '){ // Not the end of a word
				if(write(fd_merge,&buff,rebytes2)==-1){
						perror("Writing error");close(fd_file1);close(fd_file2);close(fd_merge);return -3;}	
			}
			else{ // End of a word
				if(buff == ' '||buff=='\n')
					count2++;
				buff=' ';
				if(write(fd_merge,&buff,rebytes2)==-1){
						perror("Writing error");close(fd_file1);close(fd_file2);close(fd_merge);return -3;}
				break;
			}
		}
		if(rebytes1<=0 &&rebytes2<=0){ // The loop ends when there is nothing left to read from both of the files
			close(fd_file1);close(fd_file2);close(fd_merge);
			break;}
	}
	// The merged file has completed, now whats left is to keep the amount of words of the two input files in a helper file
	// Creates the name of the helper file and chooses the file will be in Commands/Advanced directory: Commands/Advanced/file1_file2.txt
	char numOfWords[256] ="Commands/Advanced/";
	strcat(numOfWords,argv[1]);
	strcat(numOfWords,"_");
	strcat(numOfWords,argv[2]);
	strcat(numOfWords,".txt");
	// Opens the helper file
	if((fd_numOfWords = open(numOfWords,O_WRONLY | O_CREAT,0664))==-1){
		perror("Open failed:numofwords");return -2;
	}
	// Writes onto the helper file the amount of words in file1
	char buffer1[256]="";
	sprintf(buffer1,"%d",count1);
	if(write(fd_numOfWords,&buffer1,strlen(buffer1))==-1){
		perror("Writing error");close(fd_numOfWords);return -3;}
	// // Writes onto the helper file " " to seperate between the amounts
	strcpy(buffer1," ");
	if(write(fd_numOfWords,&buffer1,strlen(buffer1))==-1){
		perror("Writing error");close(fd_numOfWords);return -3;}
	// Writes onto the helper file the amount of words in file2
	sprintf(buffer1,"%d",count2);
	if(write(fd_numOfWords,&buffer1,strlen(buffer1))==-1){
		perror("Writing error");close(fd_numOfWords);return -3;}
	close(fd_numOfWords);
	fprintf(stdout,"Mission Complete\n");
	exit(0);
}

