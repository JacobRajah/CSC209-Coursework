#include <stdio.h>
#include <stdlib.h>



int count_neighbours(int *board, int index, int cols){
  int total = 0;
  total = (board[index - (cols + 1)] + board[index - cols] +
   board[index - (cols - 1)] + board[index - 1] + board[index + 1] +
   board[index + (cols - 1)] + board[index + cols] +
   board[index + (cols + 1)]);

   return total;
}

int *copyboard(int *board, int length){
  int *copy = malloc(sizeof(int)*length);
  int i;

  for(i = 0; i < length; i++){
    copy[i] = board[i];
  }

  return copy;
}

void print_state(int *board, int num_rows, int num_cols) {
    for (int i = 0; i < num_rows * num_cols; i++) {
        printf("%d", board[i]);
        if (((i + 1) % num_cols) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void update_state(int *board, int num_rows, int num_cols) {
    // TODO: Implement.
    //make copy of board
    int *board_copy = copyboard(board, (num_cols*num_rows));
    //This is the number of cells in a row which we must modify
    int numbers_per_row = (num_cols - 2);
    //This is the value to move to the next cell in the row
    int increment;

    for(increment = 1; increment <= numbers_per_row; increment++){
      //This value i is the starting cell, we edit cells down each
      //colomn, not by row.
      int i = (num_cols + increment);

      while(i <= (num_cols*(num_rows-2)+increment)){
        int count = count_neighbours(board_copy, i, num_cols);
        //check the copied original board
        if(board_copy[i] == 1 && (count < 2 || count > 3)){
          //update the actual board
          board[i] = 0;
        }
        else if(board_copy[i] == 0 && (count == 2 || count == 3)){
          board[i] = 1;
        }
        //change i to be the next cell in the column
        i = i + num_cols;
      }
      //increment++ so that next column is updated
    }
    free(board_copy);
    return;
}
