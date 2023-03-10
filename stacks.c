/**
 * This is the implementation file for the two stacks (directories and wait statuses),
 * that the program uses.
 *
 * @file stacks.c
 * @author Jakob Mukka
 * @date 2022-11-19
 */
  
#include "stacks.h"

// The directories stack.
struct directory {
    char directoryName[PATH_MAX];
    struct directory *next;
};

// The wait statuses stack.
struct waitStatus {
	int threadNumber;
	int status;
	struct waitStatus *next;
};

// Pointer to the top of the directories stack.
struct directory *directoryTop;

// Pointer to the top of the wait statuses stack.
struct waitStatus *waitStatusTop;

/**
 * Adds a directory to the directories stack.
 *
 * @param dirName	The name of the directory.
 */
void addDirectory(char *dirName) {
	
	// Creates a new directory.
    struct directory *newdirectory;
    newdirectory = (struct directory *)malloc(sizeof(struct directory));
	strcpy(newdirectory->directoryName, dirName);
	
	// If the stack is empty.
    if (directoryTop == NULL) {
        newdirectory->next = NULL;
    } 
	
	// If the stack is not empty.
	else {
        newdirectory->next = directoryTop; 
    }
	
    directoryTop = newdirectory;
	return;
}

/**
 * Gets a directory from the directories stack. 
 *
 * @return tempDirectoryName	The name of the directory.
 */
char *getDirectory(void) {
	
	// Temporary pointer to the current top.
	struct directory *temp = directoryTop;
	
	// Gets the directory name of the current top.
	char *tempDirectoryName = malloc(PATH_MAX*sizeof(char*));
	strcpy(tempDirectoryName, directoryTop->directoryName);
	
	// Sets the new top.
    directoryTop = directoryTop->next;
	
    free(temp);
	return tempDirectoryName;
}

/**
 * Adds a threads wait status to the wait statuses stack.
 *
 * @param thread		The thread number.
 * @param threadStatus  The threads wait status.
 */
void addWaitStatus(int thread, int threadStatus) {
	
	// Creates a new wait status.
	struct waitStatus *newWaitStatus;
	newWaitStatus = (struct waitStatus *)malloc(sizeof(struct waitStatus));
	newWaitStatus->threadNumber = thread;
	newWaitStatus->status = threadStatus;
	
	// If the stack is empty.
	if (waitStatusTop == NULL) {
		newWaitStatus->next = NULL;
	}
	
	// If the stack is not empty.
	else {
		newWaitStatus->next = waitStatusTop;
	}
	
	waitStatusTop = newWaitStatus;
	return;
}

/**
 * Changes a threads wait status.
 *
 * @param thread		The thread number.
 * @param threadStatus  The threads new wait status.
 */
void changeWaitStatus(int thread, int threadStatus) {
	
	// Goes through the stack.
	struct waitStatus *temp = waitStatusTop;
	while (temp->next != NULL) {

		// Changes the wait status if the current thread is the one to be changed.
		if (temp->threadNumber == thread) {
			temp->status = threadStatus;
		}
		temp = temp->next;
	}
	
	// Changes the wait status if the last thread is the one to be changed.
	if (temp->threadNumber == thread) {
		temp->status = threadStatus;
	}
	
	return;
}

/**
 * Checks how many threads are waiting.
 *
 * @return waitCount	The amount of threads that are currently waiting.
 */
int checkWaitStatuses(void) {
	 
	int waitCount = 0;
	struct waitStatus *temp = waitStatusTop;
	// Goes through the stack.
	while (temp->next != NULL) {
		
		// Increments the wait count if the threads status is in waiting mode.
		if (temp->status == 1) {
			waitCount++;
		}
		temp = temp->next;
	}
	
	// Checks if the last status is in waiting mode.
	if (temp->status == 1) {
		waitCount++;
	}
	
	return waitCount;
}

/**
 * Checks if the directories stack is empty.
 *
 * @return 0 or 1	0 if the stack is empty, else 1.
 */
int directoriesIsEmpty(void) {
	
	if (directoryTop == NULL) {
		return 0;
	}
	
	else {
		return 1;
	}
}


/**
 * Frees the wait status stack. 
 */
void freeWaitStatuses(void) {
	
	struct waitStatus *temp;
	// Goes through the stack and frees each wait status.
	while (waitStatusTop != NULL) {	
		temp = waitStatusTop;
		waitStatusTop = waitStatusTop->next;		
		free(temp);
	}

	return;
}