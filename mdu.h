/**
 * This is the header file for the mdu program.
 * 
 * @file mdu.h
 * @author Jakob Mukka
 * @date 2022-11-19
 */
  
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/statvfs.h>
#include <getopt.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

// Gets the files/directories that the user has specified.
char **getFiles(int argc, char **argv, char *optind, int *fileAmountPointer);

// Calculates the size a list of files takes on the disk recursively.
int calculateSizeOnDiskRecursive(char **files, int fileAmount);

// Does a recursive search of a directory.
blkcnt_t searchDirectoryRecursive(char *directory, blkcnt_t totalBlockAmount, int *exitValuePointer, char *pathPointer);

// Calculates the size a list of files takes on the disk in parallel.
int calculateSizeOnDiskParallel(char **files, int fileAmount, int threadAmount);

// Does a parallel search of a directory.
void *searchDirectoryParallel(void *info);
					
// Gets the files in the directory.
char **getFilesInDirectory(DIR *dirp, int *fileAmountPointer);

// Checks if a directory can be opened.
int checkDirectory(char *directory, char *pathPointer);







