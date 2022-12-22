#include "./../include/client_utilities.h"

int main(int argc, char *argv[])
{
    int sockfd;
    if ((sockfd = establish_connection(argv)) == -1)
    {
        close(sockfd);
        return -1;
    }

    if (connection(sockfd) == -1)
    {
        close(sockfd);
        return -1;
    }

    // close the socket
    close(sockfd);
    return 0;
}