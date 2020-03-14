#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...
#include <stdlib.h>
#include "studentnumber.h"

#define MAXLINE 80

int writemessage(const char *message, int sockfd)
{
    size_t length = strlen(message);
    int i = 0;
    int n = 0;
    while ((n = write(sockfd, message + i, length - i)) > 0) {
        i += n;
    }
    if (n < 0) {
        perror("Error writing to socket");
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
    struct sockaddr_in servaddr;  // tietorakenne, joka esittää osoitetta

    const char *address = "195.148.124.236";

    // Luodaan pistoke, joka käyttää IPv4 - protokollaa (AF_INET)
    // ja TCP-protokollaa (SOCK_STREAM)
    // Paluuarvo on pistokkeen tunniste, tai -1 jos luominen ei onnistunut
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    // Alustetaan osoitetta esittävä tietorakenne nollilla.
    // Sen jälkeen kerrotaan että osoiteperhe on IPv4,
    // ja määritellään palvelimen portti johon tullaan ottamaan yhteyttä
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(5000);

    // Seuraava funktio muuntaa ASCII-muotoisen IP-osoitteen binääriseksi.
    // Se talletetaan servaddr - rakenteeseen.
    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s\n", address);
        return 1;
    }

    // Avataan TCP-yhteys käyttäen edellä määriteltyä servaddr - rakennetta.
    // Jos yhteydenotto onnistui, palautetaan 0. Muuten negatiivinen arvo.
    if (connect(sockfd,
                (struct sockaddr *) &servaddr,
                sizeof(servaddr)) < 0) {
        perror("connect error");
        return 1;
    }

    char *number = student_number();
    char message[17];
    memcpy(message, number, 6);
    memcpy(message + 6, "\n3-large\n", 11);
    free(number);
    if (!writemessage(message, sockfd)) return 1;
    while (1) {
        uint32_t bytes_to_read;
        uint8_t *buf = (uint8_t *)&bytes_to_read;
        int ret = read_bytes(sockfd, buf, 4);
        if (ret != 4) return 1;
        bytes_to_read = ntohl(bytes_to_read);
        uint8_t *msg = malloc(bytes_to_read);
        if (!msg) {
            perror("Error allocating memory");
            return 1;
        }
        ret = read_bytes(sockfd, msg, bytes_to_read);
        free(msg);
        if (ret != bytes_to_read) return 1;
        uint32_t bytes = htonl(bytes_to_read);
        ret = write_bytes(sockfd, &bytes, 4);
        if (ret != 4) return 1;
        if (!bytes_to_read) return 0;
    }
    return 0;
}
