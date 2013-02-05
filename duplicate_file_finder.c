#include<stdio.h>
#include<dirent.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

//Created By:   		Ibrahim Ahmed...
//Date: 		    	15 Oct, 2012...
//Compile: 			gcc DuplicateFileFinder.c -o duplicatefilefinder.exe
//Run: 				./DuplicateFileFinder.exe parent_dir, filename...

#define false 0
#define true  1

int duplicateCount = 0;

int FindDuplicates(char* path, char* fileName);
int CompareFiles(char* originalFile, char* currFile);

int main(int argc, char *argv[])
{

	if (argc!=3)					//Two additional arguments are expected: Parent dir, file to find duplicates of...
	{
		printf("Usage: %s 'Base Directory' 'File Name'\n", argv[0]);
		return -1;
	}

	FindDuplicates(argv[1], argv[2]);		//argv[1] = base dir, argv[2] = file to find duplicates of; e.g argv[1] = /home, argv[2] = "file.txt"...
	printf("\n\nFound %d duplicate(s)\n", duplicateCount); 
	return 0;
}


int FindDuplicates(char* path, char* fileName)
{
	DIR *dir;
	struct dirent *dp;
	struct dirent *result;
	struct stat statp;

	char absoluteFilePath[255];

	if ((dir = opendir(path))== NULL)
	{
		perror("Failed to open directory");
		return -1;
	}


	while ((dp =readdir(dir)) != NULL)
	{	
		//readdir returns . and .. which we should ignore...
		if (strcmp(dp->d_name, ".") && strcmp(dp->d_name,".."))			//if its not . or ..
		{
			//find file full path, relative to base path. e.g, a /home/file.txt...

			strcpy(absoluteFilePath, path);				//copy path to absoluteFilePath
			strcat(absoluteFilePath, "/");    	 		//append / at end...
 			strcat(absoluteFilePath, dp->d_name); 			//append filename to path...
		

			//check if the current file is actually file or dir...
			stat(absoluteFilePath, &statp);

			if (S_ISDIR(statp.st_mode))					//is a dir...
			{
				FindDuplicates(absoluteFilePath, fileName);		//recurse through this dir...
			}
			else if (S_ISREG(statp.st_mode))				//is a file...
			{
				//check for duplicates here...
				//compare current file with the file specified by user. If true(duplicate), print file name...

				if (strcmp(fileName, absoluteFilePath))	   //if curr file is not the same file as file specified(obviously, they gonna be same)...
				{
					if (CompareFiles(fileName, absoluteFilePath))			//yes, duplicate; print it...
					{
						printf("%s\n", absoluteFilePath);
						duplicateCount++;
					}
				}
				
			}				//end else if (S_ISREG(statp.st_mode))...

		}				//if (strcmp(dp->d_name, ".") && strcmp(dp->d_name,".."))...
	}				//end while...
	
	closedir(dir);
	return 0;

}


int CompareFiles(char* originalFile, char* currFile)
{
	//two step comparison: (1) first check size; if not same, return false. If equal, (2) compare file content. If equal, return true, false otherwise...

	struct stat statOriginal, statCurr;
	stat(originalFile, &statOriginal);
	stat(currFile, &statCurr);

	//Step 1...
	if ((int) statOriginal.st_size != (int) statCurr.st_size)			//size not same...
		return false;

	//Step 2...
	//size matches, files can be same; confirm it by matching both file contents...

	int fdOriginal  = open(originalFile, O_RDONLY);
	int fdCurr	= open(currFile, O_RDONLY);
	
	if (fdOriginal == -1 || fdCurr == -1)
		return false;					//error occurred, not sure if file is duplicate...


	//we will read file in small chunks and compare. If compare not true at any stage, return false...
	int chunkSize = 1024, bytesRead;
	char *bufferOriginal  = (char*) malloc(chunkSize * sizeof(char));
	char *bufferCurr      = (char*) malloc(chunkSize * sizeof(char));

	
	while (true)
	{
		//read file in chunk...
		bytesRead = read(fdOriginal, bufferOriginal, chunkSize);
		if (bytesRead <= 0)
			break;								//end of file...

		bytesRead = read(fdCurr, bufferCurr, bytesRead);
		
		//compare buffer...
		if (strcmp(bufferOriginal, bufferCurr))					//if content not matching...
			return false;			
	}
	
	return true;
}

