#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
    This program has two arguments: the first is a greeting message, and the
    second is a name.

    The message is an impersonal greeting, such as "Hi" or "Good morning".
    name is set to refer to a string holding a friend's name, such as
    "Emmanuel" or "Xiao".

    First copy the first argument to the array greeting. (Make sure it is
    properly null-terminated.)

    Write code to personalize the greeting string by appending a space and
    then the string pointed to by name.
    So, in the first example, greeting should be set to "Hi Emmanuel", and
    in the second it should be "Good morning Xiao".

    If there is not enough space in greeting, the resulting greeting should be
    truncated, but still needs to hold a proper string with a null terminator.
    For example, "Good morning" and "Emmanuel" should result in greeting
    having the value "Good morning Emmanu" and "Top of the morning to you" and
    "Patrick" should result in greeting having the value "Top of the morning ".

    Do not make changes to the code we have provided other than to add your
    code where indicated.
*/

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: greeting message name\n");
        exit(1);
    }
    char greeting[20];
    char *name = argv[2];

    // Your code goes here
    int length = strlen(argv[1]);
    char *greet = argv[1];

    //must be 19 chars or less to fit in array because we need '\0' and ' ' (space)
    if(length > 19){
      length = 19;
    }
    else{
      greeting[length] = ' ';
    }
    //set last character to the null terminate
    greeting[19] = '\0';

    // int i;
    // for(i = 0; i < length; i++){
    //   greeting[i] = greet[i];
    // }

    strncpy(greeting,greet,length);


    // int j = length + 1;
    //
    // int name_pos = 0;
    // while(j < 19){
    //
    //   greeting[j] = name[name_pos];
    //   j++;
    //   name_pos++;
    // }
    int v = 19 - strlen(greeting);
    strncat(greeting,name,v);

    printf("%s\n", greeting);
    return 0;
}
