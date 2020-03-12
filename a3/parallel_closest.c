#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "utilities_closest.h"

void *Malloc(int size) {
    void *ptr;
    if ((ptr = malloc(size)) == NULL) {
        perror("malloc");exit(1);
    }
    return ptr;
}

int Fork() {
    int res;
    if ((res = fork()) == -1) {
        perror("fork");exit(1);
    }
    return res;
}

int Wait(int *status) {
    int res;
    if((res = wait(status)) == -1) {
        perror("wait");
        exit(1);
    }
    return res;
}

void Pipe(int *fd) {
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }
}

/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {

    if(n < 4 || pdmax == 0){
        return closest_serial(p, n);
    }

    int mid = n / 2;
    struct Point mid_point = p[mid];

    int fd[2][2];
    Pipe(fd[0]);

    pdmax--;

    int f1 = Fork();

    if(f1 == 0){

        double left_closest = closest_parallel(p, mid, pdmax, pcount);
        if(close(fd[0][0]) == -1){
            perror("close read");
            exit(1);
        }
        if(write(fd[0][1],&left_closest, sizeof(left_closest)) == -1){
            perror("write");
            exit(1);
        }
        if(close(fd[0][1]) == -1){
            perror("close write");
            exit(1);
        }
        exit(1 + *pcount);
    }

    Pipe(fd[1]);

    int f2 = Fork();


    if(f2 == 0){

        double right_closest =  closest_parallel(p + mid, n - mid, pdmax, pcount);
        if(close(fd[1][0]) == -1){
            perror("close read");
            exit(1);
        }
        if(write(fd[1][1],&right_closest, sizeof(right_closest)) == -1){
            perror("write");
            exit(1);
        }
        if(close(fd[1][1]) == -1){
            perror("close write");
            exit(1);
        }
        exit(1 + *pcount);
    }

    if(f1 > 0 || f2 > 0){
        if(close(fd[0][1]) == -1 || close(fd[1][1]) == -1){
            perror("close write");
            exit(1);
        }
        double left_min, right_min;

        int status;
        for(int i = 0; i < 2; i++){

            Wait(&status);

            if(WIFEXITED(status)){
                *pcount += WEXITSTATUS(status);
                if(i == 0){
                    if(read(fd[0][0],&left_min, sizeof(left_min)) != sizeof(left_min)){
                        perror("read child 1");
                        exit(1);
                    }
                }
                else{
                    if(read(fd[1][0],&right_min, sizeof(left_min)) != sizeof(left_min)){
                        perror("read child 2");
                        exit(1);
                    }
                }
            }
        }

        // Find the smaller of two distances
        double d = min(left_min, right_min);

        // Build an array strip[] that contains points close (closer than d) to the line passing through the middle point.
        struct Point *strip = Malloc(sizeof(struct Point) * n);

        int j = 0;
        for (int i = 0; i < n; i++) {
            if (abs(p[i].x - mid_point.x) < d) {
                strip[j] = p[i], j++;
            }
        }

        // Find the closest points in strip.  Return the minimum of d and closest distance in strip[].
        double new_min = min(d, strip_closest(strip, j, d));
        free(strip);

        return new_min;
    }
    return 0.0;
}