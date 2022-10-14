/* Helper library. DO NOT MODIFY */
#define __GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdarg.h>

#include "shim.h"

static void *libc_handle;
static int printf_usage;

/* A note that is not related to this project:
   The aim of shim library is to prevent the easy usage of printf and dl-family functions
   such that students can easily bypass the certain tests, which is not the best thing for me to see.
   During debugging, fprintf is used to see if it is working expectedly,
   but after I finish all shim functions, fprintf will also be interposed and thus failing the tests. */

__attribute__((constructor))
static void shim_private_init()
{
    // fprintf(stdout, "initializing shim library\n");
    // libc_handle = dlopen("libc.so.6", RTLD_LAZY);
    // if (!libc_handle) {
    //     fprintf(stderr, "load shim library failed because: %s\n", dlerror());
    //     exit(-1);
    // }
}

int printf(const char *__restrict__ __format, ...)
{   
    va_list args;
    va_start(args, __format);

    int (*real_printf)(const char *__restrict__, va_list) = dlsym(RTLD_NEXT, "vprintf");
    if (!real_printf) {
        fprintf(stderr, "shim library cannot find the address of printf\n");
        exit(-1);
    }

    real_printf(__format, args);
    fprintf(stdout, "current printf usage: %d\n", printf_usage);
    printf_usage++;
}

// void *dlopen(const char *name, int mode)
// {
//     void (*real_dlopen)(const char *, int) = dlsym(libc_handle, "dlopen");
// }