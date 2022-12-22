#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <bmpfile.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SEM_PATH_WRITER "/sem_writer"
#define SEM_PATH_READER "/sem_reader"

#define W 1600
#define H 600
#define D 4

sem_t *sem_id_writer;
sem_t *sem_id_reader;

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

    refresh();
}

void reset_console_ui()
{

    // Clear screen
    erase();
    refresh();
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
void get_position(int *ptr, int *x, int *y)
{
    sem_wait(sem_id_reader);
    int c = 0;
    int max = -1;
    int index_x = -1;
    int index_y = -1;
    int equal_rows = 0;

    for (int row = 0; row < H; row++)
    {
        c = 0;
        for (int col = 0; col < W; col++)
        {
            int index = col + row * W;
            if (ptr[index] == 255)
            {
                c++;
            }
            else
            {
                if (c == max)
                {
                    equal_rows = equal_rows + 1;
                }
                if (c > max)
                {
                    max = c;
                    index_x = col;
                    index_y = row;
                    equal_rows = 0;
                }

                c = 0;
            }
        }
    }

    int max_2 = -1;
    int equal_cols = 0;

    for (int col = 0; col < W; col++)
    {
        c = 0;
        for (int row = 0; row < H; row++)
        {
            int index = col + row * W;
            if (ptr[index] == 255)
            {
                c++;
            }
            else
            {
                if (c == max_2)
                {
                    equal_cols = equal_cols + 1;
                }
                if (c > max_2)
                {
                    max_2 = c;
                    index_y = row;
                    equal_cols = 0;
                }

                c = 0;
            }
        }
    }

    *x = (index_x - (max / 2) - 1) / 20;
    if (*x == 0)
        *x = W / 20;
    *y = (index_y - (max_2 / 2) - 1) / 20;

    sem_post(sem_id_writer);
}

int open_semaphores()
{
    sem_id_writer = sem_open(SEM_PATH_WRITER, 0);
    if (sem_id_writer == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }

    sem_id_reader = sem_open(SEM_PATH_READER, 0);
    if (sem_id_reader == (void *)-1)
    {
        perror("sem_open failure");
        return -1;
    }
    return 0;
}
