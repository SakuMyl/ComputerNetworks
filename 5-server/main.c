#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...
#include <stdlib.h>
#include "studentnumber.h"

#define MAXLINE 80

int write_message(const char *message, int sockfd)
{
    size_t length = strlen(message);
    int i = 0, n = 0;
    for (; i < length && (n = write(sockfd, message + i, length - i)) > 0; i += n);
    if (n < 0) {
        perror("Error writing to socket");
        return 0;
    }
    return 1;
}

int read_message(char *buf, int sockfd)
{
    int i = 0, n = 0;
    for (; (i == 0 || buf[i - 1] != '\n') && (n = read(sockfd, buf + i, MAXLINE - i)) > 0; i += n);
    buf[i] = 0;
    if (n < 0) {
        perror("Error reading from socket");
        return 0;
    }
    return 1;
}

int write_bytes(int sockfd, void *buf, int bytes)
{   
    uint8_t *buffer = buf;
    int i = 0, n = 0;
    for (; i < bytes && (n = write(sockfd, buffer + i, bytes - i)) > 0; i += n);
    if (n < 0) {
        perror("Error reading from socket");
        return 0;
    }
    return i;
}

int read_bytes(int sockfd, void *buf, int bytes)
{
    uint8_t *buffer = buf;
    int i = 0, n = 0;
    for (; i < bytes && (n = read(sockfd, buffer + i, bytes - i)) > 0; i += n);
    if (n < 0) {
        perror("Error reading socket");
        return 0;
    }
    return i;
}

int main(int argc, char **argv)
{
    int sockfd;
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

    char *num = student_number();
    char number[7];
    memcpy(number, num, 7);
    free(num);
    char message[17];
    memcpy(message, number, 6);
    memcpy(message + 6, "\n5-server\n", 11);
    if (!write_message(message, sockfd)) return 1;
    
    printf("Student number sent\n");
    while (1) {
        char msg[MAXLINE];
        if (!read_message(msg, sockfd)) return 1;
        printf("First message read %s\n", msg);
        if (strstr(msg, "OK") || strstr(msg, "FAIL")) {
            printf("%s", msg);
            return 0;
        }
        int sockfd2;
        int connfd;
        if (strcmp(msg, "MORE\n") == 0) {
            if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket error");
                return 1;
            }
            struct sockaddr_in own;
            bind(sockfd2, (struct sockaddr *)&own, sizeof(struct sockaddr));
            listen(sockfd2, 10);
            /*if (getsockname(sockfd2, (struct sockaddr *)&own, &ownlen) < 0) {
                perror("getsockname error");
                return 1;
            }*/
            strcpy(msg, "SERV ");
            inet_ntop(AF_INET, &own.sin_addr, msg + 5, INET_ADDRSTRLEN);
            if (sprintf(msg + strlen(msg), " %hu %c", ntohs(own.sin_port), '\n') < 2) {
                perror("sprintf error");
                return 1;
            }
            printf("Before writing SERV message: %s\n", msg);
            if (!write_message(msg, sockfd)) return 1;
            struct sockaddr_in incoming;
            socklen_t incominglen = sizeof(struct sockaddr);
            printf("Before accepting\n");
            if((connfd = accept(sockfd2, (struct sockaddr *)&incoming, &incominglen)) < 0) {
                perror("accept error");
                return 1;
            }
        }
        uint32_t bytes;
        printf("Before reading byte count\n");
        if (read_bytes(connfd, &bytes, 4) < 4) return 1;
        bytes = ntohl(bytes);
        uint8_t msg_back[bytes];
        memset(msg_back, 'A', bytes);
        printf("Before writing bytes\n");
        if (write_bytes(connfd, msg_back, bytes) < bytes) return 1;
    }
    
    return 0;
}
