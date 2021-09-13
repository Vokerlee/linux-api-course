#include "biz_error.h"
#include "biz_handler.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

const char BIZZ[]      = "bizz";
const char BUZZ[]      = "buzz";
const char BIZZ_BUZZ[] = "bizz_buzz";

void biz_strings(int argc, char *argv[])
{
    assert(argv);

// Files opening
    errno = 0;
    int file_descr_in = open(argv[1], O_RDONLY);
    if (file_descr_in < 0)
        ASSERT_OK(errno)

    errno = 0;
    int file_descr_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
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
    errno = 0;
    int n_read = read(file_descr_in, buffer, input_size);
    if (n_read == -1)
        ASSERT_OK(errno)
    else if (n_read != input_size)
        ASSERT_OK(READ_SIZE)

    char* output_buffer = number_parsing(buffer, &n_read);
    int   output_size   = strlen(output_buffer);

// Output is written
    int n_write = write(file_descr_out, output_buffer, output_size); // writing whole file
    if (n_write == -1)
        ASSERT_OK(errno)

// Files closing
    close(file_descr_in);
    close(file_descr_out);
}

char *number_parsing(char *buffer, int *buffer_size)
{
    assert(buffer);

    const char bizz[]      = "bizz";
    const char buzz[]      = "buzz";
    const char bizz_buzz[] = "bizzbuzz";

    char*      p           = buffer; // counter (for convenience)
    char**     endptr      = (char**) calloc(1, sizeof(char*)); 
    if (!endptr)
        ASSERT_OK(BAD_ALLOC)

    unsigned buf_offset    = 0;
    long     number        = 0; 

    while(*p)
    {
        number = strtol(p, endptr, 0);
        unsigned digits = 0;

        if (number > 0)
        {
            digits = count_digits(number);

            if (number % 15 == 0)
            {
                p = *endptr - digits; // the beginning of number
                buffer = str_replace(buffer, p, BIZZ_BUZZ, buffer_size, sizeof(BIZZ_BUZZ), digits);
            } 
            else if (number % 3 == 0)
            {
                p = *endptr - digits; // the beginning of number
                buffer = str_replace(buffer, p, BIZZ,      buffer_size, sizeof(BIZZ),      digits);

            }
            else if (number % 5 == 0) 
            {
                p = *endptr - digits; // the beginning of number
                buffer = str_replace(buffer, p, BUZZ,      buffer_size, sizeof(BUZZ),      digits);
            }

            p = buffer + buf_offset; // refreshing p pointer
        }

        strtol(p, endptr, 0); // refreshing endptr pointer

        if (number > 0)       // if a number was parsed
        { 
            p = *endptr;
            buf_offset = p - buffer;
        }
        else 
        {
            p++;
            buf_offset++; 
        }
    }

    free(endptr);

    return buffer;
}

char* str_replace(char* buffer, char* pos, const char* str, unsigned* buf_size, unsigned insert_sz, unsigned offset)
{
    assert(buffer);
    assert(pos);
    assert(str);
    assert(buf_size);

    unsigned pos_offset = pos - buffer;      // the part of buffer that is to be saved (before)
    char*    remaining  = pos + offset;      // the part of buffer that is to be appended after insertion
    int      rem_len    = strlen(remaining);

    // Copy remaining part
    char* copy_remaining = (char*) calloc(rem_len + 1, sizeof(char));
    if (copy_remaining == NULL)
        ASSERT_OK(BAD_ALLOC)

    strcpy(copy_remaining, remaining);
    // End of copying

    *buf_size += insert_sz + offset + 1;

    int rem_size = strlen(copy_remaining);

    char* new_buffer = realloc(buffer, *buf_size);
    if (new_buffer== NULL)
        ASSERT_OK(BAD_ALLOC);

    strcpy(new_buffer + pos_offset, str);
    strcat(new_buffer, copy_remaining);

    free(copy_remaining);

    return new_buffer;
}

unsigned count_digits(long number)
{
    unsigned count = 0;

    while (number != 0)
    {
        number /= 10;
        ++count;
    }

    return count;
}