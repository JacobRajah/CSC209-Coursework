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

void make_node(struct TreeNode *node, int permissions, char type, char *name){
  node->fname = name;
  node->permissions = permissions;
  node->type = type;
  node->contents = NULL;
  node->next = NULL;
}
/*
 * Returns a struct dirent containing information about the next elegible file
 *
 *
 */
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

/*
 * Returns the FTree root
 *
 * This function takes a root TreeNode and recursively assigns the contents and
 * next value. The order in which the Tree is built mimics the output of readdir
 */
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
    make_node(reg_file, (curr_file.st_mode & 0777), '-', name);
    return reg_file;
  }
  else if(S_ISLNK(curr_file.st_mode)){
    //link file
    struct TreeNode *link_file;
    link_file = malloc(sizeof(struct TreeNode));
    //set values retrieved from lstat
    make_node(link_file, (curr_file.st_mode & 0777), 'l', name);
    return link_file;
  }
  else if(S_ISDIR(curr_file.st_mode)){

    //directory.. want to list all items in dir and recursively go through
    struct TreeNode *dir;
    dir = malloc(sizeof(struct TreeNode));
    //set values retrieved from lstat
    make_node(dir, (curr_file.st_mode & 0777), 'd', name);
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
    struct TreeNode *currentTree = NULL;
    //path modified so that the name of a file can be added to the end of the path
    strcat(full_path,"/");

    if(current != NULL){
      currentTree = build_tree(current->d_name,full_path);
      dir->contents = currentTree;
    }
    //while loop goes through each file in the directory and recursively connects
    //each file.
    while (current != NULL) {

      next = find_next_file(d_ptr);
      if(next != NULL){
        struct TreeNode *nextTree = build_tree(next->d_name,full_path);
        currentTree->next = nextTree;
        currentTree = nextTree;
      }
      current = next;

    }
    if(closedir(d_ptr) != 0){
      printf("Unable to close the directory");
      return NULL;
    }
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

    struct stat curr_file;

    if(lstat(fname,&curr_file) == -1){
      fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
      return NULL;
    }

    char path[200] = "";

    struct TreeNode *root = build_tree(fname,path);

    return root;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 */
void print_ftree(struct TreeNode *root) {

    //Keep track of the depth of the tree
    static int depth = 0;
    //If the root is a file or linked then print the file information at depth specified
    if(root->type == '-' || root->type == 'l'){
      printf("%*s", depth * 2, "");
      printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
    }
    //If the root is a directory, print the directory name and information
    //Then recusively go through data in the directory and print the data at a depth
    //increased by 1
    else{
      printf("%*s", depth * 2, "");
      printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
      depth++;

      struct TreeNode *temporary = NULL;
      temporary = root->contents;
      while(temporary != NULL){
        print_ftree(temporary);
        temporary = temporary->next;
      }
      //after printing values in the directory, reduce the depth so that indentation
      //returns to what it was prior to evalutating directory.
      depth--;
    }
}

/*
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 *
 */
void deallocate_ftree (struct TreeNode *node) {
    //If the node is a linked or regular file, simply free the TreeNode name
    //and then the Node
   if(node->type == '-' || node -> type == 'l'){
     free(node->fname);
     free(node);
   }
   else{
     //if directory then find the last node in the directory and free the Node
     //then traverse backwards freeing each node.
     struct TreeNode *curr = node->contents;
     //end represents the length of the contents linked list - 1
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
       //find last node that hasnt been freed
       while(i < end && curr != NULL){
         curr = curr->next;
         i++;
       }
       //free the memory
       deallocate_ftree(curr);
       //set the last node that hasnt been freed to end = end - 1
       end--;
       //reset values
       i = 0;
       curr = node->contents;
     }
     deallocate_ftree(curr);
     free(node->fname);
     free(node);
  }
}
