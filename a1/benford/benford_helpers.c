#include <stdio.h>

#include "benford_helpers.h"

int count_digits(int num) {
    // TODO: Implement.
    int curr_num = num;
    int digits = 0;

    if(num == 0){
      return 1;
    }
    else{
      while (curr_num != 0) {
        curr_num = curr_num/BASE;
        digits++;
      }
      return digits;
    }
}

int get_ith_from_right(int num, int i) {
    // TODO: Implement.
    int size = count_digits(num);

    int remainder = num % BASE;
    int nums[size];
    int j  = 0;

    while(num != 0){
      remainder = num % BASE;
      nums[size-1-j] = remainder;
      num = num/BASE;
      j++;
    }

    return nums[size-1-i];
}

int get_ith_from_left(int num, int i) {
    // TODO: Implement.
    int size = count_digits(num);

    int remainder = num % BASE;
    int nums[size];
    int j  = 0;

    while(num != 0){
      remainder = num % BASE;
      nums[size-1-j] = remainder;
      num = num/BASE;
      j++;
    }

    return nums[i];
}

void add_to_tally(int num, int i, int *tally) {
    // TODO: Implement.
    int value = get_ith_from_left(num, i);
    tally[value] = tally[value] + 1;
}
