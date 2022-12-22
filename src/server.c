#include "./../include/server_utilities.h"

int main(int argc, char *argv[])
{
    const int SIZE = W * H * sizeof(int);
    const char *shm_name = "/STATIC_SHARED_MEM";
    int i, shm_fd;
    int *ptr;
    int connfd;
    bmpfile_t *bmp;
    rgb_pixel_t pixel = {255, 0, 0, 0};

    if ((connfd = establish_connection(argv)) == -1)
    {
        return -1;
    }

    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == 1)
    {
        perror("Shared memory segment failed\n");
        return -1;
    }

    ftruncate(shm_fd, SIZE);
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("Map failed\n");
        return -1;
    }

    if (open_semaphores() == -1)
    {
        return -1;
    }

    sem_init(sem_id_writer, 1, 1);
    sem_init(sem_id_reader, 1, 0);

    if ((bmp = bmp_create(W, H, D)) == NULL)
    {
        perror("Error creating bitmap");
        return -1;
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    write_on_shared_mem(ptr, 30, pos_y, pos_x);
    print_circle(30, pixel, pos_x, pos_y, bmp);

    if((connection(connfd, ptr, bmp, pixel)) == -1){
        return -1;
    }

    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);

    munmap(ptr, SIZE);
    bmp_destroy(bmp);
    endwin();

    // After chatting close the socket
    close(sockfd);
}