#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//for lstat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//for opendir and readdir
#include <sys/types.h>
#include <dirent.h>

#include "ftree.h"

struct dirent *find_next_file(DIR *directory){

  struct dirent *temp;
  temp = readdir(directory);

  while(temp != NULL){

    if (strcmp((temp->d_name),".") != 0 && strcmp((temp->d_name),"..") != 0 &&
     strcmp((temp->d_name),".DS_Store") != 0) {
      return temp;
    }
    temp = readdir(directory);
  }
  return temp;
}


struct TreeNode *build_tree(const char *fname, char *path){

  //get proper PATH.. ex: concatenate "" and mydir for the first dynamically
  //then it may be ex2: "mydir/fname"
  char full_path[80];
  strcpy(full_path,path);
  strcat(full_path,fname);

  //make the file name a char * type in order to assign to tree struct
  char *name = malloc(sizeof(char)*(strlen(fname)+1));
  strcpy(name,fname);

  struct stat curr_file;
  if(lstat(full_path,&curr_file) == -1){
    printf("Error finding file%s\n",full_path);
    return NULL;
  }

  if(S_ISREG(curr_file.st_mode)){
    //regular file
    struct TreeNode *reg_file;
    reg_file = malloc(sizeof(struct TreeNode));
    //set values retrieved from lstat
    reg_file->fname = name;
    reg_file->permissions = curr_file.st_mode & 0777;
    reg_file->type = '-';

    return reg_file;
  }
  else if(S_ISLNK(curr_file.st_mode)){
    //link file
    struct TreeNode *link_file;
    link_file = malloc(sizeof(struct TreeNode));
    //set values retrieved from lstat
    link_file->fname = name;
    link_file->permissions = curr_file.st_mode & 0777;
    link_file->type = 'l';

    return link_file;
  }
  else if(S_ISDIR(curr_file.st_mode)){

    //directory.. want to list all items in dir and recursively go through
    struct TreeNode *dir;
    dir = malloc(sizeof(struct TreeNode));
    //set values retrieved from lstat
    dir->fname = name;
    dir->permissions = curr_file.st_mode & 0777;
    dir->type = 'd';
    //prepare to access files in dir
    DIR *d_ptr = opendir(full_path);
    if(d_ptr == NULL){
      fprintf(stderr, "Error opening dir\n");
      return NULL;
    }
    //read and store information about first file in dir according to read dir
    struct dirent *current = NULL;
    current = find_next_file(d_ptr);

    struct dirent *next = NULL;
    //path modified so that the name of a file can be added to the end of the path
    strcat(full_path,"/");
    struct TreeNode *currentTree = NULL;

    if(current != NULL){
      currentTree = build_tree(current->d_name,full_path);
    }
    //while loop goes through each file in the directory and recursively connects
    //each file.
    while (current != NULL) {

      //connect first file to the directory Tree contents
      if(dir->contents == NULL){
        dir->contents = currentTree;
        //printf("Set content success\n");
      }
      //After directory tree contents has been set, start conecting the rest
      //of the files in the directory using the structs 'next' value.
      else{
        //printf("add node\n");
        next = find_next_file(d_ptr);
        //printf("%s\n",path);
        if(next != NULL){
          struct TreeNode *nextTree = build_tree(next->d_name,full_path);
          currentTree->next = nextTree;
          currentTree = nextTree;
        }
        current = next;
      }
    }
    closedir(d_ptr);
    return dir;
  }
  return NULL;
}


/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
 *
 */
struct TreeNode *generate_ftree(const char *fname) {

    // Your implementation here.
    struct stat curr_file;

    if(lstat(fname,&curr_file) == -1){
      fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
      return NULL;
    }

    // char name[80];
    // strcpy(name,fname);

    char path[200] = "";

    struct TreeNode *root = build_tree(fname,path);


    // Hint: consider implementing a recursive helper function that
    // takes fname and a path.  For the initial call on the
    // helper function, the path would be "", since fname is the root
    // of the FTree.  For files at other depths, the path would be the
    // file path from the root to that file.

    return root;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 *
 * The only print statements that you may use in this function are:
 * printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions)
 * printf("%s (%c%o)\n", root->fname, root->type, root->permissions)
 *
 */
void print_ftree(struct TreeNode *root) {

    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    //printf("%*s", depth * 2, "");
    // Your implementation here.
    if(root->type == '-' || root->type == 'l'){
      printf("%*s", depth * 2, "");
      printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
    }
    else{
      printf("%*s", depth * 2, "");
      printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
      depth++;

      struct TreeNode *curr = root->contents;
      while(curr != NULL){
        print_ftree(curr);
        curr = curr->next;
      }
      depth--;

    }

}


/*
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 *
 */
void deallocate_ftree (struct TreeNode *node) {

   // Your implementation here.
   //make sure to free fname
   //printf("I'm Here 1\n");
   if(node->type == '-' || node -> type == 'l'){
     free(node->fname);
     free(node);
   }
   else{

     struct TreeNode *curr = node->contents;
     //end represents the length of the contents linked list
     int end  = 0;

     while(curr != NULL){
       curr = curr->next;
       if(curr != NULL){
         end++;
       }
     }

     //set to first value in the list
     curr = node->contents;
     int i = 0;

     while(end > 0){

       while(i < end && curr != NULL){
         curr = curr->next;
         i++;
       }

       deallocate_ftree(curr);
       end--;
       //reset values
       i = 0;
       curr = node->contents;
     }
     free(node->fname);
     free(node);

  }

}
