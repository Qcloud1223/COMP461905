## test 2: initialization
Now we will discuss about the last step of library loading, initialization, or calling
constructors of the library.

Before we call that function, we need to find the real address of those constructors. This is a relocation
in another form: *relative relocation*.
<!-- TODO: maybe an appendix on all the relocation referred here -->

Getting back to what was ignored in the relocation table:
```bash
$ readelf -r ./test_lib/SimpleIni.so

Relocation section '.rela.dyn' at offset 0x468 contains 8 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000003e08  000000000008 R_X86_64_RELATIVE                    1110
000000003e10  000000000008 R_X86_64_RELATIVE                    1119
000000003e18  000000000008 R_X86_64_RELATIVE                    10d0
000000004020  000000000008 R_X86_64_RELATIVE                    4020
...
```
The action needed to fill these entries is easy. Because it is called relative relocation, 
**we only need to add the base address of library and the addend here.** 

You can notice that relative relocations sit in different section with function relocations.
(`.rela.dyn` vs `.rela.plt`) You can find its entry in the dynamic section, too:
```bash
$ readelf -d ./test_lib/SimpleIni.so
 ...
 0x0000000000000007 (RELA)               0x468
 0x0000000000000008 (RELASZ)             192 (bytes)
 0x0000000000000009 (RELAENT)            24 (bytes)
 ...
```
`DT_RELA` contains the address of `.rela.dyn` section, `DT_RELASZ` is for the whole section in bytes,
and `DT_RELAENT` is size for each relocation entry.

---
*More on relative relocation:* (feel free to skip this block) The case to use this type of relocation
is rare, making it hard to find materials. Here, I will explain one of the reasons why it is needed 
in ELF format.

`test_lib/SimpleIni.c` defines a function that should be called before the control is handed out to 
the user, that is, during the load-time. Such functions must have their own relocaion entries, 
compared with other functions internal to the library, which can easily be accessed by PC-relative
addressing, without relocation. Also, these functions should not take any argument, for it is the 
dynamic loader that is calling them, receiving no argument from the user.

Thus, the user needs not know the name of these functions. If it is made invisible to external libraries 
using `extern`, then only the loader can call the functions. Therefore, there is no need in preserving
the function names, and during relocation, the real address of them can simply be calculated by adding
the relative address of them (can be observed by `objdump -d`, and this is stored as `addend` in relocation entry)
and the base address of the library.

```bash
$ objdump -d ./test_lib/SimpleIni.so

0000000000001119 <init>:
    1119:       f3 0f 1e fa             endbr64 
    111d:       55                      push   %rbp
    111e:       48 89 e5                mov    %rsp,%rbp
    1121:       48 8d 3d d8 0e 00 00    lea    0xed8(%rip),%rdi        # 2000 <_fini+0xeb8>
    1128:       e8 23 ff ff ff          callq  1050 <puts@plt>
    112d:       90                      nop
    112e:       5d                      pop    %rbp
    112f:       c3                      retq 
```
We can see clearly that `init` is at `0x1119`, and is addend of a relative relocation entry.

---
After we find the real address of the constructors, we can find their location and call them.
```bash
$ readelf -d ./test_lib/SimpleIni.so
 ...
 0x000000000000000c (INIT)               0x1000
 0x0000000000000019 (INIT_ARRAY)         0x3e08
 0x000000000000001b (INIT_ARRAYSZ)       16 (bytes)
 ...
```
The initialization functions have two parts: the first part is pointed by `DT_INIT`, which is 
a single function. The second part is pointed by `DT_INIT_ARRAY`, which is an array of function
pointers whose size is indicated by `DT_INIT_ARRAYSZ`. Functions in both form should have the 
same type: taking no argument and returning nothing, *i.e.*, `void (*)(void)`.

If you correctly call the initialization functions, the function `init` in `test_lib/SimpleIni.c`
will be called before `entry`.
