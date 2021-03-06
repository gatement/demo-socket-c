#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int main(void)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind");

    // send to myself
    sendto(sock, "ABCD", 4, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    char recvbuf[1];
    int n;
    int i;
    for (i = 0; i < 4; i++)
    {
	// if recv buf less than the packet size, the un-read bytes will be lost even you want to read them again as this example
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        else if(n > 0) {
            printf("i=%d, n=%d: %c\n", i, n, recvbuf[0]);
        }
    }

    close(sock);
    return 0;
}
