// compare the output of both loader, if there is one
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "test.h"

int main(int argc, char *argv[], char *env[])
{
    // argv0: program name
    // argv1: name of test file
    // argv2 and so on: the function we want to call, write to a buffer and compare it 
    //     so it always takes char* as its argument. It returns nothing for currently 
    //     I did not see the reason in doing so.
    if (argc < 3)
    {
        fprintf(stderr, "too few argument to call test program\n");
        exit(-1);
    }

    char dlBuffer[BUFFER_LEN];
    void *dlHandle = dlopen(argv[1], RTLD_LAZY);
    if(!dlHandle)
    {
        fprintf(stderr, "error in opening file: %s, reason: %s\n", argv[1], dlerror());
        exit(-1);
    }
    void (*f)(char *) = dlsym(dlHandle, argv[2]);
    if(!f)
    {
        fprintf(stderr, "error in finding symbol:%s, reason: %s\n", argv[2], dlerror());
        exit(-1);
    }
    f(dlBuffer);
    printf("%s", dlBuffer);
    return 0;
}
