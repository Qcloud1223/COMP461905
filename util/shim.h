#ifndef ELFLOADER_SHIM
#define ELFLOADER_SHIM

#include <stdlib.h>
#ifndef IN_SHIM

/* suppress warnings */
extern int myprintf(const char *__restrict__ __format, ...);
extern int myfprintf(FILE *__restrict__ __stream, const char *__restrict__ __format, ...);
extern void *mydlopen (const char *__file, int __mode);
extern void *mydlsym (void *__restrict __handle, const char *__restrict __name);
extern size_t myfwrite (const void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __s);

#define printf(format, ...) myprintf(format, ##__VA_ARGS__)
#define fprintf(stream, format, ...) myfprintf(stream, format, ##__VA_ARGS__)
#define dlopen(file, mode) mydlopen(file, mode)
#define dlsym(h, n) mydlsym(h, n)
#define fwrite(p, sz, n, s) myfwrite(p, sz, n, s)

#endif

#endif