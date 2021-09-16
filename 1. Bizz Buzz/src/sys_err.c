#include "../include/sys_err.h"
#include "../include/ename.h"

static const int ERR_BUFF_SIZE = 0x1000;                                              

static void output_error(Boolean use_err, int err, Boolean flush_stdout, const char *format, va_list ap)
{
    char buf[ERR_BUFF_SIZE];
    char user_msg[ERR_BUFF_SIZE];
    char err_text[ERR_BUFF_SIZE];

    vsnprintf(user_msg, ERR_BUFF_SIZE, format, ap);

    if (use_err)
        snprintf(err_text, ERR_BUFF_SIZE, " [%s: %s]", (err > 0 && err <= MAX_ENAME) ? ename[err] : "?UNKNOWN?", strerror(err));
    else
        snprintf(err_text, ERR_BUFF_SIZE, ":");

    snprintf(buf, ERR_BUFF_SIZE, "ERROR%s %s\n", err_text, user_msg);

    if (flush_stdout)
        fflush(stdout); // Flush any pending stdout

    fputs(buf, stderr);
    fflush(stderr);     // In case stderr is not line-buffered
}

void error_msg(const char *format, ...)
{
    va_list arg_list;

    int saved_errno = errno; // In case we change it here

    va_start(arg_list, format);
    output_error(TRUE, errno, TRUE, format, arg_list);
    va_end(arg_list);

    errno = saved_errno;
}