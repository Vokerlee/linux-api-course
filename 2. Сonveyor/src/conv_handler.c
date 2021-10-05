#include "conv_handler.h"

void conv_handler(const int fd_input)
{
    // BUFFER CREATING
    struct stat input_stat = {0};

    int stat_err = fstat(fd_input, &input_stat);
    ERR_CHECK(stat_err == -1, errno)

    size_t input_size = input_stat.st_size;

    char* buf = (char*) calloc(input_size + 1, sizeof(char));
    ERR_CHECK(buf == NULL, BAD_ALLOC)

    int n_read = read(fd_input, buf, input_size);
    ERR_CHECK(n_read == -1, errno)
    ERR_CHECK(n_read != input_size, READ_SIZE)

    // COMMANDS PARSING
    struct cmds_t commands = {0};

    size_t   n_cmds = get_cmds_amount(buf);
    commands.n_cmds = n_cmds;

    commands.cmds = (struct cmd_t *) calloc(n_cmds, sizeof(struct cmd_t));
    ERR_CHECK(commands.cmds == NULL, BAD_ALLOC)

    parse_cmds(buf, input_size, &commands);   

    // EXECUTING ALL COMMANDS
    execute_cmds(&commands);

    // GARBAGE COLLECTION
    free(commands.cmds);
    free(buf);
}

static void parse_cmds(char *buf, const size_t buf_size, struct cmds_t *commands)
{
    assert(buf);
    assert(commands);

    // INITIAL ACTIONS

    size_t cmd_count = 0;
    char* next = NULL;
    char* arg_ptr = skip_spaces(buf);

    next = strtok(arg_ptr, "|");
    next = strtok(NULL, "|");

    if (next)
    {
        commands->cmds[cmd_count].argc = get_cmds_argc(arg_ptr);
        commands->cmds[cmd_count].argv = get_cmds_argv(arg_ptr, commands->cmds[cmd_count].argc);

        arg_ptr = next;
        cmd_count++;

        while ((next = strtok(NULL, "|")) != NULL)
        {
            arg_ptr = skip_spaces(arg_ptr);

            commands->cmds[cmd_count].argc = get_cmds_argc(arg_ptr);
            commands->cmds[cmd_count].argv = get_cmds_argv(arg_ptr, commands->cmds[cmd_count].argc);

            arg_ptr = next;
            ++cmd_count;
        }

        arg_ptr = skip_spaces(arg_ptr);

        commands->cmds[cmd_count].argc = get_cmds_argc(arg_ptr);
        commands->cmds[cmd_count].argv = get_cmds_argv(arg_ptr, commands->cmds[cmd_count].argc);

        arg_ptr = next;
        cmd_count++;
    }
    else // in case there are no pipes
    {
        commands->cmds[cmd_count].argc = get_cmds_argc(arg_ptr);
        commands->cmds[cmd_count].argv = get_cmds_argv(arg_ptr, commands->cmds[cmd_count].argc);
    }
}

static size_t get_cmds_amount(const char *buffer)
{
    assert(buffer);

    size_t n_cmds = 1;

    char* next = NULL;
    const char* token_ptr = buffer;

    while ((next = strchr(token_ptr, '|')) != NULL)
    {
        n_cmds++;
        token_ptr = next;
        token_ptr++;
    }

    return n_cmds;
}

static char *skip_spaces(const char* buf)
{
    assert(buf);

    const char* ptr = buf;

    while (isspace(*ptr))
        ptr++;

    return ptr;
}

static size_t get_cmds_argc(const char *cmd)
{
    assert(cmd);

    size_t argc = 1;
    char*  next = NULL;
    const char* token_ptr = cmd;

    while ((next = strchr(token_ptr, ' ')) != NULL)
    {
        token_ptr = next;
        token_ptr = skip_spaces(token_ptr);
        if (*token_ptr == '\0')
            break;

        token_ptr++;
        argc++;
    }

    return argc;
}

static char **get_cmds_argv(const char *cmd, const size_t argc)
{
    assert(cmd);

    char** argv = (char**) calloc(argc + 1, sizeof(char*));
    ERR_CHECK(argv == NULL, BAD_ALLOC)

    argv[argc] = NULL;

    const char* arg_ptr = cmd;
    char* next = NULL;
    size_t i = 0;

    while ((next = strchr(arg_ptr, ' ')) != NULL)
    {
        argv[i] = arg_ptr;
        arg_ptr = next;
        *next = '\0';

        arg_ptr++;
        arg_ptr = skip_spaces(arg_ptr);
        
        i++;
    } 

    argv[i] = arg_ptr;

    if ((next = strchr(arg_ptr, '\n')) != NULL)
        *next = '\0';

    argv[argc] = NULL;

    return argv;
}

static void execute_cmds(const struct cmds_t *commands)
{
    assert(commands);

    // OPENING PIPES
    int *fd = (int *) calloc((commands->n_cmds - 1) * 2, sizeof(int));
    ERR_CHECK(fd == NULL, BAD_ALLOC)

    errno = 0;
    for (size_t i = 0; i < commands->n_cmds - 1; i++)
        ERR_CHECK(pipe(fd + 2 * i) < 0, errno) // 2 * (n_cmds - 1) descriptors, (n_cmds - 1) pipes

    // CREATING CHILDS
    int status = 0;
    pid_t pid  = 0;

    for (size_t i = 0; i < commands->n_cmds; i++)
    {
        errno = 0;
        pid = fork();
        ERR_CHECK(pid == -1, errno)

        if (pid == 0) // child
        {
            if (i != 0)
                ERR_CHECK(dup2(fd[2 * i - 2], STDIN_FILENO)  < 0, errno)
            if (i != commands->n_cmds - 1)
                ERR_CHECK(dup2(fd[2 * i + 1], STDOUT_FILENO) < 0, errno)    

            for (size_t j = 0; j < 2 * (commands->n_cmds - 1); j++)
            {
                errno == 0;
                int close_state = close(fd[j]);
                ERR_CHECK(close_state == -1, errno)
            }
               

            execute_cmd(commands->cmds[i]);
        }
    }

    // CLEANING MEMORY OF PARSING (for parent process)
    free(fd);

    size_t i = 0;
    for (i = 0; i < commands->n_cmds; i++)
        free(commands->cmds[i].argv);
}

void execute_cmd(const struct cmd_t command)
{
    errno = 0;
    execvp(command.argv[0], command.argv);
    ERR_CHECK(1, errno)
}
