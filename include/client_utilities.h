#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <ncurses.h>
#include <time.h>
#include <signal.h>

#define MAX 256
#define LEN 10

int disconnect = 0;

void init_console_ui()
{
    // Initialize curses mode
    initscr();
    start_color();

    // Disable line buffering...
    cbreak();

    // Disable char echoing and blinking cursos
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);

    // Activate input listening (keybord + mouse events ...)
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    refresh();
}

void clean(int i)
{
    for (int j = 0; j < COLS; j++)
    {
        mvaddch(i, j, ' ');
    }
    refresh();
}

int connection(int sockfd)
{
    init_console_ui();
    char buff[MAX];
    int n;

    while (1)
    {
        bzero(buff, sizeof(buff));

        mvprintw(1, 1, "Please, indicate the movement to do (up, down, right, left) or press P to print");
        refresh();

        int cmd = getch();
        if (cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN || cmd == 112)
        {
            clean(1);
            sprintf(buff, "%d", cmd);

            if ((n = write(sockfd, buff, sizeof(buff))) < 0)
            {
                perror("Error writing to socket");
                return -1;
            }

            bzero(buff, sizeof(buff));

            if ((n = read(sockfd, buff, sizeof(buff))) < 0)
            {
                perror("Error reading from socket");
                return -1;
            }

            int checkout = atoi(buff);
            if (checkout == 1)
            {
                mvprintw(2, 1, "Server correctly received the information!");
                refresh();
                sleep(1);
                clean(2);
            }
        }
    }

    return 0;
}

int establish_connection(char *argv[])
{
    int port = 0;
    port = atoi(argv[2]);

    int sockfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;

    // socket create and verification
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed...\n");
        return -1;
    }
    else
        printf("Socket successfully created..\n");

    bzero(&serv_addr, sizeof(serv_addr));

    // assign IP, PORT
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(port);

    // connect the client socket to server socket
    if ((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0)
    {
        perror("Connection with the server failed...\n");
        return -1;
    }
    else
        printf("Connected to the server..\n");

    return sockfd;
}

void exit_handler(int signo)
{
    if (signo == SIGINT)
    {
        char line[LEN];
        FILE *cmd2 = popen("pidof -s master", "r");
        fgets(line, LEN, cmd2);
        pclose(cmd2);
        long pid2 = 0;
        pid2 = strtoul(line, NULL, 10);
        kill(pid2, SIGUSR2);

        exit(EXIT_SUCCESS);
    }
}

void disconnect_handler(int signo)
{
    if (signo == SIGTSTP)
    {
        char line[LEN];
        FILE *cmd2 = popen("pidof -s master", "r");
        fgets(line, LEN, cmd2);
        pclose(cmd2);
        long pid2 = 0;
        pid2 = strtoul(line, NULL, 10);
        kill(pid2, SIGUSR1);

        exit(EXIT_SUCCESS);
    }
}
