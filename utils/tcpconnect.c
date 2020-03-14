#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

void print_address(const char *prefix, const struct addrinfo *res)
{
        char outbuf[80];
        struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)res->ai_addr;
        void *address;

        if (res->ai_family == AF_INET)
                address = &(sin->sin_addr);
        else if (res->ai_family == AF_INET6)
                address = &(sin6->sin6_addr);
        else {
                printf("Unknown address\n");
                return;
        }

        const char *ret = inet_ntop(res->ai_family, address,
                                    outbuf, sizeof(outbuf));
        printf("%s %s\n", prefix, ret);
}

int tcp_connect(const char *host, const char *serv)
{
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
            fprintf(stderr, "tcp_connect error for %s, %s: %s\n",
                    host, serv, gai_strerror(n));
            return -1;
    }
    ressave = res; // so that we can release the memory afterwards

    // res-rakenne osoittaa linkitettyyn listaan. K‰yd‰‰n l‰pi linkitetty‰
    // listaa yksi kerrallaan ja yritet‰‰n yhdist‰‰ saatuun osoitteeseen.
    // res-rakenne sis‰lt‰‰ kaikki parameterit mit‰ tarvitsemme socket-
    // ja connect - kutsuissa.
    do {
           sockfd = socket(res->ai_family, res->ai_socktype,
                            res->ai_protocol);
            if (sockfd < 0)
                    continue;       /* ignore this one */

            print_address("Trying to connect", res);

            // Mik‰li yhteys onnistuu, silmukka keskeytet‰‰n v‰littˆm‰sti,
            // koska meill‰ on toimiva yhteys, eik‰ loppuja osoitteita
            // tarvitse kokeilla
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
                  break;          /* success */

            printf("connect failed\n");

            close(sockfd);  /* ignore this one */
    } while ( (res = res->ai_next) != NULL);

    // P‰‰stiinkˆ linkitetyn listan loppuun, mutta yhteys ei onnistunut?
    // ==> virhe
    if (res == NULL) {      /* errno set from final connect() */
            fprintf(stderr, "tcp_connect error for %s, %s\n", host, serv);
            sockfd = -1;
    } else {
            print_address("We are using address", res);
    }

    // J‰rjestelm‰ on varannut muistin linkitetylle listalle, se pit‰‰ vapauttaa
    freeaddrinfo(ressave);

    return sockfd;
}
