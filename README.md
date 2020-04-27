# CSC209-Coursework
This repository contains all the labs and assignments completed in the course. The course was focused on 
systems programming in a Unix environment using the programming language C. The main topics of the course were: Pointers, Dynamic Memory, File Processing, Forking, Pipes and Sockets.

## Getting Started

### Prerequisites
In order to run any of the C scripts in this repository you need to have a C compiler available. A terminal on MacOS or 
an enabled Windows Subsystem for Linux will give access to a compiler.

### Runing a Single Script
In order to compile a desired C script into an executable you can run the following command:
```
gcc -Wall -std=gnu99 -g <filename>
```
This will create an executable named **a.out**
However if you want to create an executable with a specific name you can compile the script as follows:
```
gcc -Wall -std=gnu99 -g -o <executable name> <filename.c>
```
In order to run the executable, execute:
```
$ ./<exename>
```

### Compiling a Collection of Scripts
In some of the lab/assignment folders there is a **Makefile** which allows for a series of dependant files to 
be compiled at once, producing one executable. In order to compile and produce an executable, run the following command:
```
make <filename>
```
In order to run the executable, execute:
```
$ ./<exename>
```

## Labs
Below is a list of goals for each lab:
### Lab 1
* write a simple script in order to demonstrate how to compile C files
### Lab 2
* Learn how to implement pointers and the scanf function in multifunction C scripts
### Lab 3
* Become familiar with heap memory allocation using the malloc function
### Lab 4
* Work with string functions to compare, truncate, strip and format strings
### Lab 5
* Learn how to write and implement structs.
* Read from binary files and store data on the heap
### Lab 6
* Use scripts to record interactions with gdb, the C debugger
### Lab 7
* Work with **Fork** to create a multiple processes
### Lab 8
* Create a system which takes a username and password and validates the user based on whether the the username and password
matches one found in a text file
### Lab 9
* Explore read and write speeds by reading from a binary file, and outputting all the data read in the user specified time
period.
### Lab 10
* Work with setting up and analyzing client server connections
* Read broken up string messages from the client and output the complete string.
### Lab 11
* Learn how to deal with a server that has multiple clients using the select function in C

## Assignments
Insert Assignment information here

## Authors
* **Jacob Rajah**: wrote all code in this repository aside from starter code given by the instructors

## Acknowledgments
All labs and assignments were created by the instructors for CSC209 at the University of Toronto. Most of the folders in this 
repository contain starter code written by the Instructors. However, all the scripts were completed and made function by myself.
