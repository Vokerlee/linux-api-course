#include "biz_error.h"
#include "biz_handler.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

void biz_strings(int argc, char *argv[])
{
    assert(argv);

// Files opening
    int file_descr_in = open(argv[1], O_RDONLY);
    if (file_descr_in < 0)
        ASSERT_OK(errno)

    int file_descr_out = open(argv[2], O_WRONLY);
    if (file_descr_out < 0)
        ASSERT_OK(errno)

// Input file size
    struct stat input = {0};
    fstat(file_descr_in, &input);

    size_t input_size = input.st_size;

    char *buffer = (char *) calloc(input_size + 1, sizeof(char));
    if (buffer == NULL)
        ASSERT_OK(BAD_ALLOC)

// Input is read
    int n_read = read(file_descr_in, buffer, input_size);
    if (n_read == -1)
        ASSERT_OK(errno)
    else if (n_read != input_size)
        ASSERT_OK(READ_SIZE)

    unsigned int buffer_size = (unsigned) n_read; // auto-typecasting error escape

    char* output_buffer = number_parsing(buffer, &buffer_size);
    int   output_size   = strlen(output_buffer);

// Output is written
    int n_write = write(file_descr_out, output_buffer, output_size); // writing whole file
    if (n_write == -1)
        ASSERT_OK(errno)

// Files closing
    close(file_descr_in);
    close(file_descr_out);
}

char *number_parsing(char *buffer, int buffer_size)
{
    assert(buffer)

    ///; создать новый буфер с исправленным текстом и вернуть его


}