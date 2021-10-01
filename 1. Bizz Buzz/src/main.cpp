#include <stdlib.h>
#include <stdio.h>

int main()
{
    FILE *io = fopen("testik", "wb");

    for (int i = 0; i < 50000000; i++)
    {
        char buffer[] = {"3243243 45 fgfgshdg bdfjvbhj s6d78 sg ygsydut 67tsy dgvygusdy tgysdgvy gstd6vt gysudgv yusdg yv\n\n\n\n\n\n\\t\t\trvbrve"};
        fwrite(buffer, sizeof(char), sizeof(buffer), io);
    }


    fclose(io);


    return 0;
}