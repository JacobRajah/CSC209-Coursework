#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  /* The user will type in a user name on one line followed by a password
     on the next.
     DO NOT add any prompts.  The only output of this program will be one
	 of the messages defined above.
   */

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  // TODO
  int fd[2];
  if(pipe(fd) == -1){
    perror("pipe");
    exit(1);
  }

  int f;
  if((f = fork()) == -1){
    perror("fork");
    exit(1);
  }

  if(f == 0){

    //write the user id
    if(write(fd[1], &user_id, 10) == -1){
      perror("write user");
      exit(1);
    }
    //write the password
    //printf("%s\n", password);
    if(write(fd[1], &password, 10) == -1){
      perror("write password");
      exit(1);
    }
    //done writing, how cant close read cause its needed
    if(close(fd[1]) == -1){
      perror("close");
      exit(1);
    }

    //redirect stdin to come from pipe
    if(dup2(fd[0],fileno(stdin)) == -1){
      perror("dup2");
      exit(1);
    }

    execl("./validate", "validate", NULL);

  }
  //parent
  else{
    int status;
    if(wait(&status) == -1){
      perror("wait");
      exit(1);
    }
    if(WIFEXITED(status)){
      int response = WEXITSTATUS(status);
      if(response == 0){
        printf(SUCCESS);
      }
      else if(response == 2){
        printf(INVALID);
      }
      else{
        printf(NO_USER);
      }
    }
  }



  return 0;
}
