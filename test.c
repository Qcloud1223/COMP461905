// compare the output of both loader, if there is one
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "test.h"
#include "src/Loader.h"

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

#ifdef USE_DLOPEN
    void *handle = dlopen(argv[1], RTLD_LAZY);
#else
    void *handle = OpenLibrary(argv[1], BIND_NOW);
#endif    
    if(!handle)
    {
#ifdef USE_DLOPEN
        fprintf(stderr, "error in opening file: %s, reason: %s\n", argv[1], dlerror());
#else
        fprintf(stderr, "error in OpenLibrary with file: %s\n", argv[1]);
#endif
        exit(-1);
    }

    int func_type = atoi(argv[3]);
    switch (func_type)
    {
        case VOID_CHARP:
        {
#ifdef USE_DLOPEN
            void (*f)(char *) = dlsym(handle, argv[2]);
#else
            void (*f)(char *) = FindSymbol(handle, argv[2]);
#endif
            if(!f)
            {
#ifdef USE_DLOPEN
                fprintf(stderr, "error in finding symbol:%s, reason: %s\n", argv[2], dlerror());
#else
                fprintf(stderr, "error in FindSymbol: %s\n", argv[2]);
#endif
                exit(-1);
            }
            char dlBuffer[BUFFER_LEN];
            f(dlBuffer);
            printf("%s", dlBuffer);
            break;
        } 
            
        case INT_INTINT:
        {
#ifdef USE_DLOPEN
            int (*f)(int, int) = dlsym(handle, argv[2]);
#else
            int (*f)(int, int) = FindSymbol(handle, argv[2]);
#endif
            if(!f)
            {
#ifdef USE_DLOPEN
                fprintf(stderr, "error in finding symbol:%s, reason: %s\n", argv[2], dlerror());
#else
                fprintf(stderr, "error in FindSymbol: %s\n", argv[2]);
#endif
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
