/**
*Programmers: Andrew Huy(Skellaton code) 2/17/17
*             Rocio Salguero (Completed archiving and extracting files)
* Code reused and modified for listdir();
* How to recursively list directories in C on LINUX
* Owner: lloydm
* Source: http://stackoverflow.com/questions/8436841/
* Reused code not modified for chopping off n chars from char array 
*  Owner: Jonathan Leffler
*  Source: http://stackoverflow.com/questions/4761764/
* Date created: 02/13/17
* Last Modified: 04/12/17
*          Works well at archiving and extracting directories and files.
*          Added error checks, will return error is archive to read DNE.
*          Will also return errors if File or Directory already exists
*          and if failed to create a file/directory. Tested only in normal text files
*          but using read/write system calls should work for any files.

	  Works with pictures and any size file. No more segmentation faults or buss errors. 
* Known issues:
*           If a file in a directory is edited linux seems 
*           to create a temp file with same name plus '~'. 
 *          If such files exist archiving cases issues as well as extracting.
*           

* 
*/

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cstring>
#include <dirent.h>

#define BUFFER_SIZE 8112
#define FILE_TYPE 'F'
#define DIR_TYPE 'D'

//You must fill out your name and id below
char * studentName = (char *) "Rocio Salguero ";
char * studentCWID = (char *) "891379752";

//Do not change this section in your submission
char * usageString =
        (char *) "To archive a file: 		fzip -a INPUT_FILE_NAME  OUTPUT_FILE_NAME\n"
                "To archive a directory: 	fzip -a INPUT_DIR_NAME   OUTPUT_DIR_NAME\n"
                "To extract a file: 		fzip -x INPUT_FILE_NAME  OUTPUT_FILE_NAME\n"
                "To extract a directory: 	fzip -x INPUT_DIR_NAME   OUTPUT_DIR_NAME\n";

bool isExtract = false;
void parseArg(int argc, char *argv[], char ** inputName, char ** outputName) {
    if (argc >= 2 && strncmp("-n", argv[1], 2) == 0) {
        printf("Student Name: %s\n", studentName);
        printf("Student CWID: %s\n", studentCWID);
        exit(EXIT_SUCCESS);
    }

    if (argc != 4) {
        fprintf(stderr, "Incorrect arguements\n%s", usageString);
        exit(EXIT_FAILURE);
    }

    *inputName  = argv[2];
    *outputName = argv[3];
    if (strncmp("-a", argv[1], 2) == 0) {
        isExtract = false;
    } else if (strncmp("-x", argv[1], 2) == 0) {
        isExtract = true;
    } else {
        fprintf(stderr, "Incorrect arguements\n%s", usageString);
        exit(EXIT_FAILURE);
    }
}
//END OF: Do not change this section

/**
 *
 * Your program should archive or extract based on the flag passed in.
 * Both when extracting and archiving, it should print the output file/dir path as the last line.
 *
 * @param argc the number of args
 * @param argv the arg string table
 * @return
 */

void listdir(const char *name, int copyTo);
void readingFile(const char *name, int copyTo);
void extractingZip(const char *nameid, const char *outName);
void chopName(char *str);
struct stat st;

//Structure of archiving: Flag (char), Size of path(int), PATH (strlen), 
//if file:size of file (ssize_t), file contents
int main(int argc, char** argv) {
    char * inputName, * outputName;
    parseArg(argc, argv, &inputName, &outputName);
    if (isExtract) {
		printf("Extracting %s\n", inputName);

		extractingZip(inputName, outputName);

		char *outputPath = (char *) ""; 	//the path to the file or directory extracted
		
		printf("%s\n", outputName);//relative or absolute path
    } else {
		printf("Archiving %s\n", inputName);
		int writeTo;
		char buf[BUFFER_SIZE];
		
		//Open file to write to 
		writeTo = open(outputName, O_WRONLY|O_CREAT, 0444);
		if (writeTo == -1){
			perror("Failed to create zip/File Already Exists");
			return 1;
		}

		//check if path is a file 
		DIR *dir;
		if (!(dir = opendir(inputName))) {
			//if path  given is file put the fzip file the parents File Flag 
			snprintf(buf, sizeof(buf) - 1, "%c", FILE_TYPE);
			write(writeTo, &buf, sizeof(char));

			//if file
			readingFile(inputName, writeTo);
		}
		else {
			//if path  given is directory put in fzip the file the parents DIR Flag 
			snprintf(buf, sizeof(buf) - 1, "%c", DIR_TYPE);
			write(writeTo, &buf, sizeof(char));

			//Do a directory recursive search 
			listdir(inputName, writeTo);
		}

		//Close everthing done~
		close(writeTo);
		closedir(dir);
		printf("%s\n", outputName);
    }
    return EXIT_SUCCESS;
}


//Function
void listdir(const char *name, int copyTo)
{
	DIR *dir;
	struct dirent *entry;
	struct stat buf;
	int readFrom;
	char buffer[BUFFER_SIZE], header[BUFFER_SIZE], sizeF[BUFFER_SIZE];
	ssize_t ret_in, ret_out;


	if (!(dir = opendir(name)))
		return;
	if (!(entry = readdir(dir)))
		return;

	do {
		//Clear buffer readers first! 
		memset(buffer, 0, BUFFER_SIZE); 
		memset(header, 0, BUFFER_SIZE);
		memset(sizeF, 0, BUFFER_SIZE);

		if (entry->d_type == DT_DIR) {
			char path[1024];
			int len = snprintf(path, sizeof(path) - 1, "%s/%s", name, entry->d_name);
			path[len] = 0;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			//printf("%s[%s]\n", path, entry->d_name);

			//Copy the flag, size of path, path, 
			//fileName, input_file_size
			snprintf(header, sizeof(header) - 1, "%c", DIR_TYPE);
			write(copyTo, &header, sizeof(char));
			snprintf(header, sizeof(header) - 1, "%d", (int)strlen(path));
			write(copyTo, &header, sizeof(int));
			snprintf(header, sizeof(header) - 1, "%s", path);
			write(copyTo, &header, strlen(path));

			listdir(path, copyTo);
		}
		else {
			//printf(" %s File: %s\n",name, entry->d_name);

			char path[1024];
			int len = snprintf(path, sizeof(path) - 1, "%s/%s", name, entry->d_name);
			path[len] = 0;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

			//Copy the flag, size of path, path,
			//fileName, input_file_size
			snprintf(header, sizeof(header) - 1, "%c", FILE_TYPE);
			write(copyTo, &header, sizeof(char));
			snprintf(header, sizeof(header) - 1, "%d", (int)strlen(path));
			write(copyTo, &header, sizeof(int));
			snprintf(header, sizeof(header) - 1, "%s", path);
			write(copyTo, &header, strlen(path));

			//open file
			readFrom = open(path, O_RDWR);

			//Find file size 
			ssize_t input_file_size = lseek(readFrom, 0, SEEK_END);

			//move pointer back to beginning
			lseek(readFrom, 0, SEEK_SET);
			snprintf(sizeF, sizeof(sizeF) - 1, "%lu", input_file_size);
			write(copyTo, &sizeF, sizeof(ssize_t));

			//Copy the contents of a file
			while ((ret_in = read(readFrom, &buffer, BUFFER_SIZE)) > 0){
				ret_out = write(copyTo, &buffer, (ssize_t)ret_in);
				//printf("whileloop copying: %s\n", entry->d_name);
				if (ret_out != ret_in){
					//Write error 
					//perror("Dir file Write");
					return;
				}
			}
		}//end of if

	} while (entry = readdir(dir));

	closedir(dir);
	close(readFrom);
}

//Files just archive the size of flag and contents of file
void readingFile(const char *name, int copyTo) {
	ssize_t ret_in, ret_out;
	char buffer[BUFFER_SIZE], sizeF[BUFFER_SIZE];
	char fileName[100];
	strcpy(fileName, name);

	//Open file to read
	int readFrom;
	readFrom = open(name, O_RDWR);
	if (readFrom == -1){
		perror("Open File to read");
		return;
	}

	//Find file size 
	//ssize_t input_file_size = lseek(readFrom, 0, SEEK_END);

	//move pointer back to beginning
	//lseek(readFrom, 0, SEEK_SET);

	//Find file size
	stat(fileName, &st);
	ssize_t input_file_size = st.st_size;

	//Copy the input_file_size
	snprintf(sizeF, sizeof(sizeF) - 1, "%lu", input_file_size);
	write(copyTo, &sizeF, sizeof(ssize_t));

	//Copy the contents of a file
	while ((ret_in = read(readFrom, &buffer, BUFFER_SIZE)) > 0){
		//printf("File contents: %ssize:%lu\n",buffer, ret_in);
		ret_out = write(copyTo, &buffer, (ssize_t)ret_in);
		if (ret_out != ret_in){
			//Write error 
			perror("File write");
			return;
		}
	}

}

//Structure of archiving: Flag (char), Size of path(int), PATH (strlen), 
//if file:size of file (ssize_t), file contents
void extractingZip(const char *nameid, const char *outName) { 
	//nameid is the zip file to extract. Out name is the name of file/dir outputted
	int fzipFile, newFile, sizeofPath, sizeofFile;

	fzipFile = open(nameid, O_RDONLY);
	if (fzipFile == -1){
		perror("Failed to open zip file");
		return;
	}

	char input[BUFFER_SIZE], buffer[BUFFER_SIZE], fdname[BUFFER_SIZE];
	ssize_t ret_in = 1, ret_out;

	//Read the beginning flag
	//if path  given is directory
	//Make main directory 
	read(fzipFile, &input, sizeof(char));
	if (*input == DIR_TYPE) {
		snprintf(input, sizeof(input) - 1, "%s", outName);
//printf("Root:%s\n", outName);
		if (mkdir(input, 0700) == -1){
			perror("Directory Exists/Failed to create main");
			return;
		}


	    //read until fzip file EOF
	    while (ret_in > 0){
                //Clear buffer readers first! 
		memset(input, 0, BUFFER_SIZE); 
		memset(buffer, 0, BUFFER_SIZE);
		memset(fdname, 0, BUFFER_SIZE);
                
		//Read the flag 
		ret_in = read(fzipFile, input, sizeof(char));

		//If its a directory 
		if(*input == DIR_TYPE)  {
		   //read size of path
		    ret_in = read(fzipFile, input, sizeof(int));
		    sizeofPath =  atoi(input);

		    //Read in the path file
		    ret_in = read(fzipFile, &input, sizeofPath);
                    
                    //take off the parent from archive file 
                    //read name to fdname buffer
                    chopName(input);
		    snprintf(fdname, sizeof(fdname)-1, "./%s%s", outName, input);                    
//printf("Dir:./%s%s\n", outName, input);
		    //make directory
                    if (mkdir(fdname, 0700) == -1){
			perror("Directory Exists/Failed to make (in)");
			return;
                    }
		}
		else if(*input == FILE_TYPE){
		    //read size of path
		    ret_in = read(fzipFile, input, sizeof(int));
		    sizeofPath =  atoi(input);
		    
		    //Read in the path file
		    ret_in = read(fzipFile, &input, sizeofPath);
		    
		    //take off the parent from archive file 
                    //read name to fdname buffer
		    chopName(input);
		    snprintf(fdname, sizeof(fdname)-1, "./%s%s", outName, input);                      
//printf("File:./%s%s\n", outName, input);
		    //get size of file
		    ret_in = read(fzipFile, &input, sizeof(ssize_t));
		    sizeofFile = atoi(input);
			    
		    //makefile
		    newFile = open(fdname, O_WRONLY|O_CREAT, 0666);
		    if (newFile == -1){
                        perror("Failed to create file");
                        return;
                    }


		    //copy contents
			int readAmount;
			//Copy the contents of a file
			if (sizeofFile < BUFFER_SIZE) {
				//copy contents
				read(fzipFile, &input, sizeofFile);
				write(newFile, &input, sizeofFile);
			}
			else {
				readAmount = BUFFER_SIZE;
				while(sizeofFile > 0) {
					read(fzipFile, &input, readAmount);
					write(newFile, &input, readAmount);
					//find amount left 
					sizeofFile = sizeofFile - BUFFER_SIZE; 
					readAmount = sizeofFile; 
	
					if(readAmount>BUFFER_SIZE) readAmount=BUFFER_SIZE;  
				}//end while
			}//end if copy contents
		    
		}//endif file
		
	    }//end while
                return;
	}
	//------------------------If file ONLY---------------------------------
	else{
		//get size of file
		ret_in = read(fzipFile, &input, sizeof(ssize_t));
		sizeofFile = atoi(input);
		//makefile
		newFile = open(outName, O_WRONLY|O_CREAT, 0666);
                if (newFile == -1){
			perror("Failed to create file/File Already Exists");
			return;
                }
		
		int readAmount;
		//Copy the contents of a file
		if (sizeofFile < BUFFER_SIZE) {
			//copy contents
			read(fzipFile, &input, sizeofFile);
			write(newFile, &input, sizeofFile);
		}
		else {
			readAmount = BUFFER_SIZE;
			while(sizeofFile > 0) {
				read(fzipFile, &input, readAmount);
				write(newFile, &input, readAmount);
				//find amount left 
				sizeofFile = sizeofFile - BUFFER_SIZE; 
				readAmount = sizeofFile; 

				if(readAmount>BUFFER_SIZE) readAmount=BUFFER_SIZE;  
			}
		}
		
	} //end if
} //end extractingfZip

//take off the file name before /
void chopName(char *str)
{
    int index;
    for (index = 0; index < strlen(str); index++) {
        if(str[index] == '/') {
            break;
        }
    }
    size_t len = strlen(str);
    if (index > len)
        return;  // Or: n = len;
    memmove(str, str+index, len - index + 1);
}
