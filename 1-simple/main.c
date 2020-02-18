#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...

#define MAXLINE 80

int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];  // merkkipuskuri, johon luetaan tietoa
    struct sockaddr_in servaddr;  // tietorakenne, joka esitt‰‰ osoitetta

    const char *address = "195.148.124.236";

    // Luodaan pistoke, joka k‰ytt‰‰ IPv4 - protokollaa (AF_INET)
    // ja TCP-protokollaa (SOCK_STREAM)
    // Paluuarvo on pistokkeen tunniste, tai -1 jos luominen ei onnistunut
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    // Alustetaan osoitetta esitt‰v‰ tietorakenne nollilla.
    // Sen j‰lkeen kerrotaan ett‰ osoiteperhe on IPv4,
    // ja m‰‰ritell‰‰n palvelimen portti johon tullaan ottamaan yhteytt‰
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(5000);

    // Seuraava funktio muuntaa ASCII-muotoisen IP-osoitteen bin‰‰riseksi.
    // Se talletetaan servaddr - rakenteeseen.
    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s\n", address);
        return 1;
    }

    // Avataan TCP-yhteys k‰ytt‰en edell‰ m‰‰ritelty‰ servaddr - rakennetta.
    // Jos yhteydenotto onnistui, palautetaan 0. Muuten negatiivinen arvo.
    if (connect(sockfd,
                (struct sockaddr *) &servaddr,
                sizeof(servaddr)) < 0) {
        perror("connect error");
        return 1;
    }

    const char *message = "715298\n1-simple\n";
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
        recvline[n] = 0; // lis‰t‰‰n loppunolla, jotta tulostus onnistuu

        //  tulostetaan stdout-virtaan (eli k‰ytt‰j‰n ruudulle)
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
