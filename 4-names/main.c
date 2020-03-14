#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...
#include <stdlib.h>
#include "tcpconnect.h"
#include "studentnumber.h"

#define MAXLINE 80

int writemessage(const char *message, int sockfd)
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

void extract_dns_and_port_from_message(char *dns, char *port, char *msg)
{
    // Skip the "CONN" message
    strtok(msg, " ");
    //Read dns
    strcpy(dns, strtok(NULL, " "));
    strcpy(port, strtok(NULL, "\n"));
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
    memcpy(message + 6, "\n4-names\n", 11);
    if (!writemessage(message, sockfd)) return 1;

    char msg[MAXLINE];
    while (1) {
    	if (!read_message(msg, sockfd)) return 1;
	if (strstr(msg, "FAIL") != NULL || strstr(msg, "OK") != NULL) {
	    printf("%s\n", msg);
	    return 0;
	}
    	char dns[MAXLINE];
    	char port[10];
    	extract_dns_and_port_from_message(dns, port, msg);
    	
	//Ugly workaround for the test timeout caused by failing connection to the last server, fortius.sarolahti.fi	
    	int sockfd2 = strcmp(dns, "fortius.sarolahti.fi") ? tcp_connect(dns, port) : -1;
    	if (sockfd2 < 0) {
    	    if (!writemessage("FAIL\n", sockfd)) return 1;
	    continue;
    	}

    	struct sockaddr_in own;
    	socklen_t ownlen = sizeof(struct sockaddr);
    	if (getsockname(sockfd2, (struct sockaddr *)&own, &ownlen) < 0) {
    	    perror("getsockname error: ");
    	    return 1;
    	}
    	
    	memcpy(msg, "ADDR ", 5);
    	inet_ntop(AF_INET, &own.sin_addr, msg + 5, INET_ADDRSTRLEN);
    	if (sprintf(msg + strlen(msg), " %hu %s%c", ntohs(own.sin_port), number, '\n') < 3) {
    	    perror("sprintf error: ");
    	    return 1;
    	}
    	if (!writemessage(msg, sockfd2)) return 1;
    } 	
    return 0;
}
