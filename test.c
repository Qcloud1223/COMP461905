// compare the output of both loader, if there is one
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "test.h"

int main(int argc, char *argv[], char *env[])
{
    // argv0: program name
    // argv1: name of test file
    // argv2: name of the function
    // argv3: type of the function
    // argv4 and so on: argument for test function
    if (argc < 4)
    {
        fprintf(stderr, "too few argument to call test program\n");
        exit(-1);
    }

    void *dlHandle = dlopen(argv[1], RTLD_LAZY);
    if(!dlHandle)
    {
        fprintf(stderr, "error in opening file: %s, reason: %s\n", argv[1], dlerror());
        exit(-1);
    }

    int func_type = atoi(argv[3]);
    switch (func_type)
    {
        case VOID_CHARP:
        {
            void (*f)(char *) = dlsym(dlHandle, argv[2]);
            if(!f)
            {
                fprintf(stderr, "error in finding symbol:%s, reason: %s\n", argv[2], dlerror());
                exit(-1);
            }
            char dlBuffer[BUFFER_LEN];
            f(dlBuffer);
            printf("%s", dlBuffer);
            break;
        } 
            
        case INT_INTINT:
        {
            int (*f)(int, int) = dlsym(dlHandle,argv[2]);
            if(!f)
            {
                fprintf(stderr, "error in finding symbol:%s, reason: %s\n", argv[2], dlerror());
                exit(-1);
            }
            int op1 = atoi(argv[4]);
            int op2 = atoi(argv[5]);
            printf("%d\n", f(op1, op2));
            break;
        }
            
    }

    return 0;
}
