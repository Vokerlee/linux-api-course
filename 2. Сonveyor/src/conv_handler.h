#ifndef CONV_HANDLER_H_
#define CONV_HANDLER_H_

#include "conv_err.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct cmd_t
{
    int argc;
    char **argv;
};

struct cmds_t
{
    struct cmd_t *cmds;
    size_t n_cmds;
};

void conv_handler(const int fd_input);

void execute_cmd(const struct cmd_t command);

static void parse_cmds(char *buf, const size_t buf_size, struct cmds_t *commands);

static size_t get_cmds_amount(const char *buffer);

static char *skip_spaces(const char* buf);

static size_t get_cmds_argc(const char *cmd);

static char **get_cmds_argv(const char *cmd, const size_t argc);

static void execute_cmds(const struct cmds_t *commands);


#endif // !CONV_HANDLER_H_