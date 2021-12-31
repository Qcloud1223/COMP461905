## test 3: finding dependencies on your own
This test is a step further than test 2. 
Remember the analogy we use? This time, you are not allowed to refer to the answer.
Instead, you have to handle the dependencies yourself, and find the symbol in the relocation table
in other libraries' symbol table.

Good news is that I've done the last step for you. Check `findSymbol` function inside `src/RelocLibrary.c`, 
which takes a `LinkMap` and `const char *` as arguments. The job of this function is pretty clear:
find a symbol in a library previously loaded.

In this case you only have to decide which library to search, which is shown again in the dynamic section:
```bash
$ readelf -d ./test_lib/SimpleIni.so
 
 ...
 0x0000000000000001 (NEEDED)             Shared library: [SimpleMul.so]
 ...
```
<!-- TODO: check if 'among all object files' is arbitrary -->
When the linker find a symbol it does know the address among all the object files, 
it will generate a relocation entry, as well as searching for the shared library that defines this symbol.
Otherwise we will never know the address of that symbol.

---
This can be simply reproduced by the following code snippet:
```C
// main.c
#include <stdlib.h>
#include <dlfcn.h>

int main()
{
  void *handle = dlopen("libc.so.6", RTLD_LAZY);

  return 0;
}
```
Compile it with `gcc main.c`, and here goes the complains:
```
/usr/bin/ld: /tmp/ccDI4WTO.o: in function `main':
main.c:(.text+0x19): undefined reference to `dlopen'
collect2: error: ld returned 1 exit status
```
The linker(`ld`) is complaining about no definition is found on `dlopen`, because we don't specify `-ldl` 
when linking.

---
For each of the library the linker finds has dependency with the current file we are linking, it will generate a
`DT_NEEDED` entry in the dynamic section.
So we need to get back to phase 1, when we are mapping the library, we need to also map its dependencies as well.
Otherwise we cannot look up symbols in them.
Note that you can also deal with `libc` in test 2, and `fake` and `fakeHandle` in `LinkMap` is to indicate
a library we currently cannot use directly.

We need to find out where are these dependencies. A typical loader defines such thing as *default path* or *system path*
to unconditionally search in those directories, and users can specify the some paths themself during linking.
You can find an example of system path under Ubuntu system in `src/MapLibrary.c`. User specified path is not 
required in this project, and you can find out more in `DT_RUNPATH` entry in dynamic section if you are interested.
You can treat `./test_lib` or `/path/to/repo/test_lib` as a system path in this project
and solve all the problems.

Once you find all the dependency of a library, store it in `searchList` of `LinkMap`. This can be
used for later relocation.

OK, the task for you is clear: find and load the dependency of the library you are loading,
and finish the *real* relocation process by `findSymbol`.
