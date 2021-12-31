## test 1: function relocation
We've discussed about the major part about library mapping, now we will focus on the second part of loading:
Relocation.

The object file in ELF format produced by the linker does not know the real address of external functions 
and global variables. So it uses GOT and PLT to delay the calculation of address to load time (and later).

Let's make things simple: GOT and PLT entries are like some *puzzles*, leaving us with some blanks to fill.
The code produced by the linker are instructed to fetch real address of symbols from those blanks. 
Relocation is just a process of filling blanks. Easy one, right?

What makes this test easier is that you already *what to fill* in the blank. In this test, a function refers 
to a function that all of us is very familiar with: `printf`. While what you might have heard less is that 
`printf` is defined in `libc`, the C runtime library on your machine, which is also a ELF shared library like 
what you are trying to load.

Although different OS has different `libc` (and you can specify it if you want), the way we use them here is 
unified. **We directly ask `libc` for real address, instead of finding it on our own**. This is pretty much like
turning you textbook to last pages for standard solution when you are doing your homework. (BTW, I do it all the time)
I call it the *fake loading* of `libc`.

---
*More on fake loading:* The reason why we use fake loading is because it is **very hard** to handle `glibc` ourselves, 
and it is tightly coupled with the system dynamic loader `ld-linux.so.2`. 

You can find more info on this at: https://sourceware.org/pipermail/libc-help/2021-January/005615.html

What will happen if you *have to map it*: Mainly because some of the global variables are not initialized, when you call
some of the functions, they will segfault, while some of them will **function normally**, making it hard to debug.

---
The way to achieve fake loading is to use `dlsym`:
```c
void *handle = dlopen("libc.so.6", RTLD_LAZY);
void *address = dlsym(handle, "symbol_name");
```
You can find more details on the usage of `dlsym` both on textbook and `dlsym` man page. It's quite straightforward.  

So there is actually only one question for we to answer in this test: where are those blanks, or *relocation entries*?

---
Again, I will explain the details in an example. First we will see how does a relocation entry look like:
```bash
$ readelf --relocs ./test_lib/lib1.so

# some entries excluded for brevity
...

Relocation section '.rela.plt' at offset 0x510 contains 1 entry:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000004018  000200000007 R_X86_64_JUMP_SLO 0000000000000000 puts@GLIBC_2.2.5 + 0
```
(You can find that I left out those entries in `.rela.dyn` section. We will get to that later.)

Both the textbook and <elf.h> has the defintion of an entry in C:
```C
typedef struct
{
  Elf64_Addr	r_offset;		/* Address */
  Elf64_Xword	r_info;			/* Relocation type and symbol index */
  Elf64_Sxword	r_addend;		/* Addend */
} Elf64_Rela;
// 'a' in Rela means 'addend'. Today most OS use 'Rela' instead of 'Rel'.
```
Though you can find a better and more comprehensive discussion at:
http://bottomupcs.sourceforge.net/csbu/x3735.htm ,
I will give a TL;DR version like test 0.

`r_offset` means the offset where the address should fill, and `r_info` is a multiplex of two fields:
the type of a relocation and symbol index, which use lower and upper 32 bit of a `uint64_t` respectively.
`r_addend` is something to be added to the address found when trying to write this entry.

---
*More on `r_info`*: For example, `r_info` of the relocation entry `puts` is `0x000200000007`, which is actually
`(2 << 32) | 7`. Its relocation type is `R_X86_64_JUMP_SLOT`, defined as `7` that can be found in <elf.h>.
`2` means its index is 2 in symbol table.


```bash
$ objdump -s ./test_lib/lib1.so
Contents of section .dynsym:
 0318 00000000 00000000 00000000 00000000  ................
 0328 00000000 00000000 10000000 20000000  ............ ...
 0338 00000000 00000000 00000000 00000000  ................
 0348 **59000000 12000000 00000000 00000000  Y...............
 0358 00000000 00000000** 01000000 20000000  ............ ...
 ...
```
The size of `Elf64_Sym` is 24 bytes, so index 2 in symbol table starts from `0x348`. The first 4 bytes 
stand for offset in string table, which is `0x59` (NB: little endian), *i.e.* `89`.
```bash
$ objdump -s ./test_lib/lib1.so
Contents of section .dynstr:
 03c0 005f5f67 6d6f6e5f 73746172 745f5f00  .__gmon_start__.
 03d0 5f49544d 5f646572 65676973 74657254  _ITM_deregisterT
 03e0 4d436c6f 6e655461 626c6500 5f49544d  MCloneTable._ITM
 03f0 5f726567 69737465 72544d43 6c6f6e65  _registerTMClone
 0400 5461626c 65005f5f 6378615f 66696e61  Table.__cxa_fina
 0410 6c697a65 00666f6f 00707574 73006c69  lize.foo.puts.li
 0420 62632e73 6f2e3600 474c4942 435f322e  bc.so.6.GLIBC_2.
 0430 322e3500 
```
`0x3c0` and `0x59` is `0x419`, and volia, there lies the string `puts`!

---
You can find this relocation table inside the library's dynamic section, 
the `DT_JMPREL` entry for its address and `DT_PLTRELSZ` for the number of PLT relocation.
```bash
$ readelf -d ./test_lib/lib1.so
 ...
 0x0000000000000002 (PLTRELSZ)           24 (bytes)
 0x0000000000000014 (PLTREL)             RELA
 0x0000000000000017 (JMPREL)             0x480
 ...
```
Here is an image on all the sections talked above, hopefully it can help you understand this process better.
![Some text](./img/plt-cut.png)

So again, the problem is broken into pieces: 

find relocation table (especially for function relocation) -> process each relocation entry
-> find the address of referred symbol (we already know) -> add the address with addend and fill it
