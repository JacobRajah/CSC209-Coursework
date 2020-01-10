#!/usr/bin/env bash

//Compile hello.c and store executable in file hello
gcc -Wall -std=gnu99 -g -o hello hello.c

//Compile count.c and store executable in count.c
gcc -Wall -std=gnu99 -g -o count count.c

//Compile echo_arg.c and store executable in echo_arg
gcc -Wall -std=gnu99 -g -o echo_arg echo_arg.c

//Compile echo_stdin.c and store executable in echo_stdin
gcc -Wall -std=gnu99 -g -o echo_stdin echo_stdin.c
