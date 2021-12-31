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