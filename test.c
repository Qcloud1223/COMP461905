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
    void *handle;
    if (atoi(argv[3]) < (1 << 4))
        handle = OpenLibrary(argv[1], BIND_NOW);
    else
        handle = OpenLibrary(argv[1], LAZY_BIND);
#endif    
    if(!handle)
    {
#ifdef USE_DLOPEN
        fprintf(stderr, "error in opening file: %s, reason: %s\n", argv[1], dlerror());
#else
        fprintf(stderr, "OpenLibrary cannot open file: %s\n", argv[1]);
#endif
        // switch from exit to abort, indicating that this is an unwanted (but expected) exit
        // in this way destructor functions will not be called
        // exit(-1);
        abort();
    }

    int func_type = atoi(argv[3]);
    // there is no difference between lazy binding and non-lazy one in FindSymbol
    // so we shift it back for *_L == * << 4
    if (func_type >= (1 << 4))
        func_type >>= 4;
    switch (func_type)
    {
        case VOID_VOID:
        {
#ifdef USE_DLOPEN
            void (*f)(void) = dlsym(handle, argv[2]);
#else
            void (*f)(void) = FindSymbol(handle, argv[2]);
#endif
            if(!f)
            {
#ifdef USE_DLOPEN
                fprintf(stderr, "error in finding symbol:%s, reason: %s\n", argv[2], dlerror());
#else
                fprintf(stderr, "FindSymbol cannot find symbol: %s\n", argv[2]);
#endif
                // exit(-1);
                abort();
            }
            f();
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
                fprintf(stderr, "FindSymbol cannot find symbol: %s\n", argv[2]);
#endif
                // exit(-1);
                abort();
            }
            int op1 = atoi(argv[4]);
            int op2 = atoi(argv[5]);
            printf("%d\n", f(op1, op2));
            break;
        }
            
    }

    return 0;
}
