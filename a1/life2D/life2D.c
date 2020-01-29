#include <stdio.h>
#include <stdlib.h>

#include "life2D_helpers.h"


int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Usage: life2D rows cols states\n");
        return 1;
    }

    int rows = strtol(argv[1],NULL,10);
    int cols = strtol(argv[2],NULL,10);
    int num_states = strtol(argv[3],NULL,10);

    int board[rows*cols];
    int k;
    for (k = 0; k < (rows*cols); k++) {
      board[k] = 0;
    }

    //Setting up the array with the values from the user
    int value;
    int i = 0;
    while(fscanf(stdin,"%d",&value) != EOF){
      board[i] = value;
      i++;
    }

    //print original board
    print_state(board,rows,cols);

    int j;
    for(j = 0; j < (num_states - 1); j++){
      update_state(board, rows, cols);
      print_state(board,rows,cols);
    }

    return 0;
    // TODO: Implement.
}
