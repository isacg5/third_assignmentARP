#include "./../include/client_utilities.h"

int main(int argc, char *argv[])
{
    if (signal(SIGINT, exit_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    if (signal(SIGTSTP, disconnect_handler) == SIG_ERR)
        printf("\ncan't catch SIGTSTP\n");

    int sockfd;

    // Establish connection
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