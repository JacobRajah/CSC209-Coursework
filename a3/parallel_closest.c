#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "utilities_closest.h"


/*Allocate memory on the heap and error check to ensure malloc was successful*/
void *Malloc(int size) {
    void *ptr;
    if ((ptr = malloc(size)) == NULL) {
        perror("malloc");
        exit(1);
    }
    return ptr;
}
/*Fork a process using fork() with embedded error checking*/
int Fork() {
    int res;
    if ((res = fork()) == -1) {
        perror("fork");
        exit(1);
    }
    return res;
}
/*update the status parameter using wait, while error checking the return of 'wait'*/
int Wait(int *status) {
    int res;
    if((res = wait(status)) == -1) {
        perror("wait");
        exit(1);
    }
    return res;
}
/*Pipe the fd parameter array and check for piping errors*/
void Pipe(int *fd) {
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }
}
/*Close the pipe passed as a parameter, and check to ensure the file was closed successfully*/
void Close(int file){
    if(close(file) == -1){
        perror("close");
        exit(1);
    }
}
/*Reads data from the specified pipe and exits if the incorrect size was read from the pipe */
void Read(int file, double *distance_read){
    if(read(file, distance_read, sizeof(*distance_read)) != sizeof(*distance_read)){
        perror("read child");
        exit(1);
    }
}
/*Writes data to the specified pipe and exits if the incorrect size was written to the pipe */
void Write(int file, double *distance_write){
    if(write(file, distance_write, sizeof(*distance_write)) != sizeof(*distance_write)){
        perror("write child");
        exit(1);
    }
}

/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {
    //if the size of the array is less than 4 or the depth is 0, then we can use the single process
    //approach to finding the closest distance
    if(n < 4 || pdmax == 0){
        return closest_serial(p, n);
    }
    //find the mid point in the size and store the mid point Point struct
    int mid = n / 2;
    struct Point mid_point = p[mid];
    //create a multidimensional array used to create two pipes for each individual child
    int fd[2][2];
    //Pipe the first row of the array
    Pipe(fd[0]);
    //decrement the depth by one prior to forking
    pdmax--;
    //fork a child process to compute the shortest distance on the left of the midpoint
    int f1 = Fork();
    /*only the child process 1 can run this section. The child process will recursively call itself, collecting
     *the shortest distance on the left side of the mid point, then writing the value to the pipe for the parent
     *to read*/
    if(f1 == 0){

        double left_closest = closest_parallel(p, mid, pdmax, pcount);
        Close(fd[0][0]);
        Write(fd[0][1], &left_closest);
        Close(fd[0][1]);
        //exit with the number of processes rooted at the current process
        exit(1 + *pcount);
    }

    //Set up the second pipe and fork a new process
    Pipe(fd[1]);
    int f2 = Fork();

    /*This second child process will find the shortest distance between points on the right side of the
     * midpoint and write the value to the second pipe for the parent to read*/
    if(f2 == 0){

        double right_closest =  closest_parallel(p + mid, n - mid, pdmax, pcount);
        Close(fd[1][0]);
        Write(fd[1][1], &right_closest);
        Close(fd[1][1]);
        //exit with the number of processes rooted at the current process
        exit(1 + *pcount);
    }

    //Only parent processs will run this section
    if(f1 > 0 || f2 > 0){

        //Close the write end of the pipes, as they are unneeded
        Close(fd[0][1]);
        Close(fd[1][1]);
        //Declaration of variables to store the smallest distance from the left and right of midpoint
        double left_min, right_min;
        int status;

        //For loop which collects the status of each of the child processes using wait, and waits
        //until the child process has exited before it reads the minimum distances from the pipe
        for(int i = 0; i < 2; i++){

            Wait(&status);

            if(WIFEXITED(status)){
                //increment the process count with the exit status' from the child processes
                *pcount += WEXITSTATUS(status);
                if(i == 0){
                    Read(fd[i][0],&left_min);
                }
                else if (i == 1){
                    Read(fd[i][0],&right_min);
                }

            }
        }
        //Close the read ends of the pipe now that the data has been read
        Close(fd[0][0]);
        Close(fd[1][0]);

        // Find the smaller of two distances
        double d = min(left_min, right_min);

        // Build an array strip[] that contains points close (closer than d) to the line passing through the middle point.
        struct Point *strip = Malloc(sizeof(struct Point) * n);

        //iterate through the original list of points and gather all the points that are within a distance d from
        //the mid point.
        int j = 0;
        for (int i = 0; i < n; i++) {
            if (abs(p[i].x - mid_point.x) < d) {
                strip[j] = p[i], j++;
            }
        }

        // Find the closest points in strip.  Return the minimum of d and closest distance in strip[].
        double new_min = min(d, strip_closest(strip, j, d));
        free(strip);
        //return the minimum distance
        return new_min;
    }
    return 0.0;
}