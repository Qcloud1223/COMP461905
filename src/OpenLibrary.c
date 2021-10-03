#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Loader.h"
#include "LoaderInternal.h"

void *OpenLibrary(const char *name, int mode)
{
    LinkMap *new = MapLibrary(name);

    RelocLibrary(new, mode);

    InitLibrary(new);

    return new;
}
