#include "biz_handler.h"

const long BIZZ_NMB      = 3;
const long BUZZ_NMB      = 5;
const long BIZZ_BUZZ_NMB = 15;

const char BIZZ[]        = "bizz";
const char BUZZ[]        = "buzz";
const char BIZZ_BUZZ[]   = "bizz_buzz";

const unsigned long BUFFER_SIZE = 0x1000;

void biz_strings(int argc, char *argv[])
{
    assert(argv);

// Files opening

    errno = 0;
    int file_descr_in = open(argv[1], O_RDONLY);
    ERR_CHECK(file_descr_in < 0, errno)

    errno = 0;
    int file_descr_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IRWXU);
    ERR_CHECK(file_descr_out < 0, errno)

// Strings changes

    char buffer_in [BUFFER_SIZE];
    char buffer_out[BUFFER_SIZE];

    ERR_CHECK(memset(buffer_in,  0, BUFFER_SIZE) == 0, MEMSET)
    ERR_CHECK(memset(buffer_out, 0, BUFFER_SIZE) == 0, MEMSET)

    int read_state    = 1;

    int n_in_symbols  = 0;
    int n_out_symbols = 0;

    off_t n_already_read = 0;

    // Buffer analysis cycle
    while (read_state)
    {
        // Reading to in-buffer
        errno = 0;
        int n_read = read(file_descr_in, buffer_in, BUFFER_SIZE);
        ERR_CHECK(n_read == -1, errno)
        if (n_read != BUFFER_SIZE)
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

            //printf("%d %d\n", n_in_symbols, n_out_symbols);

            if (n_in_symbols < n_read_max &&  n_out_symbols < n_write_max)
                biz_handle_number(&buffer_in[n_in_symbols], &buffer_out[n_out_symbols], &n_in_symbols, &n_out_symbols);

            //printf("%d %d %d %d\n", n_in_symbols, n_out_symbols, n_read_max, n_write_max);
        }

        // Writing out-buffer to file
        int n_write = write(file_descr_out, buffer_out, n_out_symbols);
        ERR_CHECK(n_write == -1, errno)
        ERR_CHECK(n_write != n_out_symbols, WRITE_SIZE)

        // Preparing to continue
        n_already_read += n_in_symbols;
        off_t lseek_state = lseek(file_descr_in, n_already_read, SEEK_SET);
        ERR_CHECK(lseek_state == -1, errno)

        n_in_symbols  = 0;
        n_out_symbols = 0;

        ERR_CHECK(memset(buffer_in,  0, BUFFER_SIZE) == 0, MEMSET)
        ERR_CHECK(memset(buffer_out, 0, BUFFER_SIZE) == 0, MEMSET)
    }

// Files closing
    int close_state = 0;

    close_state = close(file_descr_in);
    ERR_CHECK(close_state == -1, errno)

    close_state = close(file_descr_out);
    ERR_CHECK(close_state == -1, errno)
}

// Reads the number and prints it in a proper way with ptrs shifts
static void biz_handle_number(char *in_start, char *out_start, int *n_in_symbols, int *n_out_symbols)
{
    // Number reading
    char *num_start = in_start;
    char *num_end   = NULL;

    long number = strtol(num_start, &num_end, 0);
    ERR_CHECK(errno == ERANGE, errno)

    long n_digits = num_end - num_start;

    // Number writing
    int n_write = 0;

    if (number % BIZZ_BUZZ_NMB == 0)
    {
        n_write = sprintf(out_start, "%s", BIZZ_BUZZ);
        ERR_CHECK(n_write != strlen(BIZZ_BUZZ), WRITE_SIZE)
    }
    else if (number % BIZZ_NMB == 0)
    {
        n_write = sprintf(out_start, "%s", BIZZ);
        ERR_CHECK(n_write != strlen(BIZZ), WRITE_SIZE)
    }
    else if (number % BUZZ_NMB == 0)
    {
        n_write = sprintf(out_start, "%s", BUZZ);
        ERR_CHECK(n_write != strlen(BUZZ), WRITE_SIZE)
    }
    else
    {
        strncpy(out_start, in_start, n_digits);
        n_write = n_digits;
    }

    *n_in_symbols  += n_digits;
    *n_out_symbols += n_write;
}