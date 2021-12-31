## Introduction

Greetings! This guide and those under `testcase/` will show you the basics of this project, 
and how to finish it:)

### Goals
As you may learn from CS:APP Chapter 7, this project is a minimal simulation of the vital part of a operating system, a dynamic
loader, which load shared objects(libraries) into process memory. 

Unlike the fully-functional loader, here we mainly focus on loading libraries on demand, *i.e.*, like `dlopen`.

This task can be simply broken up into three orthogonal parts: 
- Locate and map the library needed, as well as its dependencies
- Relocate the symbols for all the libraries loaded 
- Call the constructors of those libraries

You can find out the code doing these things at `src/OpenLibrary.c`, and the structure is pretty clear.

### Getting Started
Make sure to meet the following prerequisites:

- A Linux environment. This could be a VM, a WSL, a server from your mentor... It doesn't matter.
- 64-bit OS is installed. You can easily verify it by running `file ./test_lib/lib1.so` from the 
root directory, and the result should contain *x86_64*.
- Python >= 3.5 is installed.
- (optional) Use Ubuntu 16.04/18.04/20.04. I've done several tests on those systems, using any
of them will make it easy for me to provide debug suggestions.

<!-- Suppress it for now because I don't know how to briefly talk about these tests -->
<!-- ### Test cases
There are currently 7 test cases covered in autograder, and you may want to get back here if you
are at a loss about what to do. Tests covered in this part are marked with their topic: 
Map the library(M), Relocate symbols(R) and Call constructors(C)

0. (M)  -->

### Useful information
I hope this instruction and the course slides help you get (at least part of) familar with dynamic linking.
If you are still somehow confused, or just want to learn more, I would recommend you reading the 
following materials:

- [A rebuild of dynamic linker in one 300-line C file](https://github.com/jserv/min-dl). This implementation
is very short but will help you a lot in having a big picture of what we will do. However, it requires the
shared libraries to use some user-defined section(*i.e.* `.my_pltgot`), as well as not handling the dependency
tree and initialization. These flaws make it not practial, and even not a **linker**.

- [The source code of `glibc`](https://ftp.gnu.org/gnu/glibc/). Go download a copy of `glibc` source code, and
see how the dynamic linker running many Linux distributions works...just kidding. Simpling digging into 
`glibc`'s source code is an extremely painful experience and I highly recommend you not to do that if you 
just want to finish this project, for there are tons of historical and compatibility issues.
This is only for those who want to make things crystal clear. Go to `dlfcn/dlopen.c` to see why I said 
`dlopen` is only a wrapper, and check `elf/dl-load.c`, `elf/dl-reloc.c`, `elf/dl-init.c` and more.

- [UCI 238P HW3](https://www.ics.uci.edu/~aburtsev/238P/hw/hw3-elf/hw3-elf.html). This project is originally
inspired by this one, and you can find many useful information like ELF format and (a little bit of) relocation.
Note that this project focus on loading an **ELF executable** into arbitrary location of process memory, while 
our project focus on **ELF shared library**. The former is handled by the kernel by default, and users would 
not do that a lot. Being able to identify the differences between the two projects means that you understand 
the toolchain well.
