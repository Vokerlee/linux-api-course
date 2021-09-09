#ifndef BIZ_HANDLER_H
#define BIZ_HANDLER_H

char *number_parsing(char *buffer, int *buffer_size);

void biz_strings(int argc, char *argv[]);

unsigned count_digits(long number);

char* str_replace(char* buffer, char* pos, const char* str, unsigned* buf_size, unsigned insert_sz, unsigned offset);

#endif // BIZ_HANDLER_H