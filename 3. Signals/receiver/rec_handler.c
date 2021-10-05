#include "rec_handler.h"

char *file_name = 0;

const size_t BUFFER_SIZE = 0x1000;
const size_t FILENAME_SIZE = 0x50;

void usr1_handler(int signal)
{
    errno = 0;
    int fifo_fd = open("test_fifo", O_RDONLY);
    ERR_CHECK(fifo_fd == -1, errno)

    char* data = (char*) calloc(BUFFER_SIZE + 1, sizeof(char));
    ERR_CHECK(data == NULL, BAD_ALLOC)

    errno = 0;
    int n_read = read(fifo_fd, data, BUFFER_SIZE);
    ERR_CHECK(n_read == -1, errno)

    // here any actions can be done with received data, e. g.:

    errno = 0;
    int output_fd = open("received_data", O_WRONLY | O_CREAT, 0666);
    ERR_CHECK(output_fd == -1, errno)

    errno = 0;
    int n_out_write = write(output_fd, data, n_read);
    ERR_CHECK(n_out_write == -1, errno)
    ERR_CHECK(n_out_write != n_read, WRITE_SIZE)

    int close_state = 0;

    errno = 0;
    close_state = close(fifo_fd);
    ERR_CHECK(close_state == -1, errno)

    errno = 0;
    close_state = close(output_fd);
    ERR_CHECK(close_state == -1, errno)
}

