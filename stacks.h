/**
 * This is the header file for the two stacks (directories and wait statuses),
 * that the program uses.
 *
 * @file stacks.h
 * @author Jakob Mukka
 * @date 2022-11-19
 */
  
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

// Adds a directory to the stack.
void addDirectory(char *value);

// Gets a directory from the stack.
char *getDirectory(void);

// Adds a wait status to a thread.
void addWaitStatus(int thread, int stat);

// Changes a threads wait status.
void changeWaitStatus(int thread, int threadStatus);

// Checks all the threads wait statuses.
int checkWaitStatuses(void);

// Checks if the directories stack is empty
int directoriesIsEmpty(void);

// Frees the wait statuses.
void freeWaitStatuses(void);