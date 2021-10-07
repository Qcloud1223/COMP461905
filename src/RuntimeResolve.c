#include <elf.h>
#include <stdlib.h>
#include <stdio.h>

#include "Link.h"
#include "LoaderInternal.h"

Elf64_Addr __attribute__((visibility ("hidden"))) //this makes trampoline to call it w/o plt
runtimeResolve(LinkMap *lib, Elf64_Word reloc_entry)
{
    printf("Resolving address for entry %u\n", reloc_entry);
    /* Your code here */
}