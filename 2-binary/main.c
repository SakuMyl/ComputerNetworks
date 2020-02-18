#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...
#include <stdlib.h>
#include "template.h"
#include "studentnumber.h"

#define MAXLINE 80

int writemessage(const char *message, int sockfd)
{
    size_t length = strlen(message) + 1;
    int i = 0;
    int n = 0;
    while ((n = write(sockfd, message + i, length - i)) > 0) {
        i += n;
    }
    if (n < 0) {
        perror("error writing to socket");
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];  // merkkipuskuri, johon luetaan tietoa
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
    memcpy(message + 6, "\n2-binary\n", 11);
    free(number);
    if (!writemessage(message, sockfd)) return 1;
    int i = 0;
    while((n = read(sockfd, recvline + i, MAXLINE - i)) > 0) {
        i += n;
        recvline[i] = 0;
        if (recvline[i - 1] == '\n') break;
    }

    printf("First message: %s\n", recvline);
    // If read return value was 0, loop terminates, without error
    if (n < 0) {
        perror("read error");
        return 1;
    }
    struct numbers *numbers = malloc(sizeof(struct numbers));
    if (!numbers) return 1;
    int parse = parse_str(recvline, numbers);
    if (parse != 5) {
        free(numbers);
        perror("parsing error");
        return 1;
    }
    uint32_t b = htonl(numbers->b);
    uint16_t d = htons(numbers->d);
    uint32_t e = htonl(numbers->e);
    write(sockfd, &numbers->a, 1);
    write(sockfd, &b, 4);
    write(sockfd, &numbers->c, 1);
    write(sockfd, &d, 2);
    write(sockfd, &e, 4);

    i = 0;
    while ((n = read(sockfd, recvline + i, sizeof(struct numbers) - i)) > 0) {
        i += n;
    }
    if (n < 0) {
        perror("read error");
        free(numbers);
        return 1;
    }
    printf("second message: %hhu %u %hhu %hu %u\n", recvline[0], *((uint32_t *) (recvline + 1)), recvline[5], *((uint16_t *) (recvline + 6)), *((uint32_t * ) (recvline + 8)));
    free(numbers);

    struct numbers *new_numbers = malloc(sizeof(struct numbers));
    if (!new_numbers) return 1;
    uint32_t *pb = (uint32_t *) (recvline + 1);
    *pb = ntohl(*pb);
    uint16_t *pd = (uint16_t *) (recvline + 6);
    *pd = ntohs(*pd);
    uint32_t *pe = (uint32_t *) (recvline + 8);
    *pe = ntohl(*pe);
    new_numbers->a = recvline[0];
    new_numbers->b = *pb;
    new_numbers->c = recvline[5];
    new_numbers->d = *pd;
    new_numbers->e = *pe;
    char str[MAXLINE];
    output_str(str, MAXLINE - 1, new_numbers);
    printf("second message in host endianness: %s", str);
    if (!writemessage(str, sockfd)) {
        free(new_numbers);
        return 1;
    }
    i = 0;
    while ((n = read(sockfd, recvline + i, MAXLINE - i)) > 0) {
        i += n;
        recvline[i] = 0;
    }
    printf("third message: %s\n", recvline);

    if (n < 0) {
        perror("read error");
        free(new_numbers);
        return 1;
    }
    free(new_numbers);
    return 0;
}
