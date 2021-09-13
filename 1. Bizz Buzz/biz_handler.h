#ifndef BIZ_HANDLER_H
#define BIZ_HANDLER_H

/*!-----------------------------------------------------------------------------
Creates bizz-buzz file

    @version 1.3

    @authors Vokerlee

    @param[in] argc Default argc from main function
    @param[in] argv Default argv from main function

    @brief argv[1] is the input file, argv[2] is the output file. The function creates 
    outfile based on the input, where all numbers {x: INT_MIN <= x <= INT_MAX} are replaced 
    by "bizz" in case of division by 3, "buzz" - 5 and "bizz_buzz" - 15.
*///----------------------------------------------------------------------------

void biz_strings(int argc, char *argv[]);


/*!-----------------------------------------------------------------------------
Handles number

    @version 1.1

    @authors Vokerlee

    @param[in] in_start      Pointer to the word, where the number is located
    @param[in] out_start     Pointer to memory, where the proper word will be printed
    @param[in] n_in_symbols  Pointer to number, that should be increased by read symbols
    @param[in] n_out_symbols Pointer to number, that should be increased by written symbols

    @brief Reads number from pointer in_start and prints the result in out_start
*///----------------------------------------------------------------------------

static void biz_handle_number(char *in_start, char *out_start, int *n_in_symbols, int *n_out_symbols);

#endif // BIZ_HANDLER_H