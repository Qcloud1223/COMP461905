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
# print each segment of a library
readelf -l ./test_lib/SimpleData.so
# print each section of a library
readelf -S ./test_lib/SimpleData.so
# print the dynamic section of a library
readelf -d ./test_lib/SimpleData.so
```

