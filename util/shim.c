#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h>

/* notify the header to not replace out implementation */
#define IN_SHIM

static int printf_usage;
static int fprintf_usage;

int myprintf(const char *__restrict__ __format, ...)
{
    va_list arg;
    va_start(arg, __format);
    vprintf(__format, arg);

    // printf("intercepted\n");
    printf_usage++;
}

int myfprintf(FILE *__restrict__ __stream, const char *__restrict__ __format, ...)
{
    va_list arg;
    va_start(arg, __format);
    vfprintf(__stream, __format, arg);

    // printf("intercepted\n");
    fprintf_usage++;
}

/* C macro will always parse comma into argument separator,
   in which case, the previous expanded tokens can never included comma.
   In other words, we cannot expect a token that contains comma, e.g. "hello,world"
   which would always be interpreted as two tokens in preprocessor.
   That is to say, I cannot find a way to generate comma separated argument list using
   multiple layers of nested macros, as shown below.
   If the level of indirect call level calls ever reach 2, the macro will not correctly parse
   the number of arguments.
   
   The idea behind such failed attempt, however, is pretty useful though.
   One can use __VA_ARGS__ counter to jump to an macro entry, and get expanded all the way to the bottom.
   For example, if MERGE_TOKEN is called with 6 arguments, the expected behavior should be that
   first performing incremental task, then extracting __VA_ARGS__, and finally call JOIN_TOKEN4.
   This is the C macro way of traversing variable length macro.  */

// #define FUNCTION_MACRO(func, ret, type1, arg1, type2, arg2, type3, arg3, type4, arg4)
#define VARCOUNT_I(_, _10, _9, _8, _7, _6, _5, _4, _3, _2, X, ...) X
#define VARCOUNT(...) VARCOUNT_I(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1) 
#define CAT1(x, y) x##y
#define CAT(x, y) CAT1(x, y)
// #define JOIN_TOKEN(_, a, b, ...) _, a b
// #define JOIN_TOKEN(...) __VA_ARGS__
// #define JOIN_TOKEN2(_, a, b, ...) JOIN_TOKEN(_ a b COMMA, __VA_ARGS__)
#define JOIN_TOKEN2(a, b) a b
#define SELECT_TOKEN2(a, b) b
#define JOIN_TOKEN4(a, b, c, d) JOIN_TOKEN2(a, b) , JOIN_TOKEN2(c, d)
#define SELECT_TOKEN4(a, b, c, d) SELECT_TOKEN2(a, b) , SELECT_TOKEN2(c, d)
#define JOIN_TOKEN6(a, b, c, d, e, f) JOIN_TOKEN4(a, b, c, d) , JOIN_TOKEN2(e, f)
#define SELECT_TOKEN6(a, b, c, d, e, f) SELECT_TOKEN4(a, b, c, d) , SELECT_TOKEN2(e, f)
#define JOIN_TOKEN8(a, b, c, d, e, f, g, h) JOIN_TOKEN4(a, b, c, d) , JOIN_TOKEN4(e, f, g, h)
#define SELECT_TOKEN8(a, b, c, d, e, f, g, h) SELECT_TOKEN4(a, b, c, d) , SELECT_TOKEN4(e, f, g, h)
#define JOIN_TOKEN10(a, b, c, d, e, f, g, h, i, j) JOIN_TOKEN8(a, b, c, d, e, f, g, h) , JOIN_TOKEN2(i, j)
#define SELECT_TOKEN10(a, b, c, d, e, f, g, h, i, j) SELECT_TOKEN8(a, b, c, d, e, f, g, h) , SELECT_TOKEN2(i, j)
#define MERGE_TOKEN(...) CAT(JOIN_TOKEN, VARCOUNT(__VA_ARGS__)) (__VA_ARGS__)
#define KEEP_NAME(...) CAT(SELECT_TOKEN, VARCOUNT(__VA_ARGS__)) (__VA_ARGS__)

#define FUNCTION_MACRO(func, ret, ...) \
static int func##_usage; \
ret my##func(MERGE_TOKEN(__VA_ARGS__)) \
{ \
    ret retval = func(KEEP_NAME(__VA_ARGS__)); \
    func##_usage++; \
    return retval; \
}

#define PRINT_FUNC_USAGE(func) \
fprintf(stderr, #func ": %d\n", func##_usage)

FUNCTION_MACRO(dlopen, void *, const char *, name, int, mode);
// FUNCTION_MACRO(dlsym, void *, void *, handle, const char *, name);
FUNCTION_MACRO(fwrite, size_t, const void *, ptr, size_t, size, size_t, nmemb, FILE *, stream);

// escape dlsym because I don't want the failed searches to generate an unreadable result
static int dlsym_usage;
void *mydlsym(void *__restrict __handle, const char *__restrict __name)
{
    void *sym = dlsym(__handle, __name);
    if (sym)    
        dlsym_usage++;
    return sym;
}

__attribute__((destructor))
void shim_summary()
{
    // printf("program exits, retrieving shim statistics\n");
    PRINT_FUNC_USAGE(printf);
    PRINT_FUNC_USAGE(fprintf);
    PRINT_FUNC_USAGE(dlopen);
    PRINT_FUNC_USAGE(dlsym);
    PRINT_FUNC_USAGE(fwrite);
}

/* Intercept mmap not for monitoring usage. 
   Instead, most of the students are not aware of where they have mmap'd, 
   which might lead to less understandable results */
void *mymmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset)
{
    #ifdef LOADER_DEBUG
    char perm[5] = "----";
    if (prot & PROT_READ)
        perm[0] = 'r';
    if (prot & PROT_WRITE)
        perm[1] = 'w';
    if (prot & PROT_EXEC)
        perm[2] = 'x';
    
    char f[64] = "";
    if (flags & MAP_PRIVATE)
        strcat(f, "MAP_PRIVATE");
    if (flags & MAP_FIXED)
        strcat(f, " MAP_FIXED");
    fprintf(stderr, "trying to mmap at: %p, length:%lu, protection: %s, flags: %s, fd: %d, offset: %ld\n",
        addr, length, perm, f, fd, offset);
    #endif
    void *ret = mmap(addr, length, prot, flags, fd, offset);
    /* TODO: check return value */
    return ret;
}