#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...
#include <stdlib.h>
#include "studentnumber.h"

#define MAXLINE 80

int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    const char *address = "195.148.124.236";

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(5000);

    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s\n", address);
        return 1;
    }

    if (connect(sockfd,
                (struct sockaddr *) &servaddr,
                sizeof(servaddr)) < 0) {
        perror("connect error");
        return 1;
    }
    char *number = student_number();
    if (!number) {
        perror("error reading student number");
    }
    char message[17];
    memcpy(message, number, 6);
    memcpy(message + 6, "\n1-simple\n", 11);
    free(number);
    size_t length = strlen(message);
    int bytes_written = 0;
    while (bytes_written < length) {
        int bytes;
        if ((bytes = write(sockfd, message, length)) < 0) {
            perror("error writing to socket");
        }
        bytes_written += bytes;
    }
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;

        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            return 1;
        }
    }

    // If read return value was 0, loop terminates, without error
    if (n < 0) {
        perror("read error");
        return 1;
    }
    return 0;
}
