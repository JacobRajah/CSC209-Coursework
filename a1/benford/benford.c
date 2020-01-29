#include <stdio.h>
#include <stdlib.h>

#include "benford_helpers.h"

/*
 * The only print statement that you may use in your main function is the following:
 * - printf("%ds: %d\n")
 *
 */
int main(int argc, char **argv) {
    //10 length array because space at end of array
    int total[10];

    int j;
    for(j = 0; j < 10; j++){
      total[j] = 0;
    }


    int position = strtol(argv[1],NULL,10);

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "benford position [datafile]\n");
        return 1;
    }

    else if(argc == 3){
      //Here a file is givem
      FILE *int_file;
      int_file = fopen(argv[2],"r");
      int value;

      while(fscanf(int_file,"%d\n",&value) == 1){
        add_to_tally(value, position, total);
      }

      fclose(int_file);
    }

    else{
      //Read a string of ints from user
      int value;
      while(fscanf(stdin,"%d ",&value) != EOF){
        add_to_tally(value, position, total);
      }
    }

    int i;
    for(i = 0; i < 10; i++){
      printf("%ds: %d\n",i,total[i]);
    }

    return 0;

    // TODO: Implement.
    //IF arguments is 3 then theres a file to read from and grab integers from
    //else read a string from stdin and use the string to find all the ints
}
