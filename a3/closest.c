#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "point.h"
#include "utilities_closest.h"
#include "serial_closest.h"
#include "parallel_closest.h"


void print_usage() {
    fprintf(stderr, "Usage: closest -f filename -d pdepth\n\n");
    fprintf(stderr, "    -d Maximum process tree depth\n");
    fprintf(stderr, "    -f File that contains the input points\n");

    exit(1);
}

int main(int argc, char **argv) {
    int n = -1;
    long pdepth = -1;
    char *filename = NULL;
    int pcount = 0;

    //integer that ensures only a file and a depth is passed as a parameter
    int d_options = 0;
    int f_options = 0;
    int ch;
    //while loop which uses get opt to scan argv for the parameters passed in after -d and -f
    while((ch = getopt(argc, argv, "d:f:")) != -1){
        switch(ch){

            case 'd':
                //store the depth inputted by user
                pdepth = strtol(argv[optind - 1], NULL, 10);
                d_options++;
                break;

            case 'f':
                //store the name of the file
                filename = argv[optind-1];
                f_options++;
                break;

            default:
                break;
        }
    }

    if((d_options + f_options) != 2){
        //only print the usage error if the file name and depth isn't given
        print_usage();
    }

    // Read the points
    n = total_points(filename);
    struct Point points_arr[n];
    read_points(filename, points_arr);

    // Sort the points
    qsort(points_arr, n, sizeof(struct Point), compare_x);

    // Calculate the result using the parallel algorithm.
    double result_p = closest_parallel(points_arr, n, pdepth, &pcount);
    printf("The smallest distance: is %.2f (total worker processes: %d)\n", result_p, pcount);

    exit(0);
}

