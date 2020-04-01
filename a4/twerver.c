#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "socket.h"

#ifndef PORT
    #define PORT 58232
#endif

#define LISTEN_SIZE 5
#define WELCOME_MSG "Welcome to CSC209 Twitter! Enter your username: "
#define ENTERED_MSG " has just joined\r\n"
#define SEND_MSG "send"
#define SHOW_MSG "show"
#define FOLLOW_MSG "follow"
#define UNFOLLOW_MSG "unfollow"
#define BUF_SIZE 256
#define MSG_LIMIT 8
#define FOLLOW_LIMIT 5

struct client {
    int fd;
    struct in_addr ipaddr;
    char username[BUF_SIZE];
    char message[MSG_LIMIT][BUF_SIZE];
    struct client *following[FOLLOW_LIMIT]; // Clients this user is following [array of pointers]
    struct client *followers[FOLLOW_LIMIT]; // Clients who follow this user
    char inbuf[BUF_SIZE]; // Used to hold input from the client
    char *in_ptr; // A pointer into inbuf to help with partial reads
    struct client *next;
};


// Provided functions. 
void add_client(struct client **clients, int fd, struct in_addr addr);
void remove_client(struct client **clients, int fd);
//declaration
int read_a_str(struct client *user, struct client **active_clients_ptr);
struct client *find_user(struct client **active_clients_ptr, char *usr_name);
void remove_follower(struct client *user, struct client *c);
void remove_following(struct client *user, struct client *c);
void Write(struct client *user, char *message, struct client **active_clients_ptr);

// The set of socket descriptors for select to monitor.
// This is a global variable because we need to remove socket descriptors
// from allset when a write to a socket fails. 
fd_set allset;

/*Allocate memory on the heap and error check to ensure malloc was successful*/
void *Malloc(int size) {
    void *ptr;
    if ((ptr = malloc(size)) == NULL) {
        perror("malloc");
        exit(1);
    }
    return ptr;
}

/*
 * Write all the messages sent by the client usr_following prior to the command
 * to the user who issued the command.
 */
void display_messages(struct client *user, struct client *usr_following, struct client **active_clients_ptr){
    for(int i = 0; i < MSG_LIMIT; i++){
        if((usr_following->message)[i][0] != '\0'){
            //11 because " wrote: " and \r\n '\0'
            char recall[(BUF_SIZE * 2) + 11] = {'\0'};
            sprintf(recall, "%s wrote: %s\r\n", usr_following->username, (usr_following->message)[i]);
            Write(user, recall, active_clients_ptr);
        }
    }
}

/*
 * iterate through all the clients the user is following and
 * call display_messages for each following
 */
void show(struct client **active_clients_ptr, struct client *user){
    printf("[%s] Showing all following messages...\n", user->username);
    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((user->following)[i] != NULL){
            display_messages(user, (user->following)[i], active_clients_ptr);
        }
    }
}

/*
 * Send a specified message to all the followers of a user
 */
void send_followers(struct client **active_clients_ptr, struct client *user, char *message){
    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((user->followers)[i] != NULL){
            char message_follower[(BUF_SIZE * 2) + 3] = {'\0'};
            sprintf(message_follower, "%s: %s\r\n", user->username, message);
            Write((user->followers)[i], message_follower, active_clients_ptr);
        }
    }
}

/*
 * Returns the index of the spot open in messages for the user, else tells
 * user that they have reached max message limit and returns -1.
 */
int message_space(struct client **active_clients_ptr, struct client *user){
    for(int i = 0; i < MSG_LIMIT; i++){
        //a message is empty if its first value is a null terminator
        if((user->message)[i][0] == '\0'){
            return i;
        }
    }
    //only reaches here if message array is full
    printf("%s has the sent the maximum amount of messages\n", user->username);
    char no_space[BUF_SIZE + 50] = {'\0'};
    sprintf(no_space, "%s, you have sent the maximum amount of messages\r\n", user->username);
    Write(user, no_space, active_clients_ptr);
    return -1;
}

/*
 * Helper Function which writes a message with embedded error checking
 */
void Write(struct client *user, char *message, struct client **clients_ptr){

    if(write(user->fd, message, strlen(message)) == -1){
        fprintf(stderr, "Write to client %s failed\n", inet_ntoa(user->ipaddr));
        remove_client(clients_ptr, user->fd);
    }
}

/*
 * returns 1 if the user is following client c*/
int is_following(struct client *user, struct client *c){
    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if (((user->following)[i] != NULL) && ((user->following)[i]->fd == c->fd)){
            return 1;
        }
    }
    return 0;
}

/*
 * Return 1 if the user has space in their following and the client c has space in their followers
 * Otherwise return 0 and notify user
 */
int check_follow_space(struct client **active_clients_ptr, struct client *user, struct client *c){
    int space_following = 0;
    int space_followers = 0;
    //1. Check if user has space in following
    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((user->following)[i] == NULL){
            space_following++;
            break;
        }
    }
    //If the user doesn't have space in their following list, notify user
    if(space_following == 0){
        char following_err[(BUF_SIZE * 2) + 50] = {'\0'};
        sprintf(following_err, "%s, you reached max following, cannot follow %s\r\n", user->username, c->username);
        Write(user, following_err, active_clients_ptr);
        return 0;
    }

    //2. Check if c has space in followers
    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((c->followers)[i] == NULL){
            space_followers++;
            break;
        }
    }
    //if the person to be followed has no space in their followers, notify user of the issue
    if(space_followers == 0){
        char follow_err[(BUF_SIZE * 2) + 60] = {'\0'};
        sprintf(follow_err, "%s, %s has the max amount of followers, cannot follow\r\n", user->username, c->username);
        Write(user, follow_err, active_clients_ptr);
        return 0;
    }

    return (space_followers && space_following);
}

/*
 * add new_follower to the users list of followers
 * assuming that the list has already been checked for space
 */
void add_follower(struct client *user, struct client *new_follower){
    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((user->followers)[i] == NULL){
            (user->followers)[i] = new_follower;
            printf("%s has %s as a follower\n", user->username, new_follower->username);
            break;
        }
    }
}

/*
 * add client to_be_followed to the users list of following
 * assuming that the list has already been checked for space
 */
void add_following(struct client *user, struct client *to_be_followed){
    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((user->following)[i] == NULL){
            (user->following)[i] = to_be_followed;
            printf("%s is following %s\n", user->username, to_be_followed->username);
            break;
        }
    }
}

/*
 * Verifies username exists and then user unfollows specified
 * user name only if user follows username
 */
void unfollow(struct client **active_clients_ptr, struct client *user, char *argument){
    //check if user exists
    printf("Looking for user...\n");
    struct client *to_be_unfollowed = find_user(active_clients_ptr, argument);

    if(to_be_unfollowed == NULL){
        char *warning = "Sorry this username does not exist\r\n";
        Write(user, warning, active_clients_ptr);
        printf("%s was unable to follow %s\n", user->username, argument);
    }
    else{
        //check that is following is true
        if(is_following(user, to_be_unfollowed)){
            remove_follower(to_be_unfollowed, user);
            printf("%s has lost %s as a follower\n", to_be_unfollowed->username, user->username);
            remove_following(user, to_be_unfollowed);
            printf("%s is no longer following %s\n", user->username, to_be_unfollowed->username);
        }
        else{
            //notify the user that they don't follow the specified target
            char follow[(BUF_SIZE * 2) + 25] = {'\0'};
            sprintf(follow, "%s, you don't follow %s\r\n", user->username, to_be_unfollowed->username);
            Write(user, follow, active_clients_ptr);
            printf("%s is not following %s\n", user->username, to_be_unfollowed->username);
        }
    }
}


/*
 * User is added as a follower to the client they specified, and the client specified is
 * added as someone User follows. If client doesnt exist then notify user*/
void follow(struct client **active_clients_ptr, struct client *user, char *argument){
    //check if user exists
    printf("Looking for user...\n");
    struct client *to_be_followed = find_user(active_clients_ptr, argument);

    if(to_be_followed == NULL){
        char *warning = "Sorry this username does not exist\r\n";
        Write(user, warning, active_clients_ptr);
        printf("%s could not follow %s\n", user->username, argument);
    }
    else{
        //check that the person to be followed isnt already being followed
        int ret = is_following(user, to_be_followed);

        if(ret == 1){
            char follow_already[(BUF_SIZE * 2) + 25] = {'\0'};
            sprintf(follow_already, "%s, you already follow %s\r\n", user->username, to_be_followed->username);
            Write(user, follow_already, active_clients_ptr);
            printf("%s already follows %s\n", user->username, to_be_followed->username);
        }
        //check both users to make sure they have space, and make sure is_following returns 0
        if(check_follow_space(active_clients_ptr, user, to_be_followed) && (ret == 0)){
            add_following(user, to_be_followed);
            add_follower(to_be_followed, user);
        }
    }
}

/*
 * Take command and apply relevant function with given argument
 */
void execute_command(struct client **active_clients_ptr, struct client *user, char *command, char *argument){

    if(strcmp(command, FOLLOW_MSG) == 0){
        //If the user enters follow then the follow() function will be executed
        follow(active_clients_ptr, user, argument);
    }
    else if(strcmp(command, UNFOLLOW_MSG) == 0){
        //If the user enters unfollow then the unfollow() function will be executed
        unfollow(active_clients_ptr, user, argument);
    }
    else if(strcmp(command, SEND_MSG) == 0){
        //If the user enters send then the following will be executed
        //1. check if space in message list
        int index = message_space(active_clients_ptr, user);
        //2. store in message list
        if(index != -1){
            strncpy((user->message)[index], argument, strlen(argument));
            printf("[%s] Successfully saved the message to account\n", user->username);
            //3. send message to all followers
            send_followers(active_clients_ptr, user, argument);
        }
    }
    else{
        printf("Invalid Command\n");
        char *warning = "Invalid Command\r\n";
        Write(user, warning, active_clients_ptr);
    }
}

/*
 * Takes a client ready to be read from and assess' what action
 * they want to perform*/
void active_user_input(struct client **active_clients_ptr, struct client *user){
    //read input from user
    if(read_a_str(user, active_clients_ptr) == 0){
        return;
    }
    printf("%s: %s\n", user->username, user->inbuf);
    //search for a space in the inputted string
    char *space = strchr(user->inbuf, ' ');
    //perform either the show or quit action based on whats in user->in_buf
    if(space == NULL){
        if(strcmp(user->inbuf, SHOW_MSG) == 0){
            show(active_clients_ptr, user);
        }
        else if(strcmp(user->inbuf, "quit") == 0){
            printf("%s has quit...\n", user->username);
            remove_client(active_clients_ptr, user->fd);
        }
        else{
            printf("Invalid Command\n");
            char *warning = "Invalid Command\r\n";
            Write(user, warning, active_clients_ptr);
        }
        return;
    }
    //Allocate space for the command so that it can be passed to the execute_command helper
    int cmd_size = strlen(user->inbuf) - strlen(space);
    char *command = Malloc(sizeof(char)*cmd_size + 1);
    strncpy(command, user->inbuf, cmd_size);
    command[cmd_size] = '\0';
    //Allocate space for the argument given after the command
    char *argument = Malloc(sizeof(char)*(strlen(space)));
    strncpy(argument, (space + sizeof(char)), strlen(space) - sizeof(char));
    argument[strlen(space) - 1] = '\0';

    execute_command(active_clients_ptr, user, command, argument);
    //free space allocated
    free(argument);
    free(command);
}

/*
 * Takes a specified user name and a pointer to a list of active clients
 * and searches through the active clients for the username. if not found
 * return null, else return pointer to that client*/
struct client *find_user(struct client **active_clients_ptr, char *usr_name){

    struct client *curr = *(active_clients_ptr);
    while(curr != NULL){
        if(strcmp(curr->username, usr_name) == 0){
            printf("Found the user %s\n", curr->username);
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/*
 * Send the message in s to all clients in active_clients.
 */
void announce(struct client *active_clients, char *s){
    //announce user has joined to all active clients
    struct client *curr = active_clients;
    while(curr != NULL){
        Write(curr, s, &active_clients);
        curr = curr->next;
    }
}

/*
 * Move client c from new_clients list to active_clients list.
 */
void activate_client(struct client *c,
                     struct client **active_clients_ptr, struct client **new_clients_ptr){
    //Remove client from new clients
    struct client **p;
    for (p = new_clients_ptr; *p && (*p)->fd != c->fd; p = &(*p)->next)
        ;

    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        //set value of pointer to next client, removing client c
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd, but couldn't from new_client\n");
    }

    //add client c to head of active clients
    c->next = *(active_clients_ptr);
    *(active_clients_ptr) = c;
}

/*
 * Read input from client until network newline is
 * found. Updates in buf with the full string */
int read_a_str(struct client *user, struct client **clients_ptr){
    int num_chars;
    if((num_chars = read(user->fd, user->inbuf, BUF_SIZE)) <= 0){
        //if error occurs, disconnect client
        fprintf(stderr, "Read from client %s failed\n", inet_ntoa(user->ipaddr));
        remove_client(clients_ptr, user->fd);
        return 0;
    }
    (user->inbuf)[num_chars] = '\0';
    //store pointer to next char in buf
    (user->in_ptr) = &((user->inbuf)[num_chars]);
    printf("[%d] Read %d bytes\n", user->fd, num_chars);

    // it may take more than one read to get all of the data that was written
    while(strstr((user->inbuf), "\r\n") == NULL) {
        int curr_chars;
        if((curr_chars = read(user->fd, (user->in_ptr), BUF_SIZE-num_chars)) <= 0){
            //if error occurs, disconnect client
            fprintf(stderr, "Read from client %s failed\n", inet_ntoa(user->ipaddr));
            remove_client(clients_ptr, user->fd);
            return 0;
        }
        printf("[%d] Read %d bytes\n", user->fd, curr_chars);
        num_chars += curr_chars;
        (user->inbuf)[num_chars] = '\0';
        //update pointer
        (user->in_ptr) = &((user->inbuf)[num_chars]);
    }

    (user->inbuf)[num_chars -2] = '\0';
    printf("[%d] Found Newline: %s\n", user->fd, user->inbuf);
    return 1;
}

/*
 * Reads a username from the client and returns
 * 1 if the client has entered a proper username
 * else returns 0
 */
int read_new_client(struct client *new_user, struct client **active_clients_ptr, struct client **new_clients_ptr){

    if(read_a_str(new_user, new_clients_ptr) == 0){
        return 0;
    }

    if(strcmp(new_user->inbuf, "") == 0){
        char *empty_message = "Please enter a non empty username: ";
        Write(new_user, empty_message, new_clients_ptr);
        return 0;
    }

    struct client *curr = *(active_clients_ptr);
    while(curr != NULL){
        //if entered username matches any existent usernames then prompt user to enter another username
        if(strcmp((curr->username), new_user->inbuf) == 0){
            char *existent_usr = "Username taken. Please enter a new username: ";
            Write(new_user, existent_usr, new_clients_ptr);
            return 0;
        }

        curr = curr->next;
    }

    //if code reaches here then the username isn't taken. Return 1 to approve username
    strncpy(new_user->username, new_user->inbuf, strlen(new_user->inbuf));
    (new_user->username)[strlen(new_user->inbuf)] = '\0';
    return 1;
}

/*
 * User has client as one of their followers so remove client
 * as a follower
 */
void remove_follower(struct client *user, struct client *c){

    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((user->followers)[i] != NULL){
            if(((user->followers)[i])->fd == c->fd){
                //Set the index where the client used to be to null
                (user->followers)[i] = NULL;
            }
        }
    }
}

/*
 * User should remove client c from their list of who they follow
 */
void remove_following(struct client *user, struct client *c){

    for(int i = 0; i < FOLLOW_LIMIT; i++){
        if((user->following)[i] != NULL){
            if(((user->following)[i])->fd == c->fd){
                //Set the index where the client used to be to null
                (user->following)[i] = NULL;
            }
        }
    }

}

void rm_followers_following(struct client *c){

    for(int i = 0; i < FOLLOW_LIMIT; i++){

        //iterate through followers and make each follower of client c unfollow client c
        if((c->followers)[i] != NULL){
            //Make the user no long follow client c
            remove_following((c->followers)[i], c);
            printf("%s is no longer following %s because they disconnected\n", (c->followers)[i]->username, c->username);
        }
        //iterate through the list of users the client c is following and remove client c
        //as a follower for the user (c->following)[i]
        if((c->following)[i] != NULL){
            //Remove client c as one of (c->following)[i] followers
            remove_follower((c->following)[i], c);
            printf("%s no longer has %s as a follower because they disconnected\n", (c->following)[i]->username, c->username);
        }

    }
}

/* 
 * Create a new client, initialize it, and add it to the head of the linked
 * list.
 */
void add_client(struct client **clients, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));
    p->fd = fd;
    p->ipaddr = addr;
    p->username[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *clients;

    // initialize messages to empty strings
    for (int i = 0; i < MSG_LIMIT; i++) {
        p->message[i][0] = '\0';
    }

    //set followers and following to NULL
    for (int j = 0; j < FOLLOW_LIMIT; j++) {
        (p->followers)[j] = NULL;
        (p->following)[j] = NULL;
    }

    *clients = p;
}

/* 
 * Remove client from the linked list and close its socket.
 * Also, remove socket descriptor from allset.
 */
void remove_client(struct client **clients, int fd) {
    struct client **p;

    for (p = clients; *p && (*p)->fd != fd; p = &(*p)->next)
        ;

    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        //Remove the client from other clients' following/followers
        rm_followers_following(*p);

        // Remove the client
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Unable to successfully remove client %d\n", fd);
    }
}


int main (int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    // If the server writes to a socket that has been closed, the SIGPIPE
    // signal is sent and the process is terminated. To prevent the server
    // from terminating, ignore the SIGPIPE signal. 
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // A list of active clients (who have already entered their names). 
    struct client *active_clients = NULL;

    // A list of clients who have not yet entered their names. This list is
    // kept separate from the list of active clients, because until a client
    // has entered their name, they should not issue commands or 
    // or receive announcements. 
    struct client *new_clients = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, LISTEN_SIZE);
    free(server);

    // Initialize allset and add listenfd to the set of file descriptors
    // passed into select 
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            exit(1);
        } else if (nready == 0) {
            continue;
        }

        // check if a new client is connecting
        if (FD_ISSET(listenfd, &rset)) {
            printf("A new client is connecting\n");
            //returns client fd and updates sock_addr_in
            clientfd = accept_connection(listenfd, &q);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_client(&new_clients, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            //ASKS TO ENTER USERNAME
            if (write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, 
                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_client(&new_clients, clientfd);
            }
        }

        // Check which other socket descriptors have something ready to read.
        // The reason we iterate over the rset descriptors at the top level and
        // search through the two lists of clients each time is that it is
        // possible that a client will be removed in the middle of one of the
        // operations. This is also why we call break after handling the input.
        // If a client has been removed, the loop variables may no longer be 
        // valid.
        int cur_fd, handled;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if (FD_ISSET(cur_fd, &rset)) {
                handled = 0;

                // Check if any new clients are entering their names
                for (p = new_clients; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {
                        // TODO: handle input from a new client who has not yet entered an acceptable name
                        if(read_new_client(p,&active_clients, &new_clients)){
                            printf("Activating client...\n");
                            activate_client(p, &active_clients, &new_clients);
                            char notification[BUF_SIZE + 20] = {'\0'};
                            sprintf(notification, "%s has just joined\r\n", p->username);
                            announce(active_clients, notification);
                            printf("%s", notification);
                        }
                        handled = 1;
                        break;
                    }
                }

                if (!handled) {
                    // Check if this socket descriptor is an active client
                    for (p = active_clients; p != NULL; p = p->next) {
                        if (cur_fd == p->fd) {
                            // TODO: handle input from an active client
                            active_user_input(&active_clients, p);
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
