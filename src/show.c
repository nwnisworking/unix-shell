#include <stdio.h>

int mains(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        printf("Argument %d: %s\n", i, argv[i]);
    }
    return 0;
}
