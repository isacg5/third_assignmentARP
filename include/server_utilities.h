#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <bmpfile.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <arpa/inet.h>

#define SEM_PATH_WRITER "/sem_writer"
#define SEM_PATH_READER "/sem_reader"

#define W 1600
#define H 600
#define D 4
#define MAX 256

int pos_x = 45;
int pos_y = 15;
int index_snapshot = 0;
sem_t *sem_id_writer;
sem_t *sem_id_reader;
int sockfd;

// Typedef for circle struct
typedef struct
{
    int x, y;
} CIRCLE;

// Circle variable definition
CIRCLE circle;
// Window for print button
WINDOW *print_btn;
// Mouse event variable
MEVENT event;

int BTN_SIZE_Y = 3;
int BTN_SIZE_X = 7;

// Method to instantiate button window
void make_print_button()
{
    print_btn = newwin(BTN_SIZE_Y, BTN_SIZE_X, (LINES - BTN_SIZE_Y) / 2, (COLS - BTN_SIZE_X));
}

// Draw button with colored background
void draw_btn(WINDOW *btn, char label, int color)
{

    wbkgd(btn, COLOR_PAIR(color));
    wmove(btn, floor(BTN_SIZE_Y / 2), floor(BTN_SIZE_X / 2));

    attron(A_BOLD);
    waddch(btn, label);
    attroff(A_BOLD);

    wrefresh(btn);
}

// Utility method to check if button has been pressed
int check_button_pressed(WINDOW *btn, MEVENT *event)
{

    if (event->y >= btn->_begy && event->y < btn->_begy + BTN_SIZE_Y)
    {
        if (event->x >= btn->_begx && event->x < btn->_begx + BTN_SIZE_X)
        {
            return TRUE;
        }
    }
    return FALSE;
}

// Method to draw lateral elements of the UI (button)
void draw_side_ui()
{

    mvvline(0, COLS - BTN_SIZE_X - 1, ACS_VLINE, LINES);
    draw_btn(print_btn, 'P', 2);
    refresh();
}

// Set circle's initial position in the center of the window
void set_circle()
{
    circle.y = LINES / 2;
    circle.x = COLS / 2;
}

// Draw filled circle according to its equation
void draw_circle()
{
    attron(COLOR_PAIR(1));
    mvaddch(circle.y, circle.x, '@');
    mvaddch(circle.y - 1, circle.x, '@');
    mvaddch(circle.y + 1, circle.x, '@');
    mvaddch(circle.y, circle.x - 1, '@');
    mvaddch(circle.y, circle.x + 1, '@');
    attroff(COLOR_PAIR(1));
    refresh();
}

// Move circle window according to user's input
void move_circle(int cmd)
{
    // First, clear previous circle positions
    mvaddch(circle.y, circle.x, ' ');
    mvaddch(circle.y - 1, circle.x, ' ');
    mvaddch(circle.y + 1, circle.x, ' ');
    mvaddch(circle.y, circle.x - 1, ' ');
    mvaddch(circle.y, circle.x + 1, ' ');

    // Move circle by one character based on cmd
    switch (cmd)
    {
    case KEY_LEFT:
        if (circle.x - 1 > 0)
        {
            circle.x--;
        }
        break;
    case KEY_RIGHT:
        if (circle.x + 1 < COLS - BTN_SIZE_X - 2)
        {
            circle.x++;
        }
        break;
    case KEY_UP:
        if (circle.y - 1 > 0)
        {
            circle.y--;
        }
        break;
    case KEY_DOWN:
        if (circle.y + 2 < LINES)
        {
            circle.y++;
        }
        break;
    default:
        break;
    }
    refresh();
}

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

    // Initialize UI elements
    set_circle();
    make_print_button();

    // Draw UI elements
    draw_circle();
    draw_side_ui();

    // Activate input listening (keybord + mouse events ...)
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    refresh();
}

void reset_console_ui()
{

    // Free resources
    delwin(print_btn);

    // Clear screen
    erase();
    refresh();

    // Initialize UI elements
    set_circle();
    make_print_button();

    // Draw UI elements
    draw_circle();
    draw_side_ui();
}

int establish_connection(char *argv[])
{
    int port = 1;
    if ((port = atoi(argv[2])) == 0)
    {
        perror("Can not convert into integer\n");
        return -1;
    }

    int connfd, len;
    struct sockaddr_in serv_addr, cli_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed...\n");
        return -1;
    }
    else
        printf("Socket successfully created..\n");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    // assign IP, PORT
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        perror("Socket bind failed...\n");
        return -1;
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0)
    {
        perror("Listen failed...\n");
        return -1;
    }
    else
        printf("Server listening..\n");

    len = sizeof(cli_addr);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);
    if (connfd < 0)
    {
        perror("Server accept failed...\n");
        return -1;
    }
    else
    {
        printf("Server accept the client...\n");
        return connfd;
    }
}

void write_on_shared_mem(int *ptr, int radius, int x_pos, int y_pos)
{
    x_pos = 20 * x_pos;
    y_pos = 20 * y_pos;

    sem_wait(sem_id_writer);

    // Remove previous circle
    for (int row = 0; row < H; row++)
    {
        for (int col = 0; col < W; col++)
        {
            ptr[col + row * W] = 0;
        }
    }

    // Write the new one given the center coords and the radious
    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            // If distance is smaller, point is within the circle
            if (sqrt(x * x + y * y) < radius)
            {
                /*
                 * Color the pixel at the specified (x,y) position
                 * with the given pixel values
                 */
                int x_p = x_pos + x;
                int y_p = y_pos + y;
                ptr[y_p + W * x_p] = 255;
            }
        }
    }

    sem_post(sem_id_reader);
}

void remove_previous_circle(bmpfile_t *bmp)
{
    rgb_pixel_t *pixel;
    rgb_pixel_t white = {255, 255, 255, 0};
    for (int x = 0; x <= W; x++)
    {
        for (int y = 0; y <= H; y++)
        {
            bmp_set_pixel(bmp, x, y, white);
        }
    }
}

void print_circle(int radius, rgb_pixel_t pixel, int x_pos, int y_pos, bmpfile_t *bmp)
{
    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            // If distance is smaller, point is within the circle
            if (sqrt(x * x + y * y) < radius)
            {
                /*
                 * Color the pixel at the specified (x,y) position
                 * with the given pixel values
                 */
                bmp_set_pixel(bmp, (x_pos * 20) + x, (y_pos * 20) + y, pixel);
            }
        }
    }
}

int get_pos(int number, bmpfile_t *bmp)
{
    char filename[128];
    switch (number)
    {
    case KEY_LEFT:
        if (pos_x > 1)
        {
            pos_x -= 1;
        }
        break;

    case KEY_RIGHT:
        if (pos_x < 80)
        {
            pos_x += 1;
        }
        break;

    case KEY_UP:
        if (pos_y > 2)
        {
            pos_y -= 1;
        }
        break;

    case KEY_DOWN:
        if (pos_y < 28)
        {
            pos_y += 1;
        }
        break;

    case 112:
        if (sprintf(filename, "./out/snapshot_%d.bmp", index_snapshot) < 0)
        {
            perror("Error using sprintf");
            return -1;
        }
        index_snapshot++;
        // Save image as .bmp file
        bmp_save(bmp, filename);

        mvprintw(LINES - 1, 1, "Print button pressed");
        refresh();
        sleep(1);
        for (int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
        {
            mvaddch(LINES - 1, j, ' ');
        }
        break;
    }

    return 0;
}

int open_semaphores()
{
    sem_id_writer = sem_open(SEM_PATH_WRITER, O_CREAT, 0644, 1);
    if (sem_id_writer == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }

    sem_id_reader = sem_open(SEM_PATH_READER, O_CREAT, 0644, 1);
    if (sem_id_reader == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }
    return 0;
}

int get_number(int connfd)
{
    char buff[MAX];
    int n;
    char format_string[80] = "%d";
    int number;

    bzero(buff, MAX);

    // read the message from client and copy it in buffer
    if ((n = read(connfd, buff, sizeof(buff))) < 0)
    {
        perror("Error reading from socket");
        return -1;
    }

    sscanf(buff, format_string, &number);

    // and send that buffer to client
    if ((n = write(connfd, "1", 1)) < 0)
    {
        perror("Error writing to socket");
        return -1;
    }

    return number;
}

int connection(int connfd, int *ptr, bmpfile_t *bmp, rgb_pixel_t pixel)
{
    int number;
    while (1)
    {
        if ((number = get_number(connfd)) == -1)
        {
            return -1;
        }
        if ((get_pos(number, bmp)) == -1)
        {
            return -1;
        }

        remove_previous_circle(bmp);
        print_circle(30, pixel, pos_x, pos_y, bmp);

        // Static shared memory
        write_on_shared_mem(ptr, 30, pos_y, pos_x);

        move_circle(number);
        draw_circle();
    }
    return 0;
}
