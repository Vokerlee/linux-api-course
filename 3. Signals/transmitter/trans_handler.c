#include "trans_handler.h"

void transmit(const char *file_name, const pid_t reciever_pid)
{
    assert(file_name);

    // FILE OPENING
    size_t filename_size = strlen(file_name);

    errno = 0;
    int fd = open(file_name, O_RDONLY);
    ERR_CHECK(fd == -1, errno)

    errno = 0;
    struct stat file_stat = {0};
    int fstat_state = fstat(fd, &file_stat);
    ERR_CHECK(fstat_state == -1, errno)

    int data_size = file_stat.st_size;

    // BUFFER CREATING & FILE READING
    char* buf = (char*) calloc(data_size, sizeof(char));
    ERR_CHECK(buf == NULL, BAD_ALLOC)

    errno = 0;
    int n_read = read(fd, buf, data_size);
    ERR_CHECK(n_read == -1, errno)
    ERR_CHECK(n_read != data_size, READ_SIZE)

    // TRANSMIITING
    errno = 0;
    int kill_state = kill(reciever_pid, SIGUSR1); // send signal for the receiver to get data from fifo (see further)
    ERR_CHECK(kill_state == -1, errno)

    unlink("test_fifo");
    errno = 0;
    int fifo_state = mkfifo("test_fifo", 0666);
    ERR_CHECK(fifo_state == -1, errno)

    errno = 0;
    int fifo_fd = open("test_fifo", O_WRONLY);
    ERR_CHECK(fifo_fd == -1, errno)

    errno = 0;
    int n_write = write(fifo_fd, buf, data_size);
    ERR_CHECK(n_write == -1, errno)
    ERR_CHECK(n_write != data_size, WRITE_SIZE)
    
    errno = 0;
    fifo_state = close(fifo_fd);
    ERR_CHECK(fifo_state == -1, errno)

    errno = 0;
    int close_state = close(fd);
    ERR_CHECK(close_state == -1, errno)
}
