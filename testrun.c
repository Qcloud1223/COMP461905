#include <stdio.h>
#include <stdlib.h>
#include "src/Loader.h"

int main()
{
    void *handle = OpenLibrary("./test_lib/SimpleMul.so", BIND_NOW);
    int (*mul)(int a, int b);
    mul = FindSymbol(handle, "multiply");

    printf("test result: %d\n", mul(2, 3));

    return 0;
}