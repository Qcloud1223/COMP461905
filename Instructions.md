## Step-by-step instruction

Greetings! This guide will show you the basics of this project, and how to finish it:)
<!-- TODO: more smooth intro -->
### Goals
As you may learn from CS:APP Chapter 7, this project is a minimal simulation of the vital part of a operating system, a dynamic
loader, which load shared objects(libraries) into process memory. 

Unlike the fully-functional loader, here we mainly focus on loading libraries on demand, *i.e.*, like `dlopen`.

This task can be simply broken up into three orthogonal parts: 
- Locate and load the library needed, as well as its dependencies
- Relocate the symbols for all the libraries loaded 
- Call the constructors of those libraries

You can find out the code doing these things at `src/OpenLibrary.c`, and the structure is pretty clear.

### Getting Started
First of all, you need a Linux environment to finish this project.
This could be a WSL(Windows Subsystem for Linux), a virtual machine, or a desktop/server version of any Linux distribution.
No further libraries are needed.

However, due to the various implementations of linker on those distributions, the shared libraries produced by them will 
have minor differences. This will **NOT** cause problems when you trying to finish this project, but **will** cause me 
some problems when verifying your submissions, because your code is only tested under your environment.

Thus, it is highly recommended to use Ubuntu 18.04 or 20.04, where you can have the exact environment with me. Yet you can
use a different distribution if you really want that, and in that case please send me your setup.

If you never use Linux before, that's OK. You can always use a virtual machine.
<!-- TODO: add installing Linux, a link or another tutorial -->

### Background
On Linux, dynamic libraries are compiled into ELF(Executable and Linkable) format. You can find out how it is organized by 
simple Googling. 

<!-- 'functional' isn't precise enough -->
Basically, its minimum functional unit is call *section*, which starts with a '.', like '.text' and '.data'.
Each section has their own functionality, for example, '.text' contains the code of this library and '.data' is for global data.
You can find more about ELF at: https://www.ics.uci.edu/~aburtsev/238P/hw/hw3-elf/hw3-elf.html

Several sections together are recongnized by the loader, called *segment*. It is the loader's job to process every segment of 
a library and decide how to deal with them. For example, the loader needs to load a segment whose flag is `PT_LOAD` into memory.

There are some useful commands you can use to inspect a library:
```bash
# need to install binutils first
# print each program header of a library
$ readelf -l ./test_lib/SimpleData.so
# print each section header of a library
$ readelf -S ./test_lib/SimpleData.so
# print the dynamic section of a library
$ readelf -d ./test_lib/SimpleData.so
```

If you don't understand how things go, that's OK. Next we will go through each testcase, and show you what you need to do,
with more details of ELF format.

### Testcases

#### test0: load a library
This test only requires you to map a shared library into memory(the location does not matter).
And it is pretty obvious we need to know that:
- Where can I find the file
- Which part of the file needs loading

For the first question, all the testcases come with their absolute path, like `./test_lib/SimpleMul.so`, 
instead of `SimpleMul.so`. If it is the latter case, the loader will need to search for a bunch of directories
to find out where the library is. We will see more about this in the next testcase.

Therefore, find and access this file in your filesystem is quite straightforward now. Just simple C-style file operations.

The second question is a little complicated, and we will first see the segments of this library:
```bash
$ readelf -l ./test_lib/SimpleMul.so

...
# The number of LOAD segment varies under different linkers
Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x00000000000004b0 0x00000000000004b0  R      0x1000
  LOAD           0x0000000000001000 0x0000000000001000 0x0000000000001000
                 0x000000000000011d 0x000000000000011d  R E    0x1000
  LOAD           0x0000000000002000 0x0000000000002000 0x0000000000002000
                 0x00000000000000a4 0x00000000000000a4  R      0x1000
  LOAD           0x0000000000002e80 0x0000000000003e80 0x0000000000003e80
                 0x00000000000001a0 0x00000000000001a8  RW     0x1000

...
```
Remember that we need to load the `PT_LOAD` segments? The segment with type `LOAD` is exactly the case.
Then, `readelf` gives us some other useful information:

`Offset`: the offset in number of bytes of this segment in the file
`VirtAddr`: where this segment should start in virtual memory
`PhysAddr`: ignored
`FileSiz`: the size of the segment inside filesystem
`MemSiz`: the size of the segment when loaded into memory
`Flags`: permission of this segment
`Align`: alignment requirement in virtual memory

The header <elf.h> shows us where we can find these information in our C program:
```c
typedef struct
{
  Elf64_Word	p_type;			/* Segment type */
  Elf64_Word	p_flags;		/* Segment flags */
  Elf64_Off	    p_offset;		/* Segment file offset */
  Elf64_Addr	p_vaddr;		/* Segment virtual address */
  Elf64_Addr	p_paddr;		/* Segment physical address */
  Elf64_Xword	p_filesz;		/* Segment size in file */
  Elf64_Xword	p_memsz;		/* Segment size in memory */
  Elf64_Xword	p_align;		/* Segment alignment */
} Elf64_Phdr;
// Phdr is short for "program header"
```
And the file header of this library tells us where we can traverse these segments:
```c
typedef struct
{
  unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
  Elf64_Half	e_type;			/* Object file type */
  Elf64_Half	e_machine;		/* Architecture */
  Elf64_Word	e_version;		/* Object file version */
  Elf64_Addr	e_entry;		/* Entry point virtual address */
  **Elf64_Off  e_phoff;**		/* Program header table file offset */
  Elf64_Off	    e_shoff;		/* Section header table file offset */
  Elf64_Word	e_flags;		/* Processor-specific flags */
  Elf64_Half	e_ehsize;		/* ELF header size in bytes */
  **Elf64_Half	e_phentsize;**		/* Program header table entry size */
  **Elf64_Half	e_phnum;**		/* Program header table entry count */
  ...
} Elf64_Ehdr;
// Ehdr is short for "ELF header"
```
So, to pinpoint the segments, we need to find **program header table**,
which stores the information we see in `readelf -l`.
So called **program header** is actually an abstract of a segment, containing
information shown in the fields of `Elf64_Phdr`.
 
First, we need to find out the *ELF header*, or `Elf64_Ehdr` above.
It always starts at offset 0 of an ELF file, so use `read()`, `fread()`, or `pread()`
to read `sizeof(Elf64_Ehdr)` number of bytes can find that.

`e_phoff`, `e_phentsize`, and `e_phnum` denote the file offset of program header table,
the size of each entry in program header table, and the number of program headers in it.
Again, you can use the file operation you like to load the table into a buffer,
and then traverse it to get information of each segment.

To sum up, the working order would be:

`Elf64_Ehdr` -> `e_phoff` -> `Elf64_Phdr` -> `p_type`, `p_offset`, ...

Once we have found a segment, it's time we load it into memory.
`mmap()` is intended for this. It create a mapping from a file in the disk to somewhere in the memory,
and this is what is called "loading a file into memory" throughout this document.
**Use `mmap()` to create mappings for each segment**, and you can find more about mmap in this man page:
https://man7.org/linux/man-pages/man2/mmap.2.html

*More on `mmap()`*: here is a TL;DR version of man page.
A `mmap()` call takes 6 arguments, and now we take the first segment we see above(at line 82) as an example:

This segment starts from offset 0 in file, has an aligment requirement of `0x1000`, and needs `0x4b0` bytes of 
memory. Therefore, we need to call `mmap()` like this:
```c
// Elf64_Phdr *first_segment;
// int fd = open(path_to_library);
int prot = 0;
prot |= (first_segment->prot && PF_R)? PROT_READ : 0;
prot |= (first_segment->prot && PF_W)? PROT_WRITE : 0;
prot |= (first_segment->prot && PF_X)? PROT_EXEC : 0;
// NULL means "allow OS to pick up address for you"
void *start_addr = mmap(NULL, ALIGN_UP(first_segment->p_memsz, getpagesize()), prot, 
     MAP_FILE | MAP_PRIVATE, fd, first_segment->offset);
```
Few things you need know:

- `PF_R` is used to signify a *segment* has read permission,
while `PROT_READ` is used to show a *virtual memory page* has read permission.
We need to convert that when calling `mmap()`.

- `mmap()` has alignment requirement, expecting the beginning and end of this call 
exactly on a page boundary(you should understand this after learing virtual memory).
The macros `ALIGN_UP` and `ALIGN_DOWN` in `src/MapLibrary.c` are prepared for that,
and you may want to double check the first two arguments.

- Due to the *position-independent* feature, code in shared library often use PC-relative
address to access a function/variable. This implicitly demands segments to be mapped at
the address according to the program header table(remember VirtAddr?). 
Though the address of the first mappingdoes not matter(like `NULL` above), 
the following mappings probably need fixed address.

Alright, the memory mappings are now ready to go. 
Now, to make `FindSymbol` able to find a function provided by this library, 
we need to store the location of **symbol table** and **string table**.

A symbol table contains the information of all the symbols(a function or global variable), and the 
string table contains the name corresponds to each symbol. The definition can also be found in 
<elf.h>:
```c
typedef struct
{
  Elf64_Word	st_name;		/* Symbol name (string tbl index) */
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char st_other;		/* Symbol visibility */
  Elf64_Section	st_shndx;		/* Section index */
  Elf64_Addr	st_value;		/* Symbol value */
  Elf64_Xword	st_size;		/* Symbol size */
} Elf64_Sym;
```
You need not fully understand how a symbol works because symbol searching is handled for you.

Let's check the dynamic section of this library:
```bash
readelf -d ./test_lib/SimpleMul.so

  Tag        Type                         Name/Value
 ...
 0x0000000000000005 (STRTAB)             0x3a8
 0x0000000000000006 (SYMTAB)             0x318
 ...
```
This gives the address of string table and symbol table in virtual memory. 
However, it assumes that the library are mapped into memory starting from `0x0`, 
which can never be true because it is protected by kernel.

Thus, we need to find the correct base address of the library.
Assume that address is `0x555555000000`, and then the symbol table will be at `0x555555000318`.

Getting back to what you need to implement: 
The internal data structure `LinkMap` in `src/Link.h` is designed to share info among all modules,
like `addr` indicating the base address, and `dynInfo` containing every entry inside dynamic section,
including location of symbol table(`dynInfo[DT_SYMTAB]`) and string table(`dynInfo[DT_STRTAB]`).

Find the correct base address returned by `mmap()`, store it in `addr`.
Calculate the real address of dynamic segment with that base address, store it in `dyn`.
And the helper functions in `src/MapLibrary.c` will do the job left.

Congrats! You've finished 80% of this project!

<!-- For the first question, we can first look into `src/MapLibrary.c`:
```c
// system path may vary under different operating systems
static const char *sys_path[] = {
    "/usr/lib/x86_64-linux-gnu/",
    "/lib/x86_64-linux-gnu/",
    ""
};
```
The loader will typically configure some directories to search on default -->