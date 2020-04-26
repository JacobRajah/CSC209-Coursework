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
Insert lab information here

## Assignments
Insert Assignment information here

## Authors
* **Jacob Rajah**: wrote all code in this repository aside from starter code given by the instructors

## Acknowledgments
All labs and assignments were created by the instructors for CSC209 at the University of Toronto. Most of the folders in this 
repository contain starter code written by the Instructors. However, all the scripts were completed and made function by myself.
