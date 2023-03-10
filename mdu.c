/**
 * This is the implementation file for the mdu program.
 *  
 * @file mdu.c
 * @author Jakob Mukka
 * @date 2023-03-10
 */

#include "mdu.h"
#include "stacks.h"
 
/** 
 * Struct that keeps information that each thread needs,
 * if the search is to be done in parallel.
 */
struct threadInformation {
	int threadNumber;
	int threadAmount;
	char startDirectory[PATH_MAX];
	pthread_cond_t *cond;
	pthread_mutex_t *mutex;
	int *exitValuePointer;
};

/**
 * Main method for the mdu program.
 *
 * @param argc			The amount of arguments for the program.
 * @param argv  		The list of arguments for the program.
 * @return exitValue	The exit value for the program.
 */
int main (int argc, char **argv) {
		
	char *threadAmountString = NULL;
	int threadAmount;
	int option;
	int jflag = 0;
	// Goes through the arguments in order to find the -j option (for multiple threads).
	while((option = getopt(argc, argv, "j:")) != -1) {
		switch (option) {	
			case 'j':
				jflag = 1;
				threadAmountString = strdup(optarg);
				break;
		}
	}
	
	// Sets the thread amount.
	if (threadAmountString != NULL) {
		sscanf(threadAmountString, "%d", &threadAmount);
		free(threadAmountString);
	}
	
	/* The amount of files/directories that the user has entered,
	 * gets set via a pointer in the getFiles method.
	 */
	int fileAmount = 0;
	int *fileAmountPointer = &fileAmount;
	
	// Gets the files from the program arguments.
	char **files = getFiles(argc, argv, argv[optind-1], fileAmountPointer);
		
	int exitValue;
	// If the search is to be done recursively.
	if (jflag == 0) {	
		exitValue = calculateSizeOnDiskRecursive(files, fileAmount);
	}
	
	// If the search is to be done in parallel.
	else if (jflag == 1) {
		exitValue = calculateSizeOnDiskParallel(files, fileAmount, threadAmount);
	}
	
	exit(exitValue);
}

/**
 * Gets the files/directories from the program arguments.
 *
 * @param argc				The amount of arguments.
 * @param argv				The list of arguments.
 * @param optind			The option argument (ex -j3).
 * @param fileAmountPointer	The pointer to the fileAmount variable.
 * @return files			The list of files/directories.
 */
char **getFiles(int argc, char **argv, char *optind, int *fileAmountPointer) {
		
	// Allocates memory for the list of files/directories.
	char **files = malloc(argc*sizeof(char*));
	
	// Error checks the allocation of memory for the list of files/directories.
	if (files == NULL) {
		perror("Fatal Error:");
		exit(EXIT_FAILURE);
	}
	
	int fileIndex = 0;
	int argumentIndex = 1;
	// Goes through the arguments one by one.
	while (argumentIndex < argc) {
		
		// If the file is not equal to the option argument it gets added to the list.
		if (strcmp(argv[argumentIndex], optind) != 0) {				
			files[fileIndex] = argv[argumentIndex];
			fileIndex++;
		}	
		
		argumentIndex++;
	}
	
	*fileAmountPointer = fileIndex;
	
	return files;
}

/**
 * Calculates the size a list of files/directories takes on the disk recursively.
 *
 * @param files			The list of files/directories.
 * @param fileAmount	The amount of files/directories.
 * @return exitVal		The exit value of the program.
 */
int calculateSizeOnDiskRecursive(char **files, int fileAmount) {
	
	// String to store the current path in the search.
	char currentPath[PATH_MAX];
	
	/**
	 * Pointer to the current path string which gets sent in to,
	 * the searchDirectoryRecursive function.
	 */
	char *pathPointer = currentPath;

	// Sets the default exit value to success.
	int exitVal = EXIT_SUCCESS;
	
	/**
	 * Pointer to the exit value which gets sent in to,
	 * the searchDirectoryRecursive function.
	 */
	int *exitValuePointer = &exitVal;
	
	// The total block amount for all files.
	blkcnt_t  totalBlockAmount = 0;
	
	// The block amount for one individual file.
	blkcnt_t  blockAmountForFile = 0;
	
	struct stat fileStat;
	int index = 0;
	// Goes through the list of files.
	while (index < fileAmount) {
			
		// Stores the file info in the fileStat struct.
		int statCheck = lstat(files[index], &fileStat);

		// Error checks the storing of the file info.
		if (statCheck == -1) {
			perror("stat");
			free(files);
			exit(EXIT_FAILURE);
		}
			
		// Checks if the current file is a directory.
		int fileCheck = S_ISDIR(fileStat.st_mode);
		
		// If the current file is a directory.
		if (fileCheck != 0) {
			
			// Copies the file into the current path.
			strcpy(pathPointer, files[index]);
			
			// Checks if the directory can be opened.
			int directoryCheck = checkDirectory(files[index], pathPointer);
			
			// If the directory can be opened it can be recursively searched.
			if (directoryCheck == 0) {
				
				// Starts the recursive search of the directory.				
				totalBlockAmount = searchDirectoryRecursive(files[index], 0, exitValuePointer, pathPointer);
				
				// Changes back to the previous directory.
				int changeDirectoryCheck = chdir("..");
				
				// Error checks the directory change.
				if (changeDirectoryCheck == -1) {
					perror("Fatal Error:");
					exit(EXIT_FAILURE);
				} 
			}

			/** 
			 * If the directory can not be opened the exit value is set to failure,
             * and the function continues to the next file instead. 
			 */
			else if (directoryCheck == 1) {
				exitVal = EXIT_FAILURE;
			}
		}
		
		// Gets the number of blocks allocated to the file.
		blockAmountForFile = fileStat.st_blocks;
				
		// Adds it to the total amount of blocks.
		totalBlockAmount = blockAmountForFile + totalBlockAmount;
			
		// Prints out the disk usage of the current file.
		printf("%ld	%s\n", totalBlockAmount, files[index]);
		
		// Resets the block amount.
		totalBlockAmount = 0;
			
		index++;
	}
		
	free(files);
	return exitVal;
}

/**
 * Calculates the size a directory takes on the disk recursively.
 *
 * @param directory			The directory to be searched.
 * @param totalBlockAmount	The amount of blocks the directory takes on the disk.
 * @param exitValuePointer	A pointer to the programs exit value.
 * @param pathPointer		A pointer to the current path in the search.
 * @return totalBlockAmount The amount of blocks the directory takes on the disk.
 */
blkcnt_t searchDirectoryRecursive(char *directory, blkcnt_t totalBlockAmount, int *exitValuePointer, char *pathPointer) {
	
	// Opens the directory.
	DIR *directoryPointer;	
	directoryPointer = opendir(directory);
		
	// Error checks the opening of the directory.
	if (directoryPointer == NULL) {
		perror("Fatal Error:");
		exit(EXIT_FAILURE);
	}
	
	/* The amount of files/directories in the directory,
	 * gets set via a pointer in the getFilesInDirectory method.
	 */
	int fileAmount = 0;
	int *fileAmountPointer = &fileAmount;
	
	// Gets the files in the directory.
	char **files = getFilesInDirectory(directoryPointer, fileAmountPointer);
		
	// Changes the current working directory to the directory to be searched.
	int changeDirectoryCheck = chdir(directory);
	
	// Error checks the directory change.
	if (changeDirectoryCheck == -1) {
		perror("Fatal Error:");
		exit(EXIT_FAILURE);
	}
	
	// Variable to hold the block count for each file.
	blkcnt_t  blockAmountForFile;
	
	int index = 0;
	// Goes through each file in the directory.
	while (index < fileAmount) {
		
		// Struct to store info about the current file.
		struct stat fileStat;
			
		// Stores the file info in the fileStat struct.
		int statCheck = lstat(files[index], &fileStat);
		
		// Error checks the storing of the file info.
		if (statCheck == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}
		
		// Checks if the current file is a directory.
		int fileCheck = S_ISDIR(fileStat.st_mode);
					
		// If the current file is a directory.
		if (fileCheck != 0) {
			
			// Copies the current file into the current path.
			strcat(pathPointer, "/");
			strcat(pathPointer, files[index]);
			
			// Checks if the directory can be opened.
			int directoryCheck = checkDirectory(files[index], pathPointer);
			
			/**
			 * If the directory can be opened the function continues with the,
			 * recursive search of the directory.
			 */
			if (directoryCheck == 0) {
								
				// The method calls itself recursively with the current file as a directory.
				totalBlockAmount = searchDirectoryRecursive(files[index], totalBlockAmount, exitValuePointer, pathPointer);
				
				// Changes back to the previous directory.
				changeDirectoryCheck = chdir("..");
				
				// Error checks the directory change.
				if (changeDirectoryCheck == -1) {
					perror("Fatal Error:");
					exit(EXIT_FAILURE);
				} 
			}
			
			/**
			 * If the directory can not be opened the exit value is set to failure,
			 * and the function continues to the next file instead. 
			 */
			else if (directoryCheck == 1) {
				*exitValuePointer = EXIT_FAILURE;
			}
		}
				
		// Gets the number of blocks allocated to the file.
		blockAmountForFile = fileStat.st_blocks;
			
		// Adds it to the total amount of blocks.
		totalBlockAmount = blockAmountForFile + totalBlockAmount;
		
		index++;
	}
	
	// Removes the last directory in the current path.
	for (int i = (strlen(pathPointer)-1); i >= 0; i--) {		
		if (pathPointer[i] == '/') {
			pathPointer[i] = '\0';
			break;
		}
	}
	
	// Closes the directory and frees the files.
	closedir(directoryPointer);
	free(files);
	
	return totalBlockAmount;
}

/**
 * Calculates the size a list of files/directories takes on the disk in parallel.
 * 
 * @param files			The list of files/directories.
 * @param fileAmount	The amount of files/directories.
 * @param threadAmount	The amount of threads to be used.
 * @return exitVal		The exit value of the program.
 */
int calculateSizeOnDiskParallel(char **files, int fileAmount, int threadAmount) {
	
	// Initiates the lock.
	pthread_mutex_t mutex;
	int lockCheck = pthread_mutex_init(&mutex, NULL);
	
	// Error checks the initiation of the lock.
	if (lockCheck != 0) {
		perror("mutex");
		exit(EXIT_FAILURE);
	}
	
	// Initiates the conditional variable.
	pthread_cond_t cond;
	int conditionCheck = pthread_cond_init(&cond, NULL);
	
	// Error checks the initiation of the conditional variable.
	if (conditionCheck != 0) {
		perror("condition variable");
		exit(EXIT_FAILURE);
	}
	
	// Gets the current working directory of where the program was started from.
	char startingDirectory[PATH_MAX];
	getcwd(startingDirectory, sizeof(startingDirectory));
		
	// Error checks the getting of the current working directory.
	if (startingDirectory == NULL) {
		perror("getcwd");
		exit(EXIT_FAILURE);
	}
		
	// Creates an array of thread info structs.
	struct threadInformation threadInfos[threadAmount];
	
	// Sets the default exit value to be EXIT_SUCCESS.
	int exitval = EXIT_SUCCESS;
	
	int threadIndex = 0;
	// Adds default wait statuses (-1).
	while (threadIndex <= threadAmount) {
		addWaitStatus(threadIndex, -1);	
		threadIndex++;
	}
	
	// Initiates an array of threads.
	pthread_t threads[threadAmount];
	
	// The total block amount for one of the files/directories in the files list.
	blkcnt_t  totalBlockAmount = 0;
	
	// Block amount for a single file/directory.
	blkcnt_t  blockAmountForFile = 0;
	
	// Block amount for a whole directory.
	blkcnt_t  blockAmountForDirectory = 0;
	
	int index = 0;
	// Goes through the list of files/directories.
	while (index < fileAmount) {
		
		/**
		 * Resets the block amounts (block amount for file does not need resetting,
		 * as it gets assigned a new value each iteration)
		 */
		blockAmountForDirectory = 0;
		totalBlockAmount = 0;
				
		// Struct to store info about the current file.
		struct stat fileStat;
		
		// Stores the file info in the fileStat struct.
		int statCheck = lstat(files[index], &fileStat);

		// Error checks the storing of the file info.
		if (statCheck == -1) {
			perror("stat");
			free(files);
			exit(EXIT_FAILURE);
		}
		
		// Checks if the current file is a directory.
		int fileCheck = S_ISDIR(fileStat.st_mode);
		
		// If the current file is a directory.
		if (fileCheck == 1) {
			
			// Adds the first directory to the stack.
			addDirectory(files[index]);
	
			threadIndex = 0;
			// Goes through each thread.
			while (threadIndex <= threadAmount) {
								
				/**
				 * The program should always run with it atleast 1 thread, 
				 * so if the user has specified 0 threads (-j0) the program will,
				 * execute with 1 thread instead.
				 */ 
				if (threadAmount == 0) {
					threadInfos[threadIndex].threadAmount = 1;
				}
				
				// If the thread amount is not 0.
				else {
					threadInfos[threadIndex].threadAmount = threadAmount;
				}
				
				// Prepares the rest of the thread info struct that gets sent into the function.
				threadInfos[threadIndex].threadNumber = threadIndex;
				strcpy(threadInfos[threadIndex].startDirectory, startingDirectory);
				threadInfos[threadIndex].cond = &cond;
				threadInfos[threadIndex].mutex = &mutex;
				threadInfos[threadIndex].exitValuePointer = &exitval;
				
				// Creates a thread to run the searchDirectoryParallel function.
				int createCheck = pthread_create(&threads[threadIndex], NULL, searchDirectoryParallel, &threadInfos[threadIndex]);

				// Error checks the creation of the thread.
				if (createCheck != 0) {
					perror("pthread_create");
					exit(EXIT_FAILURE);
				}
				
				if ((threadIndex == threadAmount-1) || (threadAmount == 0)) {
					break;
				}
				
				threadIndex++;
			}
			
			threadIndex = 0;
			// Goes through each thread.
			while (threadIndex <= threadAmount) {
				
				void *sumPointer;
				
				// Waits for the thread to terminate.
				int joinCheck = pthread_join(threads[threadIndex], &sumPointer);
				
				// Error checks the waiting of the thread.
				if (joinCheck != 0) {
					perror("pthread_join");
					exit(EXIT_FAILURE);
				}
				
				// Adds the block amount that the thread has summed to the block amount for the directory.
				blockAmountForDirectory = blockAmountForDirectory + (blkcnt_t)sumPointer;
				
				if ((threadIndex == threadAmount-1) || (threadAmount == 0)) {
					break;
				}
				
				threadIndex++;
			}
			
			// Changes back to the start directory.
			int changeDirectoryCheck = chdir(startingDirectory);
			
			// Error checks the switch to the start directory.
			if (changeDirectoryCheck != 0) {
				perror("chdir");
				exit(EXIT_FAILURE);
			}
		}
		
		// Gets the number of blocks allocated to the file.
		blockAmountForFile = fileStat.st_blocks;
				
		// Adds the block amount for the file/directory to the total block amount.
		totalBlockAmount = blockAmountForFile + blockAmountForDirectory;
			
		// Prints out the disk usage of the current file.
		printf("%ld	%s\n", totalBlockAmount, files[index]);
				
		index++;
	}
	
	// Destroys the lock and the condition variable.
	pthread_mutex_destroy(&mutex);	
	pthread_cond_destroy(&cond);	

	// Frees the files and the wait statuses.
	free(files);
	freeWaitStatuses();
	
	return exitval;
}

/**
 * Calculates the size a directory takes on the disk in parallel.
 *
 * @param info	The information that each thread needs in order to do the search.
 */
void *searchDirectoryParallel(void *info) {
	
	// Stores the thread info in local variables (for easier use).
	struct threadInformation *threadInfo = (struct threadInformation*)info;
	int threadNumber = (*threadInfo).threadNumber;
	int threadAmount = (*threadInfo).threadAmount;
	char startdir[PATH_MAX];
	strcpy(startdir, (*threadInfo).startDirectory);	
	pthread_mutex_t *mutex = (*threadInfo).mutex;
	pthread_cond_t *cond = (*threadInfo).cond;
	
	// Sets the default exit value to success.
	pthread_mutex_lock(mutex);
	*(*threadInfo).exitValuePointer = EXIT_SUCCESS;
	pthread_mutex_unlock(mutex);
	
	blkcnt_t totalBlockAmount = 0;
	// Loop that will iterate until all threads are waiting.
	while (1) {
		
		pthread_mutex_lock(mutex);
		 
		// Waits while the stack is empty.
		int exitcondition = 1;
		while (directoriesIsEmpty() == 0) {
			
			// Sets the wait status for the thread.
			changeWaitStatus(threadNumber, 1);
			
			// Checks the wait statuses for all threads.
			int waitCount = checkWaitStatuses();
			
			// If all threads are waiting.
			if (waitCount == (threadAmount)) {
				
				// Sets the exit condition and wakes all other threads with a broadcast.
				exitcondition = 0;
				pthread_cond_broadcast(cond);
				pthread_mutex_unlock(mutex);		
				break;
			}
			pthread_cond_wait(cond, mutex);
		}
		
		// Exits the loop so the thread can exit.
		if (exitcondition == 0) {
			break;
		}
		
		// If the thread gets here its no longer waiting so the status gets changed to active.
		changeWaitStatus(threadNumber, 0);
		
		// Gets a directory from the stack.
		char *directory = getDirectory();
		
		pthread_mutex_unlock(mutex);
		
		// Opens the directory
		DIR *directoryPointer = opendir(directory);
		
		// Error checks the opening of the directory.
		if (directoryPointer == NULL) {			
			char errorString[PATH_MAX];
			strcpy(errorString, "du: cannot read directory '");
			strcat(errorString, directory);
			perror(errorString);
			exit(EXIT_FAILURE);
		}

		struct stat fileStat;
		struct dirent *entry;
		// Goes through each entry in the directory.
		while ((entry = readdir(directoryPointer)) != NULL) {
			
			// If the current entry is not "." (link to current directory) or ".." (link to previous directory).
			if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
								
				// Creates the path for the file.
				char fileToCheck[PATH_MAX];
				strcpy(fileToCheck, directory);
				strcat(fileToCheck, "/");
				strcat(fileToCheck, entry->d_name);
				
				// Stores the file info in the fileStat struct.
				int statCheck = lstat(fileToCheck, &fileStat);
				
				// Error checks the storing of the file info.
				if (statCheck == -1) {
					perror("stat");
					exit(EXIT_FAILURE);
				}
				
				// Adds the files block amount to the total block amount.
				totalBlockAmount = fileStat.st_blocks + totalBlockAmount;
				
				// Checks if the file is a directory.
				int directoryCheck = S_ISDIR(fileStat.st_mode);
				
				// If the file is a directory.
				if (directoryCheck == 1) {
					
					// Opens the directory.
					DIR *directoryPointer = opendir(fileToCheck);
					
					// If the directory can't be opened.
					if (directoryPointer == NULL) {
						
						// Prepares the error message.
						char errorString[PATH_MAX];
						strcpy(errorString, "du: cannot read directory '");
						strcat(errorString, fileToCheck);
						strcat(errorString, "'");
						
						pthread_mutex_lock(mutex);
						// Sets the exit value to failure.
						*(*threadInfo).exitValuePointer = EXIT_FAILURE;
						pthread_mutex_unlock(mutex);
						
						// Prints out the error message and closes the directory.
						perror(errorString);
						closedir(directoryPointer);
					}
					
					// If the directory can be opened.
					else {
						// Closes the directory.
						closedir(directoryPointer);
						
						// Prepares the new directory (with it's path) to be added to the stack.
						char newDirectory[PATH_MAX];
						strcpy(newDirectory, directory); 					
						strcat(newDirectory, "/");
						strcat(newDirectory, entry->d_name);
						
						pthread_mutex_lock(mutex);
						// Adds the directory to the stack and wakes the next thread.
						addDirectory(newDirectory);
						pthread_cond_signal(cond);
						pthread_mutex_unlock(mutex);
					}
				}	
			}
		}
		
		// Frees and closes the directory.
		free(directory);
		closedir(directoryPointer);
	}
	
	return (void*)totalBlockAmount;
}

/**
 * Gets all the files/subdirectories in a directory.
 *
 * @param dirp				The directory pointer.
 * @param fileAmountPointer	The pointer to the fileAmount variable.
 * @return files			The files/subdirectories in the directory.
 */
char **getFilesInDirectory(DIR *dirp, int *fileAmountPointer) {
		
	// Allocates memory for the list of files/directories.
	char **files = malloc(1*sizeof(char*));
	
	// Error checks the allocation of memory for the list of files/directories.
	if (files == NULL) {
		perror("Fatal Error:");
		exit(EXIT_FAILURE);
	}
	
	struct dirent *entry;
	int index = 0;
	// Goes through the files in the directory.
	while ((entry = readdir(dirp)) != NULL) {
				
		// Error checks the reading of the next entry.
		if (entry == NULL) {
			perror("readdir");
			exit(EXIT_FAILURE);
		}
		
		/**
		 * If the current file is not "." or ".." (which are placeholders,
		 * for the current directory and the parent directory) the file gets,
		 * added to the list of files.
		 */	
		if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
			
			files[index] = entry->d_name;
			
			files = realloc(files, (index+2)*sizeof(char*));
			index++;
		}
	}
	
	*fileAmountPointer = index;
	
	return files;
}

/**
 * Checks if a directory can be opened.
 *
 * @param directory		The directory to be checked.
 * @param pathPointer	The current path of the search.
 * @return 0 or 1		0 if it can be opened, 1 if it can not be opened. 
 */
int checkDirectory(char *directory, char *pathPointer) {

	// Opens the directory.
	DIR *directoryPointer = opendir(directory);
	
	// Creates the error string.
	char errorString[PATH_MAX];
	strcpy(errorString, "du: cannot read directory '");
	strcat(errorString, pathPointer);
	strcat(errorString, "'");
	
	// Error checks the opening of the directory.
	if (directoryPointer == NULL) {		
		perror(errorString);
		closedir(directoryPointer);
		return 1;
	}
	
	// Closes the directory.
	closedir(directoryPointer);
	return 0;
}











