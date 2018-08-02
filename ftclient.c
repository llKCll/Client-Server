/*
 ** By: Kevin Conner
 ** Credits: Brian “Beej Jorgensen” Hall, https://www.tutorialspoint.com/c_standard_library/c_function_toupper.htm
 */

#include "ftclient.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXDATASIZE 1024 // Max number of bytes we can get at once.

// Get sockaddr, IPv4 or IPv6.
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Returns 1 if user wishes to overwrite the file, 0 otherwise.
int getOverwrite(char *fname)
{
    char response[100];
    printf("%s already exists. Do you wish to overwrite? Yes or No?\n", fname);
    
    while (1) {
        scanf("%s", response);
        upperall(response);
        if (strncmp(response, "YES", 5) == 0) {
            return 1;
        }
        
        else if (strncmp(response, "NO", 5) == 0) {
            return 0;
        }
        
        else {
            printf("%s is an invalid response. Please try again.\n", response);
            printf("%s already exists. Do you wish to overwrite? Yes or No?\n", fname);
            memset(&response, 0, sizeof response);
        }
    }
}

// Uppercase all letters.
void upperall(char *str)
{
    int i = 0;

    while(str[i]) {
        str[i] = toupper(str[i]);
        i++;
    }
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    int hnsize = 1024;
    char buf[MAXDATASIZE];
    char ip[INET6_ADDRSTRLEN];
    struct addrinfo hints, *servinfo, *p;
    char hostname[hnsize];
    
    gethostname(hostname, hnsize);
    
    // Validate number of args.
    if (argc != 5 && argc != 4) {
        fprintf(stderr, "3 Parameters if -1 command is used: <HOSTNAME> <PORT> <COMMAND> or\n");
        fprintf(stderr, "4 Parameters if -g command is used: <HOSTNAME> <PORT> <COMMAND> <FILE>\n");
        exit(1);
    }
    
    // Ensure 3 parameters if -1 used.
    else if (argc == 5 && strcmp(argv[3], "-1") == 0) {
        fprintf(stderr, "3 Parameters if -1 command is used: <HOSTNAME> <PORT> <COMMAND>\n");
        exit(1);
    }
    
    // Ensure 4 parameters if -g used.
    else if (argc == 4 && strcmp(argv[3], "-g") == 0) {
        fprintf(stderr, "4 Parameters if -g command is used: <HOSTNAME> <PORT> <COMMAND> <FILE>\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(argv[1], argv[2], &hints, &servinfo);
        
    // Loop through all the results and connect to the first we can.
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1 ) {
            perror("Error");
            continue;
        }
        // Attempt to connect. Keep going if unable to connect.
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    // All results checked and unable to connect.
    if (p == NULL) {
        fprintf(stderr, "Failed to connect to the server.\n");
        return 2;
    }
    
    // Connect to IP address.
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              ip, sizeof ip);
    
    printf("Connecting to %s.\n", ip);
    
    freeaddrinfo(servinfo);

    // Append to end of message so server can split on \n.
    char *newLine = "\n";

    // Send number of args so server knows what to expect.
    char *numOfArgs = (char*) malloc(sizeof(char)*10);
    sprintf(numOfArgs, "%d", argc);

    // Send number of args.
    if ((numbytes = send(sockfd, numOfArgs, strlen(numOfArgs), 0)) == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    } else {
        send(sockfd, newLine, 1, 0);
    }
       
    // Send command.
    if ((numbytes = send(sockfd, argv[3], strlen(argv[3]), 0)) == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    } else {
        send(sockfd, newLine, 1, 0);
    }

    // Send hostname.
    if ((numbytes = send(sockfd, hostname, strlen(hostname), 0)) == -1) {
        perror("send");
        close(sockfd);
        exit(1);
    } else {
        send(sockfd, newLine, 1, 0);
    }

     // Send requested filename to server.
    if (argc == 5) {
        if ((numbytes = send(sockfd, argv[4], strlen(argv[4]), 0)) == -1) {
            perror("send");
            close(sockfd);
            exit(1);
        } else {
            send(sockfd, newLine, 1, 0);
        }
    }

    // Receive from server.
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv");
        close(sockfd);
        exit(1);
    }
   
    // Invalid command was sent to the server. Print server message, close the socket, and exit.
    if (strncmp("INVALID COMMAND", buf, 15) == 0) {
        printf("%s: %s says...\n%s\n", argv[1], argv[2], buf);
        close(sockfd);
        exit(1);
    }

    // Check which command was chosen.

    // Print directory command.
    if (strncmp(argv[3], "-1", 2) == 0) {
        printf("Receiving directory structure from %s: %s.\n", argv[1], argv[2]);
        printf("%s\n", buf);
    }

    // Copy file from the server command.
    if (strncmp(argv[3], "-g", 2) == 0) {
        
        // Open the file to see if it exists.
        FILE *f = fopen(argv[4], "r");
        int overwriteChoice = 1;

        // If the file exists then find out if user wants to overwrite it. Close it when done. 
        if (f) {
            overwriteChoice = getOverwrite(argv[4]);
            fclose(f);
        }

        // Overwrite or create the file.
        if (overwriteChoice) {

            printf("Receiving %s from %s: %s.\n", argv[4], argv[1], argv[2]);

            // Build content to write to file.
            if (strncmp(buf, "FILE NOT FOUND", 14) == 0) {
                printf("%s: %s says...\n%s\n", argv[1], argv[2], buf);
                close(sockfd);
                exit(1);
            }

            // Open file to write.
            f = fopen(argv[4], "w");

            // Write content if open, otherwise unable to open.
            if (f) {
                fprintf(f, "%s", buf);
                fclose(f);
                printf("File transfer complete.\n");
            } else {
                perror("Unable to open file");
            }
        }
    }

    close(sockfd);
    return 0;
}
