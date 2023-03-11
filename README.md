# mdu
The mdu (my directory usage) is a program written in C that calculates the disk usage of a set of files and/or directories. If the file is a directory, the user can decide whether to traverse it recursively or in parallel by using the -j option followed by the number of threads that should be used. 

# Compiling the program
In order to compile the program in a linux environment go to the mdu folder and enter the 'make' command.

# Running the program
The following section presents different examples of how to run the program.

## Single file/directory recursively
  - ./mdu filename
 
## Single file/directory in parallel (3 threads)
  - ./mdu filename -j3

## Multiple files/directories recursively
  - ./mdu filename1 filename2

## Multiple files/directories in parallel (3 threads)
  - ./mdu filename1 filename2 -j3
