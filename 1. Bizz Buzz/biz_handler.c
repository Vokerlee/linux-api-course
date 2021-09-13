#include "errors_vh.h"
#include "biz_handler.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

const char BIZZ[]      = "bizz";
const char BUZZ[]      = "buzz";
const char BIZZ_BUZZ[] = "bizz_buzz";

const unsigned long BUFFER_SIZE = 4096;

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

// Strings changes

    char buffer_in [BUFFER_SIZE];
    char buffer_out[BUFFER_SIZE];

    if (memset(buffer_in,  0, BUFFER_SIZE) == 0)
        ASSERT_OK(MEMSET)

    if (memset(buffer_out, 0, BUFFER_SIZE) == 0)
        ASSERT_OK(MEMSET)

    int read_state    = 1;

    int n_in_symbols  = 0;
    int n_out_symbols = 0;

    int n_already_read = 0;

    // Buffer analysis cycle
    while (read_state)
    {
        // Reading to in-buffer
        errno = 0;
        int n_read = read(file_descr_in, buffer_in, BUFFER_SIZE);
        if (n_read == -1)
            ASSERT_OK(errno)
        else if (n_read != BUFFER_SIZE)
            read_state = 0;

        int n_read_max  = n_read;
        if (n_read == BUFFER_SIZE)
        {
            while (isdigit(buffer_in[n_read_max - 1]) && n_read_max > 0)
                n_read_max--;
        }

        int n_write_max = BUFFER_SIZE;

        while (n_in_symbols < n_read_max &&  n_out_symbols < n_write_max)
        {
            // Writing all non-number symbols to out-buffer
            while (isdigit(buffer_in[n_in_symbols]) == 0 && n_in_symbols < n_read_max &&  n_out_symbols < n_write_max)
            {
                buffer_out[n_out_symbols] = buffer_in[n_in_symbols];
                n_out_symbols++;
                n_in_symbols++;
            }

            printf("%d %d\n", n_in_symbols, n_out_symbols);

            if (n_in_symbols < n_read_max &&  n_out_symbols < n_write_max)
                biz_handle_number(&buffer_in[n_in_symbols], &buffer_out[n_out_symbols], &n_in_symbols, &n_out_symbols);

            printf("%d %d %d %d\n", n_in_symbols, n_out_symbols, n_read_max, n_write_max);
        }

        // Writing out-buffer to file
        int n_write = write(file_descr_out, buffer_out, n_out_symbols);
        if (n_write == -1)
            ASSERT_OK(errno)
        else if (n_write != n_out_symbols)
            ASSERT_OK(WRITE_SIZE) 

        // Preparing to continue
        n_already_read += n_in_symbols;
        int lseek_state = lseek(file_descr_in, n_already_read, SEEK_SET);
        if (lseek_state == -1)
            ASSERT_OK(errno)

        n_in_symbols  = 0;
        n_out_symbols = 0;

        if (memset(buffer_in,  0, BUFFER_SIZE) == 0)
            ASSERT_OK(MEMSET)

        if (memset(buffer_out, 0, BUFFER_SIZE) == 0)
            ASSERT_OK(MEMSET)
    }

// Files closing

    close(file_descr_in);
    close(file_descr_out);
}

// Reads the number and prints it in a proper way with ptrs shifts
static void biz_handle_number(char *in_start, char *out_start, int *n_in_symbols, int *n_out_symbols)
{
    // Number reading
    char *num_start = in_start;
    char *num_end   = NULL;

    long number = strtol(num_start, &num_end, 0);
    if (errno == ERANGE)
        ASSERT_OK(ERANGE)

    long n_digits = num_end - num_start;

    // Number writing
    int n_write = 0;

    if (number % 15 == 0)
    {
        n_write = sprintf(out_start, "%s", BIZZ_BUZZ);
        if (n_write != strlen(BIZZ_BUZZ))
            ASSERT_OK(WRITE_SIZE)
    }
    else if (number % 3 == 0)
    {
        n_write = sprintf(out_start, "%s", BIZZ);
        if (n_write != strlen(BIZZ))
            ASSERT_OK(WRITE_SIZE)
    }
    else if (number % 5 == 0)
    {
        n_write = sprintf(out_start, "%s", BUZZ);
        if (n_write != strlen(BUZZ))
            ASSERT_OK(WRITE_SIZE)
    }
    else
    {
        strncpy(out_start, in_start, n_digits);
        n_write = n_digits;
    }

    *n_in_symbols  += n_digits;
    *n_out_symbols += n_write;
}