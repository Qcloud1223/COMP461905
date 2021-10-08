// In fact, this is not a good way to test if initialization is called
// I hope to find a way to decouple each functionality, but sadly 
// currently I haven't thought of a way to achieve this w/o libc
// auditing functions could work, but that might mess the internals 
// of both linker
#include <stdio.h>
__attribute__((constructor))
static void init()
{
    printf("Init is called\n");
}

void entry()
{
    printf("Entry function is called\n");
}